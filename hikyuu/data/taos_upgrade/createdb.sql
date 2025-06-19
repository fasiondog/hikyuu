CREATE DATABASE IF NOT EXISTS hku_base PRECISION 'us' KEEP 365000 WAL_LEVEL 2;
CREATE TABLE IF NOT EXISTS hku_base.version (
	id timestamp,
    version INT 
);
insert into hku_base.version values(631152000000000, 1);

CREATE TABLE hku_base.n_market (
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
INSERT INTO hku_base.n_market (id,marketid,market,name,description,code,lastDate,opentime1,closetime1,opentime2,closetime2) VALUES (631152000000000,1,'SH','上海证券交易所','上海市场','000001',19901219,930,1130,1300,1500);
INSERT INTO hku_base.n_market (id,marketid,market,name,description,code,lastDate,opentime1,closetime1,opentime2,closetime2) VALUES (631152000001000,2,'SZ','深圳证券交易所','深圳市场','399001',19901219,930,1130,1300,1500);
INSERT INTO hku_base.n_market (id,marketid,market,name,description,code,lastDate,opentime1,closetime1,opentime2,closetime2) VALUES (631152000002000,3,'BJ','北京证券交易所','北京市场','899050',19901219,930,1130,1300,1500);

create table hku_base.n_stock (
    stockid TIMESTAMP,
    marketid INT,
    code VARCHAR(20),
    name VARCHAR(60),
    type INT,
    valid INT,
    startDate INT,
    endDate INT);

CREATE TABLE IF NOT EXISTS hku_base.s_block (
	id timestamp,
    market_code VARCHAR(30)
) TAGS (category VARCHAR(100), name VARCHAR(100), index_code VARCHAR(30));

CREATE STABLE IF NOT EXISTS hku_base.s_stkweight (
	date TIMESTAMP, 
	countAsGift DOUBLE, 
	countForSell DOUBLE, 
	priceForSell DOUBLE, 
	bonus DOUBLE, 
	countOfIncreasement DOUBLE, 
	totalCount DOUBLE, 
	freeCount DOUBLE,
	suogu DOUBLE
) TAGS (stockid TIMESTAMP);

CREATE TABLE IF NOT EXISTS hku_base.n_coderuletype (
	id timestamp,
    marketid INT,
    codepre VARCHAR(20),
    type INT,
    description VARCHAR(100)
);
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000000000, 1, '000', 2, '上证指数');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000001000, 1, '60', 1, '上证A股');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000002000, 1, '900', 3, '上证B股');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000003000, 2, '00', 1, '深证A股');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000004000, 2, '20', 3, '深证B股');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000005000, 2, '39', 2, '深证指数');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000006000, 2, '150', 4, '深证基金');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000007000, 2, '16', 4, '深证基金');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000008000, 2, '18', 4, '深证基金');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000009000, 2, '159', 5, '深证ETF');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000010000, 1, '51', 5, '上证ETF');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000011000, 1, '50', 4, '上证基金');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000012000, 2, '300', 8, '创业板');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000013000, 1, '009', 6, '国债');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000014000, 1, '010', 6, '国债');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000015000, 1, '10', 7, '可转债');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000016000, 1, '11', 7, '可转债');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000017000, 1, '12', 7, '其他债券');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000018000, 1, '13', 7, '地方政府债');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000019000, 2, '100', 6, '国债');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000020000, 2, '101', 6, '国债');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000021000, 2, '108', 7, '贴债');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000022000, 2, '109', 7, '地方政府债');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000023000, 2, '11', 7, '其他债券');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000024000, 2, '12', 7, '其他债券');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000025000, 1, '688', 9, '科创板');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000026000, 2, '301', 8, '创业板');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000027000, 3, '43', 11, '北证A股');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000028000, 3, '83', 11, '北证A股');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000029000, 3, '87', 11, '北证A股');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000030000, 3, '88', 11, '北证A股');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000031000, 3, '92', 11, '北证A股');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000032000, 1, '880', 2, '通达信板块指数');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000033000, 1, '56', 5, '上证ETF');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000034000, 1, '58', 5, '上证ETF');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000035000, 3, '899', 2, '北证指数');
INSERT INTO hku_base.n_coderuletype (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (631152000036000, 2, '302', 8, '创业板');

CREATE TABLE IF NOT EXISTS hku_base.n_stocktypeinfo (
    `id` TIMESTAMP,
    `type` INT,
    `precision` INT, 
    `tick` DOUBLE, 
    `tickValue` DOUBLE ,
    `minTradeNumber` INT, 
    `maxTradeNumber` INT, 
    `description` VARCHAR(100)
);
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000000000, 0, 2, 100, 100, 100, 1000000, 'Block');
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000001000, 1, 2, 0.01, 0.01, 100, 1000000, 'A股');
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000002000, 2, 3, 0.001, 0.001, 1, 1000000, '指数');
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000003000, 3, 3, 0.001, 0.001, 100, 1000000, 'B股');
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000004000, 4, 3, 0.001, 0.001, 100, 1000000, '基金（不含ETF）');
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000005000, 5, 3, 0.001, 0.001, 1000, 1000000, 'ETF');
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000006000, 6, 2, 0.01, 0.01, 10, 10000, '国债');
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000007000, 7, 2, 0.01, 0.01, 10, 10000, '其他债券');
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000008000, 8, 2, 0.01, 0.01, 100, 1000000, '创业板');
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000009000, 9, 2, 0.01, 0.01, 1, 1000000, '科创板');
INSERT INTO hku_base.n_stocktypeinfo (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (631152000010000, 11, 2, 0.01, 0.01, 1, 1000000, '北交所');

CREATE STABLE IF NOT EXISTS hku_base.s_bond10 (
    `date` TIMESTAMP,
    `value` INT 
) TAGS (country VARCHAR(3));
CREATE TABLE IF NOT EXISTS hku_base.z_zh_bond10 using hku_base.s_bond10 (country) TAGS ('zh');

CREATE TABLE IF NOT EXISTS hku_base.n_historyfinancefield (
    `ts` TIMESTAMP,
    `id` INT,
    `name` VARCHAR(200)
);
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000000000, 1, '基本每股收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000001000, 2, '扣除非经常性损益每股收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000002000, 3, '每股未分配利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000003000, 4, '每股净资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000004000, 5, '每股资本公积金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000005000, 6, '净资产收益率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000006000, 7, '每股经营现金流量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000007000, 8, '资产负债表_货币资金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000008000, 9, '资产负债表_交易性金融资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000009000, 10, '资产负债表_应收票据');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000010000, 11, '资产负债表_应收账款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000011000, 12, '资产负债表_预付款项');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000012000, 13, '资产负债表_其他应收款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000013000, 14, '资产负债表_应收关联公司款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000014000, 15, '资产负债表_应收利息');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000015000, 16, '资产负债表_应收股利');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000016000, 17, '资产负债表_存货');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000017000, 18, '资产负债表_消耗性生物资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000018000, 19, '资产负债表_一年内到期的非流动资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000019000, 20, '资产负债表_其他流动资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000020000, 21, '资产负债表_流动资产合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000021000, 22, '资产负债表_可供出售金融资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000022000, 23, '资产负债表_持有至到期投资');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000023000, 24, '资产负债表_长期应收款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000024000, 25, '资产负债表_长期股权投资');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000025000, 26, '资产负债表_投资性房地产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000026000, 27, '资产负债表_固定资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000027000, 28, '资产负债表_在建工程');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000028000, 29, '资产负债表_工程物资');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000029000, 30, '资产负债表_固定资产清理');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000030000, 31, '资产负债表_生产性生物资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000031000, 32, '资产负债表_油气资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000032000, 33, '资产负债表_无形资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000033000, 34, '资产负债表_开发支出');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000034000, 35, '资产负债表_商誉');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000035000, 36, '资产负债表_长期待摊费用');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000036000, 37, '资产负债表_递延所得税资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000037000, 38, '资产负债表_其他非流动资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000038000, 39, '资产负债表_非流动资产合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000039000, 40, '资产负债表_资产总计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000040000, 41, '资产负债表_短期借款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000041000, 42, '资产负债表_交易性金融负债');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000042000, 43, '资产负债表_应付票据');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000043000, 44, '资产负债表_应付账款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000044000, 45, '资产负债表_预收款项');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000045000, 46, '资产负债表_应付职工薪酬');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000046000, 47, '资产负债表_应交税费');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000047000, 48, '资产负债表_应付利息');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000048000, 49, '资产负债表_应付股利');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000049000, 50, '资产负债表_其他应付款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000050000, 51, '资产负债表_应付关联公司款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000051000, 52, '资产负债表_一年内到期的非流动负债');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000052000, 53, '资产负债表_其他流动负债');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000053000, 54, '资产负债表_流动负债合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000054000, 55, '资产负债表_长期借款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000055000, 56, '资产负债表_应付债券');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000056000, 57, '资产负债表_长期应付款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000057000, 58, '资产负债表_专项应付款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000058000, 59, '资产负债表_预计负债');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000059000, 60, '资产负债表_递延所得税负债');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000060000, 61, '资产负债表_其他非流动负债');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000061000, 62, '资产负债表_非流动负债合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000062000, 63, '资产负债表_负债合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000063000, 64, '资产负债表_实收资本（或股本）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000064000, 65, '资产负债表_资本公积');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000065000, 66, '资产负债表_盈余公积');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000066000, 67, '资产负债表_库存股');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000067000, 68, '资产负债表_未分配利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000068000, 69, '资产负债表_少数股东权益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000069000, 70, '资产负债表_外币报表折算价差');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000070000, 71, '资产负债表_非正常经营项目收益调整');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000071000, 72, '资产负债表_所有者权益（或股东权益）合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000072000, 73, '资产负债表_负债和所有者（或股东权益）合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000073000, 74, '利润表_营业收入');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000074000, 75, '利润表_营业成本');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000075000, 76, '利润表_营业税金及附加');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000076000, 77, '利润表_销售费用');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000077000, 78, '利润表_管理费用');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000078000, 79, '利润表_勘探费用');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000079000, 80, '利润表_财务费用');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000080000, 81, '利润表_资产减值损失');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000081000, 82, '利润表_公允价值变动净收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000082000, 83, '利润表_投资收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000083000, 84, '利润表_对联营企业和合营企业的投资收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000084000, 85, '利润表_影响营业利润的其他科目');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000085000, 86, '利润表_营业利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000086000, 87, '利润表_补贴收入');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000087000, 88, '利润表_营业外收入');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000088000, 89, '利润表_营业外支出');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000089000, 90, '利润表_非流动资产处置净损失');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000090000, 91, '利润表_影响利润总额的其他科目');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000091000, 92, '利润表_利润总额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000092000, 93, '利润表_所得税');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000093000, 94, '利润表_影响净利润的其他科目');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000094000, 95, '利润表_净利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000095000, 96, '利润表_归属于母公司所有者的净利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000096000, 97, '利润表_少数股东损益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000097000, 98, '现金流量表_销售商品、提供劳务收到的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000098000, 99, '现金流量表_收到的税费返还');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000099000, 100, '现金流量表_收到其他与经营活动有关的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000100000, 101, '现金流量表_经营活动现金流入小计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000101000, 102, '现金流量表_购买商品、接受劳务支付的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000102000, 103, '现金流量表_支付给职工以及为职工支付的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000103000, 104, '现金流量表_支付的各项税费');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000104000, 105, '现金流量表_支付其他与经营活动有关的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000105000, 106, '现金流量表_经营活动现金流出小计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000106000, 107, '现金流量表_经营活动产生的现金流量净额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000107000, 108, '现金流量表_收回投资收到的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000108000, 109, '现金流量表_取得投资收益收到的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000109000, 110, '现金流量表_处置固定资产、无形资产和其他长期资产收回的现金净额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000110000, 111, '现金流量表_处置子公司及其他营业单位收到的现金净额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000111000, 112, '现金流量表_收到其他与投资活动有关的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000112000, 113, '现金流量表_投资活动现金流入小计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000113000, 114, '现金流量表_购建固定资产、无形资产和其他长期资产支付的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000114000, 115, '现金流量表_投资支付的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000115000, 116, '现金流量表_取得子公司及其他营业单位支付的现金净额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000116000, 117, '现金流量表_支付其他与投资活动有关的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000117000, 118, '现金流量表_投资活动现金流出小计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000118000, 119, '现金流量表_投资活动产生的现金流量净额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000119000, 120, '现金流量表_吸收投资收到的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000120000, 121, '现金流量表_取得借款收到的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000121000, 122, '现金流量表_收到其他与筹资活动有关的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000122000, 123, '现金流量表_筹资活动现金流入小计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000123000, 124, '现金流量表_偿还债务支付的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000124000, 125, '现金流量表_分配股利、利润或偿付利息支付的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000125000, 126, '现金流量表_支付其他与筹资活动有关的现金');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000126000, 127, '现金流量表_筹资活动现金流出小计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000127000, 128, '现金流量表_筹资活动产生的现金流量净额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000128000, 129, '现金流量表_汇率变动对现金的影响');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000129000, 130, '现金流量表_其他原因对现金的影响');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000130000, 131, '现金流量表_现金及现金等价物净增加额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000131000, 132, '现金流量表_期初现金及现金等价物余额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000132000, 133, '现金流量表_期末现金及现金等价物余额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000133000, 134, '现金流量表_净利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000134000, 135, '现金流量表_资产减值准备');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000135000, 136, '现金流量表_固定资产折旧、油气资产折耗、生产性生物资产折旧');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000136000, 137, '现金流量表_无形资产摊销');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000137000, 138, '现金流量表_长期待摊费用摊销');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000138000, 139, '现金流量表_处置固定资产、无形资产和其他长期资产的损失');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000139000, 140, '现金流量表_固定资产报废损失');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000140000, 141, '现金流量表_公允价值变动损失');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000141000, 142, '现金流量表_财务费用');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000142000, 143, '现金流量表_投资损失');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000143000, 144, '现金流量表_递延所得税资产减少');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000144000, 145, '现金流量表_递延所得税负债增加');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000145000, 146, '现金流量表_存货的减少');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000146000, 147, '现金流量表_经营性应收项目的减少');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000147000, 148, '现金流量表_经营性应付项目的增加');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000148000, 149, '现金流量表_其他');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000149000, 150, '现金流量表_经营活动产生的现金流量净额2');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000150000, 151, '现金流量表_债务转为资本');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000151000, 152, '现金流量表_一年内到期的可转换公司债券');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000152000, 153, '现金流量表_融资租入固定资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000153000, 154, '现金流量表_现金的期末余额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000154000, 155, '现金流量表_现金的期初余额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000155000, 156, '现金流量表_现金等价物的期末余额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000156000, 157, '现金流量表_现金等价物的期初余额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000157000, 158, '现金流量表_现金及现金等价物净增加额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000158000, 159, '偿债能力_流动比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000159000, 160, '偿债能力_速动比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000160000, 161, '偿债能力_现金比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000161000, 162, '偿债能力_利息保障倍数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000162000, 163, '偿债能力_非流动负债比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000163000, 164, '偿债能力_流动负债比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000164000, 165, '偿债能力_现金到期债务比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000165000, 166, '偿债能力_有形资产净值债务率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000166000, 167, '偿债能力_权益乘数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000167000, 168, '偿债能力_股东的权益/负债合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000168000, 169, '偿债能力_有形资产/负债合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000169000, 170, '偿债能力_经营活动产生的现金流量净额/负债合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000170000, 171, '偿债能力_EBITDA/负债合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000171000, 172, '营运能力_应收帐款周转率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000172000, 173, '营运能力_存货周转率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000173000, 174, '营运能力_运营资金周转率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000174000, 175, '营运能力_总资产周转率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000175000, 176, '营运能力_固定资产周转率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000176000, 177, '营运能力_应收帐款周转天数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000177000, 178, '营运能力_存货周转天数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000178000, 179, '营运能力_流动资产周转率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000179000, 180, '营运能力_流动资产周转天数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000180000, 181, '营运能力_总资产周转天数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000181000, 182, '营运能力_股东权益周转率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000182000, 183, '成长能力_营业收入增长率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000183000, 184, '成长能力_净利润增长率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000184000, 185, '成长能力_净资产增长率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000185000, 186, '成长能力_固定资产增长率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000186000, 187, '成长能力_总资产增长率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000187000, 188, '成长能力_投资收益增长率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000188000, 189, '成长能力_营业利润增长率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000189000, 190, '成长能力_扣非每股收益同比');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000190000, 191, '成长能力_扣非净利润同比');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000191000, 192, '成长能力_暂无');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000192000, 193, '盈利能力_成本费用利润率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000193000, 194, '盈利能力_营业利润率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000194000, 195, '盈利能力_营业税金率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000195000, 196, '盈利能力_营业成本率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000196000, 197, '盈利能力_净资产收益率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000197000, 198, '盈利能力_投资收益率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000198000, 199, '盈利能力_销售净利率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000199000, 200, '盈利能力_总资产报酬率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000200000, 201, '盈利能力_净利润率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000201000, 202, '盈利能力_销售毛利率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000202000, 203, '盈利能力_三费比重');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000203000, 204, '盈利能力_管理费用率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000204000, 205, '盈利能力_财务费用率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000205000, 206, '盈利能力_扣除非经常性损益后的净利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000206000, 207, '盈利能力_息税前利润(EBIT)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000207000, 208, '盈利能力_息税折旧摊销前利润(EBITDA)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000208000, 209, '盈利能力_EBITDA/营业总收入');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000209000, 210, '资本结构_资产负债率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000210000, 211, '资本结构_流动资产比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000211000, 212, '资本结构_货币资金比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000212000, 213, '资本结构_存货比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000213000, 214, '资本结构_固定资产比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000214000, 215, '资本结构_负债结构比');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000215000, 216, '资本结构_归属于母公司股东权益/全部投入资本');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000216000, 217, '资本结构_股东的权益/带息债务');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000217000, 218, '资本结构_有形资产/净债务');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000218000, 219, '现金能力_每股经营性现金流');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000219000, 220, '现金能力_营业收入现金含量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000220000, 221, '现金能力_经营活动产生的现金流量净额/经营活动净收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000221000, 222, '现金能力_销售商品提供劳务收到的现金/营业收入');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000222000, 223, '现金能力_经营活动产生的现金流量净额/营业收入');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000223000, 224, '现金能力_资本支出/折旧和摊销');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000224000, 225, '现金能力_每股现金流量净额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000225000, 226, '现金能力_经营净现金比率（短期债务）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000226000, 227, '现金能力_经营净现金比率（全部债务）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000227000, 228, '现金能力_经营活动现金净流量与净利润比率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000228000, 229, '现金能力_全部资产现金回收率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000229000, 230, '利润表_营业收入_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000230000, 231, '利润表_营业利润_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000231000, 232, '利润表_归属于母公司所有者的净利润_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000232000, 233, '利润表_扣除非经常性损益后的净利润_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000233000, 234, '现金流量表_经营活动产生的现金流量净额_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000234000, 235, '现金流量表_投资活动产生的现金流量净额_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000235000, 236, '现金流量表_筹资活动产生的现金流量净额_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000236000, 237, '现金流量表_现金及现金等价物净增加额_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000237000, 238, '股本股东_总股本');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000238000, 239, '股本股东_已上市流通A股');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000239000, 240, '股本股东_已上市流通B股');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000240000, 241, '股本股东_已上市流通H股');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000241000, 242, '股本股东_股东人数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000242000, 243, '股本股东_第一大股东的持股数量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000243000, 244, '股本股东_十大流通股东持股数量合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000244000, 245, '股本股东_十大股东持股数量合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000245000, 246, '股本股东_机构总量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000246000, 247, '股本股东_机构持股总量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000247000, 248, '股本股东_QFII机构数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000248000, 249, '股本股东_QFII持股量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000249000, 250, '股本股东_券商机构数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000250000, 251, '股本股东_券商持股量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000251000, 252, '股本股东_保险机构数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000252000, 253, '股本股东_保险持股量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000253000, 254, '股本股东_基金机构数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000254000, 255, '股本股东_基金持股量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000255000, 256, '股本股东_社保机构数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000256000, 257, '股本股东_社保持股量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000257000, 258, '股本股东_私募机构数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000258000, 259, '股本股东_私募持股量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000259000, 260, '股本股东_财务公司机构数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000260000, 261, '股本股东_财务公司持股量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000261000, 262, '股本股东_年金机构数');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000262000, 263, '股本股东_年金持股量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000263000, 264, '股本股东_十大流通股东中持有A股合计');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000264000, 265, '股本股东_第一大流通股东持股量');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000265000, 266, '股本股东_自由流通股');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000266000, 267, '股本股东_受限流通A股');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000267000, 268, '资产负债表_一般风险准备');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000268000, 269, '利润表_其他综合收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000269000, 270, '利润表_综合收益总额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000270000, 271, '资产负债表_归属于母公司股东权益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000271000, 272, '股本股东_银行机构数(机构持股)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000272000, 273, '股本股东_银行持股量(机构持股)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000273000, 274, '股本股东_一般法人机构数(机构持股)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000274000, 275, '股本股东_一般法人持股量(机构持股)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000275000, 276, '利润表_近一年净利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000276000, 277, '股本股东_信托机构数(机构持股)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000277000, 278, '股本股东_信托持股量(机构持股)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000278000, 279, '股本股东_特殊法人机构数(机构持股)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000279000, 280, '股本股东_特殊法人持股量(机构持股)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000280000, 281, '盈利能力_加权净资产收益率(每股指标)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000281000, 282, '利润表_扣非每股收益_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000282000, 283, '利润表_最近一年营业收入（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000283000, 284, '股本股东_国家队持股数量（万股）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000284000, 285, '业绩预告_本期净利润同比增幅下限%');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000285000, 286, '业绩预告_本期净利润同比增幅上限%');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000286000, 287, '业绩快报_归母净利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000287000, 288, '业绩快报_扣非净利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000288000, 289, '业绩快报_总资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000289000, 290, '业绩快报_净资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000290000, 291, '业绩快报_每股收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000291000, 292, '业绩快报_摊薄净资产收益率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000292000, 293, '业绩快报_加权净资产收益率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000293000, 294, '业绩快报_每股净资产');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000294000, 295, '资产负债表_应付票据及应付账款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000295000, 296, '资产负债表_应收票据及应收账款');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000296000, 297, '资产负债表_递延收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000297000, 298, '资产负债表_其他综合收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000298000, 299, '资产负债表_其他权益工具');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000299000, 300, '利润表_其他收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000300000, 301, '利润表_资产处置收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000301000, 302, '利润表_持续经营净利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000302000, 303, '利润表_终止经营净利润');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000303000, 304, '利润表_研发费用');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000304000, 305, '利润表_利息费用');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000305000, 306, '利润表_利息收入');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000306000, 307, '现金流量表_近一年经营活动现金流净额');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000307000, 308, '现金流量表_近一年归母净利润（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000308000, 309, '现金流量表_近一年扣非净利润（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000309000, 310, '现金流量表_近一年现金净流量（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000310000, 311, '利润表_基本每股收益_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000311000, 312, '利润表_营业总收入（万元）_单季度');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000312000, 313, '业绩预告公告日期 ');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000313000, 314, '财报公告日期');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000314000, 315, '业绩快报公告日期');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000315000, 316, '现金流量表_近一年投资活动现金流净额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000316000, 317, '业绩预告_业绩预告-本期净利润下限（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000317000, 318, '业绩预告_业绩预告-本期净利润上限（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000318000, 319, '利润表_营业总收入TTM（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000319000, 320, '员工总数(人)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000320000, 321, '现金流量表_每股企业自由现金流');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000321000, 322, '现金流量表_每股股东自由现金流');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000322000, 323, '近一年营业利润（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000323000, 324, '净利润（单季度）(万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000324000, 325, '北上资金数（家）(机构持股）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000325000, 326, '北上资金持股量（股）(机构持股）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000326000, 327, '有息负债率');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000327000, 328, '营业成本（单季度）(万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000328000, 329, '投入资本回报率（ROIC）(获利能力分析)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000329000, 330, '业绩快报-营业收入（本期）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000330000, 331, '业绩快报-营业收入（上期）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000331000, 332, '业绩快报-营业利润（本期）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000332000, 333, '业绩快报-营业利润（上期）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000333000, 334, '业绩快报-利润总额（本期）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000334000, 335, '业绩快报-利润总额（上期）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000335000, 336, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000336000, 337, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000337000, 338, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000338000, 339, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000339000, 340, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000340000, 341, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000341000, 342, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000342000, 343, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000343000, 344, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000344000, 345, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000345000, 346, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000346000, 347, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000347000, 348, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000348000, 349, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000349000, 350, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000350000, 351, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000351000, 352, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000352000, 353, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000353000, 354, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000354000, 355, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000355000, 356, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000356000, 357, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000357000, 358, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000358000, 359, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000359000, 360, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000360000, 361, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000361000, 362, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000362000, 363, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000363000, 364, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000364000, 365, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000365000, 366, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000366000, 367, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000367000, 368, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000368000, 369, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000369000, 370, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000370000, 371, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000371000, 372, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000372000, 373, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000373000, 374, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000374000, 375, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000375000, 376, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000376000, 377, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000377000, 378, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000378000, 379, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000379000, 380, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000380000, 381, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000381000, 382, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000382000, 383, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000383000, 384, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000384000, 385, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000385000, 386, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000386000, 387, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000387000, 388, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000388000, 389, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000389000, 390, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000390000, 391, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000391000, 392, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000392000, 393, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000393000, 394, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000394000, 395, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000395000, 396, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000396000, 397, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000397000, 398, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000398000, 399, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000399000, 400, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000400000, 401, '资产负债表_专项储备（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000401000, 402, '资产负债表_结算备付金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000402000, 403, '资产负债表_拆出资金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000403000, 404, '资产负债表_发放贷款及垫款（万元）(流动资产科目)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000404000, 405, '资产负债表_衍生金融资产（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000405000, 406, '资产负债表_应收保费（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000406000, 407, '资产负债表_应收分保账款（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000407000, 408, '资产负债表_应收分保合同准备金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000408000, 409, '资产负债表_买入返售金融资产（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000409000, 410, '资产负债表_划分为持有待售的资产（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000410000, 411, '资产负债表_发放贷款及垫款（万元）(非流动资产科目)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000411000, 412, '资产负债表_向中央银行借款（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000412000, 413, '资产负债表_吸收存款及同业存放（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000413000, 414, '资产负债表_拆入资金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000414000, 415, '资产负债表_衍生金融负债（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000415000, 416, '资产负债表_卖出回购金融资产款（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000416000, 417, '资产负债表_应付手续费及佣金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000417000, 418, '资产负债表_应付分保账款（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000418000, 419, '资产负债表_保险合同准备金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000419000, 420, '资产负债表_代理买卖证券款（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000420000, 421, '资产负债表_代理承销证券款（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000421000, 422, '资产负债表_划分为持有待售的负债（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000422000, 423, '资产负债表_预计负债（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000423000, 424, '资产负债表_递延收益（万元）（流动负债科目）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000424000, 425, '资产负债表_其中:优先股（万元）(非流动负债科目)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000425000, 426, '资产负债表_永续债（万元）(非流动负债科目)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000426000, 427, '资产负债表_长期应付职工薪酬（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000427000, 428, '资产负债表_其中:优先股（万元）(所有者权益科目)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000428000, 429, '资产负债表_永续债（万元）(所有者权益科目)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000429000, 430, '资产负债表_债权投资（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000430000, 431, '资产负债表_其他债权投资（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000431000, 432, '资产负债表_其他权益工具投资（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000432000, 433, '资产负债表_其他非流动金融资产（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000433000, 434, '资产负债表_合同负债（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000434000, 435, '资产负债表_合同资产（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000435000, 436, '资产负债表_其他资产（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000436000, 437, '资产负债表_应收款项融资（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000437000, 438, '资产负债表_使用权资产（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000438000, 439, '资产负债表_租赁负债（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000439000, 440, '发放贷款及垫款(万元)');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000440000, 441, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000441000, 442, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000442000, 443, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000443000, 444, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000444000, 445, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000445000, 446, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000446000, 447, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000447000, 448, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000448000, 449, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000449000, 450, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000450000, 451, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000451000, 452, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000452000, 453, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000453000, 454, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000454000, 455, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000455000, 456, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000456000, 457, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000457000, 458, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000458000, 459, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000459000, 460, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000460000, 461, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000461000, 462, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000462000, 463, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000463000, 464, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000464000, 465, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000465000, 466, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000466000, 467, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000467000, 468, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000468000, 469, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000469000, 470, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000470000, 471, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000471000, 472, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000472000, 473, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000473000, 474, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000474000, 475, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000475000, 476, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000476000, 477, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000477000, 478, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000478000, 479, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000479000, 480, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000480000, 481, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000481000, 482, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000482000, 483, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000483000, 484, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000484000, 485, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000485000, 486, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000486000, 487, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000487000, 488, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000488000, 489, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000489000, 490, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000490000, 491, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000491000, 492, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000492000, 493, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000493000, 494, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000494000, 495, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000495000, 496, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000496000, 497, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000497000, 498, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000498000, 499, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000499000, 500, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000500000, 501, '利润表_稀释每股收益');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000501000, 502, '利润表_营业总收入（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000502000, 503, '利润表_汇兑收益（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000503000, 504, '利润表_其中:归属于母公司综合收益（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000504000, 505, '利润表_其中:归属于少数股东综合收益（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000505000, 506, '利润表_利息收入（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000506000, 507, '利润表_已赚保费（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000507000, 508, '利润表_手续费及佣金收入（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000508000, 509, '利润表_利息支出（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000509000, 510, '利润表_手续费及佣金支出（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000510000, 511, '利润表_退保金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000511000, 512, '利润表_赔付支出净额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000512000, 513, '利润表_提取保险合同准备金净额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000513000, 514, '利润表_保单红利支出（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000514000, 515, '利润表_分保费用（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000515000, 516, '利润表_其中:非流动资产处置利得（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000516000, 517, '利润表_信用减值损失（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000517000, 518, '利润表_净敞口套期收益（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000518000, 519, '利润表_营业总成本（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000519000, 520, '利润表_信用减值损失（万元、2019格式）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000520000, 521, '利润表_资产减值损失（万元、2019格式）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000521000, 522, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000522000, 523, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000523000, 524, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000524000, 525, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000525000, 526, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000526000, 527, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000527000, 528, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000528000, 529, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000529000, 530, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000530000, 531, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000531000, 532, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000532000, 533, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000533000, 534, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000534000, 535, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000535000, 536, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000536000, 537, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000537000, 538, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000538000, 539, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000539000, 540, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000540000, 541, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000541000, 542, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000542000, 543, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000543000, 544, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000544000, 545, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000545000, 546, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000546000, 547, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000547000, 548, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000548000, 549, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000549000, 550, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000550000, 551, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000551000, 552, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000552000, 553, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000553000, 554, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000554000, 555, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000555000, 556, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000556000, 557, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000557000, 558, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000558000, 559, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000559000, 560, '未知');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000560000, 561, '现金流量表_加:其他原因对现金的影响（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000561000, 562, '现金流量表_客户存款和同业存放款项净增加额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000562000, 563, '现金流量表_向中央银行借款净增加额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000563000, 564, '现金流量表_向其他金融机构拆入资金净增加额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000564000, 565, '现金流量表_收到原保险合同保费取得的现金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000565000, 566, '现金流量表_收到再保险业务现金净额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000566000, 567, '现金流量表_保户储金及投资款净增加额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000567000, 568, '现金流量表_处置以公允价值计量且其变动计入当期损益的金融资产净增加额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000568000, 569, '现金流量表_收取利息、手续费及佣金的现金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000569000, 570, '现金流量表_拆入资金净增加额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000570000, 571, '现金流量表_回购业务资金净增加额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000571000, 572, '现金流量表_客户贷款及垫款净增加额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000572000, 573, '现金流量表_存放中央银行和同业款项净增加额（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000573000, 574, '现金流量表_支付原保险合同赔付款项的现金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000574000, 575, '现金流量表_支付利息、手续费及佣金的现金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000575000, 576, '现金流量表_支付保单红利的现金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000576000, 577, '现金流量表_其中:子公司吸收少数股东投资收到的现金（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000577000, 578, '现金流量表_其中:子公司支付给少数股东的股利、利润（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000578000, 579, '现金流量表_投资性房地产的折旧及摊销（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000579000, 580, '现金流量表_信用减值损失（万元）');
INSERT INTO hku_base.n_historyfinancefield (`ts`, `id`, `name`) VALUES(631152000580000, 581, '使用权资产折旧（万元）');

