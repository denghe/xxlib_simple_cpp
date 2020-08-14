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
    uint64_t pingCount = 0;
    uint64_t pingSum = 0;
    uint64_t minPing = 10000;
    uint64_t maxPing = 0;
    uint64_t goodCount = 0;
    uint64_t lagCount250 = 0;
    uint64_t lagCount500 = 0;
    uint64_t lagCount1000 = 0;
    uint64_t lagCount1500 = 0;
    uint64_t lagCount2000 = 0;
    uint64_t lagCount2500 = 0;
    uint64_t lagCount3000 = 0;
    uint64_t lagCount5000 = 0;
    uint64_t lagCount7000 = 0;
    uint64_t dialCount = 0;

    [[nodiscard]] std::string ToString() const {
        return xx::ToString("runSeconds: ", totalRunSeconds, ", dialCount: ", dialCount, ", ping avg = ", (pingSum / pingCount + 1), ", min = ", minPing, ", max = ", maxPing,
                            ", <250 = ", goodCount, ", 250 = ", lagCount250, ", 500 = ", lagCount500, ", 1000 = ", lagCount1000, ", 1500 = ", lagCount1500, ", 2000 = ",
                            lagCount2000, ", 2500 = ", lagCount2500, ", 3000 = ", lagCount3000, ", 5000 = ", lagCount5000, ", 7000+ = ", lagCount7000);
    }

    void Clear() {
        memset(this, 0, sizeof(*this));
        minPing = 10000;
    }

    void Calc(int64_t const& ms) {
        if (ms > maxPing) maxPing = ms;
        if (ms < minPing) minPing = ms;
        pingSum += ms;
        if (ms <= 250) goodCount++;
        if (ms > 250) lagCount250++;
        if (ms > 500) lagCount500++;
        if (ms > 1000) lagCount1000++;
        if (ms > 1500) lagCount1500++;
        if (ms > 2000) lagCount2000++;
        if (ms > 2500) lagCount2500++;
        if (ms > 3000) lagCount3000++;
        if (ms > 5000) lagCount5000++;
        if (ms > 7000) lagCount7000++;
        pingCount++;
    }
};

NetInfo tni, kni;

struct KcpPeer : EP::KcpPeer {
    using BT = EP::KcpPeer;
    using BT::BT;
    std::string logPrefix = "KCP: ";
    NetInfo *ni = &kni;

    // 预填好的每次发的包
    xx::Data pkg;

    // 发包时间（拿来算ping）
    std::chrono::steady_clock::time_point lastSendTP;

    // Accept 事件中调用下以初始化一些东西
    inline void Init() {
        // 在 pkg 中填充一个合法的 内部指令包. 网关收到后会立刻 echo 回来并续命
        xx::DataWriter dw(pkg);
        pkg.Resize(4);
        dw.WriteFixed((uint32_t) 0xFFFFFFFF);
        dw.WriteBuf("12345678901234567890123456789012345678901234567890", 50);
        *(uint32_t *) pkg.buf = pkg.len - sizeof(uint32_t);

        // 加持
        Hold();

        // 记录日志
        LOG_SIMPLE(logPrefix, "connected. wait open...");

        // 设置超时时间
        SetTimeoutSeconds(10);

        // 开始等 open
        lastSendTP = std::chrono::steady_clock::now();

        // 统计
        ni->dialCount++;
    }

    inline void SendPing() {
        this->BT::Send(pkg.buf, pkg.len);
        Flush();
    }

