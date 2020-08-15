// The Computer Language Benchmarks Game
// https://salsa.debian.org/benchmarksgame-team/benchmarksgame/
//
// Contributed by Markus Flad
//
// compile with following g++ flags
//  -std=c++17 -O3 -Wall -march=native -mno-fma

#include <string>
#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include <thread>
#include <climits>
#if defined(__AVX512BW__) || defined(__AVX__) || defined(__SSE__)
#include <immintrin.h>
#endif
#include <stdlib.h>

// Put everything in a namespace forces inlining
namespace {

    const auto numberOfCpuCores = std::thread::hardware_concurrency();

// The PortableBinaryBitmap manages access to the pbm output file and provides
// interlaced canvases that allow threads to write to the bitmap in parallel.
    class PortableBinaryBitmap {
    public:
        using Size=std::size_t;
        PortableBinaryBitmap(std::ostream& ostr, Size width, Size height)
                : _ostr (ostr)
                , _width (roundToMultiple(width, CHAR_BIT))
                , _height (roundToMultiple(height, numberOfCpuCores))
                , _data ((_width * _height) / CHAR_BIT) {
            _ostr << "P4" << '\n';
            _ostr << _width << ' ' << _height << '\n';
        }
        ~PortableBinaryBitmap() {
            _ostr.write(_data.data(), _data.size());
        }
        Size width() const {
            return _width;
        }
        Size height() const {
            return _height;
        }
        Size widthInBytes() const {
            return _width / CHAR_BIT;
        }
        struct Line {
            constexpr static Size pixelsPerWrite() {
                return CHAR_BIT;
            }
            Size y;
            Size width;
            char* data;
        };
        // The InterlacedCanvas provides interlaced access to the bitmap data. Each
        // thread must use its own InterlacedCanvas to write to the bitmap.
        class InterlacedCanvas {
        public:
            class Iterator {
            public:
                Iterator(Size y, Size _width, char* data,
                         Size interlaceIncrement, Size dataPointerIncrement)
                        : _il {y, _width, data}
                        , _interlaceIncrement (interlaceIncrement)
                        , _dataPointerIncrement (dataPointerIncrement) {
                }
                Line& operator*() {
                    return _il;
                }
                bool operator!=(const Iterator& other) const {
                    return _il.data != other._il.data;
                }
                Iterator& operator++() {
                    _il.y += _interlaceIncrement;
                    _il.data += _dataPointerIncrement;
                    return *this;
                }
            private:
                Line _il;
                Size _interlaceIncrement;
                Size _dataPointerIncrement;
            };
            InterlacedCanvas(PortableBinaryBitmap& pbm, Size yStart, Size increment)
                    : _pbm (pbm)
                    , _yStart (yStart)
                    , _increment (increment)
                    , _dataStart (yStart * pbm.widthInBytes())
                    , _dataPointerIncrement (increment * pbm.widthInBytes()) {
            }
            Size width() const {
                return _pbm.width();
            }
            Size height() const {
                return _pbm.height();
            }
            Iterator begin() {
                return Iterator(_yStart, _pbm.width(),
                                _pbm._data.data() + _dataStart,
                                _increment, _dataPointerIncrement);
            }
            Iterator end() {
                return Iterator(_yStart + _pbm.height(), _pbm.width(),
                                _pbm._data.data() + _pbm._data.size() + _dataStart,
                                _increment, _dataPointerIncrement);
            }
        private:
            PortableBinaryBitmap& _pbm;
            Size _yStart;
            Size _increment;
            Size _dataStart;
            Size _dataPointerIncrement;
        };
        std::vector<InterlacedCanvas> provideInterlacedCanvas(Size increment)
        noexcept {
            std::vector<InterlacedCanvas> interlacedCanvasVector;
            for (Size yStart=0; yStart<increment; yStart++) {
                interlacedCanvasVector.emplace_back(*this, yStart, increment);
            }
            return interlacedCanvasVector;
        }
        static Size roundToMultiple (Size number, Size base) {
            return number + ((number % base) ? (base - number % base) : 0);
        }
    private:
        std::ostream& _ostr;
        Size _width;
        Size _height;
        std::vector<char> _data;
    };

// If the system does not support SIMD, NoSimdUnion can be used.
    struct NoSimdUnion {
        using NumberType = double;
        using SimdRegisterType = double;
        NoSimdUnion()
                : reg(val) {
        }
        NoSimdUnion(const NoSimdUnion& other)
                : reg(val) {
            std::copy(std::begin(other.val), std::end(other.val), std::begin(val));
        }
        NoSimdUnion& operator=(const NoSimdUnion& other) {
            std::copy(std::begin(other.val), std::end(other.val), std::begin(val));
            return *this;
        }
        bool operator>(const double& threshold) const {
            return std::all_of(std::begin(val), std::end(val),
                               [&threshold](double v) {
                                   return v > threshold;
                               });
        }
        char lteToPixels(double threshold) const {
            char result = 0;
            if (val[0] <= threshold) result |= 0b10000000;
            if (val[1] <= threshold) result |= 0b01000000;
            if (val[2] <= threshold) result |= 0b00100000;
            if (val[3] <= threshold) result |= 0b00010000;
            if (val[4] <= threshold) result |= 0b00001000;
            if (val[5] <= threshold) result |= 0b00000100;
            if (val[6] <= threshold) result |= 0b00000010;
            if (val[7] <= threshold) result |= 0b00000001;
            return result;
        }
        SimdRegisterType* reg;
        NumberType val[8];
    };

#if defined(__AVX512BW__) || defined(__AVX__) || defined(__SSE__)
    union Simd128DUnion {
        using NumberType = double;
        using SimdRegisterType = __m128d;
        SimdRegisterType reg[4];
        NumberType val[8];
        bool operator>(const __m128d& threshold) const {
            // Note: Architectures like core2 provide SSE, but no VCMPGTPD
            // (greater-than) instruction. Therefore we use vcmplepd (less-equal)
            // and invert.
            return std::all_of(std::begin(reg), std::end(reg),
                               [&threshold](__m128d r) {
                                   __m128d cmpRes = _mm_cmple_pd(r, threshold);
                                   return !_mm_movemask_pd(cmpRes);
                               });
        }
        char lteToPixels(const __m128d& threshold) const {
            __m128d r0 = _mm_cmple_pd(reg[0], threshold);
            __m128d r1 = _mm_cmple_pd(reg[1], threshold);
            __m128d r2 = _mm_cmple_pd(reg[2], threshold);
            __m128d r3 = _mm_cmple_pd(reg[3], threshold);
            char c0 = _mm_movemask_pd(r0);
            char c1 = _mm_movemask_pd(r1);
            char c2 = _mm_movemask_pd(r2);
            char c3 = _mm_movemask_pd(r3);
            c0 <<= 6;
            c1 <<= 4;
            c2 <<= 2;
            return c0 | c1 | c2 | c3;
        }
    };

