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
	{
		xx::Shared<FF::Foo> a;
		a.Emplace();
		//a->children.emplace_back().Emplace();
		om.CoutN(a);

		xx::Data d;
		om.WriteTo(d, a);
		om.CoutN(d);

		d[4] = 1;
		om.CoutN(d);
		xx::Shared<FF::Foo> b;
		auto b_ = xx::MakeScopeGuard([&] { om.RecursiveResetRoot(b); });
		om.ReadFrom(d, b);
		om.CoutN(b);
	}

	//auto b = a;
	//b.Reset();
	//auto c = b.ToWeak();


	return 0;
}