    inline void Receive() override {
        // 取出指针备用
        auto buf = recv.buf;
        auto end = recv.buf + recv.len;
        uint32_t dataLen = 0;
        uint32_t serverId = 0;

        // 确保包头长度充足
        while (buf + sizeof(dataLen) <= end) {
            // 取长度
            dataLen = *(uint32_t *) buf;

            // 长度异常则断线退出( 不含地址? 超长? 256k 不够可以改长 )
            if (dataLen > 1024 * 256) {
                Close(-21, __LINESTR__" Peer Receive if (dataLen < sizeof(addr) || dataLen > 1024 * 256)");
                return;
            }

            // 数据未接收完 就 跳出
            if (buf + sizeof(dataLen) + dataLen > end) break;

            // 跳到数据区开始调用处理回调
            buf += sizeof(dataLen);
            {
                // 取出地址( 手机上最好用 memcpy )
                serverId = *(uint32_t *) buf;

                // 包类型判断
                if (serverId == 0xFFFFFFFFu) {
                    // 内部指令
                    ReceiveCommand(buf + sizeof(serverId), dataLen - sizeof(serverId));
                } else {
                    // 普通包. serverId 打头
                    ReceivePackage(serverId, buf + sizeof(serverId), dataLen - sizeof(serverId));
                }

                // 如果当前类实例已自杀则退出
                if (!Alive()) return;
            }

            // 跳到下一个包的开头
            buf += dataLen;
        }

        // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
        recv.RemoveFront(buf - recv.buf);
    }

    void ReceiveCommand(char *const &buf, size_t const &len) {
        // 算 ping, 日志, 续命, 再发
        auto &&now = std::chrono::steady_clock::now();
        auto &&ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSendTP).count();
        LOG_SIMPLE(logPrefix, "ping = ", ms);
        lastSendTP = now;
        SetTimeoutSeconds(10);
        SendPing();

        // 统计
        ni->Calc(ms);
    }

    inline void ReceivePackage(uint32_t const &serverId, char *const &buf, size_t const &len) {
        // 忽略
    }

    inline bool Close(int const &reason, char const *const &desc) override {
        if (this->BT::Close(reason, desc)) {
            LOG_SIMPLE(logPrefix, "Close reason = ", reason, ", desc = ", desc);
            DelayUnhold();
            return true;
        }
        return false;
    }
};

struct KcpDialer : EP::KcpDialer<KcpPeer> {
    using EP::KcpDialer<KcpPeer>::KcpDialer;

    inline void Connect(std::shared_ptr<KcpPeer> const &peer) override {
        if (!peer) {
            LOG_ERROR("KcpDialer Dial Timeout.");
            return; // 没连上
        }
        peer->Init();
    }
};

/*********************************************************************************************************/
// TCP 和 KCP 代码几乎相同
/*********************************************************************************************************/


struct TcpPeer : EP::TcpPeer {
    using BT = EP::TcpPeer;
    using BT::BT;
    std::string logPrefix = "TCP: ";
    NetInfo *ni = &tni;

    // 预填好的每次发的包
    xx::Data pkg;

    // 发包时间（拿来算ping）
    std::chrono::steady_clock::time_point lastSendTP;

    // Accept 事件中调用下以初始化一些东西
    inline void Init() {
        // 在 pkg 中填充一个合法的 内部指令包. 网关收到后会立刻 echo 回来并续命
        xx::DataWriter dw(pkg);
        pkg.Resize(4);
        dw.WriteFixed((uint32_t) 0xFFFFFFFF);
        dw.WriteBuf("12345678901234567890123456789012345678901234567890", 50);
        *(uint32_t *) pkg.buf = pkg.len - sizeof(uint32_t);

        // 加持
        Hold();

        // 记录日志
        LOG_SIMPLE(logPrefix, "connected. wait open...");

        // 设置超时时间
        SetTimeoutSeconds(10);

        // 开始等 open
        lastSendTP = std::chrono::steady_clock::now();

        // 统计
        ni->dialCount++;
    }

    inline void SendPing() {
        this->BT::Send(pkg.buf, pkg.len);
        Flush();
    }