    union Simd256DUnion {
        using NumberType = double;
        using SimdRegisterType = __m256d;
        SimdRegisterType reg[2];
        NumberType val[8];
        bool operator>(const __m256d& threshold) const {
            // Note: Architectures like Haswell provide AVX-2, but no VCMPGTPD
            // (greater-than) instruction. Therefore we use vcmplepd (less-equal).
            return std::all_of(std::begin(reg), std::end(reg),
                               [&threshold](__m256d r) {
                                   __m256d cmpRes = _mm256_cmp_pd(r, threshold, _CMP_LE_OQ);
                                   return _mm256_testz_pd(cmpRes, cmpRes);
                               });
        }
        char lteToPixels(const __m256d& threshold) const {
            __m256d r0 = _mm256_cmp_pd(reg[0], threshold, _CMP_LE_OQ);
            __m256d r1 = _mm256_cmp_pd(reg[1], threshold, _CMP_LE_OQ);
            char c0 = _mm256_movemask_pd(r0);
            char c1 = _mm256_movemask_pd(r1);
            c0 <<= 4;
            return c0 | c1;
        }
    };

    union Simd512DUnion {
        using NumberType = double;
        using SimdRegisterType = __m512d;
        SimdRegisterType reg[1];
        NumberType val[8];
        bool operator>(const __m512d& threshold) const {
            return _mm512_cmp_pd_mask(reg[0], threshold, _CMP_GT_OQ);
        }
        char lteToPixels(const __m512d& threshold) const {
            return _mm512_cmp_pd_mask(reg[0], threshold, _CMP_LE_OQ);
        }
    };
#endif // defined(__AVX512BW__) || defined(__AVX__) || defined(__SSE__)

