#include "xx_object.h"
#include "PKG_class_lite.h"

// todo: 分析 NanoLog
// nanolog 延迟写是先带 typeId( 利用 tuple 模板 来实现根据 类型 推断 id ) 序列化数据到一段 buf. 然后在另外一个线程 pop 出来，反序列得到原始类型后 ToString 写盘
// 一些结构 基于 256 字节对齐，有些塞了 pad 避免 cache line 冲突
// QueueBuffer 为基于 SpinLock 的线程安全队列，其 Items 为一组 Buffer

namespace xx {

    template<size_t size = 256>
    struct FixedData {
        char *buf;
        size_t len;
        size_t cap;
        static const size_t headerLen = sizeof(buf) + sizeof(len) + sizeof(cap);
        char innerBuf[size - headerLen];

        FixedData() {
            buf = innerBuf;
            len = 0;
            cap = size - headerLen;
        }

        FixedData(FixedData &&o) noexcept {
            if (o.buf == o.innerBuf) {
                buf = innerBuf;
                memcpy(&len, &o.len, sizeof(len) + sizeof(cap) + o.len);
            } else {
                memcpy(this, &o, headerLen);
            }
            memset(&o, 0, headerLen);
        }

        FixedData(FixedData const &o) = delete;

        FixedData &operator=(FixedData const &o) = delete;

        FixedData &operator=(FixedData &&o) = delete;

        void Ensure(size_t const &siz) {
            if (len + siz <= cap) return;
            while (cap < len + siz) {
                cap *= 2;
            }
            auto &&newBuf = (char *) malloc(cap);
            memcpy(newBuf, buf, len);
            if (buf != innerBuf) {
                free(buf);
            }
            buf = newBuf;
        }

        void Clear() {
            if (buf != innerBuf) {
                free(buf);
                buf = innerBuf;
                cap = size - headerLen;
            }
            len = 0;
        }

        ~FixedData() {
            if (buf != innerBuf) {
                free(buf);
            }
        }
    };

    // 类型适配模板 for FixedData<size>::Write
    template<typename T, typename ENABLED = void>
    struct DataTypeId;

    template<>
    struct DataTypeId<char *> {
        static const char value = 0;

        inline static void Dump(std::ostream &o, char *&v) {
            o << std::string_view(v + sizeof(size_t), *(size_t *) v);
            v += sizeof(size_t) + *(size_t *) v;
        }
    };

