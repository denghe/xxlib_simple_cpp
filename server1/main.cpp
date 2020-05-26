﻿#include "xx_signal.h"
#include "config.h"
#include "service.h"

int main() {
	// 禁掉 SIGPIPE 信号避免因为连接关闭出错
	xx::IgnoreSignal();

	// 加载配置
	ajson::load_from_file(::config, "server1_cfg.json");

	// 显示配置内容
	std::cout << ::config << std::endl;

	// 创建服务类实例
	Service service;

	// 开始运行
	return service.Run();
}
