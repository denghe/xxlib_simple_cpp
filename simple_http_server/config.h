#pragma once
#include <ajson.hpp>
#include <iostream>

/*
用类结构映射到 json 格式:
{
  "httpListenPort": 80
}
*/
struct Config {
	int httpListenPort = 0;
};
AJSON(Config, httpListenPort);

// 适配 std::cout
inline std::ostream& operator<<(std::ostream& o, Config const& c) {
    ajson::save_to(o, c);
    return o;
}

// 全局单例, 便于访问
inline Config config;
