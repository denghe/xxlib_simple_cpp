#include "coromain.h"
#include "corolobby.h"
#include "client.h"
#include "dialer.h"
#include "peer.h"
#include "config.h"

int CoroMain::Update() {
    COR_BEGIN {
            // 初始化拨号器
            c.dialer = c.CreateTcpDialer<Dialer>();

            // 添加拨号地址
            for (auto &&addr : config.addrs) {
                c.dialer->AddAddress(addr.ip, addr.port);
            }
            std::cout << "inited." << std::endl;

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


            std::cout << "dial..." << std::endl;
            // 开始拨号( 5秒超时 ) // todo: 域名解析, 包协议版本比对
            r = c.dialer->DialSeconds(5);

            // 如果立刻发生错误: 重新拨号
            if (r) {
                std::cout << "dial failed. r = " << r << std::endl;
                goto LabDial;
            }

            // 等拨号器变得不忙
            while (c.dialer->Busy()) {
                COR_YIELD
            }

            // 如果 peer 为空: 表示没有连上, 或者刚连上就被掐线. 重新拨号
            if (!c.peer) {
                std::cout << "dial timeout or peer disconnected." << std::endl;
                goto LabDial;
            }

            std::cout << "wait 0 open..." << std::endl;
            // 连上了：带 5 秒超时 等 0 号服务 open
            nowSecs = xx::NowSteadyEpochSeconds();
            while (xx::NowSteadyEpochSeconds() - nowSecs < 5) {
                COR_YIELD

                // 如果断线, 重新拨号
                if (!c.peer) {
                    std::cout << "peer disconnected." << std::endl;
                    goto LabDial;
                }

                // 如果检测到 0 号服务已 open 就跳出循环
                if (c.peer->IsOpened(0)) goto LabAuth;
            }

            // 等 0 号服务 open 超时: 重连
            std::cout << "wait 0 open timeout" << std::endl;
            goto LabDial;

            LabAuth:
            // 简单模拟 send request
            // 开始发包. 格式: serial, cmd, args...
            c.peer->SendTo(0, -1, "auth");

            std::cout << "wait auth response..." << std::endl;
            // 带 5 秒超时 等回包
            nowSecs = xx::NowSteadyEpochSeconds();
            while (xx::NowSteadyEpochSeconds() - nowSecs < 5) {
                COR_YIELD

                // 如果断线, 重新拨号
                if (!c.peer) {
                    std::cout << "peer disconnected." << std::endl;
                    goto LabDial;
                }

                {
                    auto &&pkgs = c.peer->receivedPackages;
                    // 如果有收到包，就开始处理
                    if (!pkgs.empty()) {
                        // 定位到最前面一条
                        auto &&pkg = pkgs.front();
                        // 弹出最前面一条
                        pkgs.pop_front();

                        // 此处不应该收到来自 非0 serverId 的数据. 收到则 断线重拨. 别处可将消息分发到具体逻辑
                        if (pkg.serverId != 0) {
                            std::cout << "wrong package serverId. expect 0. pkg.serverId = " << pkg.serverId
                                      << std::endl;
                            goto LabDial;
                        }

                        // 此处不应该收到别的数据. serial 对不上: 断线重拨
                        if (pkg.serial != 1) {
                            std::cout << "wrong package serial. expect 1. pkg.serial = " << pkg.serial << std::endl;
                            goto LabDial;
                        }

                        xx::DataReader dr(pkg.data);
                        // 开始读出 Data 内容
                        std::string cmd;

                        // 如果解包错误: 重新拨号
                        if (int rtv = dr.Read(cmd)) {
                            std::cout << "Read cmd from d error. rtv = " << rtv << std::endl;
                            goto LabDial;
                        }

                        // 进一步分析回包内容
                        if (cmd == "lobby") {
                            // 如果是进大厅：进一步取出目标 serverId 保存下来
                            if (int rtv = dr.Read(c.lobbyServerId)) {
                                std::cout << "Read lobbyServerId from d error. rtv = " << rtv << std::endl;
                                goto LabDial;
                            }
                            if (!c.ExistsCoroName("lobby")) {
                                std::cout << "load lobby coro..." << std::endl;
                                // 载入 lobby 功能协程. 协程内部扫自己的消息队列并处理
                                c.CreateCoro<CoroLobby>();
                            }
                            goto LabDispatch;

                        } else if (cmd == "game") {
                            // 如果是进游戏：进一步取出目标 serverId 保存下来
                            if (int rtv = dr.Read(c.gameServerId)) {
                                std::cout << "Read gameServerId from d error. rtv = " << rtv << std::endl;
                                goto LabDial;
                            }
                            std::cout << "load game coro..." << std::endl;
                            if (!c.ExistsCoroName("game")) {
                                std::cout << "load game coro..." << std::endl;
                                // 载入 game 功能协程. 协程内部扫自己的消息队列并处理
                                //c.CreateCoro<CoroGame>();
                            }
                            goto LabDispatch;

                        } else {
                            // 不认识的包内容
                            std::cout << "receive unknown package." << std::endl;
                            // dump d ?
                            goto LabDial;
                        }
                    }
                }
            }

            // 等回包超时: 重连
            std::cout << "wait auth response timeout" << std::endl;
            goto LabDial;


            LabDispatch:
            std::cout << "begin dispatch package to server's coro..." << std::endl;
            //  进入数据分发模式. 具体处理代码在别的协程. 这里只是将包挪到相应容器
            while (c.peer) {
                COR_YIELD

                // 如果断线, 重新拨号
                if (!c.peer) goto LabDial;

                // 如果有收到包，就开始处理
                while (!c.peer->receivedPackages.empty()) {
                    // 定位到最前面一条
                    auto &&pkg = c.peer->receivedPackages.front();

                    // 定位到相应服务的队列
                    auto &&dq = c.serverPackages[pkg.serverId];

                    // 将数据挪到服务队列
                    dq.emplace_back(std::move(pkg));

                    // 弹出最前面一条
                    c.peer->receivedPackages.pop_front();
                }
            }

            // 断线了. 自动重连? ( 可能是别的协程或事件代码造成 c.peer 断开, 或者是被服务器断开 )
            goto LabDial;
        };
    COR_END
}
