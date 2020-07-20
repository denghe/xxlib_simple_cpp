#include "server.h"
#include "listener.h"
#include "config.h"
#include "xx_chrono.h"

void Server::Log(std::string &&txt) {
    // 这只是功能展示。不事务合并性能很差。
    auto&& now = xx::NowEpoch10m();
    sqliteJobs.Add([this, now, txt = std::move(txt)] {
        insertQuery.SetParameters(now, txt);
        insertQuery.Execute();
    });
}

int Server::Run() {
    // 初始化回收sg, 以便退出 Run 时清理会加持宿主的成员
    xx::ScopeGuard sg([&] {
        DisableCommandLine();
        listener.reset();
        holdItems.clear();
        assert(shared_from_this().use_count() == 2);
    });
    // 按配置的端口创建监听器
    xx::MakeTo(listener, shared_from_this());
    if (listener->Listen(config.listenPort)) {
        std::cout << "listen to port " << std::to_string(config.listenPort) << " failed." << std::endl;
        return __LINE__;
    }

    EnableCommandLine();
    cmds["log"] = [this](auto args) {
        // todo: 解析参数传入查询?
        sqliteJobs.Add([this] {
            selectQuery.SetParameters(10);
            std::string out;
            selectQuery.Execute([&out](xx::SQLite::Reader &r) {
                out = out + std::to_string(r.ReadInt32(0))
                      + " | " + std::to_string(r.ReadInt64(1))
                      + " | " + r.ReadString(2)
                      + "\r\n";
            });

            Dispatch([out = std::move(out)]{
                std::cout << out << std::endl;
            });
        });
    };

    // 初始化数据库
    sqliteJobs.Add([this] {
        if (!db.TableExists("log")) {
            db.Execute(R"=-=(
CREATE TABLE [log](
    [id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,
    [time] INTEGER NOT NULL,
    [desc] TEXT NOT NULL
);)=-=");
        } else {
            db.TruncateTable("log");
        }
        insertQuery.SetQuery("insert into log (`time`, `desc`) values (?, ?)");
        selectQuery.SetQuery("select `id`, `time`, `desc` from `log` order by `id` desc limit ?");
    });

    // 进入循环
    return this->EP::Context::Run();
}
