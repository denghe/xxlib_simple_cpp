#include "xx_lua.h"
#include <iostream>

namespace XL = xx::Lua;

struct TableStore : xx::Data {
};

namespace xx::Lua {
    template<>
    struct PushToFuncs<TableStore, void> {
        static int Push(lua_State *const &L, TableStore &&in) {
            DataReader dr(in);
            if (int r = dr.Read(L)) Error(L, " TableStore read error. r = ", r);
            return 1;
        }

        static void To(lua_State *const &L, int const &idx, TableStore &out) {
            if (!lua_istable(L, idx)) Error(L, " to TableStore error. idx = ", idx, " is not table");
            auto top = lua_gettop(L);
            if (top != idx) {
                CheckStack(L, 1);
                lua_pushvalue(L, idx);
            }
            DataWriter dw(out);
            dw.Write(L);
            if (top != idx) {
                lua_pop(L, 1);
            }
        }
    };
}

struct FishBase {
    int n = 0;
    virtual void Update() = 0;
    virtual ~FishBase() = default;
};

struct CppFish : FishBase {
    void Update() override {
        n = 1;
    }
    static std::shared_ptr<FishBase> Create(/* cfg ?*/) {
        return std::make_shared<CppFish>();
    }
};

struct LuaFish : FishBase {
    std::function<void()> onUpdate;
    void Update() override {
        onUpdate();
    }
    static std::shared_ptr<FishBase> Create(lua_State *const &L, std::string const &fileName) {
        auto self = std::make_shared<LuaFish>();
        XL::CallFile(L, fileName, xx::ToWeak(self));
        return self;
    }

    std::function<void(TableStore const &ts)> onLoadData;
    std::function<TableStore()> onSaveData;
};

namespace xx::Lua {
    template<typename T>
    struct MetaFuncs<T, std::enable_if_t<std::is_same_v<T, std::weak_ptr<LuaFish>>>> {
        inline static char const *const name = "LuaFish";
        static inline void Fill(lua_State *const &L) {
            Meta<T>(L)
                    .Prop("Get_n", "Set_n", &LuaFish::n)
                    .Prop(nullptr, "Set_onUpdate", &LuaFish::onUpdate)
                    .Prop(nullptr, "Set_onLoadData", &LuaFish::onLoadData)
                    .Prop(nullptr, "Set_onSaveData", &LuaFish::onSaveData);
        }
    };
}

int main() {
    XL::State L;
    auto r = XL::Try(L, [&] {
        std::vector<std::shared_ptr<FishBase>> fishs;
        fishs.emplace_back(CppFish::Create());
        fishs.emplace_back(LuaFish::Create(L, "fish.lua"));
        for (auto &&o : fishs) {
            o->Update();
            xx::CoutN("Fish n = ", o->n);
        }
    });
    if (r) xx::CoutN(r.m);
    xx::CoutN("end.");
    return 0;
}

//
//namespace xx::Lua {
//    template<>
//    struct MetaFuncs<Foo_u, void> {
//        inline static char const *const name = "Foo";
//
//        static inline void Fill(lua_State *const &L) {
//            Meta<Foo_u>(L)
//                    .Func("Add", &Foo::Add)
//                    .Func("CallFunc", &Foo::CallFunc)
//                    .Prop("GetA", "SetA", &Foo::a)
//                    .Prop("GetFunc", "SetFunc", &Foo::func)
//                    .Lambda("Clear", [](Foo_u &o) {
//                        o->a = 0;
//                    })
//                    .Lambda("Create", []() {
//                        return std::make_unique<Foo>();
//                    });
//        }
//    };
//}

//        XL::SetGlobalMeta<Foo_u>(L);
//        XL::DoString(L, R"(
//local f = Foo.Create()
//f:Clear()
//print(f:GetA())
//f:SetA(3)
//print(f:Add(2))
//f:SetFunc(function() print("func") end)
//f:CallFunc()
//func = f:GetFunc()
//f = nil
//collectgarbage("collect")
//)");
//        XL::CallGlobalFunc(L, "func");
//    });
//    if (r) xx::CoutN(r.m);
//    else xx::CoutN("end.");

//int main() {
//    auto x = std::make_unique<int>(1);
//    auto f = [](decltype(x) &x, int const& y) {
//        *x = y;
//    };
//    xx::FuncA_t<decltype(f)> args(x, 2);
//    std::apply([&](auto &... args) {
//        f(args...);
//    }, args);
//    std::cout << *x << std::endl;
//    return 0;
//}















