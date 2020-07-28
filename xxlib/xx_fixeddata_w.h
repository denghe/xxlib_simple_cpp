#include "xx_fixeddata.h"
#include "xx_chrono.h"

namespace xx {
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

    template<typename C, typename D>
    struct DataTypeId<std::chrono::time_point<C, D>> {
        static const char value = 12;

        inline static void Dump(std::ostream &o, char *&v) {

            auto&& t = std::chrono::system_clock::to_time_t(*(std::chrono::time_point<C, D>*)v);
            o << std::put_time(std::localtime(&t), "%F %T");
            v += sizeof(std::chrono::time_point<C, D>);
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
            BufFuncs<size, std::pair<char *, size_t>>::Write(data, {(char*)in.data(), in.size()});
        }
    };

    // 适配 std::chrono::time_point
    template<size_t size, typename C, typename D>
    struct BufFuncs<size, std::chrono::time_point<C, D>, void> {
        static inline void Write(FixedData<size> &data, std::chrono::time_point<C, D> const &in) {
            data.Ensure(1 + sizeof(std::chrono::time_point<C, D>));
            data.buf[data.len] = DataTypeId<std::chrono::time_point<C, D>>::value;
            memcpy(data.buf + data.len + 1, &in, sizeof(std::chrono::time_point<C, D>));
            data.len += 1 + sizeof(std::chrono::time_point<C, D>);
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
            DataTypeId<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>::Dump,
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
    }
}