    template<>
    struct DataTypeId<bool> {
        static const char value = 1;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *(bool *) v;
            v += sizeof(bool);
        }
    };

    template<>
    struct DataTypeId<char> {
        static const char value = 2;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *v;
            v += sizeof(char);
        }
    };

    template<>
    struct DataTypeId<short> {
        static const char value = 3;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *(short *) v;
            v += sizeof(short);
        }
    };

    template<>
    struct DataTypeId<int> {
        static const char value = 4;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *(int *) v;
            v += sizeof(int);
        }
    };

    template<>
    struct DataTypeId<long long> {
        static const char value = 5;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *(long long *) v;
            v += sizeof(long long);
        }
    };

    template<>
    struct DataTypeId<float> {
        static const char value = 6;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *(float *) v;
            v += sizeof(float);
        }
    };

    template<>
    struct DataTypeId<double> {
        static const char value = 7;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *(double *) v;
            v += sizeof(double);
        }
    };

    template<>
    struct DataTypeId<unsigned char> {
        static const char value = 8;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *(unsigned char *) v;
            v += sizeof(unsigned char);
        }
    };

    template<>
    struct DataTypeId<unsigned short> {
        static const char value = 9;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *(unsigned short *) v;
            v += sizeof(unsigned short);
        }
    };

    template<>
    struct DataTypeId<unsigned int> {
        static const char value = 10;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *(unsigned int *) v;
            v += sizeof(unsigned int);
        }
    };

    template<>
    struct DataTypeId<unsigned long long> {
        static const char value = 11;

        inline static void Dump(std::ostream &o, char *&v) {
            o << *(unsigned long long *) v;
            v += sizeof(unsigned long long);
        }
    };


    // 适配模板 for FixedData<size>::Write
    template<size_t size, typename T, typename ENABLED = void>
    struct BufFuncs {
        static inline void Write(FixedData<size> &data, T const &in) {
            // 不支持的数据类型
            assert(false);
        }
    };

    // 适配所有数字类型
    template<size_t size, typename T>
    struct BufFuncs<size, T, std::enable_if_t<std::is_arithmetic_v<T>>> {
        // 1 byte typeId + data
        static inline void Write(FixedData<size> &data, T const &in) {
            data.Ensure(1 + sizeof(T));
            data.buf[data.len] = DataTypeId<T>::value;
            memcpy(data.buf + data.len + 1, &in, sizeof(T));
            data.len += 1 + sizeof(T);
        }
    };

    // 适配 pair<char*, len>
    template<size_t size>
    struct BufFuncs<size, std::pair<char *, size_t>> {
        // 1 byte typeId + len + data
        static inline void Write(FixedData<size> &data, std::pair<char *, size_t> const &in) {
            data.Ensure(1 + sizeof(in.second) + in.second);
            data.buf[data.len] = DataTypeId<char *>::value;
            memcpy(data.buf + data.len + 1, &in.second, sizeof(in.second));
            memcpy(data.buf + data.len + 1 + sizeof(in.second), in.first, in.second);
            data.len += 1 + sizeof(in.second) + in.second;
        }
    };

    // 适配 literal char[len] string
    template<size_t size, size_t len>
    struct BufFuncs<size, char[len], void> {
        static inline void Write(FixedData<size> &data, char const(&in)[len]) {
            BufFuncs<size, std::pair<char *, size_t>>::Write(data, {(char *) in, len});
        }
    };

    // 适配 char const* \0 结尾 字串
    template<size_t size>
    struct BufFuncs<size, char const *, void> {
        static inline void Write(FixedData<size> &data, char const *const &in) {
            BufFuncs<size, std::pair<char *, size_t>>::Write(data, {in, strlen(in)});
        }
    };

    // 适配 char* \0 结尾 字串
    template<size_t size>
    struct BufFuncs<size, char *, void> {
        static inline void Write(FixedData<size> &data, char *const &in) {
            BufFuncs<size, std::pair<char *, size_t>>::Write(data, {in, strlen(in)});
        }
    };

    // 适配 std::string
    template<size_t size>
    struct BufFuncs<size, std::string, void> {
        static inline void Write(FixedData<size> &data, std::string const &in) {
            BufFuncs<size, std::pair<char *, size_t>>::Write(data, {in.data(), in.size()});
        }
    };


    /*************************************************************************************/

    template<size_t size, typename ...TS>
    void WriteTo(FixedData<size> &data, TS const &...vs) {
        std::initializer_list<int> n{(BufFuncs<size, TS>::Write(data, vs), 0)...};
        (void) n;
    }

    typedef void (*DumpFunc)(std::ostream &o, char *&v);

    inline DumpFunc dumpFuncs[] = {
            DataTypeId<char *>::Dump,
            DataTypeId<bool>::Dump,
            DataTypeId<char>::Dump,
            DataTypeId<short>::Dump,
            DataTypeId<int>::Dump,
            DataTypeId<long long>::Dump,
            DataTypeId<float>::Dump,
            DataTypeId<double>::Dump,
            DataTypeId<unsigned char>::Dump,
            DataTypeId<unsigned short>::Dump,
            DataTypeId<unsigned int>::Dump,
            DataTypeId<unsigned long long>::Dump,
    };

    template<typename OS, size_t size>
    void DumpTo(OS &o, FixedData<size> const &v) {
        auto begin = v.buf;
        auto end = v.buf + v.len;
        while (begin < end) {
            auto typeId = (int) *begin;
            ++begin;
            dumpFuncs[typeId](o, begin);
        }
        o.flush();
    }

}

