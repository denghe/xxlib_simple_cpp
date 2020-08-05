-- --------------------------------------------------------
-- 主机:                           192.168.1.144
-- 服务器版本:                        10.3.22-MariaDB-1ubuntu1 - Ubuntu 20.04
-- 服务器操作系统:                      debian-linux-gnu
-- HeidiSQL 版本:                  11.0.0.5919
-- --------------------------------------------------------

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

-- 导出  存储过程 test1.sp_acc_add_money 结构
DELIMITER //
CREATE PROCEDURE `sp_acc_add_money`(
	IN `acc_id` BIGINT,
	IN `val` BIGINT,
	IN `create_time` BIGINT,
	OUT `rtv` BIGINT
)
    COMMENT '填充出参 rtv：0 成功   -1 参数不正确   -2 找不到acc_id   -3 日志插入失败'
Lab1:BEGIN
-- 采用 lock free 的写法。使用 SELECT + INTO 语法不会产生输出结果集. DECLARE 要写在最前面
	DECLARE m BIGINT;
-- 前置检查
	IF acc_id = NULL OR val = NULL OR val < 0 THEN
		SET rtv = -1;
		LEAVE Lab1;
	END IF;

Lab2:LOOP
-- 读出当前 money
	SELECT `money` FROM `acc` WHERE `id` = acc_id INTO m;
	IF m is NULL THEN
		SET rtv = -2;
		LEAVE Lab1;
	END IF;

-- 尝试事务改钱并日志
	START TRANSACTION;
-- 带条件改钱
	UPDATE `acc` SET `money` = m + val WHERE `id` = acc_id AND `money` = m;
-- 如果受影响行数 != 1( 可能是 money 已经变了, 也可能是 id 突然找不到了) 就重新循环
	IF ROW_COUNT() <> 1 THEN
		ROLLBACK;
		ITERATE Lab2;
	END IF;

-- 插日志
	INSERT INTO `acc_log` (`acc_id`, `money_before`, `money_add`, `money_after`, `create_time` ) VALUES (acc_id, m, val, m + val, create_time);
	IF ROW_COUNT() <> 1 THEN
		ROLLBACK;
		SET rtv = -3;
		LEAVE Lab1;
	END IF;

-- 提交事务，返回操作成功
	COMMIT;
	SET rtv = 0;
	LEAVE Lab1;
END LOOP Lab2;
END//
DELIMITER ;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;