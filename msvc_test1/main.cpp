#include "FF_class_lite.h"
#include "xx_chrono.h"
#include "xx_lua.h"

// todo: 针对容器类型的各种 读 长度保护。简单判断如果 size + offset > len 就失败

enum class 花色 {
	筒, 条, 万
};
struct 牌 {
	花色 花色;
	int 点数;
};
constexpr 牌 operator"" 筒(uint64_t v) {
	return { 花色::筒, (int)v };
}
constexpr 牌 operator"" 条(uint64_t v) {
	return { 花色::条, (int)v };
}
constexpr 牌 operator"" 万(uint64_t v) {
	return { 花色::万, (int)v };
}

int main() {
	std::vector<牌> ps = { 5条, 3万 };

	xx::ObjManager om;
	FF::PkgGenTypes::RegisterTo(om);

	using namespace xx::Lua;
	State L;
	DoString(L, R"(
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
