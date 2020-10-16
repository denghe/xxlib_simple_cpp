#include "xx_signal.h"
#include "server.h"

int main() {
    // 禁掉 SIGPIPE 信号避免因为连接关闭出错
    xx::IgnoreSignal();

    // 创建类实例. 轮长 = 最大超时秒数 * 帧数 的 2^n 对齐. kcp 需要每秒 100 帧
    auto &&s = xx::Make<Server>(32768, 100);

    // 开始运行
    return s->Run();
}




// 测试 lua 的 coroutine 效率。结论是比 c++ 的慢 180 倍( 或许更多, 毕竟代码不对等 )
//
//#include "xx_lua.h"
//
//int main() {
//    xx::Lua::State L;
//    xx::Lua::DoString(L, R"(
//
//coroutine_create = coroutine.create
//coroutine_status = coroutine.status
//resume = coroutine.resume
//yield = coroutine.yield
//table_unpack = table.unpack
//
//cs = {}
//cs.Add = function(o)
//    cs[#cs + 1] = o
//end
//cs.Go = function(func, ...)
//	local args = {...}
//	local co = nil
//	if #args == 0 then
//		co = coroutine_create(func)
//	else
//		co = coroutine_create(function() func(table_unpack(args)) end)
//	end
//	cs.Add(co)
//	return co
//end
//cs.SwapRemoveAt = function(idx)
//    local dl = #cs
//    assert(idx > 0 and idx <= dl)
//    if idx < dl then
//        cs[idx] = cs[dl]
//    end
//    cs[dl] = nil
//end
//cs.Update = function()
//    if #cs > 0 then
//		for i = #cs, 1, -1 do
//			local co = cs[i]
//			local ok, msg = resume(co)
//			--if not ok then
//				--print(msg)
//			--end
//			if coroutine_status(co) == "dead" then
//				cs.SwapRemoveAt(i)
//			end
//		end
//	end
//end
//
//count = 0
//
//function Yield2()
//    yield()
//    count = count + 1
//end
//
//function Yield()
//    Yield2()
//end
//
//function Delay()
//    while (true)
//    do
//        Yield();
//    end
//end
//
//cs.Go(Delay)
//)");
//    auto secs = xx::NowSteadyEpochSeconds();
//    xx::Lua::DoString(L, R"(
//for i = 1, 1000000 do
//    cs.Update()
//end
//print(count)
//)");
//    std::cout << xx::NowSteadyEpochSeconds() - secs << std::endl;
//    return 0;
