-- Note: MySQL DDL auto-commits; this script does not support rerun after interruption (rerun hits
-- "Duplicate column" at ADD COLUMN). If interrupted between ADD COLUMN and the version bump,
-- manually run the following first, then rerun this script:
--   ALTER TABLE `hku_base`.`holiday` DROP COLUMN `market`;
ALTER TABLE `hku_base`.`holiday` ADD COLUMN `market` VARCHAR(20) NOT NULL DEFAULT 'SH';
-- old schemas have no unique key; repeated imports may leave duplicate (market,date) rows - dedup first, then add the unique key
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
