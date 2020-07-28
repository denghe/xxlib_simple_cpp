#include "xx_object.h"
#include "PKG_class_lite.h"

// todo: 分析 NanoLog
// nanolog 延迟写是先带 typeId( 利用 tuple 模板 来实现根据 类型 推断 id ) 序列化数据到一段 buf. 然后在另外一个线程 pop 出来，反序列得到原始类型后 ToString 写盘
// 一些结构 基于 256 字节对齐，有些塞了 pad 避免 cache line 冲突
// QueueBuffer 为基于 SpinLock 的线程安全队列，其 Items 为一组 Buffer

namespace xx {

    template<size_t size = 256>
    union FixedData {
        char all[size];
        struct {
            char *buf;
            size_t len;
            size_t cap;
        };
        static const size_t headerLen = sizeof(buf) + sizeof(len) + sizeof(cap);
        static_assert(size > headerLen);

        FixedData() {
            buf = all + headerLen;
            len = 0;
            cap = size - headerLen;
        }

        FixedData(FixedData &&o) noexcept {
            if (o.buf == o.all) {
                memcpy(all, o.all, headerLen + o.len);
                buf = all + headerLen;
            } else {
                memcpy(all, o.all, headerLen);
            }
            memset(o.all, 0, headerLen);
        }

        FixedData(FixedData const &o) = delete;

        FixedData &operator=(FixedData const &o) = delete;

        FixedData &operator=(FixedData &&o) = delete;

        void Ensure(size_t const &siz, size_t const &grouth = 0) {
            if (len + siz <= cap) return;
            cap = grouth ? len + siz + grouth : (len + siz) * 2;
            auto &&newBuf = (char *) malloc(cap);
            memcpy(newBuf, buf, len);
            if (buf != all) {
                free(buf);
            }
            buf = newBuf;
        }

        ~FixedData() {
            if (buf != all) {
                free(buf);
            }
        }

        template<typename T>
        void Write(T const &v);

        // 先写长度再写内容( 含 type id )
        void WriteString(char const *const &data, size_t const &siz);
    };


    template<> struct TypeId<bool> { static const uint16_t value = 1; };
    template<> struct TypeId<char> { static const uint16_t value = 2; };
    template<> struct TypeId<short> { static const uint16_t value = 3; };
    template<> struct TypeId<int> { static const uint16_t value = 4; };
    template<> struct TypeId<long> { static const uint16_t value = 5; };
    template<> struct TypeId<unsigned char> { static const uint16_t value = 6; };
    template<> struct TypeId<unsigned short> { static const uint16_t value = 7; };
    template<> struct TypeId<unsigned int> { static const uint16_t value = 8; };
    template<> struct TypeId<unsigned long> { static const uint16_t value = 9; };
    template<> struct TypeId<char*> { static const uint16_t value = 10; };

    // write typeId + len + data
    template<size_t size>
    void FixedData<size>::WriteString(char const *const &data, size_t const &siz) {
        Ensure(1 + sizeof(siz) + siz);
        buf[len] = TypeId_v<char*>;
        len += 1;
        memcpy(buf + len, &siz, sizeof(siz));
        len += sizeof(siz);
        memcpy(buf + len, data, siz);
        len += siz;
    }

    // 适配模板 for FixedData<size>::Write
    template<size_t size, typename T, typename ENABLED = void>
    struct BufFuncs {
        static inline void Write(FixedData<size> &data, T const &in) {
            // 不支持的数据类型
            assert(false);
        }
    };

    // 函数转发到适配模板
    template<size_t size>
    template<typename T>
    void FixedData<size>::Write(T const &v) {
        BufFuncs<size, T>::Write(this, v);
    }

    // 适配所有数字类型
    template<size_t size, typename T>
    struct BufFuncs<size, T, std::enable_if_t<std::is_arithmetic_v<T>>> {
        // 1 byte typeId + data
        static inline void Write(FixedData<size> &data, T const &in) {
            data.Ensure(1 + sizeof(T));
            data.buf[data.len] = (char)TypeId_v<T>;
            memcpy(data.buf + data.len + 1, in, sizeof(T));
            data.len += 1 + sizeof(T);
        }
    };

    // 适配 literal char[len] string
    template<size_t size, size_t len>
    struct BufFuncs<size, char[len], void> {
        static inline void Write(FixedData<size> &data, char const(&in)[len]) {
            data.WriteString(in, len);
        }
    };

    // 适配 char const* \0 结尾 字串
    template<size_t size>
    struct BufFuncs<size, char const *, void> {
        static inline void Write(FixedData<size> &data, char const *const &in) {
            data.WriteString(in, strlen(in));
        }
    };

    // 适配 char* \0 结尾 字串
    template<size_t size>
    struct BufFuncs<size, char *, void> {
        static inline void Write(FixedData<size> &data, char *const &in) {
            data.WriteString(in, strlen(in));
        }
    };

    // 适配 std::string
    template<size_t size>
    struct BufFuncs<size, std::string, void> {
        static inline void Write(FixedData<size> &data, std::string const &in) {
            data.WriteString(in.data(), in.size());
        }
    };

}

int main() {
    xx::FixedData<> fd;
    fd.Write(1);
    fd.Write("asdf");

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
