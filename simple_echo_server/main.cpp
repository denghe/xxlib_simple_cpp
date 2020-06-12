#include "xx_signal.h"
#include "config.h"
#include "server.h"

int main() {
    // 禁掉 SIGPIPE 信号避免因为连接关闭出错
    xx::IgnoreSignal();

    // 加载配置
    ajson::load_from_file(::config, "config.json");

    // 显示配置内容
    std::cout << ::config << std::endl;

    // 创建服务类实例
    auto &&s = xx::Make<Server>();

    // 初始化
    auto &&rtv = s->Init();

    // 初始化 失败: 打印错误提示并退出
    if (!rtv.empty()) {
        std::cout << rtv << std::endl;
        return -1;
    }

    // 开始运行
    int r = s->Run();

    // 扫尾
    s->Dispose();

    // 检查是否有内泄
    assert(s.use_count() == 1);

    return r;
}
