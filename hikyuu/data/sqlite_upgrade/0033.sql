BEGIN TRANSACTION;
-- 多市场数据支持：登记期货/外汇/港股/美股/数字币市场、证券类型（补建内置类型 10，新增自定义类型
-- 12~15，id == type）与各市场代表指数 Stock 占位行（日历轴）。代表指数的 K 线须随后导入，
-- 导入前其交易日历为空，属"未配置 = 不支持"的预期语义。期货类型 unit（tickValue/tick）给中性
-- 默认 1，品种乘数由品种级运行期覆盖或导入侧设置。
ALTER TABLE `Market` ADD COLUMN `tz_offset_min` INTEGER DEFAULT 480;
ALTER TABLE `Market` ADD COLUMN `dst_rule` INTEGER DEFAULT 0;
ALTER TABLE `Market` ADD COLUMN `data_lag` INTEGER DEFAULT 0;
UPDATE `Market` SET `tz_offset_min`=480, `dst_rule`=0, `data_lag`=0 WHERE `marketid` IN (1, 2, 3);
INSERT OR IGNORE INTO `stocktypeinfo` (`tickValue`, `precision`, `id`, `type`, `description`, `tick`, `minTradeNumber`, `maxTradeNumber`) VALUES (0.000001, 6, 10, 10, '数字币', 0.000001, 1, 1000000);
INSERT OR IGNORE INTO `stocktypeinfo` (`tickValue`, `precision`, `id`, `type`, `description`, `tick`, `minTradeNumber`, `maxTradeNumber`) VALUES (1, 2, 12, 12, '期货合约', 1, 1, 100000);
INSERT OR IGNORE INTO `stocktypeinfo` (`tickValue`, `precision`, `id`, `type`, `description`, `tick`, `minTradeNumber`, `maxTradeNumber`) VALUES (0.00001, 5, 13, 13, '外汇', 0.00001, 1, 1000000);
INSERT OR IGNORE INTO `stocktypeinfo` (`tickValue`, `precision`, `id`, `type`, `description`, `tick`, `minTradeNumber`, `maxTradeNumber`) VALUES (0.01, 2, 14, 14, '港股', 0.01, 1, 1000000);
INSERT OR IGNORE INTO `stocktypeinfo` (`tickValue`, `precision`, `id`, `type`, `description`, `tick`, `minTradeNumber`, `maxTradeNumber`) VALUES (0.01, 2, 15, 15, '美股', 0.01, 1, 1000000);

INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 4, 'SHF', '上海期货交易所', '上海期货市场', 'rb0000', 19900101, 930, 1130, 1330, 1500, 480, 0, 0
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'SHF')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 4);
INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 5, 'DCE', '大连商品交易所', '大连期货市场', 'i0000', 19900101, 930, 1130, 1330, 1500, 480, 0, 0
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'DCE')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 5);
INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 6, 'CZC', '郑州商品交易所', '郑州期货市场', 'SR0000', 19900101, 930, 1130, 1330, 1500, 480, 0, 0
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'CZC')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 6);
INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 7, 'CFE', '中国金融期货交易所', '中金所市场', 'IF0000', 19900101, 930, 1130, 1330, 1500, 480, 0, 0
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'CFE')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 7);
INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 8, 'GFE', '广州期货交易所', '广期所市场', 'si0000', 19900101, 930, 1130, 1330, 1500, 480, 0, 0
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'GFE')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 8);
INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 9, 'INE', '上海国际能源交易中心', '能源中心市场', 'sc0000', 19900101, 930, 1130, 1330, 1500, 480, 0, 0
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'INE')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 9);
INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 10, 'FX', '外汇市场', '外汇市场', 'EURUSD', 19900101, 0, 0, 0, 0, 0, 0, 0
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'FX')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 10);
INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 11, 'HK', '香港交易所', '香港市场', 'HSI', 19900101, 930, 1200, 1300, 1600, 480, 0, 0
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'HK')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 11);
INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 12, 'NASDAQ', '纳斯达克交易所', '纳斯达克市场', 'NDX', 19900101, 930, 1600, 0, 0, -300, 1, 1
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'NASDAQ')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 12);
INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 13, 'NYSE', '纽约证券交易所', '纽约市场', 'DJI', 19900101, 930, 1600, 0, 0, -300, 1, 1
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'NYSE')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 13);
INSERT INTO `Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`, `tz_offset_min`, `dst_rule`, `data_lag`)
SELECT 14, 'BINANCE', '币安交易所', '数字币市场', 'BTCUSDT', 19900101, 0, 0, 0, 0, 0, 0, 0
WHERE NOT EXISTS (SELECT 1 FROM `Market` WHERE `market` = 'BINANCE')
  AND NOT EXISTS (SELECT 1 FROM `Market` WHERE `marketid` = 14);

INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'SHF'), 'rb0000', '螺纹钢主力连续', 12, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'SHF') AND `code` = 'rb0000')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'SHF') IS NOT NULL;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'DCE'), 'i0000', '铁矿石主力连续', 12, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'DCE') AND `code` = 'i0000')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'DCE') IS NOT NULL;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'CZC'), 'SR0000', '白糖主力连续', 12, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'CZC') AND `code` = 'SR0000')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'CZC') IS NOT NULL;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'CFE'), 'IF0000', '沪深300主力连续', 12, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'CFE') AND `code` = 'IF0000')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'CFE') IS NOT NULL;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'GFE'), 'si0000', '工业硅主力连续', 12, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'GFE') AND `code` = 'si0000')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'GFE') IS NOT NULL;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'INE'), 'sc0000', '原油主力连续', 12, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'INE') AND `code` = 'sc0000')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'INE') IS NOT NULL;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'FX'), 'EURUSD', '欧元美元', 13, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'FX') AND `code` = 'EURUSD')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'FX') IS NOT NULL;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'HK'), 'HSI', '恒生指数', 14, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'HK') AND `code` = 'HSI')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'HK') IS NOT NULL;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'NASDAQ'), 'NDX', '纳斯达克100指数', 15, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'NASDAQ') AND `code` = 'NDX')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'NASDAQ') IS NOT NULL;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'NYSE'), 'DJI', '道琼斯工业指数', 15, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'NYSE') AND `code` = 'DJI')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'NYSE') IS NOT NULL;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT (SELECT `marketid` FROM `Market` WHERE `market` = 'BINANCE'), 'BTCUSDT', '比特币USDT', 10, 1, 19900101, 99999999
WHERE NOT EXISTS (SELECT 1 FROM `Stock` WHERE `marketid` = (SELECT `marketid` FROM `Market` WHERE `market` = 'BINANCE') AND `code` = 'BTCUSDT')
  AND (SELECT `marketid` FROM `Market` WHERE `market` = 'BINANCE') IS NOT NULL;

UPDATE `version` set `version` = 33;
COMMIT;
