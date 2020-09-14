#include "xx_epoll_kcp.h"
#include "config.h"
#include "xx_data_rw.h"
#include "xx_logger.h"
#include "xx_signal.h"
#include "xx_chrono.h"

/*
    通过 tcp & kcp 连接到网关，以类似游戏正常操作频率发送 相似字节数的 ping 包，测试传输稳定性。
    不停的一发一收, 超时就断线重连，并记录断线日志。
    收发本身也计算出延迟并记录日志，
*/

namespace EP = xx::Epoll;

// 汇总信息. 一分钟输出一条?
struct NetInfo {
    uint64_t totalRunSeconds = 0;
    uint64_t dialCount = 0;
    uint64_t timeoutCount = 0;
    uint64_t dialTicks = 0;

    [[nodiscard]] std::string ToString() const {
        return xx::ToString("runSeconds: ", totalRunSeconds, ", dialCount: ", dialCount, ", timeoutCount: ", timeoutCount, ", dialTicks: ", dialTicks, ", avg: ",
                            double(dialTicks / (dialCount - timeoutCount)) / 10000000.0);
    }

    void Clear() {
        totalRunSeconds = 0;
        dialCount = 0;
        dialTicks = 0;
    }
};

NetInfo tni, kni;

struct KcpPeer : EP::KcpPeer {
    using EP::KcpPeer::KcpPeer;
    inline void Receive() override {
        recv.Clear();
    }
};

struct KcpDialer : EP::KcpDialer<KcpPeer> {
    using EP::KcpDialer<KcpPeer>::KcpDialer;
    int64_t dialBeginTime = 0;

    inline void Connect(std::shared_ptr<KcpPeer> const &peer) override {
        ++kni.dialCount;
        if (!peer) {
            ++kni.timeoutCount;
            return; // 没连上
        }
        kni.dialTicks += xx::NowEpoch10m() - dialBeginTime;
    }
};

/*********************************************************************************************************/
// TCP 和 KCP 代码几乎相同
/*********************************************************************************************************/


struct TcpPeer : EP::TcpPeer {
    using EP::TcpPeer::TcpPeer;
    inline void Receive() override {
        recv.Clear();
    }
};

struct TcpDialer : EP::TcpDialer<TcpPeer> {
    using EP::TcpDialer<TcpPeer>::TcpDialer;
    int64_t dialBeginTime = 0;

    inline void Connect(std::shared_ptr<TcpPeer> const &peer) override {
        ++tni.dialCount;
        if (!peer) {
            ++tni.timeoutCount;
            return; // 没连上
        }
        tni.dialTicks += xx::NowEpoch10m() - dialBeginTime;
    }
};


struct Client : EP::Context {
    using EP::Context::Context;
    std::shared_ptr<TcpDialer> tcpDialer;
    std::shared_ptr<KcpDialer> kcpDialer;
    std::shared_ptr<EP::GenericTimer> dialTimer;

    int Run() override {
        xx::ScopeGuard sg1([&] {
            dialTimer.reset();
            kcpDialer->Stop();
            kcpDialer.reset();
            tcpDialer->Stop();
            tcpDialer.reset();
            holdItems.clear();
            assert(shared_from_this().use_count() == 2);
        });

        xx::MakeTo(tcpDialer, shared_from_this());
        for (auto &&da : config.dialAddrs) {
            if (da.protocol != "tcp") continue;
            tcpDialer->AddAddress(da.ip, da.port);
        }

        xx::MakeTo(kcpDialer, shared_from_this());
        if (int r = kcpDialer->MakeFD()) {
            throw std::runtime_error("kcpDialer->MakeFD() failed");
        } else {
            for (auto &&da : config.dialAddrs) {
                if (da.protocol != "kcp") continue;
                kcpDialer->AddAddress(da.ip, da.port);
            }
        }

        xx::MakeTo(dialTimer, shared_from_this());
        dialTimer->onTimeout = [this] {
            if (!tcpDialer->Busy()) {
                tcpDialer->dialBeginTime = xx::NowEpoch10m();
                tcpDialer->DialSeconds(5);
            }

            if (!kcpDialer->Busy()) {
                kcpDialer->dialBeginTime = xx::NowEpoch10m();
                kcpDialer->DialSeconds(5);
            }

            dialTimer->SetTimeout(1);
        };
        dialTimer->SetTimeout(1);

        SetFrameRate(100);
        return this->EP::Context::Run();
    }
};

int main(int argc, char const *argv[]) {
    // 禁掉 SIGPIPE 信号避免因为连接关闭出错
    xx::IgnoreSignal();

    // 加载配置
    ajson::load_from_file(::config, "config.json");

    // 显示配置内容
    std::cout << ::config << std::endl;

    bool running = true;

    std::thread t([&] {
        std::ofstream ofs;
        ofs.open("sum.txt", std::ios_base::app);
        if (ofs.fail()) {
            std::cerr << "ERROR!!! open sum.txt failed" << std::endl;
        }
        ofs << "begin time( UTC ): " << xx::ToString(xx::Now()) << std::endl;
        ofs.flush();
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            tni.totalRunSeconds += 10;
            LOG_SIMPLE("TCP: ", tni.ToString());
            kni.totalRunSeconds += 10;
            LOG_SIMPLE("KCP: ", kni.ToString());
            ofs << "TCP: " << tni.ToString() << std::endl;
            ofs << "KCP: " << kni.ToString() << std::endl;
            ofs.flush();
        }
    });

    // 运行
    auto &&r = xx::Make<Client>()->Run();
    running = false;
    t.join();
    return r;
}
