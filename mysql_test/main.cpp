#include "xx_mysql.h"
#include "xx_logger.h"
#include "xx_chrono.h"

// 得先用脚本建库

struct Test {
    std::string sqlHost = "192.168.1.144";
    int sqlPort = 3306;
    std::string sqlUsername = "root";
    std::string sqlPassword = "1";
    std::string sqlDBName = "test1";

    xx::MySql::Connection conn;

    Test() {
        try {
            {
                LOG_INFO(1);
                conn.Open(sqlHost, sqlPort, sqlUsername, sqlPassword, sqlDBName);
            }
            {
                LOG_INFO(2);
                // 粗暴清除数据
                conn.Execute(R"(
set foreign_key_checks = 0;
truncate table `acc_log`;
truncate table `acc`;
set foreign_key_checks = 1;
)");
                conn.ClearResult();
            }
            {
                LOG_INFO(3);
                // 模拟账号创建
                conn.Execute(R"(
insert into `acc` (`id`, `money`)
values
 (1, 0)
,(2, 0)
,(3, 0)
)");
                // 看看受影响行数
                conn.Fetch([](xx::MySql::Info &info) {
                    LOG_INFO("Fetch info.numFields = ", info.numFields, " info.numRows = ", info.numRows,
                             " info.affectedRows = ", info.affectedRows);
                    return true;
                });
            }
            {
                LOG_INFO(4);
                // 调用存储过程加钱
                conn.Execute(xx::ToString("CALL `acc_add_money`(", 2, ", ", 100, ", ", xx::NowEpoch10m(),
                                          ", @rtv);SELECT @rtv;"));
                // todo conn.FetchInto(
            }

//            while(conn.Fetch([](xx::MySql::Info &info) -> bool {
//                LOG_INFO("Fetch info.numFields = ", info.numFields, " info.numRows = ", info.numRows, " info.affectedRows = ", info.affectedRows);
//                return true;
//            }, [](xx::MySql::Reader &reader) -> bool {
//                return true;
//            })) {}
        }
        catch (std::exception const &e) {
            LOG_ERROR("errCode: ", conn.errCode, " errText: ", e.what());
        }
    }
};

int main() {
    Test t;
    return 0;
}
