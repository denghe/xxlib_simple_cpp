#pragma once
#include <ajson.hpp>
#include <iostream>

/*
用类结构映射到 json 格式:
{
  "dialAddrs": [{"ip":"xx.xx.xx.xx","port":123,"protocol":"tcp"},{"ip":"xx.xx.xx.xx","port":123,"protocol":"kcp"}]
}
*/
struct DialAddr {
    std::string ip;
    int port;
    std::string protocol;
};
AJSON(DialAddr, ip, port, protocol);
struct Config {
	std::vector<DialAddr> dialAddrs;
};
AJSON(Config, dialAddrs);

// 适配 std::cout
inline std::ostream& operator<<(std::ostream& o, Config const& c) {
    ajson::save_to(o, c);
    return o;
}

// 全局单例, 便于访问
inline Config config;
