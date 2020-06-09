#pragma once
#include <ajson.hpp>
#include <iostream>

// 用类结构映射到 json 格式:

struct IpPort {
    std::string ip;
    uint32_t port = 0;
    // protocol?
};
AJSON(IpPort, ip, port);

struct Config {
    std::vector<IpPort> addrs;	    // 拨号地址表
};
AJSON(Config, addrs);


// 适配 std::cout
inline std::ostream& operator<<(std::ostream& o, Config const& c) {
    ajson::save_to(o, c);
    return o;
}

// 全局单例, 便于访问
inline Config config;
