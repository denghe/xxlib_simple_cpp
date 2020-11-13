#include "FF_class_lite.h"
#include "xx_chrono.h"

//struct Foo {
//	std::vector<xx::Shared<Foo>> children;
//	Foo() {
//		std::cout << "Foo()" << std::endl;
//	}
//	~Foo() {
//		std::cout << "~Foo()" << std::endl;
//	}
//	void Init(xx::Shared<Foo>& self) {
//		children.push_back(self);
//	}
//	void KillRecursive() {
//		for (auto& o : children) {
//			if ((void*)o.pointer == (void*)this) {
//				o.Reset();
//			}
//		}
//	}
//};

int main() {
	xx::ObjManager om;
	FF::PkgGenTypes::RegisterTo(om);
	xx::Data d;
	d.Reserve(1024);

	{
		xx::Shared<FF::Foo> a;
		a.Emplace();
		a->children.emplace_back().Emplace();
		//om.CoutN(a);

		d.Clear();
		om.WriteTo(d, a);
		//om.CoutN(d);

		//d[3] = 1;
		//d.len -= 2;
		//om.CoutN(d);
		auto sec = xx::NowEpochSeconds();
		xx::Shared<FF::Foo> b;
		auto b_ = xx::MakeScopeGuard([&] { om.RecursiveResetRoot(b); });
		for (size_t i = 0; i < 10000000; i++) {
			d.offset = 0;
			om.ReadFrom(d, b);
		}
		auto sec2 = xx::NowEpochSeconds();
		om.CoutN(sec2 - sec);
		om.CoutN(b);
	}


	{
		xx::Shared<FF::Foo2> a;
		a.Emplace();
		a->children.emplace_back().Emplace();
		//om.CoutN(a);

		d.Clear();
		om.WriteTo(d, a);
		//om.CoutN(d);

		//d[3] = 1;
		//d.len -= 2;
		//om.CoutN(d);
		auto sec = xx::NowEpochSeconds();
		xx::Shared<FF::Foo2> b;
		auto b_ = xx::MakeScopeGuard([&] { om.RecursiveResetRoot(b); });
		for (size_t i = 0; i < 10000000; i++) {
			d.offset = 0;
			om.ReadFrom(d, b);
		}
		auto sec2 = xx::NowEpochSeconds();
		om.CoutN(sec2 - sec);
		om.CoutN(b);
	}


	return 0;
}
