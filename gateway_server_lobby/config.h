#pragma once
#include <ajson.hpp>
#include <iostream>

// 用类结构映射到 json 格式:

struct Config {
    int serverId = 0;						// 服务id, 各个服务之间不可重复. 可以和 gatewayId 重复
    int listenPort = 0;						// 监听端口
};
AJSON(Config, serverId, listenPort);


// 适配 std::cout
inline std::ostream& operator<<(std::ostream& o, Config const& c) {
    ajson::save_to(o, c);
    return o;
}

// 全局单例, 便于访问
inline Config config;
