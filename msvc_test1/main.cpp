#include "xx_ptr_obj.h"
#include "xx_chrono.h"
#include "xx_string.h"
#include <iostream>
#include <memory>

struct A : xx::ObjBase {
	xx::Weak<A> parent;
	std::vector<xx::Shared<A>> children;

	inline void Write(xx::ObjManager& o) const override { o.Write(parent, children); }
	inline void Read(xx::ObjManager& o) override { o.Read(parent, children); }
	inline void ToString(xx::ObjManager& o) const override { }
	inline void ToStringCore(xx::ObjManager& o) const override { }
	inline void Clone1(xx::ObjManager& o, xx::ObjBase_s const& tar) const override { }
	inline void Clone2(xx::ObjManager& o, xx::ObjBase_s const& tar) const override { }
};
template<> struct xx::TypeId<A> { static const uint16_t value = 1; };


int main() {
	//	xx::ObjManager om;
	//	om.Register<A>();
	//
	//	auto&& a = xx::MakeShared<A>();
	//	a->parent = a;
	//	a->children.emplace_back(xx::MakeShared<A>());
	//	a->children.emplace_back(xx::MakeShared<A>());
	//
	//	xx::Data d;
	//
	//	{
	//		auto secs = xx::NowSteadyEpochSeconds();
	//		for (size_t i = 0; i < 10000000; i++) {
	//			d.Clear();
	//			om.WriteTo(d, a);
	//		}
	//		xx::CoutN((xx::NowSteadyEpochSeconds() - secs), " ", d);
	//	}
	//	{
	//		auto secs = xx::NowSteadyEpochSeconds();
	//		xx::ObjBase_s o;
	//		for (size_t i = 0; i < 10000000; i++) {
	//			d.offset = 0;
	//			om.ReadFrom(d, o);
	//		}
	//		xx::CoutN((xx::NowSteadyEpochSeconds() - secs), " ", o.As<A>()->children.size());
	//	}

		//try {
		//	auto c = xx::MakeShared<int>(2);
		//	auto wc = c.ToWeak();
		//	std::cout << wc.Lock().Value() << std::endl;
		//	c.Reset();
		//	std::cout << wc.Lock().Value() << std::endl;
		//}
		//catch (std::exception const& ex) {
		//	std::cout << ex.what() << std::endl;
		//}

	//	{
	//		auto secs = xx::NowSteadyEpochSeconds();
	//		auto c = xx::MakeShared<int>(1);
	//		auto d = c.ToWeak();
	//		c.Reset();
	//		int x = 0;
	//		for (size_t i = 0; i < 1000000; i++) {
	//			try {
	//				x += d.Lock().Value();
	//			}
	//			catch (...) {
	//			}
	//		}
	//		std::cout << (xx::NowSteadyEpochSeconds() - secs) << " " << x << std::endl;
	//	}

	{
		auto ticks = xx::NowEpoch10m();
		auto c = xx::MakeShared<int>(1);
		auto d = c.ToWeak();
		//c.Reset();
		int x = 0;
		for (size_t i = 0; i < 1000000000; i++) {
			//if (auto&& o = d.Lock()) {
			//	x += *o;
			//}
			if (d.useCount()) {
				x += *d.Get();
			}
		}
		std::cout << (xx::NowEpoch10m() - ticks) << " " << x << std::endl;
	}

	//{
	//	auto secs = xx::NowSteadyEpochSeconds();
	//	struct D { int n = 1; };
	//	auto c = xx::MakeShared<D>();
	//	auto d = c.ToWeak();
	//	int x = 0;
	//	for (size_t i = 0; i < 1000000000; i++) {
	//		try {
	//			x += d.Lock()->n;
	//		}
	//		catch (std::exception const& ex) {
	//			std::cout << ex.what() << std::endl;
	//		}
	//	}
	//	std::cout << (xx::NowSteadyEpochSeconds() - secs) << " " << x << std::endl;
	//}

	//{
	//	auto secs = xx::NowSteadyEpochSeconds();
	//	auto c = std::make_shared<int>(1);
	//	std::weak_ptr<int> d = c;
	//	int x = 0;
	//	for (size_t i = 0; i < 1000000000; i++) {
	//		if (auto&& e = d.lock()) {
	//			x += *e;
	//		}
	//	}
	//	std::cout << (xx::NowSteadyEpochSeconds() - secs) << " " << x << std::endl;
	//}
	//__builtin_trap();
	return 0;
}

