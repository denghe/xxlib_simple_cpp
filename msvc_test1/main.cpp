#include "FF_class_lite.h"
#include "xx_chrono.h"

int main() {
	xx::ObjManager om;
	FF::PkgGenTypes::RegisterTo(om);

	{
		auto&& a = xx::MakeShared<FF::A>();
		auto a_sg = xx::MakeScopeGuard([&] {
			om.RecursiveResetRoot(a);
			});
		{
			a->id = 1;
			a->parent = a;
			a->children.emplace_back(a);	// recursive

			auto b = xx::MakeShared<FF::B>();
			a->children.emplace_back(b);
			b->children.emplace_back(b);	// recursive
			{
				b->id = 2;
				b->parent = b;
				b->data.WriteBuf("abcd1234", 8);
				b->c.x = 2.1f;
				b->c.y = 3.2f;
				b->c.targets.emplace_back(a);
			}
			{
				auto&& c = b->c2.emplace();
				c.x = 4.1f;
				c.y = 5.2f;
				c.targets.emplace_back(a);
			}
			{
				auto&& c = b->c3.emplace_back().emplace_back().emplace();
				c.x = 6.1f;
				c.y = 7.2f;
				c.targets.emplace_back(a);
			}
		}
		om.CoutN(a);
		{
			xx::Shared<FF::A> a2;
			auto a2_sg = xx::MakeScopeGuard([&] {
				om.RecursiveResetRoot(a2);
				});
			om.Clone(a, a2);
			om.CoutN(a2);
		}
		{
			xx::Data d;
			{
				auto secs = xx::NowEpochSeconds();
				for (size_t i = 0; i < 10000000; i++) {
					d.Clear();
					om.WriteTo(d, a);
				}
				om.CoutN((xx::NowEpochSeconds() - secs), " ", d);
				d.Clear();
			}
			om.WriteTo(d, a);
			om.CoutN(d);
			xx::Shared<FF::A> a2;
			auto b_sg = xx::MakeScopeGuard([&] {
				om.RecursiveResetRoot(a2);
				});
			{
				auto secs = xx::NowEpochSeconds();
				for (size_t i = 0; i < 10000000; i++) {
					d.offset = 0;
					if (int r = om.ReadFrom(d, a2)) {
						om.CoutN("read from error. r = ", r);
					}
				}
				om.CoutN((xx::NowEpochSeconds() - secs), " ", d);
				d.Clear();
			}
			om.CoutN(a2);
		}
	}

	return 0;
}




//#include "xx_obj.h"
//#include "xx_chrono.h"


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



//template<typename T, bool needReserve = true, typename ENABLED = std::enable_if_t<std::is_integral_v<T>>>
//__forceinline void WriteVarIntger(char* const& buf, size_t& len, T const& v) {
//	using UT = std::make_unsigned_t<T>;
//	UT u(v);
//	if constexpr (std::is_signed_v<T>) {
//		u = xx::ZigZagEncode(v);
//	}
//	while (u >= 1 << 7) {
//		buf[len++] = char((u & 0x7fu) | 0x80u);
//		u = UT(u >> 7);
//	};
//	buf[len++] = char(u);
//}
//
//template<typename T>
//__forceinline void Write(char* const& buf, size_t& len, T const& v) {
//	if constexpr (xx::IsVector_v<T>) {
//		WriteVarIntger(buf, len, v.size());
//		for (auto&& o : v) {
//			Write(buf, len, o);
//		}
//	}
//	else if constexpr (std::is_same_v<int, T>) {
//		WriteVarIntger(buf, len, v);
//	}
//}
//
//int main() {
//	char buf[1024];
//	size_t len = 0;
//	int i = 0;
//	std::cout << "plz type a int:" << std::endl;
//	std::cin >> i;
//	std::vector<std::vector<std::vector<int>>> v = { {{i}} };
//	auto secs = xx::NowEpochSeconds();
//	for (size_t i = 0; i < 10000000; i++) {
//		len = 0;
//		Write(buf, len, v);
//	}
//	xx::CoutN((xx::NowEpochSeconds() - secs), " ", xx::DataView{ buf, len });

//
//struct A : xx::ObjBase {
//	int id = 0;
//	xx::Weak<A> parent;
//	std::vector<xx::Shared<A>> children;
//	~A() {
//		std::cout << "~A() id = " << id << std::endl;
//	}
//
//	#pragma region overrides
//	inline void Write(xx::ObjManager& o) const override {
//		o.Write(id, parent, children);
//	}
//
//	inline int Read(xx::ObjManager& o) override {
//		return o.Read(id, parent, children);
//	}
//
//	inline void ToString(xx::ObjManager& o) const override {
//		o.Append("{");
//		this->ToStringCore(o);
//		o.Append("}");
//	}
//
//	inline void ToStringCore(xx::ObjManager& o) const override {
//		o.Append("\"id\":", id, ",\"parent\":", parent, ",\"children\":", children);
//	}
//
//	inline void Clone1(xx::ObjManager& o, void* const& tar) const override {
//		auto&& out = (A*)tar;
//		o.Clone1(this->id, out->id);
//		o.Clone1(this->parent, out->parent);
//		o.Clone1(this->children, out->children);
//	}
//
//	inline void Clone2(xx::ObjManager& o, void* const& tar) const override {
//		auto&& out = (A*)tar;
//		o.Clone2(this->id, out->id);
//		o.Clone2(this->parent, out->parent);
//		o.Clone2(this->children, out->children);
//	}
//
//	inline void RecursiveReset(xx::ObjManager& o) override {
//		o.RecursiveReset(this->id, this->parent, this->children);
//	}
//	#pragma endregion
//};

//template<>
//struct xx::TypeId<A> {
//	static const uint16_t value = 1;
//};

//int main() {

	//xx::ObjManager om;
	//xx::Data d;
	//d.Reserve(1024);
	//using T = std::string;
	//T i;
	//std::cout << "plz input:" << std::endl;
	//std::cin >> i;
	//std::vector<std::vector<std::vector<T>>> v = { {{i}} };
	////auto& vs = v[0][0];
	////for (size_t j = 0; j < 100; j++) {
	////	ints.emplace_back(i);
	////}
	//om.data = &d;
	//{
	//	auto secs = xx::NowEpochSeconds();
	//	for (size_t i = 0; i < 10000000; i++) {
	//		d.Clear();
	//		om.Write(v);
	//	}
	//	xx::CoutN((xx::NowEpochSeconds() - secs), " ", d);
	//}

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




	//xx::Data d;

	//{
	//	auto secs = xx::NowSteadyEpochSeconds();
	//	for (size_t i = 0; i < 10000000; i++) {
	//		d.Clear();
	//		om.WriteTo(d, a);
	//	}
	//	om.CoutN((xx::NowSteadyEpochSeconds() - secs), " ", d);
	//}
	//{
	//	auto secs = xx::NowSteadyEpochSeconds();
	//	xx::ObjBase_s o;
	//	for (size_t i = 0; i < 10000000; i++) {
	//		d.offset = 0;
	//		om.ReadFrom(d, o);
	//	}
	//	om.CoutN((xx::NowSteadyEpochSeconds() - secs), " ", o.As<A>()->children.size());
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
//	return 0;
//}