//#include "xx_lua.h"
//#include "xx_chrono.h"
//#include <thread>
//
//namespace XL = xx::Lua;
//
//struct Foo {
////    inline int ToNumber(std::string const& s) {
////        int rtv = 0;
////        xx::Convert(s.c_str(), rtv);
////        return rtv;
////    }
//    ~Foo() {
//        xx::CoutN("~Foo");
//    }
//};
//
//int main(int argc, char const *argv[]) {
//    // 等同于出 scope 会 close 的 lua_State*
//    XL::State L;
//
//    // 以 pcall 方式执行 lambda 以便捕获错误
//    if (auto &&r = XL::Try(L, [&] {
//        Foo f;
//        throw -1;
////        luaL_dostring(L, R"(
////t = {
////    [1] = "asdf", qwer = 123, xxx = function() end
////}
////)");
////        xx::Data d;
////        xx::DataWriter dw(d);
////        xx::CoutN(lua_gettop(L));
////        lua_getglobal(L, "t");
////        xx::CoutN(lua_gettop(L));
////        dw.Write(L.L);
////        xx::CoutN(d);
//
//        xx::Data d{6, 5, 4, 97, 115, 100, 102, 3, 2, 3, 246, 1, 5, 4, 113, 119, 101, 114, 7};
//        xx::DataReader dr(d);
//        int rtv = dr.Read(L);
//        lua_setglobal(L, "t");
//        luaL_dostring(L, R"(
//print(type(t))
//for k, v in pairs(t) do
//    print(k, v)
//end
//)");
//
//    })) {
//        std::cout << "error: n = " << r.n << ", m = " << r.m << std::endl;
//        return r.n;
//    }
//
//
//    return 0;
//}






//        // 调用 Main 函数, 返回个 function, 调用之, 拿到返回值
//        using SubFuncRtv = std::tuple<int, std::string, char const*>;
//        using SubFunc = std::function<SubFuncRtv()>;
//        using MainFunc = std::function<SubFunc()>;
//        MainFunc mf;
//        XL::GetGlobal(L, "Main", mf);
//        xx::CoutN(mf()());


//		auto top = lua_gettop(L);
//		GetScript("xxxx");		  // ..., t
//		auto GetFunc = [&](auto key, auto& func){
//		XL::GetField(L, top+1, key)
//		XL::To(L, top+1, func)
//		lua_settop(L, top);
//		}
//		GetFunc("Update", updateFunc)
//		GetFunc("Update", updateFunc)
//		GetFunc("Update", updateFunc)
//		GetFunc("Update", updateFunc)


//
//        // 加载入口脚本文件( 如果文件里面有 local 变量，似乎会导致 L 不空 )
//        luaL_dofile(L, "main.lua");
//
//        // 调用入口函数
//        auto top = lua_gettop(L);
//        XL::CallGlobalFunc(L, "Main");
//        lua_settop(L, top);     // 忽略返回值
//
//        // 模拟一个脚本对象
//        struct Script {
//            std::string name = "test1";
//            int id = 0;
//        } script;
//
//        // 从文件加载脚本. 填充返回的 id
//        top = lua_gettop(L);
//        XL::CallGlobalFunc(L, "LoadScript", script.name);
//        XL::To(L, 1, script.id);    // 提取返回值
//        lua_settop(L, top);      // 清除返回值
//
//        // 模拟帧循环
//        auto lastSecs = xx::NowSteadyEpochSeconds();
//        for (int i = 0; i < 100; ++i) {
//            std::this_thread::sleep_for(std::chrono::milliseconds(500));
//            auto elapsedSecs = xx::NowSteadyEpochSeconds(lastSecs);
//
//            top = lua_gettop(L);
//            // 调用 lua 中声明的 更新所有脚本 函数. 传入已经历的时长
//            XL::CallGlobalFunc(L, "UpdateScripts", elapsedSecs);
//            lua_settop(L, top);      // 清除返回值
//        }


//
//
//int main(int argc, char const *argv[]) {
//    // 等同于出 scope 会 close 的 lua_State*
//    XL::State L;
//
//    // 以 pcall 方式执行 lambda 以便捕获错误
//    if (auto &&r = XL::Try(L, [&] {
//
//        XL::SetGlobal(L, "A", 123);
//        XL::SetGlobal(L, "B", "asdf");
//
//        XL::SetGlobal(L, "Now", [&](char* prefix, std::string const& suffix) {
//            return xx::ToString(prefix, xx::Now(), suffix);
//        });
//
//        luaL_dostring(L, R"(
//function Main()
//    return function()
//        return A, B, Now("[", "]")
//    end
//end
//)");
//
//        // 调用 Main 函数, 返回个 function, 调用之, 拿到返回值
//        using SubFuncRtv = std::tuple<int, std::string, char const*>;
//        using SubFunc = std::function<SubFuncRtv()>;
//        using MainFunc = std::function<SubFunc()>;
//        MainFunc mf;
//        XL::GetGlobal(L, "Main", mf);
//        xx::CoutN(mf()());





