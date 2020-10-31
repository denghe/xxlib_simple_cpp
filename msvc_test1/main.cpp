#include "xx_ptr_writer.h"
#include "xx_chrono.h"

struct A {
    xx::Weak<A> parent;
    std::vector<xx::Shared<A>> children;

    static const uint32_t typeId = 1;

    virtual void Write(xx::Data &d) {
        xx::PtrWriter::Get(d)(parent)(children);
    }
};

struct B : A {
    uint32_t abc = 3;

    static const uint32_t typeId = 2;

    void Write(xx::Data &d) override {
        this->A::Write(d);
        xx::PtrWriter::Get(d)(abc);
    }
};

struct C {
    int n = 1;
    static const uint32_t typeId = 3;
};

int main() {
    try {
        auto c = xx::MakeShared<C>();
        auto wc = c.ToWeak();
        xx::CoutN(wc.Lock()->n);
        c.Reset();
        xx::CoutN(wc.Lock()->n);
    }
    catch (std::exception const &ex) {
        xx::CoutN(ex.what());
    }



//    // make data: A(A)-BB
//    auto &&a = xx::MakeShared<A>();
//    a->parent = a;
//    a->children.emplace_back(xx::MakeShared<B>());
//    a->children.emplace_back(xx::MakeShared<B>());
//    xx::Weak<A> wa = a;
//
//    xx::Data d;
//
//    xx::PtrWriter pw;
//    {
//        auto secs = xx::NowSteadyEpochSeconds();
//        for (size_t i = 0; i < 10000000; i++) {
//            d.Clear();
//            pw.Write(d, a);
//        }
//        xx::CoutN(xx::NowSteadyEpochSeconds() - secs);
//        xx::CoutN(d);
//    }
//
//    {
//        auto secs = xx::NowSteadyEpochSeconds();
//        auto c = xx::MakeShared<C>();
//        xx::Weak<C> d = c;
//        int x = 0;
//        for (size_t i = 0; i < 1000000000; i++) {
//            try {
//                x += d.Lock()->n;
//            }
//            catch(std::exception const& ex) {
//                xx::CoutN(ex.what());
//            }
//        }
//        xx::CoutN(xx::NowSteadyEpochSeconds() - secs);
//        xx::CoutN(x);
//    }
//
//    {
//        auto secs = xx::NowSteadyEpochSeconds();
//        auto c = std::make_shared<C>();
//        std::weak_ptr<C> d = c;
//        int x = 0;
//        for (size_t i = 0; i < 1000000000; i++) {
//            if (auto &&e = d.lock()) {
//                x += e->n;
//            }
//        }
//        xx::CoutN(xx::NowSteadyEpochSeconds() - secs);
//        xx::CoutN(x);
//    }

    return 0;
}

