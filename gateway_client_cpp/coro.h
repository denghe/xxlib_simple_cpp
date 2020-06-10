#pragma once
#define COR_BEGIN    switch (lineNumber) { case 0:
#define COR_YIELD    return __LINE__; case __LINE__:;
#define COR_EXIT    return 0;
#define COR_END        } return 0;

#include <string>

struct Client;

struct Coro {
    // 共享上下文
    Client& c;

    // 获取协程名字
    virtual std::string const& GetName() = 0;

    // 协程模拟之 行号 内部变量
    int lineNumber = 0;

    // 协程模拟
    virtual int Update() = 0;

    explicit Coro(Client& c);
    Coro(Coro const &) = delete;
    Coro &operator=(Coro const &) = delete;
    virtual ~Coro() = default;
};
