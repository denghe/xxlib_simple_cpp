#include "FF_class_lite.h"
#include "xx_chrono.h"
#include "xx_lua.h"
#include "sol/sol.hpp"

void Test1() {
	xx::Lua::State L;
	xx::Lua::DoString(L, R"(
function xxx()
end
)");
	std::function<void()> xxx;
	xx::Lua::GetGlobal(L, "xxx", xxx);

	auto secs = xx::NowEpochSeconds();
	for (size_t i = 0; i < 100000000; i++) {
		xxx();
	}
	std::cout << xx::NowEpochSeconds(secs) << std::endl;
}

void Test2() {
	sol::state L;
	L.script(R"(
function xxx()
end
)");
	sol::function f = L["xxx"];
	std::function<void()> xxx = f;

	auto secs = xx::NowEpochSeconds();
	for (size_t i = 0; i < 100000000; i++) {
		xxx();
	}
	std::cout << xx::NowEpochSeconds(secs) << std::endl;
}

namespace xx::Lua {
	template<typename T>
	struct MetaFuncs<T, std::enable_if_t<xx::IsXxShared_v<T>&& std::is_same_v<typename T::ElementType, int>>> {
		inline static std::string name = std::string(TypeName_v<T>);

		static inline void Fill(lua_State* const& L) {
			Meta<T>(L, name)
				.Lambda("HasValue"
					, [](T& self)->bool {
						return (bool)self;
					})
				.Lambda("GetValue"
					, [](T& self)->int {
						if (self) return *self;
						else return 0;
					})
				.Lambda("SetValue"
					, [](T& self, int const& v) {
						if (self) {
							*self = v;
						}
					})
				.Lambda("Reset"
					, [](T& self) {
						self.Reset();
					})
				.Lambda("Assign"
					, [](T& self, T const& o) {
						self = o;
					})
						;
		}
	};
}

int main() {

	Test1();
	Test2();

	return 0;

	xx::ObjManager om;
	FF::PkgGenTypes::RegisterTo(om);

	using namespace xx::Lua;
	State L;

	SetGlobal(L, "i", xx::MakeShared<int>(123));

	DoString(L, R"(
print(i)
print(i:HasValue())
print(i:GetValue())
print(i:SetValue(2))
print(i:Reset())
print(i:HasValue())

function tprint (tbl, indent)
  if not indent then indent = 0 end
  for k, v in pairs(tbl) do
    formatting = string.rep("  ", indent) .. k .. ": "
    if type(v) == "table" then
      print(formatting)
      tprint(v, indent+1)
    elseif type(v) == 'boolean' then
      print(formatting .. tostring(v))      
    else
      print(formatting .. v)
    end
  end
end

t = {
	[1] = "asdf",
	[3] = 1.2,
	abc = {
		[5] = 3.4,
		k1 = "v1",
		k2 = "v2"
	}
}

tprint(t)
)");
	lua_getglobal(L, "t");
	AssertTop(L, 1);

	xx::Data d;
	WriteTo(L, d);
	om.CoutN(d);

	lua_pop(L, 1);
	AssertTop(L, 0);

	ReadFrom(L, d);
	AssertTop(L, 1);
	lua_setglobal(L, "t2");

	DoString(L, R"(
tprint(t2)
)");

	return 0;
}
 