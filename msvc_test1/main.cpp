#include "FF_class_lite.h"
#include "xx_chrono.h"

int main() {
	xx::ObjManager om;
	FF::PkgGenTypes::RegisterTo(om);

	auto f = xx::MakeShared<FF::Foo>();
	auto f_ = xx::MakeScopeGuard([&] { om.KillRecursive(f); });
	f->parent = f;

	auto f2 = xx::MakeShared<FF::Foo2>();
	f->children.emplace_back(f2);
	f2->children.emplace_back(f2);

	auto f3 = om.Clone(f2);
	f->children.emplace_back(f3);

	om.CoutN(om.HasRecursive(f));
	om.CoutN(f);

	xx::Data d;
	om.WriteTo(d, f);
	om.CoutN(d);

	
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
