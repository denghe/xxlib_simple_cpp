#include "FF_class_lite.h"
#include "xx_chrono.h"
#include "xx_lua.h"

namespace xx::Lua {
	// 填充针对 Shared<> 本体的函数映射( 每个最终类型都要调用一次 )
	template<typename T, typename N>
	inline void MetaFuncs_Fill_Shared(lua_State* const& L, N const& name) {
		if constexpr (xx::IsXxShared_v<T>) {
			xx::ObjManager* om = nullptr;
			xx::Lua::GetGlobal(L, "om", om);
			Meta<T>(L, name)
				.Lambda("Reset", [](T& self) { self.Reset(); })
				.Lambda("HasValue", [](T& self) { return self.HasValue(); })
				.Lambda("Empty", [](T& self) { return self.Empty(); })

				.Lambda("typeId", [](T& self) { return self.typeId(); })
				.Lambda("useCount", [](T& self) { return self.useCount(); })
				.Lambda("refCount", [](T& self) { return self.refCount(); })
				.Lambda("GetHeaderInfo", [](T& self) { return self.GetHeaderInfo(); })

				.Lambda("Copy", [om](T& self) { return self; })
				.Lambda("Clone", [om](T& self)->T { return om->Clone(self); })
				.Lambda("Assign", [](T& self, T const& o) { self = o; })
				.Lambda("HasRecursive", [om](T& self) { return om->HasRecursive(self) != 0; })
				.Lambda("KillRecursive", [om](T& self) { om->KillRecursive(self); })
				.Lambda("__eq", [](T& self, Shared<ObjBase>& o) { return self.pointer == o.pointer; })
				.Lambda("__tostring", [om](T& self) { std::string s; om->AppendTo(s, self); return s; })
				;
		}
	}

	// for Shared<ObjBase>
	template<typename T>
	struct MetaFuncs<T, std::enable_if_t<xx::IsXxShared_v<T>&& std::is_same_v<typename T::ElementType, ObjBase>>> {
		inline static std::string name = std::string(TypeName_v<T>);
		static inline void Fill(lua_State* const& L) {
			xx::ObjManager* om = nullptr;
			xx::Lua::GetGlobal(L, "om", om);
			Meta<T>(L, name)
				.Lambda("AsFoo", [om](T& self) { return om->As<FF::Foo>(self); });
			// todo: 附加一堆 AsXxxxxx / ToXxxxxx ?
		}
	};

	// for Shared<FF::Foo> 或 FF::Foo*
	template<typename T>
	struct MetaFuncs<T, std::enable_if_t<xx::IsXxShared_v<T>&& std::is_same_v<typename T::ElementType, FF::Foo>
		|| std::is_pointer_v<T> && std::is_same_v<FF::Foo*, std::decay_t<T>>>> {
		inline static std::string name = std::string(TypeName_v<T>);
		static inline void Fill(lua_State* const& L) {
			MetaFuncs_Fill_Shared<T>(L, name);
			Meta<T, std::conditional_t<xx::IsXxShared_v<T>, Shared<ObjBase>, void>>(L, name)
				// unsafe
				.Lambda("GetRnd", [](T& self)->decltype(FF::Foo::rnd)* { if (!self) return nullptr; return &self->rnd; })
				.Lambda("RndNextDouble", [](T& self)->double { if (!self) return 0; return self->rnd.NextDouble(); });
		}
	};

	// for FF::Foo::rnd*
	template<typename T>
	struct MetaFuncs<T, std::enable_if_t<std::is_same_v<decltype(FF::Foo::rnd)*, std::decay_t<T>>>> {
		inline static std::string name = std::string(TypeName_v<T>);
		static inline void Fill(lua_State* const& L) {
			Meta<T>(L, name)
				.Lambda("SetSeed", [](T& self, int const& seed) { self->seed = seed; })
				.Lambda("NextDouble", [](T& self) { return self->NextDouble(); });
		}
	};


	// for Shared<FF::Node>
	template<typename T>
	struct MetaFuncs<T, std::enable_if_t<xx::IsXxShared_v<T>&& std::is_same_v<typename T::ElementType, FF::Node>>> {
		inline static std::string name = std::string(TypeName_v<T>);
		static inline void Fill(lua_State* const& L) {
			MetaFuncs_Fill_Shared<T>(L, name);
			Meta<T, Shared<typename T::ElementType::BaseType>>(L, name)
				.Prop("GetChild", "SetChild", &FF::Node::child);
		}
	};
}

int main() {

	xx::Lua::State L;
	xx::Lua::DoString(L, R"(
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
)");


	std::cout << "------------------------------------------------" << std::endl;
	xx::Lua::SetGlobal(L, "IntArrayTest", [](std::vector<int> const& ints) {
		for (auto&& i : ints) {
			std::cout << i << std::endl;
		}
		});

	xx::Lua::DoString(L, R"(
IntArrayTest({ [1] = 12, [2] = 34 })
)");



	std::cout << "------------------------------------------------" << std::endl;
	xx::Lua::DoString(L, R"(
FFF = function(t, m, b)
	tprint(t)
	tprint(m)
	print(b)
end
)");
	std::function<void(std::vector<int> const&, std::unordered_map<std::string, int> const&, bool const&)> OnPushVectorInt;
	xx::Lua::GetGlobal(L, "FFF", OnPushVectorInt);
	OnPushVectorInt({ 12,34,56 }, { {"ab", 12}, {"cd", 34} }, true);





	std::cout << "------------------------------------------------" << std::endl;

	xx::ObjManager om;
	FF::PkgGenTypes::RegisterTo(om);

	xx::Lua::SetGlobal(L, "om", &om);	// 为一些函数执行提供上下文. 藏注册表里去似乎更好?

	xx::Lua::SetGlobal(L, "CreateNode", [] { return xx::MakeShared<FF::Node>(); });

	xx::Lua::SetGlobal(L, "CreateFoo", [] { return xx::MakeShared<FF::Foo>(); });

	xx::Lua::DoString(L, R"(

local n = CreateNode()
print(n)
n:SetChild(n)
print(n)
print(n:HasRecursive())

local n2 = n:Clone()
n:KillRecursive()

print(n2)
n2:KillRecursive()
print(n2)
print()

local foo = CreateFoo()
print(foo)
print(foo:GetRnd():NextDouble())
print(foo:RndNextDouble())




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
