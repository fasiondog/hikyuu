BEGIN TRANSACTION;
CREATE TABLE "stkWeight" (
	id INTEGER NOT NULL, 
	stockid INTEGER, 
	date INTEGER, 
	"countAsGift" INTEGER, 
	"countForSell" INTEGER, 
	"priceForSell" INTEGER, 
	bonus INTEGER, 
	"countOfIncreasement" INTEGER, 
	"totalCount" INTEGER, 
	"freeCount" INTEGER, 
	PRIMARY KEY (id), 
	 FOREIGN KEY(stockid) REFERENCES "Stock" (stockid)
);
CREATE TABLE StockTypeInfo (tickValue NUMERIC, precision INTEGER, id INTEGER PRIMARY KEY, type INTEGER, description VARCHAR(100), tick NUMERIC, minTradeNumber INTEGER, maxTradeNumber INTEGER);
INSERT INTO `StockTypeInfo` (tickValue,precision,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (100,2,0,0,'Block',100,100,1000000);
INSERT INTO `StockTypeInfo` (tickValue,precision,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.01,2,1,1,'A股',0.01,100,1000000);
INSERT INTO `StockTypeInfo` (tickValue,precision,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.001,3,2,2,'指数',0.001,1,1000000);
INSERT INTO `StockTypeInfo` (tickValue,precision,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.001,3,3,3,'B股',0.001,100,1000000);
INSERT INTO `StockTypeInfo` (tickValue,precision,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.001,3,4,4,'基金（不含ETF）',0.001,100,1000000);
INSERT INTO `StockTypeInfo` (tickValue,precision,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.001,3,5,5,'ETF',0.001,1000,1000000);
INSERT INTO `StockTypeInfo` (tickValue,precision,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.01,2,6,6,'国债',0.01,10,10000);
INSERT INTO `StockTypeInfo` (tickValue,precision,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.01,2,7,7,'其他债券',0.01,10,10000);
INSERT INTO `StockTypeInfo` (tickValue,precision,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.01,2,8,8,'创业板',0.01,100,1000000);
CREATE TABLE Stock (stockid INTEGER PRIMARY KEY, marketid INTEGER, code VARCHAR(20), name VARCHAR(60), type INTEGER, valid INTEGER, startDate INTEGER, endDate INTEGER);
CREATE TABLE Market (marketid INTEGER PRIMARY KEY, market VARCHAR(10), name VARCHAR(60), description VARCHAR(100), code VARCHAR(20), lastDate INTEGER);
INSERT INTO `Market` (marketid,market,name,description,code,lastDate) VALUES (1,'SH','上海证劵交易所','上海市场','000001',19901219);
INSERT INTO `Market` (marketid,market,name,description,code,lastDate) VALUES (2,'SZ','深圳证劵交易所','深圳市场','399001',19901219);
CREATE TABLE "LastDate" (
	id INTEGER NOT NULL, 
	date INTEGER, 
	PRIMARY KEY (id)
);
INSERT INTO `LastDate` (id,date) VALUES (1,19901219);
CREATE TABLE CodeRuleType (id INTEGER PRIMARY KEY, marketid INTEGER, codepre VARCHAR(20), type INTEGER, description VARCHAR(100));
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (1,1,'000',2,'上证指数');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (2,1,'600',1,'上证A股');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (3,1,'601',1,'上证A股');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (4,1,'900',3,'上证B股');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (5,2,'00',1,'深证A股');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (6,2,'20',3,'深证B股');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (7,2,'39',2,'深证指数');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (8,2,'150',4,'深证基金');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (9,2,'16',4,'深证基金');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (10,2,'18',4,'深证基金');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (11,2,'159',5,'深证ETF');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (12,1,'51',5,'上证ETF');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (13,1,'50',4,'上证基金');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (18,2,'300',8,'创业板');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (19,1,'603',1,'上证A股');
INSERT INTO `CodeRuleType` (id,marketid,codepre,type,description) VALUES (20,1,'519',4,'上证基金');
CREATE INDEX "ix_stkWeight_stockid" ON "stkWeight" (stockid);
CREATE INDEX "ix_stkWeight_date" ON "stkWeight" (date);
CREATE INDEX "ix_stkWeight" ON "stkWeight" (stockid, date);
COMMIT;
