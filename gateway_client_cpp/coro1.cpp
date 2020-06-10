#include "coro1.h"
#include "client.h"
#include "dialer.h"
#include "peer.h"
#include "config.h"

int Coro1::Update() {
    COR_BEGIN {
            // 初始化拨号器
            c.dialer = c.CreateTcpDialer<Dialer>();

            // 添加拨号地址
            for (auto &&addr : config.addrs) {
                c.dialer->AddAddress(addr.ip, addr.port);
            }

            LabDial:
        COR_YIELD
            // clean up: 停掉拨号器( 保险起见 ), 杀掉 peer( 方便自杀 )
            c.dialer->Stop();
            if (c.peer) {
                c.peer->Dispose();
            }

            // 等 1 秒( 必须的. 除了防止低延迟疯狂拨号以外, 还可以令别的协程有机会处理断线逻辑 )
            nowSecs = xx::NowSteadyEpochSeconds();
            while (xx::NowSteadyEpochSeconds() - nowSecs < 1) {
                COR_YIELD
            }

            // 还原各种状态变量初始值( 保险起见 )
            c.serverPackages.clear();
            c.lobbyServerId = -1;
            c.gameServerId = -1;


            // 开始拨号( 5秒超时 ) // todo: 域名解析, 包协议版本比对
            r = c.dialer->DialSeconds(5);

            // 如果立刻发生错误: 重新拨号
            if (r) goto LabDial;

            // 等拨号器变得不忙
            while (c.dialer->Busy()) {
                COR_YIELD
            }

            // 如果 peer 为空: 表示没有连上. 重新拨号
            if (!c.peer) goto LabDial;

            // 连上了：带 5 秒超时 等 0 号服务 open
            nowSecs = xx::NowSteadyEpochSeconds();
            while (xx::NowSteadyEpochSeconds() - nowSecs < 5) {
                COR_YIELD

                // 如果断线, 重新拨号
                if (!c.peer) goto LabDial;

                // 如果检测到 0 号服务已 open 就跳出循环
                if (c.peer->IsOpened(0)) break;
            }

            // 简单模拟 send request
            // 开始发包. 格式: serial, cmd, args...
            c.peer->SendTo(0, 1, "auth", "username", "password");

            // 带 5 秒超时 等回包
            nowSecs = xx::NowSteadyEpochSeconds();
            while (xx::NowSteadyEpochSeconds() - nowSecs < 5) {
                COR_YIELD

                // 如果断线, 重新拨号
                if (!c.peer) goto LabDial;

                // 如果有收到包，就开始处理
                if (!c.peer->receivedPackages.empty()) {
                    // 定位到最前面一条
                    auto &&pkg = c.peer->receivedPackages.front();

                    // 此处不应该收到来自 非0 serverId 的数据. 收到则 断线重拨. 别处可将消息分发到具体逻辑
                    if (pkg.serverId != 0) {
                        // todo: log logic error
                        goto LabDial;
                    }

                    // 此处不应该收到别的数据. serial 对不上: 断线重拨
                    if (pkg.serial != 1) {
                        // todo: log logic error
                        goto LabDial;
                    }

                    // 将数据挪出来
                    auto &&d = std::move(pkg.data);

                    // 弹出最前面一条
                    c.peer->receivedPackages.pop_front();



                    // 开始读出 Data 内容
                    std::string cmd;

                    // 如果解包错误: 重新拨号
                    if (xx::Read(d, cmd)) {
                        // todo: log logic error
                        goto LabDial;
                    }

                    // 进一步分析回包内容
                    if (cmd == "lobby") {
                        // 如果是进大厅：进一步取出目标 serverId 保存下来
                        if(xx::Read(d, c.lobbyServerId)) {
                            // todo: log logic error
                            goto LabDial;
                        }
                        goto LabLobby;

                    } else if (cmd == "game") {
                        // 如果是进游戏：进一步取出目标 serverId 保存下来
                        if(xx::Read(d, c.gameServerId)) {
                            // todo: log logic error
                            goto LabDial;
                        }
                        goto LabGame;
                    }
                    else {
                        // 不认识的包内容
                        // todo: log logic error
                        goto LabDial;
                    }
                }
            }

            LabLobby:;
            // todo: 载入或激活 lobby 功能协程. 协程内部扫自己的消息队列并处理

            LabGame:;
            // todo: 载入或激活 game 功能协程. 协程内部扫自己的消息队列并处理

            //  进入数据分发模式
            while (c.peer) {
                COR_YIELD

                // 如果断线, 重新拨号
                if (!c.peer) goto LabDial;

                // 如果有收到包，就开始处理
                while (!c.peer->receivedPackages.empty()) {
                    // 定位到最前面一条
                    auto &&pkg = c.peer->receivedPackages.front();

                    // 定位到相应服务的队列
                    auto&& dq = c.serverPackages[pkg.serverId];

                    // 将数据挪到服务队列
                    dq.emplace_back(std::move(pkg));

                    // 弹出最前面一条
                    c.peer->receivedPackages.pop_front();
                }
            }

            // todo: 根据类型分发. 如果 serial == 0 即 push 性质, 那就临时 call 相应函数处理后返回此处
        };
    COR_END
}