CREATE STABLE IF NOT EXISTS hku_base.s_historyfinance (
    file_date TIMESTAMP,
    report_date INT,
    `values` VARBINARY(5000)
) TAGS (market_code VARCHAR(60));

CREATE STABLE IF NOT EXISTS hku_base.s_stkfinance (
	`updated_date` TIMESTAMP, 
	`ipo_date` INT,       --42.上市日期
	`province` DOUBLE, 
    `industry` DOUBLE,
	`zongguben` DOUBLE,      --1.总股本(股)
    `liutongguben` DOUBLE,   --7.流通A股（股）
	`guojiagu` DOUBLE,       --2.国家股（股）
	`faqirenfarengu` DOUBLE, --3.发起人法人股（股）
	`farengu` DOUBLE,        --4.法人股（股）
	`bgu` DOUBLE,            --5.B股（股）
	`hgu` DOUBLE,            --6.H股（股）
	`zhigonggu` DOUBLE,      --8.职工股（股）
	`zongzichan` DOUBLE,     --10.总资产（元）
	`liudongzichan` DOUBLE,  --11.流动资产（元）
    `gudingzichan` DOUBLE,   --12.固定资产（元）
	`wuxingzichan` DOUBLE,   --13.无形资产（元）
	`gudongrenshu` DOUBLE,   --股东人数
	`liudongfuzhai` DOUBLE,  --15.流动负债
	`changqifuzhai` DOUBLE,  --16.长期负债
	`zibengongjijin` DOUBLE, --17.资本公积金
	`jingzichan` DOUBLE,     --净资产（元）
	`zhuyingshouru` DOUBLE,  --20.主营收入
	`zhuyinglirun` DOUBLE,   --21.主营利润
	`yingshouzhangkuan` DOUBLE, --应收账款
	`yingyelirun` DOUBLE,    --23.营业利润
	`touzishouyu` DOUBLE,    --投资收益
	`jingyingxianjinliu` DOUBLE, --经营现金流
	`zongxianjinliu` DOUBLE,     --总现金流
	`cunhuo` DOUBLE,             --存货
	`lirunzonghe` DOUBLE,    --28.利润总额
	`shuihoulirun` DOUBLE,   --29.税后利润
	`jinglirun` DOUBLE,      --30.净利润
	`weifenpeilirun` DOUBLE, --31.未分配利润
	`meigujingzichan` DOUBLE, --34.每股净资产
	`baoliu2` DOUBLE
) TAGS (stockid TIMESTAMP);

