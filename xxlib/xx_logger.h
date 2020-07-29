#include "xx_fixeddata_w.h"
#include "ajson.hpp"
#include <mutex>
#include <thread>
#include <fstream>
#include <filesystem>

// todo: 分析 NanoLog
// nanolog 延迟写是先带 typeId( 利用 tuple 模板 来实现根据 类型 推断 id ) 序列化数据到一段 buf. 然后在另外一个线程 pop 出来，反序列得到原始类型后 ToString 写盘
// 一些结构 基于 256 字节对齐，有些塞了 pad 避免 cache line 冲突
// QueueBuffer 为基于 SpinLock 的线程安全队列，其 Items 为一组 Buffer

namespace xx {

    // 获取当前执行文件名字
    inline std::string GetExecuteName() {
#if defined(PLATFORM_POSIX) || defined(__linux__)
        std::string s;
        std::ifstream("/proc/self/comm") >> s;
        return s;
#elif defined(_WIN32)
        char buf[MAX_PATH];
        GetModuleFileNameA(nullptr, buf, MAX_PATH);
        return buf;
#else
        static_assert(false, "unrecognized platform");
#endif
    }

    typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> TimePoint;
    enum class LogLevels : int {
        TRACE, DEBUG, INFO, WARN, ERROR
    };
    char const *logLevelNames[] = {
            "TRACE", "DEBUG", "INFO", "WARN", "ERROR"
    };


    /*
json 样板:
{
    "logLevel" : 1
    , "logFileName" : "log/server.log"
    , "logFileMaxBytes" : 5242880
    , "logFileCount" : 30
    , "outputConsole" : true
}
     */
    struct LoggerConfig {
        // 小于等于这个级别的才记录日志
        int logLevel = (int) LogLevels::DEBUG;
        // 日志文件 路径 & 文件名前缀( 后面可能还有日期 / 分段标志 )
        std::string logFileName = std::string("log/") + GetExecuteName() + ".log";
        // 单个日志文件体积上限( 字节 )
        size_t logFileMaxBytes = 1024 * 1024 * 5;
        // 日志文件最多个数( 滚动使用，超过个数将删除最早的 )
        size_t logFileCount = 30;
        // 写文件的同时, 是否同时输出到控制台
        bool outputConsole = true;

        LoggerConfig() = default;
        LoggerConfig(LoggerConfig const &) = default;
        LoggerConfig &operator=(LoggerConfig const &) = default;
        ~LoggerConfig() = default;
    };

}
    AJSON(xx::LoggerConfig, logLevel, logFileName, logFileMaxBytes, logFileCount, outputConsole);
namespace xx {

    // 适配 std::cout
    inline std::ostream& operator<<(std::ostream& o, LoggerConfig const& c) {
        ajson::save_to(o, c);
        return o;
    }

    /***********************************************************************************************/
    // 日志类主体
    /***********************************************************************************************/

    struct Logger {
        typedef FixedData<256> Item;
        LoggerConfig cfg;
    protected:
        std::vector<Item> items1;
        std::vector<Item> items2;
        std::thread thread;
        std::mutex mtx;
        volatile int disposing = 0;