    template<class SimdUnion>
    constexpr std::size_t numberOfNumbers() {
        return sizeof(SimdUnion::val) / sizeof(typename SimdUnion::NumberType);
    }
    template<class SimdUnion>
    constexpr std::size_t numberOfNumbersInRegister() {
        return sizeof(typename SimdUnion::SimdRegisterType) /
               sizeof(typename SimdUnion::NumberType);
    }
    template<class SimdUnion>
    constexpr std::size_t numberOfRegisters() {
        return numberOfNumbers<SimdUnion>() /
               numberOfNumbersInRegister<SimdUnion>();
    }
    template<class SimdUnion>
    void setValueInReg(typename SimdUnion::SimdRegisterType& reg,
                       typename SimdUnion::NumberType v) {
        using SimdRegisterType = typename SimdUnion::SimdRegisterType;
        constexpr auto numbersInReg = numberOfNumbersInRegister<SimdUnion>();
        if constexpr (numbersInReg == 1) {
            reg = v;
        } else if constexpr (numbersInReg == 2) {
            reg = SimdRegisterType{v, v};
        } else if constexpr (numbersInReg == 4) {
            reg = SimdRegisterType{v, v, v, v};
        } else if constexpr (numbersInReg == 8) {
            reg = SimdRegisterType{v, v, v, v, v, v, v, v};
        }
    }
    template<class SimdUnion>
    void setValue(SimdUnion& simdUnion, typename SimdUnion::NumberType v) {
        using SimdRegisterType = typename SimdUnion::SimdRegisterType;
        SimdRegisterType* vValues = simdUnion.reg;
        constexpr auto numbersInReg = numberOfNumbersInRegister<SimdUnion>();
        for (std::size_t i=0; i<numberOfNumbers<SimdUnion>(); i+=numbersInReg) {
            setValueInReg<SimdUnion>(*vValues, v);
            vValues++;
        }
    }
// Special method that reverses the order of numbers in one register. This
// helps for using SIMD functions to get bit masks already in the correct order
// needed for the portable bitmap.
    template<class SimdUnion, class Functor>
    void setRealValuesReverseInReg(SimdUnion& simdUnion, Functor f) {
        constexpr auto numbersInReg = numberOfNumbersInRegister<SimdUnion>();
        std::size_t n=0;
        for (std::size_t i=0; i<numberOfNumbers<SimdUnion>(); i+=numbersInReg) {
            for (std::size_t j=numbersInReg; j>0; j--) {
                simdUnion.val[i+j-1] = f(n);
                n++;
            }
        }
    }

// VectorizedComplex provides a convenient interface to deal with complex
// numbers and uses the power of SIMD for high execution speed.
    template <class SimdUnion>
    class VectorizedComplex {
    public:
        using NumberType = typename SimdUnion::NumberType;
        using SimdRegisterType = typename SimdUnion::SimdRegisterType;
        using Size = std::size_t;

