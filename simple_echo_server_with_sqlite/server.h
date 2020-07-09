#pragma once

#include "xx_epoll.h"
#include "xx_threadpool.h"
#include "xx_sqlite.h"

namespace EP = xx::Epoll;

// 预声明
struct Listener;

// 服务本体
struct Server : EP::Context {
    // 监听器
    std::shared_ptr<Listener> listener;

    // 单线程池. 拿来跑 sqlite
    xx::ThreadPool<void> sqliteJobs = xx::ThreadPool<void>(1);
    // sqlite 的连接对象( for 单线程池 访问 )
    xx::SQLite::Connection db = xx::SQLite::Connection("log.db3");
    // sqlite 的数据插入请求
    xx::SQLite::Query insertQuery = xx::SQLite::Query(db);
    // sqlite 的数据查询请求
    xx::SQLite::Query selectQuery = xx::SQLite::Query(db);

    // 向 db 写一条日志( 异步 )
    void Log(std::string &&txt);

    // 透传构造函数
    using EP::Context::Context;

    // run 前创建 listener 啥的. run 后清除
    int Run() override;
};
