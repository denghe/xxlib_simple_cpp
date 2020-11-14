#include "FF_class_lite.h"
#include "xx_chrono.h"

template<typename T>
XX_NOINLINE void Test1(xx::ObjManager& om, xx::Data& d, T const& v) {
	auto sec = xx::NowEpochSeconds();
	for (size_t i = 0; i < 10000000; i++) {
		d.Clear();
		om.WriteTo(d, v);
	}
	om.CoutN(xx::NowEpochSeconds() - sec);
	om.CoutN(d);
}

template<typename T>
XX_NOINLINE void Test2(xx::ObjManager& om, xx::Data& d, T& v) {
	auto sec = xx::NowEpochSeconds();
	for (size_t i = 0; i < 10000000; i++) {
		d.offset = 0;
		om.ReadFrom(d, v);
	}
	om.CoutN(xx::NowEpochSeconds() - sec);
	om.CoutN(v);
}

int main() {
	xx::ObjManager om;
	FF::PkgGenTypes::RegisterTo(om);

	xx::Data d;
	d.Reserve(1024);

	{
		auto f = xx::MakeShared<FF::Foo2>();
		f->ptr.Emplace();
		om.CoutN(f);

		Test1(om, d, f);
	}

	{
		xx::ObjBase_s o;
		Test2(om, d, o);
	}


	//{
	//	xx::Shared<FF::Foo> a;
	//	a.Emplace();asdf
	//	a->children.emplace_back().Emplace();
	//	//om.CoutN(a);

	//	d.Clear();
	//	om.WriteTo(d, a);
	//	//om.CoutN(d);

	//	//d[3] = 1;
	//	//d.len -= 2;
	//	//om.CoutN(d);
	//	auto sec = xx::NowEpochSeconds();
	//	xx::Shared<FF::Foo> b;
	//	auto b_ = xx::MakeScopeGuard([&] { om.RecursiveResetRoot(b); });
	//	for (size_t i = 0; i < 10000000; i++) {
	//		d.offset = 0;
	//		om.ReadFrom(d, b);
	//	}
	//	auto sec2 = xx::NowEpochSeconds();
	//	om.CoutN(sec2 - sec);
	//	om.CoutN(b);
	//}


	//{
	//	xx::Shared<FF::Foo2> a;
	//	a.Emplace();
	//	a->children.emplace_back().Emplace();
	//	//om.CoutN(a);

	//	d.Clear();
	//	om.WriteTo(d, a);
	//	//om.CoutN(d);

	//	//d[3] = 1;
	//	//d.len -= 2;
	//	//om.CoutN(d);
	//	auto sec = xx::NowEpochSeconds();
	//	xx::Shared<FF::Foo2> b;
	//	auto b_ = xx::MakeScopeGuard([&] { om.RecursiveResetRoot(b); });
	//	for (size_t i = 0; i < 10000000; i++) {
	//		d.offset = 0;
	//		om.ReadFrom(d, b);
	//	}
	//	auto sec2 = xx::NowEpochSeconds();
	//	om.CoutN(sec2 - sec);
	//	om.CoutN(b);
	//}


	return 0;
}
