-- 注意：MySQL DDL 自动提交，本脚本不支持中断后直接重跑（重跑会在 ADD COLUMN 处报 Duplicate column）。
-- 若在 ADD COLUMN 与版本推进之间中断，需先手动执行
--   ALTER TABLE `hku_base`.`holiday` DROP COLUMN `market`;
-- 再重跑本脚本。
ALTER TABLE `hku_base`.`holiday` ADD COLUMN `market` VARCHAR(20) NOT NULL DEFAULT 'SH';
-- 旧库无唯一约束，历史重复导入可能留下同 (market,date) 多行，先清重再加唯一键
DELETE h1 FROM `hku_base`.`holiday` h1
  JOIN `hku_base`.`holiday` h2
    ON h1.`market` = h2.`market` AND h1.`date` = h2.`date` AND h1.`id` > h2.`id`;
INSERT INTO `hku_base`.`holiday` (`market`, `date`) SELECT 'SZ', `date` FROM `hku_base`.`holiday` WHERE `market` = 'SH';
INSERT INTO `hku_base`.`holiday` (`market`, `date`) SELECT 'BJ', `date` FROM `hku_base`.`holiday` WHERE `market` = 'SH';
INSERT INTO `hku_base`.`holiday` (`market`, `date`) SELECT 'SHF', `date` FROM `hku_base`.`holiday` WHERE `market` = 'SH';
INSERT INTO `hku_base`.`holiday` (`market`, `date`) SELECT 'DCE', `date` FROM `hku_base`.`holiday` WHERE `market` = 'SH';
INSERT INTO `hku_base`.`holiday` (`market`, `date`) SELECT 'CZC', `date` FROM `hku_base`.`holiday` WHERE `market` = 'SH';
INSERT INTO `hku_base`.`holiday` (`market`, `date`) SELECT 'CFE', `date` FROM `hku_base`.`holiday` WHERE `market` = 'SH';
INSERT INTO `hku_base`.`holiday` (`market`, `date`) SELECT 'GFE', `date` FROM `hku_base`.`holiday` WHERE `market` = 'SH';
INSERT INTO `hku_base`.`holiday` (`market`, `date`) SELECT 'INE', `date` FROM `hku_base`.`holiday` WHERE `market` = 'SH';
ALTER TABLE `hku_base`.`holiday` ADD UNIQUE KEY `ix_holiday_market_date` (`market`, `date`);

UPDATE `hku_base`.`version` set `version` = 33;
