#pragma once
#include <ajson.hpp>
#include <iostream>

// 用类结构映射到 json 格式:

struct ServerInfo {
    int serverId = 0;                       // 服务id（连上之后校验用）
    std::string ip;
    int port = 0;
};
AJSON(ServerInfo, serverId, ip, port);

struct Config {
    int gatewayId = 0;						// 网关id
    int listenPort = 0;						// 监听端口
    std::vector<ServerInfo> serverInfos;	// 要连接到哪些服务
};
AJSON(Config, gatewayId, listenPort, serverInfos);


// 适配 std::cout
std::ostream& operator<<(std::ostream& o, Config const& c) {
    ajson::save_to(o, c);
    return o;
}

// 全局单例, 便于访问
inline Config config;
