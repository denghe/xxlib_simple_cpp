#include <memory_resource>
#include <iostream>
#include <chrono>
#include <array>
using namespace std::chrono;

template<typename T>
void Test(T& vv) {
    int num = 100000;
    auto&& now = system_clock::now();
    for (int i = 0; i < num; ++i) {
        auto&& v = vv.emplace_back();
        v.assign({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 });
    }
    std::cout << "fill time = " << (system_clock::now() - now).count() << std::endl;
    now = system_clock::now();
    uint64_t count = 0;
    for (auto&& v : vv) {
        for (auto&& i : v) {
            if (i == 2) {
                ++count;
            }
        }
    }
    std::cout << count << ", count time = " << (system_clock::now() - now).count() << std::endl;
}

int main() {
    for (auto i = 0; i < 10; i++) {
        std::vector<std::vector<int>> vv;
        Test(vv);
    }
    std::cout << "=============================" << std::endl;
    for (auto i = 0; i < 10; i++) {
        auto&& buf = std::make_unique<std::array<char, 1024 * 1024 * 16>>();
        std::pmr::monotonic_buffer_resource pool{ buf.get(), buf->size() };
        std::pmr::vector<std::pmr::vector<int>> vv{ &pool };
        Test(vv);
    }
    return 0;
}


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
