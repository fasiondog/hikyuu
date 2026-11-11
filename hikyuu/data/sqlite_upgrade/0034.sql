BEGIN TRANSACTION;
ALTER TABLE `Holiday` ADD COLUMN `market` TEXT NOT NULL DEFAULT 'SH';
-- old schemas have no unique key; repeated imports may leave duplicate (market,date) rows - dedup first,
-- then add the unique key (otherwise CREATE UNIQUE INDEX fails and rolls back, stalling the upgrade)
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

-- Variety-level parameter table: the per-security tick / tickValue / trade-number bounds /
-- precision that override the type-level stocktypeinfo defaults. The (marketid, code) key follows
-- the Stock table; a missing row keeps the type-level default. Rows are written by the import side
-- (the exchange is the authoritative source); the core reads it only.
CREATE TABLE IF NOT EXISTS `stockparam` (
  `marketid` INTEGER not null,
  `code` VARCHAR(20) not null,
  `tick` DOUBLE not null,
  `tickValue` DOUBLE not null,
  `minTradeNumber` DOUBLE not null,
  `maxTradeNumber` DOUBLE not null,
  `precision` INTEGER not null,
  PRIMARY KEY (marketid, code)
);

UPDATE `version` set `version` = 34;
COMMIT;
