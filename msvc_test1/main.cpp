#include "xx_ptr_writer.h"

struct A {
    xx::Weak<A> parent;
    std::vector<xx::Shared<A>> children;

    static const uint32_t typeId = 1;
    virtual void Write(xx::Data& d) {
        xx::PtrWriter::Get(d)(parent)(children);
    }
};
struct B : A {
    uint32_t abc = 3;

    static const uint32_t typeId = 2;
    void Write(xx::Data& d) override {
        this->A::Write(d);
        xx::PtrWriter::Get(d)(abc);
    }
};


int main() {
    // make data: A(A)-BB
    auto&& a = xx::MakeShared<A>();
    a->parent = a;
    a->children.push_back(xx::MakeShared<B>());
    a->children.push_back(xx::MakeShared<B>());

    xx::Data d;

    xx::PtrWriter pw;
    pw.Write(d, a);

    xx::CoutN(d);

    return 0;
}