#include "xx_chrono.h"
#include "xx_queue.h"
#include <deque>
#include <queue>
#include <mutex>
#include <thread>

namespace xx {
    struct LoggerItems : std::vector<FixedData<256>> {
        explicit LoggerItems(size_t const& cap) {
            reserve(cap);
        }
    };
    struct Logger {
        LoggerItems items1;
        LoggerItems items2;
        std::thread t;
        std::mutex mtx;
        volatile int disposing = 0;
        Logger(size_t const& capMB = 8)
            : items1(1024 * 1024 * capMB / 256)
            , items2(1024 * 1024 * capMB / 256)
            , t(&Logger::Loop, this) {
        }
        void Loop() {
            while(true) {
                // 切换前后台队列( 如果有数据. 没有就 sleep 一下继续扫 )
                {
                    std::lock_guard<std::mutex> lg(mtx);
                    if (items1.empty()) goto LabEnd;
                    std::swap(items1, items2);
                }

                // todo: items2 写盘
                items2.clear();
                continue;

                LabEnd:
                // 如果需要退出，还是要多循环一次，写光数据
                if (disposing == 1) {
                    ++disposing;
                    continue;
                }
                else if (disposing == 2) {
                    break;
                }
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
            xx::WriteTo(fd, vs...);
        }
    };
}

int main() {
    xx::Logger L;
    int64_t totalMS = 0;
    for (int j = 0; j < 100; ++j) {
        auto&& beginMS = xx::NowSteadyEpochMS();
        for (int i = 0; i < 200000; ++i) {
            L.Write("asdf ", 1, 2.3, "asdfasdf");
        }
        totalMS += xx::NowSteadyEpochMS() - beginMS;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    std::cout << "elapsed ms = " << totalMS << std::endl;
    return 0;
































    // 创建类辅助器
    xx::ObjectHelper oh;

    // 注册类型
    PKG::PkgGenTypes::RegisterTo(oh);

    xx::Data data;
    xx::DataWriter ddww(data);
    ddww.Write(std::make_pair<int, int>(1, 2));
    xx::DataReader ddrr(data);
    int a, b;
    ddrr.Read(a, b);
    data.Clear();

    {
        // 构建一个场景
        auto &&scene = std::make_shared<PKG::Scene>();
        auto &&node1 = std::make_shared<PKG::Node>();
        scene->childs.push_back(node1);
        node1->parent = scene;

        auto &&node1_1 = std::make_shared<PKG::Node>();
        node1->childs.push_back(node1_1);
        node1_1->parent = node1;

        auto &&node1_2 = std::make_shared<PKG::Node>();
        node1->childs.push_back(node1_2);
        node1_2->parent = node1;

        scene->nodes["1"] = node1;
        scene->nodes["1_1"] = node1_1;
        scene->nodes["1_2"] = node1_2;

        // 序列化进 data
        oh.WriteTo(data, scene);
        // 打印 data 的内容
        oh.CoutN(data);

        // 测试下克隆
        auto &&scene2 = oh.Clone(scene);
        // 比较数据是否相同。相同则篡改下
        if (!oh.Compare(scene, scene2)) {
            // 故意改点东西
            scene2->parent = scene2;
            // 如果比较结果不一致则输出
            if (oh.Compare(scene, scene2)) {
                oh.CoutCompareResult();
            }
        }
    }

    oh.CoutN(data);
    {
        oh.CoutN(oh.ReadObjectFrom(data));
    }

    data.Clear();
    {
        auto &&d = std::make_shared<PKG::D>();
        d->name = "d";
        d->desc = "nullable";
        d->a.x = 1;
        d->a.y = 2;
        d->a.c = d;
        d->b.x = 3;
        d->b.y = 4;
        d->b.z = 5;
        d->b.c = d;
        d->b.wc = d;

        oh.WriteTo(data, d);
    }
    oh.CoutN(data);
    {
        oh.CoutN(oh.ReadObjectFrom(data));
    }
}
