INSERT INTO hku_base.stocktypeinfo (type, precision, tick, tickValue, minTradeNumber, maxTradeNumber, description)
SELECT * FROM (SELECT 10 AS type, 6 AS precision, 0.000001 AS tick, 0.000001 AS tickValue, 1 AS minTradeNumber, 1000000 AS maxTradeNumber, '数字币' AS description)
WHERE NOT EXISTS (SELECT * FROM hku_base.stocktypeinfo WHERE type = 10);

INSERT INTO hku_base.stocktypeinfo (type, precision, tick, tickValue, minTradeNumber, maxTradeNumber, description)
SELECT * FROM (SELECT 12 AS type, 2 AS precision, 1 AS tick, 1 AS tickValue, 1 AS minTradeNumber, 100000 AS maxTradeNumber, '期货合约' AS description)
WHERE NOT EXISTS (SELECT * FROM hku_base.stocktypeinfo WHERE type = 12);

INSERT INTO hku_base.stocktypeinfo (type, precision, tick, tickValue, minTradeNumber, maxTradeNumber, description)
SELECT * FROM (SELECT 13 AS type, 5 AS precision, 0.00001 AS tick, 0.00001 AS tickValue, 1 AS minTradeNumber, 1000000 AS maxTradeNumber, '外汇' AS description)
WHERE NOT EXISTS (SELECT * FROM hku_base.stocktypeinfo WHERE type = 13);

INSERT INTO hku_base.stocktypeinfo (type, precision, tick, tickValue, minTradeNumber, maxTradeNumber, description)
SELECT * FROM (SELECT 14 AS type, 2 AS precision, 0.01 AS tick, 0.01 AS tickValue, 1 AS minTradeNumber, 1000000 AS maxTradeNumber, '港股' AS description)
WHERE NOT EXISTS (SELECT * FROM hku_base.stocktypeinfo WHERE type = 14);

INSERT INTO hku_base.stocktypeinfo (type, precision, tick, tickValue, minTradeNumber, maxTradeNumber, description)
SELECT * FROM (SELECT 15 AS type, 2 AS precision, 0.01 AS tick, 0.01 AS tickValue, 1 AS minTradeNumber, 1000000 AS maxTradeNumber, '美股' AS description)
WHERE NOT EXISTS (SELECT * FROM hku_base.stocktypeinfo WHERE type = 15);

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'SHF' AS market, '上海期货交易所' AS name, '上海期货市场' AS description, 'rb0000' AS code, 19900101 AS lastDate, 930 AS openTime1, 1130 AS closeTime1, 1330 AS openTime2, 1500 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'SHF');

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'DCE' AS market, '大连商品交易所' AS name, '大连期货市场' AS description, 'i0000' AS code, 19900101 AS lastDate, 930 AS openTime1, 1130 AS closeTime1, 1330 AS openTime2, 1500 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'DCE');

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'CZC' AS market, '郑州商品交易所' AS name, '郑州期货市场' AS description, 'SR0000' AS code, 19900101 AS lastDate, 930 AS openTime1, 1130 AS closeTime1, 1330 AS openTime2, 1500 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'CZC');

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'CFE' AS market, '中国金融期货交易所' AS name, '中金所市场' AS description, 'IF0000' AS code, 19900101 AS lastDate, 930 AS openTime1, 1130 AS closeTime1, 1330 AS openTime2, 1500 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'CFE');

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'GFE' AS market, '广州期货交易所' AS name, '广期所市场' AS description, 'si0000' AS code, 19900101 AS lastDate, 930 AS openTime1, 1130 AS closeTime1, 1330 AS openTime2, 1500 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'GFE');

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'INE' AS market, '上海国际能源交易中心' AS name, '能源中心市场' AS description, 'sc0000' AS code, 19900101 AS lastDate, 930 AS openTime1, 1130 AS closeTime1, 1330 AS openTime2, 1500 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'INE');

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'FX' AS market, '外汇市场' AS name, '外汇市场' AS description, 'EURUSD' AS code, 19900101 AS lastDate, 0 AS openTime1, 0 AS closeTime1, 0 AS openTime2, 0 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'FX');

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'HK' AS market, '香港交易所' AS name, '香港市场' AS description, 'HSI' AS code, 19900101 AS lastDate, 930 AS openTime1, 1200 AS closeTime1, 1300 AS openTime2, 1600 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'HK');

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'NASDAQ' AS market, '纳斯达克交易所' AS name, '纳斯达克市场' AS description, 'NDX' AS code, 19900101 AS lastDate, 930 AS openTime1, 1600 AS closeTime1, 0 AS openTime2, 0 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'NASDAQ');

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'NYSE' AS market, '纽约证券交易所' AS name, '纽约市场' AS description, 'DJI' AS code, 19900101 AS lastDate, 930 AS openTime1, 1600 AS closeTime1, 0 AS openTime2, 0 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'NYSE');

INSERT INTO hku_base.market (market, name, description, code, lastDate, openTime1, closeTime1, openTime2, closeTime2)
SELECT * FROM (SELECT 'BINANCE' AS market, '币安交易所' AS name, '数字币市场' AS description, 'BTCUSDT' AS code, 19900101 AS lastDate, 0 AS openTime1, 0 AS closeTime1, 0 AS openTime2, 0 AS closeTime2)
WHERE NOT EXISTS (SELECT * FROM hku_base.market WHERE market = 'BINANCE');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'SHF' AS market, 'rb0000' AS code, '螺纹钢主力连续' AS name, 12 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'SHF' AND code = 'rb0000');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'DCE' AS market, 'i0000' AS code, '铁矿石主力连续' AS name, 12 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'DCE' AND code = 'i0000');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'CZC' AS market, 'SR0000' AS code, '白糖主力连续' AS name, 12 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'CZC' AND code = 'SR0000');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'CFE' AS market, 'IF0000' AS code, '沪深300主力连续' AS name, 12 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'CFE' AND code = 'IF0000');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'GFE' AS market, 'si0000' AS code, '工业硅主力连续' AS name, 12 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'GFE' AND code = 'si0000');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'INE' AS market, 'sc0000' AS code, '原油主力连续' AS name, 12 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'INE' AND code = 'sc0000');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'FX' AS market, 'EURUSD' AS code, '欧元美元' AS name, 13 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'FX' AND code = 'EURUSD');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'HK' AS market, 'HSI' AS code, '恒生指数' AS name, 14 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'HK' AND code = 'HSI');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'NASDAQ' AS market, 'NDX' AS code, '纳斯达克100指数' AS name, 15 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'NASDAQ' AND code = 'NDX');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'NYSE' AS market, 'DJI' AS code, '道琼斯工业指数' AS name, 15 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'NYSE' AND code = 'DJI');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT * FROM (SELECT 'BINANCE' AS market, 'BTCUSDT' AS code, '比特币USDT' AS name, 10 AS type, 1 AS valid, 19900101 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market = 'BINANCE' AND code = 'BTCUSDT');

ALTER TABLE `hku_base`.`version` UPDATE `version` = 5 WHERE `id` = 0;