    inline void Receive() override {
        // 取出指针备用
        auto buf = recv.buf;
        auto end = recv.buf + recv.len;
        uint32_t dataLen = 0;
        uint32_t serverId = 0;

        // 确保包头长度充足
        while (buf + sizeof(dataLen) <= end) {
            // 取长度
            dataLen = *(uint32_t *) buf;

            // 长度异常则断线退出( 不含地址? 超长? 256k 不够可以改长 )
            if (dataLen > 1024 * 256) {
                Close(-21, __LINESTR__" Peer Receive if (dataLen < sizeof(addr) || dataLen > 1024 * 256)");
                return;
            }

            // 数据未接收完 就 跳出
            if (buf + sizeof(dataLen) + dataLen > end) break;

            // 跳到数据区开始调用处理回调
            buf += sizeof(dataLen);
            {
                // 取出地址( 手机上最好用 memcpy )
                serverId = *(uint32_t *) buf;

                // 包类型判断
                if (serverId == 0xFFFFFFFFu) {
                    // 内部指令
                    ReceiveCommand(buf + sizeof(serverId), dataLen - sizeof(serverId));
                } else {
                    // 普通包. serverId 打头
                    ReceivePackage(serverId, buf + sizeof(serverId), dataLen - sizeof(serverId));
                }

                // 如果当前类实例已自杀则退出
                if (!Alive()) return;
            }

            // 跳到下一个包的开头
            buf += dataLen;
        }

        // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
        recv.RemoveFront(buf - recv.buf);
    }

    void ReceiveCommand(char *const &buf, size_t const &len) {
        // 算 ping, 日志, 续命, 再发
        auto &&now = std::chrono::steady_clock::now();
        auto &&ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSendTP).count();
        LOG_SIMPLE(logPrefix, "ping = ", ms);
        lastSendTP = now;
        SetTimeoutSeconds(10);
        SendPing();

        // 统计
        ni->Calc(ms);
    }

    inline void ReceivePackage(uint32_t const &serverId, char *const &buf, size_t const &len) {
        // 忽略
    }

    inline bool Close(int const &reason, char const *const &desc) override {
        if (this->BT::Close(reason, desc)) {
            LOG_SIMPLE(logPrefix, "Close reason = ", reason, ", desc = ", desc);
            DelayUnhold();
            return true;
        }
        return false;
    }
};

struct TcpDialer : EP::TcpDialer<TcpPeer> {
    using EP::TcpDialer<TcpPeer>::TcpDialer;

    // 指向当前连接
    std::weak_ptr<TcpPeer> currentPeer;

    inline void Connect(std::shared_ptr<TcpPeer> const &peer) override {
        if (!peer) {
            LOG_SIMPLE("TcpDialer Dial Timeout.");
            return; // 没连上
        }

        currentPeer = peer;
        peer->Init();
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
            if (!tcpDialer->Busy() && !tcpDialer->currentPeer.lock()) {
                LOG_SIMPLE("TcpDialer dial begin.");
                tcpDialer->DialSeconds(5);
            }

            if (!kcpDialer->Busy() && kcpDialer->cps.size() < 1) {
                LOG_SIMPLE("KcpDialer dial begin.");
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
        ofs << "start time: " << xx::ToString(xx::Now()) << std::endl;
        ofs.flush();
        while (running) {
            // 每小时结存一次
            for (int i = 0; i < 60; ++i) {
                // 每分钟输出一次
                std::this_thread::sleep_for(std::chrono::seconds(60));
                tni.totalRunSeconds += 60;
                LOG_SIMPLE("TCP: ", tni.ToString());
                kni.totalRunSeconds += 60;
                LOG_SIMPLE("KCP: ", kni.ToString());
            }
            ofs << "TCP: " << tni.ToString() << std::endl;
            ofs << "KCP: " << kni.ToString() << std::endl;
            ofs.flush();
            memset(&tni, 0, sizeof(tni));
            tni.minPing = 10000;
            memset(&kni, 0, sizeof(kni));
            kni.minPing = 10000;
        }
    });

    // 运行
    auto &&r = xx::Make<Client>()->Run();
    running = false;
    t.join();
    return r;
}
