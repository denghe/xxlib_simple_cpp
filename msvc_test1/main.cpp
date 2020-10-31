#include "xx_ptr_writer.h"
#include "xx_chrono.h"

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
	a->children.emplace_back(xx::MakeShared<B>());
	a->children.emplace_back(xx::MakeShared<B>());

	xx::Data d;

	xx::PtrWriter pw;

	auto secs = xx::NowSteadyEpochSeconds();
	for (size_t i = 0; i < 10000000; i++) {
		d.Clear();
		pw.Write(d, a);
	}
	xx::CoutN(xx::NowSteadyEpochSeconds() - secs);
	xx::CoutN(d);

	return 0;
}
