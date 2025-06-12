CREATE DATABASE IF NOT EXISTS hku_base PRECISION 'us' KEEP 365000 WAL_LEVEL 2;
CREATE TABLE IF NOT EXISTS hku_base.version (
	id timestamp,
    version INT 
);
insert into hku_base.version values(631152000000000, 1);

CREATE TABLE hku_base.market (
	id TIMESTAMP,
	marketid INT,
	market VARCHAR(10),
	name VARCHAR(60),
	description VARCHAR(100),
	code VARCHAR(20),
	lastdate BIGINT,
	opentime1 INT,
	closetime1 INT,
	opentime2 INT,
	closetime2 INT
);
INSERT INTO hku_base.market (id,marketid,market,name,description,code,lastDate,opentime1,closetime1,opentime2,closetime2) VALUES (631152000000000,1,'SH','上海证券交易所','上海市场','000001',19901219,930,1130,1300,1500);
INSERT INTO hku_base.market (id,marketid,market,name,description,code,lastDate,opentime1,closetime1,opentime2,closetime2) VALUES (631152000001000,2,'SZ','深圳证券交易所','深圳市场','399001',19901219,930,1130,1300,1500);
INSERT INTO hku_base.market (id,marketid,market,name,description,code,lastDate,opentime1,closetime1,opentime2,closetime2) VALUES (631152000002000,2,'BJ','北京证券交易所','北京市场','899050',19901219,930,1130,1300,1500);

create table hku_base.stock (
    id timestamp,
    stockid INT,
    marketid INT,
    code VARCHAR(20),
    name VARCHAR(60),
    type INT,
    valid INT,
    startDate INT,
    endDate INT);

CREATE TABLE IF NOT EXISTS hku_base.block (
	id timestamp,
    category VARCHAR(100),
    name VARCHAR(100),
    market_code VARCHAR(30)
);

CREATE STABLE IF NOT EXISTS hku_base.stkWeight (
	date TIMESTAMP, 
	countAsGift DOUBLE, 
	countForSell DOUBLE, 
	priceForSell DOUBLE, 
	bonus DOUBLE, 
	countOfIncreasement DOUBLE, 
	totalCount DOUBLE, 
	freeCount DOUBLE
) TAGS (stockid INT);

CREATE TABLE IF NOT EXISTS hku_base.coderuletype (
	id timestamp,
    marketid INT,
    codepre VARCHAR(20),
    type INT,
    description VARCHAR(100)
);
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000000000, 1, '000', 2, '上证指数');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000001000, 1, '60', 1, '上证A股');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000002000, 1, '900', 3, '上证B股');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000003000, 2, '00', 1, '深证A股');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000004000, 2, '20', 3, '深证B股');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000005000, 2, '39', 2, '深证指数');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000006000, 2, '150', 4, '深证基金');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000007000, 2, '16', 4, '深证基金');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000008000, 2, '18', 4, '深证基金');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000009000, 2, '159', 5, '深证ETF');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000010000, 1, '51', 5, '上证ETF');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000011000, 1, '50', 4, '上证基金');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000012000, 2, '300', 8, '创业板');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000013000, 1, '009', 6, '国债');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000014000, 1, '010', 6, '国债');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000015000, 1, '10', 7, '可转债');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000016000, 1, '11', 7, '可转债');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000017000, 1, '12', 7, '其他债券');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000018000, 1, '13', 7, '地方政府债');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000019000, 2, '100', 6, '国债');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000020000, 2, '101', 6, '国债');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000021000, 2, '108', 7, '贴债');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000022000, 2, '109', 7, '地方政府债');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000023000, 2, '11', 7, '其他债券');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000024000, 2, '12', 7, '其他债券');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000025000, 1, '688', 9, '科创板');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000026000, 2, '301', 8, '创业板');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000027000, 3, '43', 11, '北证A股');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000028000, 3, '83', 11, '北证A股');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000029000, 3, '87', 11, '北证A股');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000030000, 3, '88', 11, '北证A股');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000031000, 3, '92', 11, '北证A股');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000032000, 1, '880', 2, '通达信板块指数');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000033000, 1, '56', 5, '上证ETF');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000034000, 1, '58', 5, '上证ETF');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000035000, 3, '899', 2, '北证指数');
INSERT INTO hku_base.coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000036000, 2, '302', 8, '创业板');

CREATE TABLE IF NOT EXISTS hku_base.stocktypeinfo (
    `id` TIMESTAMP,
    `type` INT,
    `precision` INT, 
    `tick` DOUBLE, 
    `tickValue` DOUBLE ,
    `minTradeNumber` INT, 
    `maxTradeNumber` INT, 
    `description` VARCHAR(100)
);
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000000000, 0, 2, 100, 100, 100, 1000000, 'Block');
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000001000, 1, 2, 0.01, 0.01, 100, 1000000, 'A股');
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000002000, 2, 3, 0.001, 0.001, 1, 1000000, '指数');
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000003000, 3, 3, 0.001, 0.001, 100, 1000000, 'B股');
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000004000, 4, 3, 0.001, 0.001, 100, 1000000, '基金（不含ETF）');
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000005000, 5, 3, 0.001, 0.001, 1000, 1000000, 'ETF');
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000006000, 6, 2, 0.01, 0.01, 10, 10000, '国债');
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000007000, 7, 2, 0.01, 0.01, 10, 10000, '其他债券');
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000008000, 8, 2, 0.01, 0.01, 100, 1000000, '创业板');
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000009000, 9, 2, 0.01, 0.01, 1, 1000000, '科创板');
INSERT INTO hku_base.stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000010000, 11, 2, 0.01, 0.01, 1, 1000000, '北交所');

CREATE STABLE IF NOT EXISTS hku_base.bond10 (
    `date` TIMESTAMP,
    `value` INT 
) TAGS (country VARCHAR(3));
CREATE TABLE IF NOT EXISTS hku_base.zh_bond10 using hku_base.bond10 (country) TAGS ('zh');

CREATE TABLE IF NOT EXISTS hku_base.historyfinancefield (
    `ts` TIMESTAMP,
    `id` INT,
    `name` VARCHAR(200)
);