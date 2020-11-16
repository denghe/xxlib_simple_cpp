#include "FF_class_lite.h"
#include "xx_chrono.h"
#include "xx_lua.h"

namespace xx::Lua {
	template<typename T>
	struct MetaFuncs<T, std::enable_if_t<(xx::IsXxShared_v<T> && std::is_same_v<typename T::ElementType, ObjBase>)
		|| std::is_same_v<ObjBase*, std::decay_t<T>>
		>> {
		inline static std::string name = std::string(TypeName_v<T>);

		static inline void Fill(lua_State* const& L) {
			Meta<T>(L, name)
				.Lambda("HasValue", [](T& self)->bool { return (bool)self; })
				.Lambda("Reset", [](T& self) { self.Reset(); })
				.Lambda("Copy", [](T& self)->T { return self; })
				.Lambda("Assign", [](T& self, T const& o) { self = o; });
		}
	};

	template<typename T>
	struct MetaFuncs<T, std::enable_if_t<xx::IsXxShared_v<T> && std::is_same_v<typename T::ElementType, FF::Foo>
		|| std::is_pointer_v<T> && std::is_same_v<FF::Foo*, std::decay_t<T>>>> {
		inline static std::string name = std::string(TypeName_v<T>);

		static inline void Fill(lua_State* const& L) {
			using U = std::conditional_t<std::is_pointer_v<T>, ObjBase*, Shared<ObjBase>>;
			Meta<T, U>(L, name)
				.Lambda("RandomNext", [](T& self)->double { if (!self) return 0; return self->rnd.NextDouble(); });
		}
	};
}

int main() {
	xx::ObjManager om;
	FF::PkgGenTypes::RegisterTo(om);

	xx::Lua::State L;
	xx::Lua::SetGlobal(L, "foo", xx::MakeShared<FF::Foo>());
	xx::Lua::DoString(L, R"(
print(foo:HasValue())
print(foo:RandomNext())
print(foo:Reset())
print(foo:HasValue())
print(foo:RandomNext())

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
	xx::Lua::AssertTop(L, 1);

	xx::Data d;
	WriteTo(L, d);
	om.CoutN(d);

	lua_pop(L, 1);
	xx::Lua::AssertTop(L, 0);

	ReadFrom(L, d);
	xx::Lua::AssertTop(L, 1);
	lua_setglobal(L, "t2");

	DoString(L, R"(
tprint(t2)
)");

	return 0;
}
