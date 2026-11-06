BEGIN TRANSACTION;
ALTER TABLE `Holiday` ADD COLUMN `market` TEXT NOT NULL DEFAULT 'SH';
-- 旧库无唯一约束，历史重复导入可能留下同 (market,date) 多行，先清重再加唯一键（否则 CREATE UNIQUE INDEX 失败并回滚，升级卡死）
DELETE FROM `Holiday` WHERE `id` NOT IN (SELECT MIN(`id`) FROM `Holiday` GROUP BY `market`, `date`);
INSERT INTO `Holiday` (`market`, `date`) SELECT 'SZ', `date` FROM `Holiday` WHERE `market` = 'SH';
INSERT INTO `Holiday` (`market`, `date`) SELECT 'BJ', `date` FROM `Holiday` WHERE `market` = 'SH';
INSERT INTO `Holiday` (`market`, `date`) SELECT 'SHF', `date` FROM `Holiday` WHERE `market` = 'SH';
INSERT INTO `Holiday` (`market`, `date`) SELECT 'DCE', `date` FROM `Holiday` WHERE `market` = 'SH';
INSERT INTO `Holiday` (`market`, `date`) SELECT 'CZC', `date` FROM `Holiday` WHERE `market` = 'SH';
INSERT INTO `Holiday` (`market`, `date`) SELECT 'CFE', `date` FROM `Holiday` WHERE `market` = 'SH';
INSERT INTO `Holiday` (`market`, `date`) SELECT 'GFE', `date` FROM `Holiday` WHERE `market` = 'SH';
INSERT INTO `Holiday` (`market`, `date`) SELECT 'INE', `date` FROM `Holiday` WHERE `market` = 'SH';
CREATE UNIQUE INDEX IF NOT EXISTS `ix_holiday_market_date` ON `Holiday` (`market`, `date`);

UPDATE `version` set `version` = 34;
COMMIT;
