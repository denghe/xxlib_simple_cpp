#include <xx_sharedlist.h>
#include <xx_chrono.h>
#include <iostream>

template<typename T>
double Run(T &o, uint64_t const &n) {
    uint64_t count = 0;
    auto beginTime = xx::NowSteadyEpochSeconds();
    {
        for (uint64_t i = 0; i < n; ++i) {
            for (auto &&ints : o) {
                if constexpr(std::is_same_v<T, xx::SharedList<xx::SharedList<long>>>) {
                    count += ints[1];
                } else {
                    count += (*ints)[1];
                }
            }
        }
    }
    auto endTime = xx::NowSteadyEpochSeconds();
    std::cout << (endTime - beginTime)
              << " " << count
              << std::endl;
    return endTime - beginTime;
}

int main() {
    uint64_t n = 10;
    double V = 0, L = 0;
    xx::SharedList<xx::SharedList<long>> list;
    {
        xx::SharedList<long> o;
        for (int j = 0; j < 10; ++j) {
            o.push_back(j);
        }
        for (int i = 0; i < 100000; ++i) {
            list.emplace_back(o);
        }
    }
    std::vector<std::shared_ptr<std::vector<long>>> vect;
    {
        auto o = std::make_shared<std::vector<long>>();
        for (int j = 0; j < 10; ++j) {
            (*o).push_back(j);
        }
        for (int i = 0; i < 100000; ++i) {
            vect.emplace_back(o);
        }
    }
    for (int i = 0; i < 10; ++i) {
        std::cout << "  SharedList ";
        L += Run(list, n);
        std::cout << "vector ";
        V += Run(vect, n);
    }
    std::cout << "V = " << V << ", L = " << L << std::endl;
    return 0;
}
