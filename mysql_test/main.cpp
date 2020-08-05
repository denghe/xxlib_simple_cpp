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
                LOG_INFO(1, " open");
                conn.Open(sqlHost, sqlPort, sqlUsername, sqlPassword, sqlDBName);
            }
            {
                LOG_INFO(2, " cleanup");
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
                LOG_INFO(3, " insert");
                // 模拟账号创建, 输出受影响行数
                auto&& affectedRows = conn.ExecuteNonQuery(R"(
insert into `acc` (`id`, `money`)
values
 (1, 0)
,(2, 0)
,(3, 0)
)");
                LOG_INFO("affectedRows = ", affectedRows);
            }
            int64_t tar_acc_id = 2;
            int64_t tar_add_money = 100;
            {
                LOG_INFO(4, " call sp");
                // 调用存储过程加钱
                conn.Execute(xx::ToString("CALL `sp_acc_add_money`(", tar_acc_id, ", ", tar_add_money, ", ",
                                          xx::NowEpoch10m(),
                                          ", @rtv); SELECT @rtv;"));
                // 填充出参 rtv：0 成功   -1 参数不正确   -2 找不到acc_id   -3 日志插入失败
                auto &&rtv = conn.FetchScalar<int>();
                if (rtv)
                    throw std::logic_error(
                            xx::ToString("call sp: `sp_acc_add_money` return value : ", rtv));
            }
            {
                LOG_INFO(5, " check data");
                auto &&curr_money = conn.ExecuteScalar<int>("select `money` from `acc` where `id` = ", tar_acc_id);
                LOG_INFO("acc id = ", tar_acc_id, ", curr_money = ", curr_money);
            }
            {
                LOG_INFO(6, " show all");
                auto&& results = conn.ExecuteResults("select * from `acc`");
                LOG_INFO(xx::ToString(results));
            }
            // todo: 将输出填充到类结构
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
