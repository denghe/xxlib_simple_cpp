#include "xx_fixeddata_w.h"
#include <mutex>
#include <thread>

// todo: 分析 NanoLog
// nanolog 延迟写是先带 typeId( 利用 tuple 模板 来实现根据 类型 推断 id ) 序列化数据到一段 buf. 然后在另外一个线程 pop 出来，反序列得到原始类型后 ToString 写盘
// 一些结构 基于 256 字节对齐，有些塞了 pad 避免 cache line 冲突
// QueueBuffer 为基于 SpinLock 的线程安全队列，其 Items 为一组 Buffer

namespace xx {
    struct Logger {
        std::vector<FixedData<256>> items1;
        std::vector<FixedData<256>> items2;
        std::thread t;
        std::mutex mtx;
        volatile int disposing = 0;

        explicit Logger(size_t const &capMB = 8) {
            items1.reserve(1024 * 1024 * capMB / 256);
            items2.reserve(1024 * 1024 * capMB / 256);
            t = std::thread(&Logger::Loop, this);
        }

        void Loop() {
            while (true) {
                // 切换前后台队列( 如果有数据. 没有就 sleep 一下继续扫 )
                {
                    std::lock_guard<std::mutex> lg(mtx);
                    if (items1.empty()) goto LabEnd;
                    std::swap(items1, items2);
                }

//                for(auto&& o : items2) {
//                    // todo: items2 写盘
//                    //DumpTo(std::cout, o);
//                    //std::cout << std::endl;
//                }
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

        ~Logger() {
            if (disposing) return;
            disposing = 1;
            t.join();
        }

        template<typename ...TS>
        void Write(TS const &...vs) {
            std::lock_guard<std::mutex> lg(mtx);
            auto &&fd = items1.emplace_back();
            xx::WriteTo(fd, std::chrono::system_clock::now(), vs...);
        }
    };
}
