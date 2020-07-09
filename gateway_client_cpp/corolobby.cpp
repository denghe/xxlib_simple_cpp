#include "corolobby.h"
#include "client.h"
#include "dialer.h"
#include "peer.h"
#include "config.h"

int CoroLobby::Update() {
    COR_BEGIN {
            LabBegin:
            // todo: clean up 或 跳到相应位置继续

            // 判断 lobby 协程是否可继续往下跑
            while (!c.peer || c.lobbyServerId == -1) {
                COR_YIELD
            }
            std::cout << "CoroLobby started." << std::endl;

            // 简单模拟 send request
            // 开始发包. 格式: serial, cmd, args...
            c.peer->SendTo(c.lobbyServerId, -2, "info");

            std::cout << "wait info response..." << std::endl;
            // 带 5 秒超时 等回包
            nowSecs = xx::NowSteadyEpochSeconds();
            while (xx::NowSteadyEpochSeconds() - nowSecs < 5) {
                COR_YIELD

                // 如果断线, 回到开始等待就绪
                if (!c.peer) goto LabBegin;

                // 如果有收到包，就开始处理
                {
                    auto &&pkgs = c.serverPackages[c.lobbyServerId];
                    if (!pkgs.empty()) {
                        // 定位到最前面一条 将数据挪出来
                        auto pkg = std::move(pkgs.front());
                        // 弹出最前面一条
                        pkgs.pop_front();

                        // 此处不应该收到别的数据. serial 对不上: 断线重拨
                        if (pkg.serial != 2) {
                            std::cout << "wrong package serial. expect 2. pkg.serial = " << pkg.serial << std::endl;
                            goto LabBegin;
                        }

                        // 开始读出 Data 内容
                        std::string txt;

                        // 如果解包错误: 重新拨号
                        xx::DataReader dr(pkg.data);
                        if (int rtv = dr.Read(txt)) {
                            std::cout << "Read cmd from d error. rtv = " << rtv << std::endl;
                            goto LabBegin;
                        }

                        // 输出 txt 内容
                        std::cout << "receive txt: " << txt << std::endl;

                        goto LabHandlePackages;
                    }
                }
            }

            // 等回包超时: 重连
            std::cout << "wait info response timeout" << std::endl;
            c.peer->Close(__LINE__);
            goto LabBegin;

            LabHandlePackages:
            std::cout << "begin handle lobby's packages..." << std::endl;
            //  进入包处理模式
            while (c.peer) {
                COR_YIELD

                // 如果断线, 回到开头
                if (!c.peer) goto LabBegin;

                // 如果有收到包，就开始处理
                {
                    auto &&pkgs = c.serverPackages[c.lobbyServerId];
                    while (!pkgs.empty()) {
                        // 定位到最前面一条 移动到栈上来
                        auto pkg = std::move(pkgs.front());
                        // 弹出最前面一条
                        c.peer->receivedPackages.pop_front();

                        // 模拟处理 pkg
                        std::string txt;
                        xx::DataReader dr(pkg.data);
                        if (int rtv = dr.Read(txt)) {
                            std::cout << "recv pkg from lobby. read error rtv = " << rtv << std::endl;
                        } else {
                            std::cout << "recv pkg from lobby. serverId = " << pkg.serverId << ", serial = "
                                      << pkg.serial << ", txt = " << txt << std::endl;
                        }
                    }
                }
            }

            // 断线了. 回到开头
            goto LabBegin;
        };
    COR_END
}
