#include "FF_class_lite.h"
#include "xx_chrono.h"
#include "xx_lua.h"

// todo: ����������͵ĸ��� �� ���ȱ��������ж���� size + offset > len ��ʧ��

enum class ��ɫ {
	Ͳ, ��, ��
};
struct �� {
	��ɫ ��ɫ;
	int ����;
};
constexpr �� operator"" Ͳ(uint64_t v) {
	return { ��ɫ::Ͳ, (int)v };
}
constexpr �� operator"" ��(uint64_t v) {
	return { ��ɫ::��, (int)v };
}
constexpr �� operator"" ��(uint64_t v) {
	return { ��ɫ::��, (int)v };
}

int main() {
	std::vector<��> ps = { 5��, 3�� };

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
