#include "xx_mysql.h"
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
                xx::CoutN(1, " open");
                conn.Open(sqlHost, sqlPort, sqlUsername, sqlPassword, sqlDBName);
            }
            {
                xx::CoutN(2, " cleanup");
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
                xx::CoutN(3, " insert");
                // 模拟账号创建, 输出受影响行数
                auto &&affectedRows = conn.ExecuteNonQuery(R"(
insert into `acc` (`id`, `money`)
values
 (1, 0)
,(2, 0)
,(3, 0)
)");
                xx::CoutN("affectedRows = ", affectedRows);
            }
            int64_t tar_acc_id = 2;
            int64_t tar_add_money = 100;
            {
                xx::CoutN(4, " call sp");
                // 调用存储过程加钱   rtv：0 成功   -1 参数不正确   -2 找不到acc_id   -3 日志插入失败
                auto &&rtv = conn.ExecuteScalar<int>("CALL `sp_acc_add_money`(", tar_acc_id, ", ", tar_add_money, ", ",
                                                     xx::NowEpoch10m(), ", @rtv); SELECT @rtv;");
                if (rtv) throw std::logic_error(xx::ToString("call sp: `sp_acc_add_money` return value : ", rtv));
            }
            {
                xx::CoutN(5, " show one row");
                conn.Execute("select `id`, `money` from `acc` where `id` = ", tar_acc_id);
                int64_t id, money;
                conn.FetchTo(id, money);
                xx::CoutN("id, money = ", id, ", ", money);
            }
            {
                xx::CoutN(6, " show id list");
                auto &&results = conn.ExecuteList<int64_t>("select `id` from `acc`");
                xx::CoutN(results);
            }
            {
                xx::CoutN(7, " fill to struct");
                conn.Execute(
                        "select `id`, `money` from `acc`; select `acc_id`, `money_before`, `money_add`, `money_after`, `create_time` from `acc_log`;");
                struct Acc {
                    int64_t id, money;
                };
                struct AccLog {
                    int64_t acc_id, money_before, money_add, money_after, create_time;
                };
                std::vector<Acc> accs;
                std::vector<AccLog> acclogs;
                conn.Fetch(nullptr, [&](xx::MySql::Reader &reader) -> bool {
                    auto &&o = accs.emplace_back();
                    reader.Reads(o.id, o.money);
                    return true;
                });
                conn.Fetch(nullptr, [&](xx::MySql::Reader &reader) -> bool {
                    auto &&o = acclogs.emplace_back();
                    reader.Reads(o.acc_id, o.money_before, o.money_add, o.money_after, o.create_time);
                    return true;
                });
                xx::CoutN("accs.size() = ", accs.size());
                for (auto &&o : accs) {
                    xx::CoutN("id = ", o.id, " money = ", o.money);
                }
                xx::CoutN("acclogs.size() = ", acclogs.size());
                for (auto &&o : acclogs) {
                    xx::CoutN("acc_id = ", o.acc_id, " money_before = ", o.money_before, " money_add = ", o.money_add,
                              " money_after = ", o.money_after, " create_time = ", o.create_time);
                }
            }
            {
                xx::CoutN(8, " direct show all");
                auto &&results = conn.ExecuteResults("select * from `acc`; select * from `acc_log`;");
                xx::CoutN(results);
            }
        }
        catch (std::exception const &e) {
            xx::CoutN("errCode: ", conn.errCode, " errText: ", e.what());
        }
    }
};

int main() {
    Test t;
    return 0;
}
