#include <xx_data_rw.h>
#include <xx_string.h>

int main() {
    std::unordered_map<std::string, std::map<int, std::vector<std::string>>> m;
    m["s1"][4] = {"xxxxxxxxxxxx", "sadfdf"};
    m["s1"][3] = {"qwerzxcv"};
    m["s2"][1] = {"asdf", "123", "xcvxvc"};
    m["s2"][3] = {"qwerzxcv"};

    xx::Data d;
    xx::DataWriter dw(d);
    dw.Write(m);

    xx::CoutN(d);

    xx::DataReader dr(d);
    std::unordered_map<std::string, std::map<int, std::vector<std::string>>> m2;
    dr.Read(m2);
    xx::CoutN(m2);

//
//    xx::SharedList<int> ints;
//    //ints.Emplace()
//    ints.Add(1,2,3,4,5);
//    ints.AddRange(ints);
//    ints.Emplace(6);
//    ints.EmplaceAt(0, 7);
//    std::cout << ints.Find(7) << std::endl;
    
//    for(auto&& o : ints) {
//        std::cout << o << std::endl;
//    }
//    std::cout << ints.buf << std::endl;
//    std::cout << ints.useCount() << std::endl;
//    {
//        auto ints2 = ints;
//        std::cout << ints2.buf << std::endl;
//        std::cout << ints.useCount() << std::endl;
//    }
//    std::cout << ints.useCount() << std::endl;
//    auto ints3 = std::move(ints);
//    std::cout << ints.useCount() << std::endl;
//    std::cout << ints3.useCount() << std::endl;

    return 0;
}


//#include <xx_sharedlist.h>
//#include <xx_chrono.h>
//#include <iostream>
//
//template<typename T>
//double Run(T &o, uint64_t const &n) {
//    uint64_t count = 0;
//    auto beginTime = xx::NowSteadyEpochSeconds();
//    {
//        for (uint64_t i = 0; i < n; ++i) {
//            for (auto &&ints : o) {
//                if constexpr(std::is_same_v<T, xx::SharedList<xx::SharedList<long>>>) {
//                    count += ints[1];
//                } else {
//                    count += (*ints)[1];
//                }
//            }
//        }
//    }
//    auto endTime = xx::NowSteadyEpochSeconds();
//    std::cout << (endTime - beginTime)
//              << " " << count
//              << std::endl;
//    return endTime - beginTime;
//}
//
//int main() {
//    uint64_t n = 100;
//    double V = 0, L = 0;
//    xx::SharedList<xx::SharedList<long>> list;
//    {
//        xx::SharedList<long> o;
//        for (int j = 0; j < 10; ++j) {
//            o.Emplace(j);
//        }
//        for (int i = 0; i < 100000; ++i) {
//            list.Emplace(o);
//        }
//    }
//    std::vector<std::shared_ptr<std::vector<long>>> vect;
//    {
//        auto o = std::make_shared<std::vector<long>>();
//        for (int j = 0; j < 10; ++j) {
//            (*o).push_back(j);
//        }
//        for (int i = 0; i < 100000; ++i) {
//            vect.emplace_back(o);
//        }
//    }
//    for (int i = 0; i < 100; ++i) {
//        std::cout << "  SharedList ";
//        L += Run(list, n);
//        std::cout << "vector ";
//        V += Run(vect, n);
//    }
//    std::cout << "V = " << V << ", L = " << L << std::endl;
//    return 0;
//}