        std::ofstream ofs;
        size_t wroteLen = 0;
        int fileCount;
    public:
        // 参数：开辟多少兆初始内存 cache
        explicit Logger(size_t const &capMB = 8, char const* const& cfgName = "xxlog.config") {
            // 试图加载 logger cfg
            if (std::filesystem::exists(cfgName)) {
                ajson::load_from_file(cfg, cfgName);
            }

            // 根据配置和当前时间推算出日志文件名并以追加模式打开
            time_t now = time(nullptr);
            struct tm* t = localtime(&now);
            char createTime[128];
            sprintf(createTime, "-%d-%d-%d+%d:%d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
            auto fileName = cfg.logFileName + createTime;
            ofs.open(fileName, std::ios_base::app);
            if (ofs.fail()) {
                std::cerr << "ERROR!!! open log file failed: " << fileName << std::endl;
            }

            // 初始化内存池
            items1.reserve(1024 * 1024 * capMB / 256);
            items2.reserve(1024 * 1024 * capMB / 256);

            // 初始化后台线程
            thread = std::thread(&Logger::Loop, this);
        }

        ~Logger() {
            if (disposing) return;
            disposing = 1;
            thread.join();
        }

        // 固定以 level, __LINE__, __FILE__, __FUNCTION__, now 打头的日志内容写入. 宏代指的 char* 不会丢失，故不需要复制其内容
        // 直接写入固定无 type 前缀: level + lineNumber + fileName* + funcName* + now
        template<typename ...TS>
        void Log(xx::LogLevels const &level, int const &lineNumber, char const *const &fileName, char const *const &funcName, TS const &...vs) {
            if ((int)level < cfg.logLevel) return;
            if (disposing) return;
            std::lock_guard<std::mutex> lg(mtx);
            auto &&fd = items1.emplace_back();
            fd.Ensure(sizeof(int) + sizeof(int) + sizeof(char *) + sizeof(char *) + sizeof(TimePoint));
            auto p = fd.buf;
            *(int *) p = (int)level;
            *(int *) (p + sizeof(int)) = lineNumber;
            *(char const **) (p + sizeof(int) + sizeof(int)) = fileName;
            *(char const **) (p + sizeof(int) + sizeof(int) + sizeof(char *)) = funcName;
            *(TimePoint*) (p + sizeof(int) + sizeof(int) + sizeof(char *) + sizeof(char *)) = std::chrono::system_clock::now();
            fd.len = sizeof(int) + sizeof(int) + sizeof(char *) + sizeof(char *) + sizeof(TimePoint);
            xx::WriteTo(fd, vs...);
        }

    protected:
        // 后台线程专用函数
        void Loop() {
            while (true) {
                // 切换前后台队列( 如果有数据. 没有就 sleep 一下继续扫 )
                {
                    std::lock_guard<std::mutex> lg(mtx);
                    if (items1.empty()) goto LabEnd;
                    std::swap(items1, items2);
                }

                // 输出 items2
                Dump();
                items2.clear();
                continue;

                LabEnd:
                // 如果需要退出，还是要多循环一次，写光数据
                if (disposing == 1) {
                    ++disposing;
                    continue;
                } else if (disposing == 2) break;
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        }

        void FileRename() {
            ofs.close();

            int i = fileCount;
            char old_log_name[225], new_log_name[225];
            while (i > 0) {
                snprintf(old_log_name, sizeof(old_log_name), "%s.%d", cfg.logFileName.c_str(), i);
                snprintf(new_log_name, sizeof(new_log_name), "%s.%d", cfg.logFileName.c_str(), i + 1);
                if (i == cfg.logFileCount) {
                    remove(old_log_name);
                }
                else {
                    rename(old_log_name, new_log_name);
                }
                --i;
            }

            snprintf(old_log_name, sizeof(old_log_name), "%s", cfg.logFileName.c_str());
            snprintf(new_log_name, sizeof(new_log_name), "%s.%d", cfg.logFileName.c_str(), i + 1);
            rename(old_log_name, new_log_name);

            if (fileCount < cfg.logFileCount) {
                ++fileCount;
            }

            ofs.open(old_log_name, std::ios_base::app);
            if (ofs.fail()) {
                std::cerr << "ERROR!!! open log file failed: " << old_log_name << std::endl;
            }
        }

        // 默认提供文件 dump 和 console dump. 可以覆盖实现自己的特殊需求
        virtual void Dump() {
            if (cfg.outputConsole) {
                for (auto &&item : items2) {
                    DumpItem(std::cout, item, true);
                }
            }
            std::cout.flush();

            if (ofs.is_open()) {
                for (auto &&item : items2) {
                    DumpItem(ofs, item, false);

                    // 粗略统计当前已经写了多长了. 不是很精确. 如果超长就换文件写
                    wroteLen += item.len;
                    if (wroteLen > cfg.logFileMaxBytes) {
                        wroteLen = 0;
                        FileRename();
                    }
                }
                ofs.flush();
            }
        }

        // dump 单行日志. 可覆盖实现自己的特殊需求
        virtual void DumpItem(std::ostream& o, Item& item, bool const& isConsole) {
            // dump 前缀
            auto p = item.buf;
            Dump_Prefix(o
                    , (LogLevels)*(int *) p
                    , *(int *) (p + sizeof(int))
                    , *(char const **) (p + sizeof(int) + sizeof(int))
                    , *(char const **) (p + sizeof(int) + sizeof(int) + sizeof(char *))
                    , *(TimePoint *) (p + sizeof(int) + sizeof(int) + sizeof(char *) + sizeof(char *))
                    , isConsole);
            // dump 内容
            DumpTo(o, item, sizeof(int) + sizeof(int) + sizeof(char *) + sizeof(char *) + sizeof(TimePoint));
            // 弄个换行符
            o << "\r\n";
        }

        // dump 单行日志 的前缀部分。可覆盖实现自己的写入格式
        virtual void Dump_Prefix(std::ostream &o, LogLevels const &level, int const &lineNumber, char const *const &fileName, char const *const &funcName, TimePoint const &tp, bool const& isConsole) {
            auto&& tm = std::chrono::system_clock::to_time_t(tp);
            if (isConsole) {
                o << "\033[36m";
            }
            o << std::put_time(std::localtime(&tm), "%F %T");
            if (isConsole) {
                o << "\033[37m";
            }
            o << " ["<< logLevelNames[(int)level] << "] [file:" << fileName << " line:" << lineNumber << " func:" << funcName << "] ";
        }
    };
}