CREATE STABLE IF NOT EXISTS hku_base.s_holiday (
	`id` TIMESTAMP,
    `date` INT
) TAGS (market VARCHAR(10));
CREATE TABLE hku_base.z_zh_holiday using hku_base.s_holiday (market) tags ('SH');
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000000000, 20210101);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000001000, 20210211);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000002000, 20210212);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000003000, 20210215);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000004000, 20210216);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000005000, 20210217);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000006000, 20210405);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000007000, 20210501);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000008000, 20210503);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000009000, 20210504);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000010000, 20210505);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000011000, 20210614);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000012000, 20210920);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000013000, 20210921);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000014000, 20211001);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000015000, 20211004);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000016000, 20211005);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000017000, 20211006);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000018000, 20211007);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000019000, 20220103);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000020000, 20220131);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000021000, 20220201);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000022000, 20220202);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000023000, 20220203);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000024000, 20220204);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000025000, 20220205);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000026000, 20220404);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000027000, 20220405);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000028000, 20220502);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000029000, 20220503);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000030000, 20220504);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000031000, 20220603);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000032000, 20220912);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000033000, 20221003);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000034000, 20221004);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000035000, 20221005);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000036000, 20221006);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000037000, 20221007);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000038000, 20230102);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000039000, 20230123);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000040000, 20230124);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000041000, 20230125);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000042000, 20230126);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000043000, 20230127);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000044000, 20230405);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000045000, 20230501);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000046000, 20230502);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000047000, 20230503);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000048000, 20230622);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000049000, 20230623);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000050000, 20230929);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000051000, 20231002);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000052000, 20231003);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000053000, 20231004);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000054000, 20231005);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000055000, 20231006);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000056000, 20240101);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000057000, 20240212);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000058000, 20240213);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000059000, 20240214);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000060000, 20240215);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000061000, 20240216);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000062000, 20240404);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000063000, 20240405);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000064000, 20240501);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000065000, 20240502);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000066000, 20240503);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000067000, 20240610);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000068000, 20240916);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000069000, 20240917);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000070000, 20241001);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000071000, 20241002);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000072000, 20241003);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000073000, 20241004);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000074000, 20241007);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000075000, 20250101);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000076000, 20250128);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000077000, 20250129);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000078000, 20250130);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000079000, 20250131);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000080000, 20250203);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000081000, 20250204);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000082000, 20250404);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000083000, 20250501);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000084000, 20250502);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000085000, 20250505);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000086000, 20250602);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000087000, 20251001);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000088000, 20251002);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000089000, 20251003);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000090000, 20251006);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000091000, 20251007);
INSERT INTO hku_base.z_zh_holiday (`id`, `date`) VALUES (631152000092000, 20251008);

CREATE DATABASE IF NOT EXISTS hku_data PRECISION 'us' KEEP 365000 WAL_LEVEL 2;
CREATE STABLE IF NOT EXISTS hku_data.kdata (
    `date` timestamp,
    `open` DOUBLE,
    `high` DOUBLE,
    `low` DOUBLE,
    `close` DOUBLE,
    `amount` DOUBLE,
    `volume` DOUBLE
) TAGS (market VARCHAR(10), code VARCHAR(20), ktype VARCHAR(10));
CREATE STABLE IF NOT EXISTS hku_data.timeline (
	`date` timestamp, 
	`price` DOUBLE, 
	`vol` DOUBLE
) TAGS (market VARCHAR(10), code VARCHAR(20));
CREATE STABLE IF NOT EXISTS hku_data.transdata (
	`date` timestamp, 
	`price` DOUBLE, 
	`vol` DOUBLE, 
	`direct` int
) TAGS (market VARCHAR(10), code VARCHAR(20));