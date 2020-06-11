#include "server.h"
#include "gpeer.h"
#include "vpeer.h"
#include "config.h"

GPeer::~GPeer() {
    // 如果是因 server 析构导致执行到此, 则 server 派生层 成员 已析构, 不可访问. 短路退出
    if (!GetServer().running) return;

    // 从容器移除( 如果有放入的话 )
    if (id) {
        GetServer().gps.erase(id);
    }
}

void GPeer::OnReceivePackage(char *const &buf, size_t const &len) {
    // 引用到 server 备用
    auto &&s = GetServer();

    // 创建一个数据读取器
    xx::DataReader dr(buf, len);

    // 如果数据长度不足, 直接退出
    uint32_t clientId = 0;

    // 进一步解析. 看是内部指令还是普通包
    // 读出投递地址
    if (int r = dr.ReadFixed(clientId)) {
        OnDisconnect(__LINE__);
        Dispose();
        return;
    }

    // 内部指令, 直接处理
    if (clientId == 0xFFFFFFFFu) {
        // 指令名
        std::string cmd;
        // 试读取 cmd 字串. 失败直接断开
        if (int r = dr.ReadLimit<64>(cmd)) {
            OnDisconnect(__LINE__);
            Dispose();
            return;
        }
        if (cmd == "accept") {
            // 指令参数
            std::string ip;

            // 试读出 指令参数. 失败直接断开
            if (int r = dr.Read(clientId, ip)) {
                OnDisconnect(__LINE__);
                Dispose();
                return;
            }

            std::cout << "recv accept. ip = "<< ip <<", clientId = " << clientId << std::endl;

            // 只有执行事件太久, gateway 的 自增 clientId 用尽，到最后和最初建立的重复了才有可能
            if (vpeers.find(clientId) != vpeers.end()) return;

            // 创建虚拟 peer( 会自动下发 open )
            auto vp = ep->AddItem(xx::MakeU<VPeer>(this, clientId));

            // 放入容器
            vpeers[clientId] = vp;

        } else if (cmd == "disconnect") {
            // 试读出 指令参数. 失败直接断开
            if (int r = dr.Read(clientId)) {
                OnDisconnect(__LINE__);
                Dispose();
                return;
            }

            std::cout << "recv disconnect. clientId = " << clientId << std::endl;

            // 定位到虚拟 peer
            auto &&iter = vpeers.find(clientId);
            // 如果没找到就退出( 有可能刚刚被逻辑代码杀掉, close 指令还在路上, 此时 gateway 并不知道 )
            if (iter == vpeers.end()) return;
            // 杀掉 vp( 会自动下发 close 并从容器移除, 这之后 iter 会失效 )
            iter->second->Disconnect(__LINE__);
        }
    } else {
        std::cout << "recv package. clientId = " << clientId << std::endl;

        // 一般数据: 转发给 虚拟 peer
        // 定位到 虚拟 peer
        auto &&iter = vpeers.find(clientId);
        // 如果没找到就退出( 有可能刚刚被逻辑代码杀掉, close 指令还在路上, 此时 gateway 并不知道 )
        if (iter == vpeers.end()) return;
        // 转发给 vp
        iter->second->OnReceive(buf + sizeof(clientId), len - sizeof(clientId));
    }
}

void GPeer::OnReceiveFirstPackage(char *const &buf, size_t const &len) {
    // 解析首包. 内容应该由 string + uint 组成
    uint32_t addr = 0;
    std::string cmd;
    uint32_t gatewayId = 0;
    xx::DataReader dr(buf, len);

    // 读出投递地址
    if (int r = dr.ReadFixed(addr)) {
        OnDisconnect(__LINE__);
        Dispose();
        return;
    }

    // 如果不是内部指令: 断线退出
    if (addr != 0xFFFFFFFFu) {
        OnDisconnect(__LINE__);
        Dispose();
        return;
    }

    // 读取指令失败: 断线退出
    if (dr.Read(cmd, gatewayId)) {
        OnDisconnect(__LINE__);
        Dispose();
        return;
    }

    // 前置检查失败: 断线退出
    if (cmd != "gatewayId" || !gatewayId) {
        OnDisconnect(__LINE__);
        Dispose();
        return;
    }

    // for easy use
    auto &&gps = GetServer().gps;

    // 如果 gatewayId 已存在: 断线退出
    if (gps.find(gatewayId) != gps.end()) {
        OnDisconnect(__LINE__);
        Dispose();
        return;
    }

    // 放入相应容器( 建立 gatewayId 到 gpeer 的映射 )
    gps[gatewayId] = this;
    id = gatewayId;

    // 设置不超时
    SetTimeoutSeconds(0);
}

void GPeer::Disconnect(int const &reason) {
    // 触发断线事件
    OnDisconnect(reason);
    // 将容器挪到栈内存 以便在 Dispose() 后访问
    auto vps = std::move(vpeers);
    // 自杀
    Dispose();
    // 干掉所有虚拟 peer
    for (auto &&iter : vps) {
        if (auto &&p = iter.second.Lock()) {
            // OnDisconnect + Dispose + callbacks timeout
            p->Disconnect(reason);
        }
    }
}