        VectorizedComplex() = default;
        VectorizedComplex(const VectorizedComplex&) = default;
        VectorizedComplex& operator=(const VectorizedComplex&) = default;
        VectorizedComplex(const SimdUnion& reals, NumberType commonImagValue)
                : _reals(reals) {
            setValue(_imags, commonImagValue);
        }
        VectorizedComplex& squareAndAdd(const VectorizedComplex& c,
                                        SimdUnion& squaredAbs) {
            for (Size i=0; i<numberOfRegisters<SimdUnion>(); i++) {
                auto realSquared = _reals.reg[i] * _reals.reg[i];
                auto imagSquared = _imags.reg[i] * _imags.reg[i];
                auto realTimesImag = _reals.reg[i] * _imags.reg[i];
                _reals.reg[i] = realSquared - imagSquared + c._reals.reg[i];
                _imags.reg[i] = realTimesImag + realTimesImag + c._imags.reg[i];
                squaredAbs.reg[i] = realSquared + imagSquared;
            }
            return *this;
        }
    private:
        SimdUnion _reals;
        SimdUnion _imags;
    };

// The ComplexPlaneCalculator performs function f(c), with c as a
// VectorizedComplex and a byte as the return value. Due to its eightfold
// vectorization, each returned bit can return a Boolean value from the
// calculation f(c). The full byte is then written to the canvas. This is done
// until the whole bitmap is filled.
    template <class SimdUnion, class Functor>
    class ComplexPlaneCalculator {
    public:
        using VComplex = VectorizedComplex<SimdUnion>;
        using NumberType = typename SimdUnion::NumberType;
        using Line = typename PortableBinaryBitmap::Line;
        using Size = std::size_t;

        ComplexPlaneCalculator(const std::complex<NumberType>& cFirst,
                               const std::complex<NumberType>& cLast,
                               PortableBinaryBitmap::InterlacedCanvas& canvas, Functor f)
                : _cFirst(cFirst)
                , _cLast(cLast)
                , _canvas(canvas)
                , _f(f) {
            static_assert(numberOfNumbers<SimdUnion>() == Line::pixelsPerWrite());
        }
        void operator()() noexcept {
            const NumberType realRange = _cLast.real() - _cFirst.real();
            const NumberType imagRange = _cLast.imag() - _cFirst.imag();
            const NumberType rasterReal = realRange / _canvas.width();
            const NumberType rasterImag = imagRange / _canvas.height();
            std::vector<SimdUnion> cRealValues;
            cRealValues.reserve(_canvas.width() / Line::pixelsPerWrite());
            for (Size x=0; x<_canvas.width(); x+=Line::pixelsPerWrite()) {
                SimdUnion cReals;
                setRealValuesReverseInReg(cReals, [&](Size i){
                    return _cFirst.real() + (x+i)*rasterReal;
                });
                cRealValues.push_back(cReals);
            }
            for (Line& line : _canvas) {
                char* nextPixels = line.data;
                char lastPixels = 0x00;
                const NumberType cImagValue = _cFirst.imag() + line.y*rasterImag;
                for (const SimdUnion& cReals : cRealValues) {
                    const VComplex c(cReals, cImagValue);
                    *nextPixels = _f(c, lastPixels);
                    lastPixels = *nextPixels;
                    nextPixels++;
                }
            }
        }
    private:
        std::complex<NumberType> _cFirst;
        std::complex<NumberType> _cLast;
        PortableBinaryBitmap::InterlacedCanvas _canvas;
        Functor _f;
    };

// Functor calculating a Mandelbrot iteration for a VectorizedComplex. This
// means that for eight complex numbers the Mandelbrot calculation is
// (potentially) executed in parallel. The result is a byte that contains a 1
// for each bit if the corresponding complex number is in the Mandelbrot set,
// and a 0 if it is not.
    template <class SimdUnion>
    class MandelbrotFunction {
    public:
        using VComplex = VectorizedComplex<SimdUnion>;
        using SimdRegisterType = typename SimdUnion::SimdRegisterType;
        using NumberType = typename SimdUnion::NumberType;
        using Size = std::size_t;
        constexpr static Size ITERATIONS_WITHOUT_CHECK = 5;
        constexpr static char NONE_IN_MANDELBROT_SET = 0x00;

