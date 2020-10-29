#pragma once

#include "xx_data_rw.h"
#include "xx_string.h"

namespace xx {

    struct Random1 {
        int seed = 123456789;
        static const int m = 1 << 31, a = 1103515245, c = 12345;

        // maybe negative
        inline int Next() {
            seed = (a * seed + c) % m;
            return seed;
        }

        inline double NextDouble() {
            return (double)(uint32_t)Next() / (double)std::numeric_limits<uint32_t>::max();
        };
    };

    struct Random2 {
        uint64_t x = 0, w = 0;
        static const uint64_t s = 0xb5ad4eceda1ce2a9;

        inline uint32_t Next() {
            x *= x;
            x += (w += s);
            return x = (x >> 32) | (x << 32);
        }

        inline double NextDouble() {
            return (double)Next() / (double)std::numeric_limits<uint32_t>::max();
        };
    };

    struct Random3 {
        uint64_t seed = 123456789123456789;
        inline uint64_t Next() {
            seed ^= (seed << 21u);
            seed ^= (seed >> 35u);
            seed ^= (seed << 4u);
            return seed;
        }

        inline double NextDouble() {
            return (double)Next() / (double)std::numeric_limits<uint64_t>::max();
        };
    };

    template<typename T>
    struct DataFuncs<T, std::enable_if_t<std::is_same_v<std::decay_t<T>, Random1>
                                         || std::is_same_v<std::decay_t<T>, Random2>
                                         || std::is_same_v<std::decay_t<T>, Random3>
    >> {
        static inline void Write(DataWriter &dw, T const &in) {
            dw.WriteFixed(in);
        }

        static inline int Read(DataReader &dr, T &out) {
            return dr.ReadFixed(out);
        }
    };

    template<typename T>
    struct StringFuncs<T, std::enable_if_t<std::is_same_v<std::decay_t<T>, Random1>
                                           || std::is_same_v<std::decay_t<T>, Random3>
    >> {
        static inline void Append(std::string &s, T const &in) {
            s.append("{\"seed\":");
            s += std::to_string(in.seed);
            s.push_back('}');
        }
    };

    template<typename T>
    struct StringFuncs<T, std::enable_if_t<std::is_same_v<std::decay_t<T>, Random2>
    >> {
        static inline void Append(std::string &s, T const &in) {
            s.append("{\"x\":");
            s += std::to_string(in.x);
            s.append(",\"w\":");
            s += std::to_string(in.w);
            s.push_back('}');
        }
    };
}
