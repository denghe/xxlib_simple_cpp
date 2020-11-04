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

	inline void ToString(xx::ObjManager& o) const override {}

	inline void ToStringCore(xx::ObjManager& o) const override {}

	inline void Clone1(xx::ObjManager& o, xx::ObjBase_s const& tar) const override {}

	inline void Clone2(xx::ObjManager& o, xx::ObjBase_s const& tar) const override {}
};

template<>
struct xx::TypeId<A> {
	static const uint16_t value = 1;
};

//struct Foo {
//	int n = 1;
//
//	~Foo() {
//		std::cout << "~Foo()" << std::endl;
//	}
//};
//
///*__attribute__ ((noinline))*/ int64_t Test1() {
//	std::vector<xx::Shared<Foo>> c;
//	auto w = c.emplace_back(xx::MakeShared<Foo>()).ToWeak();
//	uint64_t x = 0;
//	for (size_t i = 0; i < 1000000000; i++) {
//		if (auto&& o = w.Lock()) x += o->n;
//		//if (w) x += w->n;
//		if (i == 1000000000 / 2) {
//			std::cout << i << std::endl;
//		}
//		if (i == 999999999) {
//			std::cout << c.size() << std::endl;
//			c.clear();
//			std::cout << c.size() << std::endl;
//		}
//	}
//	return x;
//}

//#include <variant>
//using IFS = std::variant<int, float>;
//
//uint64_t fib(uint64_t x) {
//	if (x < 2) return x;
//	return fib(x - 1) + fib(x - 2);
//}


// for test
template<typename T>
void Write(xx::Data& d, T const& v) {
    if constexpr ( xx::IsVector_v<T>) {
        d.WriteVarIntger(v.size());
        for (auto &&o : v) {
            Write(d, o);
        }
    }
    else if constexpr (std::is_same_v<std::string, T>) {
        auto siz = v.size();
        d.WriteVarIntger(siz);
        d.WriteBuf(v.data(), siz);
    }
}

int main() {

    xx::Data d;
	std::vector<std::vector<std::vector<std::string>>> v = {{{"asdfqwer"}}};
	{
		auto secs = xx::NowEpochSeconds();
		for (size_t i = 0; i < 10000000; i++) {
			d.Clear();
			Write(d, v);
		}
		xx::CoutN((xx::NowEpochSeconds() - secs), " ", d);
	}


	//size_t input;
	//std::cin >> input;

	//for (size_t i = 0; i < 10; i++)
	//{
	//	auto secs = xx::NowEpochSeconds();
	//	auto res = fib(input);
	//	std::cout << (xx::NowEpochSeconds() - secs) << std::endl;
	//	std::cout << res << std::endl;
	//}
	//return 0;

	//size_t numInts = 0, numFloats = 0;
	//std::cout << "plz input numInts  numFloats: " << std::endl;
	//std::cin >> numInts >> numFloats;
	//std::cout << numInts << "," << numFloats << std::endl;

	//std::vector<IFS> vars;
	//vars.reserve(numInts + numFloats);
	//for (size_t i = 0; i < numInts; i++) {
	//	vars.emplace_back((int)1);
	//}
	//for (size_t i = 0; i < numFloats; i++) {
	//	vars.emplace_back((float)1.1);
	//}

	//for (size_t i = 0; i < 10; i++)
	//{
	//	auto secs = xx::NowEpochSeconds();
	//	size_t counts0{};
	//	double counts1{};
	//	for (auto&& v : vars) {
	//		switch (v.index()) {
	//		case 0:
	//			counts0 += *(int*)&v;
	//			break;
	//		case 1:
	//			counts1 += *(float*)&v;
	//			break;
	//		}
	//	}
	//	std::cout << (xx::NowEpochSeconds() - secs) << std::endl;
	//	std::cout << counts0 << "," << counts1 << std::endl;
	//}


	


	//{
	//	auto secs = xx::NowEpochSeconds();
	//	auto x = Test1();
	//	xx::CoutN(xx::NowEpochSeconds() - secs, " ", x);
	//}


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


	//__builtin_trap();
	return 0;
}

