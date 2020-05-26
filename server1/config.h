#pragma once
#include <ajson.hpp>
#include <iostream>

/*
json 长相:
{
  "listenPort": 10000
}
*/


// 用类结构映射到 json 格式
struct Config {
	int listenPort = 0;
};
AJSON(Config, listenPort);


// 适配 std::cout
std::ostream& operator<<(std::ostream& o, Config const& c) {
    return o << "listenPort = " << c.listenPort;
}


// 全局单例, 便于访问
inline Config config;