//
//    luaL_dofile(L, "test1.lua");
//    assert(lua_gettop(L) == 1);
//    assert(lua_type(L, -1) == LUA_TTABLE);
//
//    // 找个地方放置 test1.lua return t
//
//    //lua_getglobal(L, "t");          // t
//
//    xx::Data d;
//    xx::DataWriter dw(d);
//    dw.Write(L);
//    xx::CoutN(d);
//
//    lua_pop(L, 1);                  //
//    assert(lua_gettop(L) == 0);
//
//    lua_getglobal(L, "Dump");       // func
//    assert(lua_type(L, -1) == LUA_TFUNCTION);
//
//    xx::DataReader dr(d);
//    int r = dr.Read(L);             // func, t
//    assert(r == 0);
//    assert(lua_gettop(L) == 2);
//    assert(lua_type(L, -1) == LUA_TTABLE);
//
//    if ((r = lua_pcall(L, 1, LUA_MULTRET, 0))) {
//        auto s = lua_tostring(L, -1);
//        std::cout << r << " " << (s ? s : "") << std::endl;
//        lua_pop(L, 1);
//        return r;
//    }


//    xx::Data d;
//    auto ms = xx::NowSteadyEpochMS();
//    for (int i = 0; i < 1000000; ++i) {
//        lua_getglobal(L, "t");          // t
//
//        xx::DataWriter dw(d);
//        dw.Write(L);
//
//        lua_pop(L, 1);                  //
//
//        xx::DataReader dr(d);
//        int r = dr.Read(L);             // t
//
//        lua_pop(L, 1);                  //
//        d.Clear();
//    }
//    xx::CoutN(xx::NowSteadyEpochMS() - ms);


//    std::unordered_map<std::string, int> intint;
//
//    lua_pushlightuserdata(L, &intint);
//    lua_pushcclosure(L, [](lua_State *L) {
//        auto &&intint = *(std::unordered_map<std::string, int> *) lua_topointer(L, lua_upvalueindex(1));
//        auto idx = lua_tostring(L, 1);
//        auto val = lua_tointeger(L, 2);
//        intint[idx] = val;
//        return 0;
//    }, 1);
//    lua_setglobal(L, "SV");
//
//    {
//        auto r = luaL_dostring(L, R"-(
//f1 = function()
//    for i = 1, 100000 do
//        SV(tostring(i), i)
//    end
//end
//
//f2 = function()
//    local t = {}
//    for i = 1, 100000 do
//        t[tostring(i)] = i
//    end
//end
//)-");
//        if (r) {
//            std::cout << "luaL_dostring r = " << r << ", str = " << lua_tostring(L, -1) << std::endl;
//        }
//    }
//
//    // todo: 测试一下映射简单存取值函数到 lua 的性能
//
//    for (int i = 0; i < 10; ++i) {
//        intint.clear();
//        auto bt = xx::NowSteadyEpochMS();
//        lua_getglobal(L, "f1");
//        if (int r = lua_pcall(L, 0, LUA_MULTRET, 0)) {
//            auto s = lua_tostring(L, -1);
//            printf("%d %s", r, (s ? s : ""));
//            lua_pop(L, 1);
//            return r;
//        }
//        xx::CoutN(xx::NowSteadyEpochMS() - bt);
//    }
//    xx::CoutN(intint.size());
//
//    for (int i = 0; i < 10; ++i) {
//        intint.clear();
//        auto bt = xx::NowSteadyEpochMS();
//        lua_getglobal(L, "f2");
//        if (int r = lua_pcall(L, 0, LUA_MULTRET, 0)) {
//            auto s = lua_tostring(L, -1);
//            printf("%d %s", r, (s ? s : ""));
//            lua_pop(L, 1);
//            return r;
//        }
//        xx::CoutN(xx::NowSteadyEpochMS() - bt);
//    }








//--[[
//local count = 0
//for _, _ in pairs(t) do
//   count = count + 1
//end
//print(count)
//]]

//
//    auto&& r = luaL_dostring(L, R"-(
//
//__int64__ = 0LL
//__uint64__ = 0ULL
//
//function i64()
//    return 0LL
//end
//function u64()
//    return 0ULL
//end
//
//x = 12345678901234567ULL
//x = x * 1.5 * 100LL
//print(x)
//
//)-");
//    if(r) {
//        std::cout << "luaL_dostring r = " << r << ", str = " << lua_tostring(L, -1) << std::endl;
//    }
//
//    std::cout << lua_gettop(L) << std::endl;
//
//    // [-0, +1, e]
//    lua_getglobal(L, "x");
//
//    // [-0, +0, -]
//    auto&& p = lua_topointer(L, -1);
//
//    // 1234567890123456700
//    std::cout << *(uint64_t*)p << std::endl;
//
//    // [-0, +0, -]
//    std::cout << lua_typename(L, lua_type(L, -1)) << std::endl;
//
//    // [-n, +0, -]
//    lua_pop(L, 1);
//
//    std::cout << lua_gettop(L) << std::endl;
//
//
//    luaL_dostring(L, "return 0LL");
//    p = lua_topointer(L, -1);
//    *(int64_t*)p = 1;
//    lua_setglobal(L, "a");
//
//    r = luaL_dostring(L, R"-(
//print(a)
//local b = a
//a = a + 1
//print(a, b)
//)-");
//    if(r) {
//        std::cout << "luaL_dostring r = " << r << ", str = " << lua_tostring(L, -1) << std::endl;
//    }