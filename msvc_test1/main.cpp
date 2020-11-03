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


#include <variant>
#include <iostream>


struct VariantStater {
	inline static size_t intSum = 0;
	inline static double floatSum = 0;
	inline static size_t stringLenSum = 0;
	void operator() (int& i) const {
		intSum += i;
	}
	void operator() (double& f) const {
		floatSum += f;
	}
	void operator() (std::string& s) const {
		stringLenSum += s.size();
	}
};

struct VariantPrinter {
	template <class T>
	void operator() (T t) const {
		std::cout << t << std::endl;
	}
};

int main() {
	using IFS = std::variant<int, double, std::string>;

	size_t numInts = 0, numFloats = 0, numStrings = 0;
	std::cout << "plz input numInts  numFloats  numStrings: " << std::endl;
	std::cin >> numInts >> numFloats >> numStrings;
	std::cout << numInts << "," << numFloats << "," << numStrings << std::endl;

	std::vector<IFS> vars;
	for (size_t i = 0; i < numInts; i++) {
		//vars.emplace_back((int)i);
		vars.emplace_back((int)1);
	}
	for (size_t i = 0; i < numFloats; i++) {
		//vars.emplace_back((float)i);
		vars.emplace_back((double)1.1);
	}
	for (size_t i = 0; i < numStrings; i++) {
		//vars.emplace_back(std::to_string(i));
		vars.emplace_back("aaaaaaaaaaaaabbbbbbbbbbbbbbbccccccccccccc");
	}

	auto secs = xx::NowEpochSeconds();
	for (auto&& v : vars) {
		//std::visit(VariantPrinter(), v);
		std::visit(VariantStater(), v);
		//switch (v.index()) {
		//case 0: { intSum += std::get<int>(v); break; }
		//case 1: { floatSum += std::get<double>(v); break; }
		//case 2: { stringLenSum += std::get<std::string>(v).size(); break; }
		//}
	}
	std::cout << (xx::NowEpochSeconds() - secs) << std::endl;
	std::cout << VariantStater::intSum << "," << VariantStater::floatSum << "," << VariantStater::stringLenSum << std::endl;

	//{
	//	auto secs = xx::NowEpochSeconds();
	//	auto x = Test1();
	//	xx::CoutN(xx::NowEpochSeconds() - secs, " ", x);
	//}


	//xx::ObjManager om;
	//om.Register<A>();

	//auto&& a = xx::MakeShared<A>();
	//a->parent = a;
	//a->children.emplace_back(xx::MakeShared<A>());
	//a->children.emplace_back(xx::MakeShared<A>());

	//xx::Data d;

	//{
	//	auto secs = xx::NowSteadyEpochSeconds();
	//	for (size_t i = 0; i < 10000000; i++) {
	//		d.Clear();
	//		om.WriteTo(d, a);
	//	}
	//	xx::CoutN((xx::NowSteadyEpochSeconds() - secs), " ", d);
	//}
	//{
	//	auto secs = xx::NowSteadyEpochSeconds();
	//	xx::ObjBase_s o;
	//	for (size_t i = 0; i < 10000000; i++) {
	//		d.offset = 0;
	//		om.ReadFrom(d, o);
	//	}
	//	xx::CoutN((xx::NowSteadyEpochSeconds() - secs), " ", o.As<A>()->children.size());
	//}

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

