#include "server.h"
#include "gpeer.h"

bool GPeer::Close(int const &reason) {
    // close fd 解绑 并触发 OnDisconnect
    if (this->Peer::Close(reason)) {
        // 从容器移除( 如果有放入的话 )
        if (id) {
            GetServer().gps.erase(id);
        }
        DelayUnhold();
        return true;
    }
    return false;
}

void GPeer::ReceivePackage(char *const &buf, size_t const &len) {
    // 引用到 server 备用
    auto &&s = GetServer();

    // 创建一个数据读取器
    xx::DataReader dr(buf, len);

    // 如果数据长度不足, 直接退出
    uint32_t addr = 0;

    // 进一步解析. 看是内部指令还是普通包
    // 读出投递地址
    if (int r = dr.ReadFixed(addr)) {
        Close(__LINE__);
        return;
    }

    // 内部指令, 直接处理
    if (addr == 0xFFFFFFFFu) {
        // 指令名
        std::string cmd;
        // 试读取 cmd 字串. 失败直接断开
        if (int r = dr.ReadLimit<64>(cmd)) {
            Close(__LINE__);
            return;
        }
        if (cmd == "accept") {
            // 指令参数
            uint32_t clientId = 0;
            std::string ip;

            // 试读出 指令参数. 失败直接断开
            if (int r = dr.Read(clientId, ip)) {
                Close(__LINE__);
                return;
            }

            // todo: 创建虚拟 peer
        } else if (cmd == "disconnect") {
            // 指令参数
            uint32_t clientId = 0;

            // 试读出 指令参数. 失败直接断开
            if (int r = dr.Read(clientId)) {
                Close(__LINE__);
                return;
            }

            // todo: 干掉虚拟 peer
        }
    } else {
        // 一般数据: 转发给 虚拟 peer
        // todo
    }
}

void GPeer::ReceiveFirstPackage(char *const &buf, size_t const &len) {
    // 解析首包. 内容应该由 string + uint 组成
    uint32_t addr = 0;
    std::string cmd;
    uint32_t gatewayId = 0;
    xx::DataReader dr(buf, len);

    // 读出投递地址
    if (int r = dr.ReadFixed(addr)) {
        Close(__LINE__);
        return;
    }

    // 如果不是内部指令: 断线退出
    if (addr != 0xFFFFFFFFu) {
        Close(__LINE__);
        return;
    }

    // 读取指令失败: 断线退出
    if (dr.Read(cmd, gatewayId)) {
        Close(__LINE__);
        return;
    }

    // 前置检查失败: 断线退出
    if (cmd != "gatewayId" || !gatewayId) {
        Close(__LINE__);
        return;
    }

    // for easy use
    auto &&gps = GetServer().gps;

    // 如果 serverId 已存在: 断线退出
    if (gps.find(gatewayId) != gps.end()) {
        Close(__LINE__);
        return;
    }

    // 放入相应容器( 建立 gatewayId 到 gpeer 的映射 )
    gps[gatewayId] = xx::As<GPeer>(shared_from_this());
    id = gatewayId;

    // 设置不超时
    SetTimeoutSeconds(0);
}
