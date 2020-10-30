#pragma once

#include "xx_data_rw.h"
#include "xx_string.h"
#include <chrono>
#include <random>

// some simple random impl

#define RANDOM_CALC_FUNCS \
inline int32_t NextInt() { \
    return Next() & 0x7FFFFFFFu; \
}; \
inline double NextDouble() { \
    return (double) Next() / (double) std::numeric_limits<uint32_t>::max(); \
};

// todo: more Next funcs?

namespace xx {

    struct Random1 {
        int seed = 123456789;
        static const int m = 1 << 31, a = 1103515245, c = 12345;

        Random1() = default;

        explicit Random1(int const &seed) : seed(seed) {}

        Random1(Random1 const &) = default;

        Random1 &operator=(Random1 const &) = default;

        Random1(Random1 &&o) noexcept {
            std::swap(seed, o.seed);
        }

        Random1 &operator=(Random1 &&o) noexcept {
            std::swap(seed, o.seed);
            return *this;
        }

        inline uint32_t Next() {
            seed = (a * seed + c) % m;
            return (uint32_t)seed;
        }

        RANDOM_CALC_FUNCS
    };

    struct Random2 {
        uint64_t x = 0, w = 0;
        static const uint64_t s = 0xb5ad4eceda1ce2a9;

        Random2() = default;

        explicit Random2(uint64_t const &x, uint64_t const &w) : x(x), w(w) {}

        Random2(Random2 const &) = default;

        Random2 &operator=(Random2 const &) = default;

        Random2(Random2 &&o) noexcept {
            std::swap(x, o.x);
            std::swap(w, o.w);
        }

        Random2 &operator=(Random2 &&o) noexcept {
            std::swap(x, o.x);
            std::swap(w, o.w);
            return *this;
        }

        inline uint32_t Next() {
            x *= x;
            x += (w += s);
            return x = (x >> 32) | (x << 32);
        }

        RANDOM_CALC_FUNCS
    };

    struct Random3 {
        uint64_t seed = 123456789123456789;

        Random3() = default;

        explicit Random3(uint64_t const &seed) : seed(seed) {}

        Random3(Random3 const &) = default;

        Random3 &operator=(Random3 const &) = default;

        Random3(Random3 &&o) noexcept {
            std::swap(seed, o.seed);
        }

        Random3 &operator=(Random3 &&o) noexcept {
            std::swap(seed, o.seed);
            return *this;
        }

        inline uint32_t Next() {
            seed ^= (seed << 21u);
            seed ^= (seed >> 35u);
            seed ^= (seed << 4u);
            return (uint32_t)seed;
        }

        RANDOM_CALC_FUNCS
    };

    // deserialize maybe slow: rand.discard(count);
    struct Random4 {
        using SeedType = typename std::mt19937::result_type;
        uint64_t count;
        SeedType seed;
        std::mt19937 rand;

        explicit Random4(SeedType const &seed = std::random_device{}()) : count(0), seed(seed), rand(seed) {
            if (count) {
                rand.discard(count);
            }
        }

        Random4(Random4 const &) = default;

        Random4 &operator=(Random4 const &) = default;

        Random4(Random4 &&o) noexcept {
            std::swap(rand, o.rand);
        }

        Random4 &operator=(Random4 &&o) noexcept {
            std::swap(rand, o.rand);
            return *this;
        }

        // for DataReader
        inline void InitBySeedCount() {
            rand.seed(seed);
            rand.discard(count);
        }

        inline uint32_t Next() {
            ++count;
            return (uint32_t)rand();
        }

        RANDOM_CALC_FUNCS
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
    struct DataFuncs<T, std::enable_if_t<std::is_same_v<std::decay_t<T>, Random4>
    >> {
        static inline void Write(DataWriter &dw, T const &in) {
            dw.WriteFixed(in.count);
            dw.WriteFixed(in.seed);
        }

        static inline int Read(DataReader &dr, T &out) {
            if (int r = dr.ReadFixed(out.count)) return r;
            if (int r = dr.ReadFixed(out.seed)) return r;
            out.InitBySeedCount();
            return 0;
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

    template<typename T>
    struct StringFuncs<T, std::enable_if_t<std::is_same_v<std::decay_t<T>, Random4>
    >> {
        static inline void Append(std::string &s, T const &in) {
            s.append("{\"seed\":");
            s += std::to_string(in.seed);
            s.append(",\"count\":");
            s += std::to_string(in.count);
            s.push_back('}');
        }
    };
}
