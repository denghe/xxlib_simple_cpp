#include "xx_mysql.h"
#include "xx_string.h"

struct Test {
    std::string sqlHost = "192.168.1.144";
    int sqlPort = 3306;
    std::string sqlUsername = "root";
    std::string sqlPassword = "1";
    std::string sqlDBName = "test_xx_1";

    std::string sqlCreateDB_DefaultDbName = "test1";
    std::string sqlCreateDB = R"#(
/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- 导出 test1 的数据库结构
CREATE DATABASE IF NOT EXISTS `test1` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;
USE `test1`;

-- 导出  表 test1.acc 结构
CREATE TABLE IF NOT EXISTS `acc` (
  `id` bigint(20) NOT NULL DEFAULT 0,
  `money` bigint(20) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 数据导出被取消选择。

-- 导出  表 test1.acc_log 结构
CREATE TABLE IF NOT EXISTS `acc_log` (
  `acc_id` bigint(20) NOT NULL,
  `money_before` bigint(20) NOT NULL,
  `money_add` bigint(20) NOT NULL,
  `money_after` bigint(20) NOT NULL,
  `create_time` bigint(20) NOT NULL,
  KEY `索引 1` (`acc_id`),
  KEY `索引 2` (`create_time`),
  CONSTRAINT `FK_acc_log_acc` FOREIGN KEY (`acc_id`) REFERENCES `acc` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 数据导出被取消选择。

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
)#";

    xx::MySql::Connection conn;

    Test() {
        if (int r = conn.TryOpen(sqlHost.c_str(), sqlPort, sqlUsername.c_str(), sqlPassword.c_str(), "")) {
            xx::CoutN(r);
        }

        for (std::string::size_type pos = 0; pos != std::string::npos;) {
            pos = sqlCreateDB.find(sqlCreateDB_DefaultDbName, pos);
            if (pos != std::string::npos) {
                sqlCreateDB.replace(pos, sqlCreateDB_DefaultDbName.size(), sqlDBName);
                pos += sqlCreateDB_DefaultDbName.size();
            }
        }

        conn.Execute(sqlCreateDB);

    }
};


int main() {
    Test t;
    return 0;
}