        MandelbrotFunction(Size maxIterations, NumberType pointOfNoReturn = 2.0)
                : _maxOuterIterations(maxIterations / ITERATIONS_WITHOUT_CHECK - 2) {
            setValueInReg<SimdUnion>(_squaredPointOfNoReturn,
                                     pointOfNoReturn * pointOfNoReturn);
        }
        inline static void doMandelbrotIterations(VComplex& z, const VComplex& c,
                                                  SimdUnion& squaredAbs) {
            for (Size j=0; j<ITERATIONS_WITHOUT_CHECK; j++) {
                z.squareAndAdd(c, squaredAbs);
            }
        }
        char operator()(const VComplex& c, char lastPixels) const {
            VComplex z = c;
            SimdUnion squaredAbs;
            if (lastPixels == NONE_IN_MANDELBROT_SET) {
                for (Size i=0; i<_maxOuterIterations; i++) {
                    doMandelbrotIterations(z, c, squaredAbs);
                    if (squaredAbs > _squaredPointOfNoReturn) {
                        return NONE_IN_MANDELBROT_SET;
                    }
                }
            } else {
                for (Size i=0; i<_maxOuterIterations; i++) {
                    doMandelbrotIterations(z, c, squaredAbs);
                }
            }
            doMandelbrotIterations(z, c, squaredAbs);
            doMandelbrotIterations(z, c, squaredAbs);
            return squaredAbs.lteToPixels(_squaredPointOfNoReturn);
        }
    private:
        Size _maxOuterIterations;
        SimdRegisterType _squaredPointOfNoReturn;
    };

#if defined(__AVX512BW__)
    using SystemSimdUnion = Simd512DUnion;
#elif defined __AVX__
    using SystemSimdUnion = Simd256DUnion;
#elif defined __SSE__
    using SystemSimdUnion = Simd128DUnion;
#else
    using SystemSimdUnion = NoSimdUnion;
#endif

} // end namespace

int main(int argc, char** argv) {
    using NumberType = SystemSimdUnion::NumberType;
    using ComplexNumber = std::complex<NumberType>;
    using MandelbrotCalculator = ComplexPlaneCalculator<SystemSimdUnion,
            MandelbrotFunction<SystemSimdUnion>>;
    std::size_t n = 16000;
    if (argc>=2) {
        n = atoi(argv[1]);
    }
    const std::size_t maxIterations = 50;
    PortableBinaryBitmap pbm(std::cout, n, n);
    auto canvasVector = pbm.provideInterlacedCanvas(numberOfCpuCores);
    std::vector<std::thread> threads;
    for (auto& canvas : canvasVector) {
        threads.emplace_back(MandelbrotCalculator (ComplexNumber(-1.5, -1.0),
                                                   ComplexNumber(0.5, 1.0), canvas,
                                                   MandelbrotFunction<SystemSimdUnion> (maxIterations)));
    }
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}






//#include <stdio.h>
//#include <stdlib.h>
//#include <iostream>
//#include "xx_chrono.h"
//
//struct Node {
//    Node* l, * r;
//    Node() : l(0), r(0) {}
//    Node(Node* l2, Node* r2) : l(l2), r(r2) {}
//    ~Node() { delete l; delete r; }
//    int check() const {
//        if (l)
//            return l->check() + 1 + r->check();
//        else return 1;
//    }
//};
//
//Node* make(int d) {
//    if (d == 0) return new Node();
//    return new Node(make(d - 1), make(d - 1));
//}
//
//int main(int argc, char* argv[]) {
//    auto&& ms = xx::NowSteadyEpochMS();
//
//    int min_depth = 4,
//            max_depth = std::max(min_depth + 2,
//                                 (argc == 2 ? atoi(argv[1]) : 21)),
//            stretch_depth = max_depth + 1;
//
//    {
//        Node* c = make(stretch_depth);
//        std::cout << "stretch tree of depth " << stretch_depth << "\t "
//                  << "check: " << c->check() << std::endl;
//        delete c;
//    }
//
//    Node* long_lived_tree = make(max_depth);
//
//    {
//        for (int d = min_depth; d <= max_depth; d += 2) {
//            int iterations = 1 << (max_depth - d + min_depth), c = 0;
//            for (int i = 1; i <= iterations; ++i) {
//                Node* a = make(d);
//                c += a->check();
//                delete a;
//            }
//            std::cout << iterations << "\t trees of depth " << d << "\t "
//                      << "check: " << c << std::endl;
//        }
//    }
//
//    std::cout << "long lived tree of depth " << max_depth << "\t "
//              << "check: " << (long_lived_tree->check()) << "\n";
//
//    delete long_lived_tree;
//
//    std::cout << "ms = " << xx::NowSteadyEpochMS() - ms << std::endl;
//    return 0;
//}





