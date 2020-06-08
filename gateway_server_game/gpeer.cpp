#include "server.h"
#include "gpeer.h"

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
    uint32_t addr = 0;

    // 进一步解析. 看是内部指令还是普通包
    do {
        // 读出投递地址
        if (int r = dr.ReadFixed(addr)) break;

        // 内部指令, 直接处理
        if (addr == 0xFFFFFFFFu) {
            // 指令名
            std::string cmd;
            // 试读取 cmd 字串. 失败直接断开
            if (int r = dr.ReadLimit<64>(cmd)) break;
            if (cmd == "accept") {
                // 指令参数
                uint32_t clientId = 0;
                std::string ip;

                // 试读出 指令参数. 失败直接断开
                if (int r = dr.Read(clientId, ip)) break;

                // todo: 创建虚拟 peer
                return;
            } else if (cmd == "disconnect") {
                // 指令参数
                uint32_t clientId = 0;

                // 试读出 指令参数. 失败直接断开
                if (int r = dr.Read(clientId)) break;

                // todo: 干掉虚拟 peer
                return;
            }
        } else {
            // 一般数据: 转发给 虚拟 peer
            // todo
        }
    } while (0);

    // 触发断线事件并自杀
    OnDisconnect(__LINE__);
    Dispose();
}

void GPeer::OnReceiveFirstPackage(char *const &buf, size_t const &len) {
    // 解析首包. 内容应该由 string + uint 组成
    uint32_t addr = 0;
    std::string cmd;
    uint32_t gatewayId = 0;
    xx::DataReader dr(buf, len);

    do {
        // 读出投递地址
        if (int r = dr.ReadFixed(addr)) break;

        // 如果不是内部指令: 断线退出
        if (addr != 0xFFFFFFFFu) break;

        // 读取指令失败: 断线退出
        if (dr.Read(cmd, gatewayId)) break;

        // 前置检查失败: 断线退出
        if (cmd != "gatewayId" || !gatewayId) break;

        // for easy use
        auto &&gps = GetServer().gps;

        // 如果 serverId 已存在: 断线退出
        if (gps.find(gatewayId) != gps.end()) break;

        // 放入相应容器( 建立 gatewayId 到 gpeer 的映射 )
        gps[gatewayId] = this;
        id = gatewayId;

        // 设置不超时
        SetTimeoutSeconds(0);

        return;
    } while (0);

    // 触发断线事件并自杀
    OnDisconnect(__LINE__);
    Dispose();
}
