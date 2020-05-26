#pragma once
#include <ajson.hpp>
#include <iostream>

/*
json ����:
{
  "listenPort": 10000
}
*/


// ����ṹӳ�䵽 json ��ʽ
struct Config {
	int listenPort = 0;
};
AJSON(Config, listenPort);


// ���� std::cout
std::ostream& operator<<(std::ostream& o, Config const& c) {
    return o << "listenPort = " << c.listenPort;
}


// ȫ�ֵ���, ���ڷ���
inline Config config;