//#define _CRT_SECURE_NO_WARNINGS 1
//
///* The Computer Language Benchmarks Game
// * https://salsa.debian.org/benchmarksgame-team/benchmarksgame/
// *
// * contributed by Danial Klimkin
// *
// */
//
//#include <functional>
//#include <iostream>
//#include <memory_resource>
//#include <mutex>
//#include <queue>
//#include <thread>
//#include <array>
//#include <atomic>
//#include <optional>
//
//
//#include "xx_chrono.h"
//
//
//
////using MemoryPool = std::pmr::monotonic_buffer_resource;
//struct MemoryPool {
//    std::array<char, 16>* bufs;
//    size_t cursor = 0;
//    MemoryPool() {
//        bufs = new std::array<char, 16>[1024 * 1024 * 8];
//    }
//    ~MemoryPool() {
//        delete[] bufs;
//    }
//    void* allocate(size_t const& siz) {
//        return &bufs[cursor++];
//    }
//    void release() {
//        cursor = 0;
//    }
//};
//
//
//using Lambda = std::function<void(void)>;
//
//template<typename T>
//class LockingQueue final {
//public:
//    LockingQueue() = default;
//
//    void emplace(T item) {
//        {
//            std::scoped_lock lock(mutex);
//            queue.emplace(std::move(item));
//        }
//        signal.notify_one();
//    }
//
//    void waitAndPop(T* out) {
//        std::unique_lock lock(mutex);
//        while (queue.empty()) {
//            signal.wait(lock);
//        }
//        *out = queue.front();
//        queue.pop();
//    }
//
//private:
//    std::mutex mutex;
//    std::condition_variable signal;
//    std::queue<T> queue;
//};
//
//class ThreadPool final {
//public:
//    explicit ThreadPool(
//            const unsigned int workers = std::thread::hardware_concurrency()) {
//        threads_.reserve(workers);
//        for (size_t i = 0; i < workers; ++i)
//            threads_.emplace_back(std::thread([this]() {
//                Lambda job;
//                while (true) {
//                    queue_.waitAndPop(&job);
//                    if (job) {
//                        job();
//                    }
//                    else {
//                        enqueue_task(nullptr);
//                        break;
//                    }
//                }
//            }));
//    }
//
//    ~ThreadPool() noexcept {
//        enqueue_task(nullptr);
//        for (std::thread& thread : threads_) {
//            thread.join();
//        }
//    }
//
//    void enqueue_task(Lambda f) {
//        queue_.emplace(std::move(f));
//    }
//
//private:
//    std::vector<std::thread> threads_;
//    LockingQueue<Lambda> queue_;
//};
//
//struct Node {
//    Node* l, * r;
//
//    int check() const {
//        if (l)
//            return l->check() + 1 + r->check();
//        else
//            return 1;
//    }
//};
//
//namespace {
//    constexpr size_t LINE_SIZE = 64;
//    constexpr size_t SIZEOF_NODE = sizeof(Node);
//    constexpr auto MIN_DEPTH = 4;
//}
//
//Node* make(const int d, MemoryPool& store) {
//    Node* root = static_cast<Node*>(store.allocate(SIZEOF_NODE));
//    if (d > 0) {
//        root->l = make(d - 1, store);
//        root->r = make(d - 1, store);
//    }
//    else {
//        root->l  = nullptr; //= root->r
//    }
//    return root;
//}
//
//int main(int argc, char* argv[]) {
//    auto&& ms = xx::NowSteadyEpochMS();
//
//    const int max_depth =
//            std::max(MIN_DEPTH + 2, (argc == 2 ? atoi(argv[1]) : 21));
//    const int stretch_depth = max_depth + 1;
//
//    // Alloc then dealloc stretchdepth tree.
//    {
//        MemoryPool store;
//
//        Node* c = make(stretch_depth, store);
//        std::cout << "stretch tree of depth " << stretch_depth << "\t "
//                  << "check: " << c->check() << std::endl;
//    }
//
//    MemoryPool long_lived_store;
//    Node* long_lived_tree = make(max_depth, long_lived_store);
//
//    // Buffer to store output of each thread.
//    std::vector<char> buf(
//            static_cast<size_t>(LINE_SIZE * (max_depth + 1) * sizeof(char)), 0);
//
//    {
//        ThreadPool pool;
//        for (int d = MIN_DEPTH; d <= max_depth; d += 2) {
//            const int iterations = 1 << (max_depth - d + MIN_DEPTH);
//            pool.enqueue_task([iterations, d, &buf]() {
//                int c = 0;
//
//                // Create a memory pool for this thread to use.
//                MemoryPool store;
//
//                for (int i = 1; i <= iterations; ++i) {
//                    Node* a = make(d, store);
//                    c += a->check();
//                    store.release();
//                }
//
//                // each thread write to separate location
//                sprintf(buf.data() + LINE_SIZE * d,
//                        "%d\t trees of depth %d\t check: %d\n",
//                        iterations, d, c);
//            });
//        }
//    }
//
//    // print all results
//    for (int d = MIN_DEPTH; d <= max_depth; d += 2) {
//        printf("%s", buf.data() + (d * LINE_SIZE));
//    }
//
//    std::cout << "long lived tree of depth " << max_depth << "\t "
//              << "check: " << (long_lived_tree->check()) << "\n";
//
//    std::cout << "ms = " << xx::NowSteadyEpochMS() - ms << std::endl;
//    return 0;
//}


