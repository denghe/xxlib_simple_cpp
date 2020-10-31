#include "xx_ptr_writer.h"
#include "xx_chrono.h"
#include <iostream>

// todo: BytesReader StringWriter

struct A {
	xx::Weak<A> parent;
	std::vector<xx::Shared<A>> children;

	template<typename H> void ForeachMembers(H& h) {
		h(this->parent)(this->children);
	}
	inline virtual void operator()(xx::BytesWriter& o) {
		ForeachMembers(o);
	}
};

struct B : A {
	uint32_t abc = 5;

	template<typename H> void ForeachMembers(H& h) {
		h(this->abc);
	}
	inline virtual void operator()(xx::BytesWriter& o) {
		this->A::operator()(o);
		ForeachMembers(o);
	}
};

template<> struct xx::PtrTypeId<A> { static const uint32_t value = 1; };
template<> struct xx::PtrTypeId<B> { static const uint32_t value = 2; };


inline std::ostream& operator<<(std::ostream& os, xx::SharedBytes const& c) {
	os << '[';
	auto len = c.len();
	if (len > 1) {
		for (uint32_t i = 0; i < len - 1; ++i) {
			os << (uint32_t)c.pointer[i] << ',';
		}
	}
	os << (uint32_t)c.pointer[len - 1];
	os << ']';
	return os;
}

int main() {
	try {
		auto c = xx::MakeShared<int>(2);
		auto wc = c.ToWeak();
		std::cout << wc.Lock().Value() << std::endl;
		c.Reset();
		std::cout << wc.Lock().Value() << std::endl;
	}
	catch (std::exception const& ex) {
		std::cout << ex.what() << std::endl;
	}

	auto&& a = xx::MakeShared<A>();
	a->parent = a;
	a->children.emplace_back(xx::MakeShared<B>());
	a->children.emplace_back(xx::MakeShared<B>());
	xx::Weak<A> wa = a;

	xx::BytesWriter pw;

	xx::SharedBytes d;
	pw.Write(d, a);
	std::cout << d << std::endl;

	// todo: BytesReader, StringWriter

	    {
	        auto secs = xx::NowSteadyEpochSeconds();
	        for (size_t i = 0; i < 10000000; i++) {
	            d.Clear();
	            pw.Write(d, a);
	        }
			std::cout << (xx::NowSteadyEpochSeconds() - secs) << " " << d << std::endl;
	    }

	    {
	        auto secs = xx::NowSteadyEpochSeconds();
	        auto c = xx::MakeShared<int>(1);
	        auto d = c.ToWeak();
	        int x = 0;
	        for (size_t i = 0; i < 1000000000; i++) {
	            try {
	                x += d.Lock().Value();
	            }
	            catch(std::exception const& ex) {
					std::cout << ex.what() << std::endl;
	            }
	        }
			std::cout << (xx::NowSteadyEpochSeconds() - secs) << " " << x << std::endl;
	    }
	
	    {
	        auto secs = xx::NowSteadyEpochSeconds();
	        auto c = std::make_shared<int>(1);
	        std::weak_ptr<int> d = c;
	        int x = 0;
	        for (size_t i = 0; i < 1000000000; i++) {
	            if (auto &&e = d.lock()) {
	                x += *e;
	            }
	        }
			std::cout << (xx::NowSteadyEpochSeconds() - secs) << " " << x << std::endl;
	    }

	return 0;
}

