#include "xx_object.h"
#include "xx_lua.h"
#include <iostream>
#include <chrono>

namespace XL = xx::Lua;

#include "TimeLineConfig_class_lite.h"

// 动画基类
// 每种动画都含有多个 动作， 每个动作有自己的时间线, 含有 锁定点线，碰撞区域，移动距离 等关键帧事件
struct AnimBase {
    // 改变当前播放的动画( 会改变 timeLine 的指向 )
    virtual void SetAction(std::string const &actionName) = 0;

    // 动画播放完毕后触发( 非循环播放的情况下 ), 可能切换动画继续播放返回 true，也可能自杀返回 false?
    virtual bool OnFinish() = 0;

    // 根据传入的 经历时长，调整动画状态. 返回 移动距离
    virtual float Update(float const &elapsedSeconds) = 0;

    // 判断 点(r==0) 或 圆 是否和某 cdCircle 相交( touch, bullet hit 判断需要 )
    [[nodiscard]] virtual bool IsIntersect(float const &x, float const &y, float const &r) const = 0;

    // 获取目标锁定点
    [[nodiscard]] virtual TimeLineConfig::LockPoint GetLockPoint() const = 0;
};

// 文件类动画基类( spine, 3d, frames )
struct Anim : AnimBase {
    // 指向当前 action 的 timeline
    TimeLineConfig::TimeLine const *timeLine = nullptr;

    // 记录 timeline 的下标演进索引
    int timeLineIndex = 0;

    // 当前 action 已经历的总时长
    float totalElapsedSeconds = 0;

    // 指向当前使用的 lockPoints
    TimeLineConfig::LockPoints const *lockPoints = nullptr;

    // 指向当前使用的 cdCircles
    TimeLineConfig::CDCircles const *cdCircles = nullptr;

    // 保存当前速度
    float speed = 0;

    // 保存当前图片名( spine, 3d 用不到 )
    std::string const *picName = nullptr;

    // 应用当前时间点的数据
    inline void ApplyTimePoint() {
        auto &&tp = timeLine->timePoints[timeLineIndex];
        if (tp.lps.has_value()) {
            lockPoints = &tp.lps.value();
        }
        if (tp.cdcs.has_value()) {
            cdCircles = &tp.cdcs.value();
        }
        if (tp.speed.has_value()) {
            speed = tp.speed.value();
        }
        if (tp.pic.has_value()) {
            picName = &tp.pic.value();
        }
    }

protected:
    // 内部函数. 被 Update 调用。确保传入的时长不会导致 timeLine 变化. 返回距离
    inline float UpdateCore(float const &elapsedSeconds) {

    }
public:

    // 只实现了更新指针和计算移动距离。更新显示要覆写
    inline float Update(float const &elapsedSeconds) override {
        // todo: 处理 传入时长 大于动画剩余播放时长的问题: call OnFinish?
        // todo: 从 timeLineIndex 开始遍历，直到经过传入时长？
        // todo: 如果下个时间点还小于 totalElapsedSeconds 就 ++timeLineIndex 否则就退出
        float rtv = 0;
        // 记录起始时间点
        auto last = totalElapsedSeconds;
        // 剩余未处理时长
        auto left = elapsedSeconds;
        LabRetry:
        // 如果未处理时长当前时间线消耗不完
        if (timeLine->totalSeconds < last + left) {

        }
        // 本次结束时间点
        totalElapsedSeconds += elapsedSeconds;
        // 剩余未处理时长
        auto left = totalElapsedSeconds - timeLine->totalSeconds;
        // 如果 时间线结束时间点 小于 本次结束时间点
        if (left > 0) {
            totalElapsedSeconds = timeLine->totalSeconds;
        }
        // 计算出 lastTime 到 timeLine 结束时间 的实际间隔
        auto currET = timeLine->totalSeconds - last;
        auto &&tps = timeLine->timePoints;
        auto &&tpsSize = tps.size();
        LabBegin:
        // 如果存在下一个时间点 且 位于本次 update 时间段内
        auto &&nextIdx = timeLineIndex + 1;
        if (tpsSize > nextIdx && tps[nextIdx].time <= totalElapsedSeconds) {
            // 计算出和这个时间点的时间差, 算距离
            auto es = lastElapsedSeconds - tp.time;
            rtv += speed * es;
            // 指向下一个时间点
            ++timeLineIndex;
            ApplyTimePoint();
        } else {
            // 用剩余时长算距离
            //auto es = totalElapsedSeconds - lastElapsedSeconds;
            //rtv += speed * es;
        }
        if (left > 0) {
            // todo: call OnFinish ?
            // init vars?
            goto LabRetry;
        }
        return rtv;
    }

    // 判断 点(r==0) 或 圆 是否和单个 cdCircle 相交
    inline static bool IsIntersect(TimeLineConfig::CDCircle const &c, float const &x, float const &y, float const &r) {
        return (c.x - x) * (c.x - x) + (c.y - y) * (c.y - y) <= (c.r + r) * (c.r + r);
    }

    // 判断 点(r==0) 或 圆 是否和某 cdCircle 相交( touch, bullet hit 判断需要 )
    [[nodiscard]] inline bool IsIntersect(float const &x, float const &y, float const &r) const override {
        assert(cdCircles);
        if (!IsIntersect(cdCircles->maxCDCircle, x, y, r)) return false;
        for (auto &&c : cdCircles->cdCircles) {
            if (IsIntersect(c, x, y, r)) return true;
        }
        return false;
    }

    [[nodiscard]] inline TimeLineConfig::LockPoint GetLockPoint() const override {
        // todo: 需结合屏幕裁剪范围来算, 屏幕尺寸函数自行获取?
        return lockPoints->mainLockPoint;
    }
};

// todo: AnimSpine, AnimFrames, AnimC3b ... 补充实现 Update 的显示更新部分

// Lua 类动画基类, 虚函数调用到和 lua 函数绑定的 std::function
struct AnimLua : AnimBase {
    lua_State *L;
    std::string scriptName;
    // AnimXxxx 容器? lua 可能创建 n 个 并控制它们. 通常创建 1 个

    // 加载脚本并映射函数
    AnimLua(lua_State *const &L, std::string scriptName) : L(L), scriptName(std::move(scriptName)) {
        // todo: load script, bind func
    };

    std::function<void(std::string const &actionName)> onSetAction;

    void SetAction(std::string const &actionName) override {
        onSetAction(actionName);
    }

    // more
};


int main() {
    xx::Lua::State L;
    if (auto r = XL::Try(L, [&] {
        int x = 0;
        XL::SetGlobal(L, "beep", [&x] { return ++x; });

        auto t = std::chrono::steady_clock::now();
        auto &&Show = [&] {
            std::cout << "x = " << x << " ms = " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count() << std::endl;
            t = std::chrono::steady_clock::now();
        };

        luaL_dostring(L, R"(
for i = 1, 1000000 do
    beep()
end
)");
        Show();

        std::function<int()> f;
        XL::GetGlobal(L, "beep", f);
        for (int i = 0; i < 1000000; ++i) {
            f();
        }
        Show();

    })) {
        std::cout << "error! " << r.m << std::endl;
    }
}



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