//#include <memory_resource>
//#include <iostream>
//#include <chrono>
//#include <array>
//using namespace std::chrono;
//
//template<typename T>
//void Test(T& vv) {
//    auto&& now = system_clock::now();
//    int num = 100000;
//    vv.reserve(num);
//    for (int i = 0; i < num; ++i) {
//        auto&& v = vv.emplace_back();
//        v.assign({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 });
//    }
//    std::cout << "fill time = " << duration_cast<nanoseconds>(system_clock::now() - now).count() << std::endl;
//    now = system_clock::now();
//    uint64_t count = 0;
//    for (auto&& v : vv) {
//        for (auto&& i : v) {
//            if (i == 2) {
//                ++count;
//            }
//        }
//    }
//    std::cout << count << ", count time = " << duration_cast<nanoseconds>(system_clock::now() - now).count() << std::endl;
//}
//
//int main() {
//    for (auto i = 0; i < 10; i++) {
//        std::vector<std::vector<int>> vv;
//        Test(vv);
//    }
//    std::cout << "=============================" << std::endl;
//    for (auto i = 0; i < 10; i++) {
//        auto&& buf = std::make_unique<std::array<char, 1024 * 1024 * 16>>();
//        std::pmr::monotonic_buffer_resource pool{ buf.get(), buf->size() };
//        std::pmr::vector<std::pmr::vector<int>> vv{ &pool };
//        Test(vv);
//    }
//    return 0;
//}



//#include <memory_resource>
//#include <iostream>
//
//struct Foo {
//    typedef std::pmr::polymorphic_allocator<char> allocator_type;
//    allocator_type a;
//    explicit Foo(allocator_type const &a) : a(a) {
//        std::cout << "Foo(allocator_type const &a) a.resource() = " << a.resource() << std::endl;
//    }
//    Foo(Foo const &o, allocator_type a = {}) : Foo(a) {
//        std::cout << "Foo(Foo const &o, allocator_type a = {}) a.resource() = " << a.resource() << std::endl;
//    }
//    Foo &operator=(Foo const &o) {
//        std::cout << "Foo &operator=(Foo const &o) {" << std::endl;
//        return *this;
//    }
//    Foo(Foo &&o, allocator_type a = {}) noexcept : a(a) {
//        if (a == o.a) {
//            std::cout << "Foo(Foo&&o, allocator_type a = {}) a == o.a a.resource() = " << a.resource() << std::endl;
//        }
//        else {
//            std::cout << "Foo(Foo&&o, allocator_type a = {}) a.resource() = " << a.resource() << "o.a.resource() = " << o.a.resource() << std::endl;
//        }
//    }
//};
//
//int main() {
//    constexpr int bufLen = 200;
//    char buf[bufLen];
//    std::pmr::monotonic_buffer_resource pool{buf, bufLen};
//    std::cout << &pool << std::endl;
//    std::pmr::vector<Foo> foos{&pool};
//    foos.emplace_back();
//    foos.emplace_back();
//    foos.emplace_back();
//    foos.emplace_back();
//    foos.emplace_back();
//    return 0;
//}



//#include "xx_object.h"
//#include "PKG_class_lite.h"
//
//#include "mylog.h"
//#include "xx_chrono.h"
//
//int main() {
//    // 预热
//    for (int i = 0; i < 500'000; ++i) {
//        LOG_INFO("asdf ", i, 2.3, "asdfasdf");
//    }
//    // 等落盘
//    std::this_thread::sleep_for(std::chrono::seconds (2));
//
//    auto &&beginMS = xx::NowSteadyEpochMS();
//    for (int i = 0; i < 500'000; ++i) {
//        LOG_INFO("asdf ", i, 2.3, "asdfasdf");
//    }
//    std::cout << "elapsed ms = " << xx::NowSteadyEpochMS() - beginMS << std::endl;
//    return 0;
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//    // 创建类辅助器
//    xx::ObjectHelper oh;
//
//    // 注册类型
//    PKG::PkgGenTypes::RegisterTo(oh);
//
//    xx::Data data;
//    xx::DataWriter ddww(data);
//    ddww.Write(std::make_pair<int, int>(1, 2));
//    xx::DataReader ddrr(data);
//    int a, b;
//    ddrr.Read(a, b);
//    data.Clear();
//
//    {
//        // 构建一个场景
//        auto &&scene = std::make_shared<PKG::Scene>();
//        auto &&node1 = std::make_shared<PKG::Node>();
//        scene->childs.push_back(node1);
//        node1->parent = scene;
//
//        auto &&node1_1 = std::make_shared<PKG::Node>();
//        node1->childs.push_back(node1_1);
//        node1_1->parent = node1;
//
//        auto &&node1_2 = std::make_shared<PKG::Node>();
//        node1->childs.push_back(node1_2);
//        node1_2->parent = node1;
//
//        scene->nodes["1"] = node1;
//        scene->nodes["1_1"] = node1_1;
//        scene->nodes["1_2"] = node1_2;
//
//        // 序列化进 data
//        oh.WriteTo(data, scene);
//        // 打印 data 的内容
//        oh.CoutN(data);
//
//        // 测试下克隆
//        auto &&scene2 = oh.Clone(scene);
//        // 比较数据是否相同。相同则篡改下
//        if (!oh.Compare(scene, scene2)) {
//            // 故意改点东西
//            scene2->parent = scene2;
//            // 如果比较结果不一致则输出
//            if (oh.Compare(scene, scene2)) {
//                oh.CoutCompareResult();
//            }
//        }
//    }
//
//    oh.CoutN(data);
//    {
//        oh.CoutN(oh.ReadObjectFrom(data));
//    }
//
//    data.Clear();
//    {
//        auto &&d = std::make_shared<PKG::D>();
//        d->name = "d";
//        d->desc = "nullable";
//        d->a.x = 1;
//        d->a.y = 2;
//        d->a.c = d;
//        d->b.x = 3;
//        d->b.y = 4;
//        d->b.z = 5;
//        d->b.c = d;
//        d->b.wc = d;
//
//        oh.WriteTo(data, d);
//    }
//    oh.CoutN(data);
//    {
//        oh.CoutN(oh.ReadObjectFrom(data));
//    }
//}
