BEGIN TRANSACTION;
CREATE TABLE Block (blockid INTEGER PRIMARY KEY, name VARCHAR(60), category VARCHAR(60), description VARCHAR(100), valid INTEGER, startDate INTEGER, endDate INTEGER);
INSERT INTO Block VALUES(1,'全部A股','大盘','',1,19901219,99999999);
INSERT INTO Block VALUES(2,'上证A股','大盘','',1,19901219,99999999);
INSERT INTO Block VALUES(3,'深证A股','大盘','',1,19901219,99999999);
INSERT INTO Block VALUES(4,'ETF基金','基金','',1,20050223,99999999);
INSERT INTO Block VALUES(5,'LOF基金','基金','',1,19931210,99999999);
INSERT INTO Block VALUES(6,'全部指数','指数','',1,19901219,99999999);
INSERT INTO Block VALUES(7,'大盘指数','指数','',1,19901219,99999999);
INSERT INTO Block VALUES(8,'上证行业指数','指数','上海交易所制定的行业指数',1,19941206,99999999);
INSERT INTO Block VALUES(9,'深证行业指数','指数','深圳交易所制定的行业指数',1,20010702,99999999);
INSERT INTO Block VALUES(10,'全部B股','大盘','',1,19920221,99999999);
INSERT INTO Block VALUES(11,'上证B股','大盘','',1,19920221,99999999);
INSERT INTO Block VALUES(12,'深证B股','大盘','',1,19930111,99999999);

CREATE TABLE "BlockInfo" (
            id INTEGER NOT NULL, 
            blockid INTEGER, 
            stockid INTEGER, 
            valid INTEGER, 
            "startDate" INTEGER, 
            "endDate" INTEGER, 
            PRIMARY KEY (id), 
             FOREIGN KEY(blockid) REFERENCES "Block" (blockid), 
             FOREIGN KEY(stockid) REFERENCES "Stock" (stockid)
        );

CREATE TABLE CodeRuleType (id INTEGER PRIMARY KEY, marketid INTEGER, codepre VARCHAR(20), type INTEGER, description VARCHAR(100));
INSERT INTO CodeRuleType VALUES(1,1,000,2,'上证指数');
INSERT INTO CodeRuleType VALUES(2,1,600,1,'上证A股');
INSERT INTO CodeRuleType VALUES(3,1,601,1,'上证A股');
INSERT INTO CodeRuleType VALUES(4,1,900,3,'上证B股');
INSERT INTO CodeRuleType VALUES(5,2,00,1,'深证A股');
INSERT INTO CodeRuleType VALUES(6,2,20,3,'深证B股');
INSERT INTO CodeRuleType VALUES(7,2,39,2,'深证指数');
INSERT INTO CodeRuleType VALUES(8,2,15,4,'深证基金');
INSERT INTO CodeRuleType VALUES(9,2,16,4,'深证基金');
INSERT INTO CodeRuleType VALUES(10,2,18,4,'深证基金');
INSERT INTO CodeRuleType VALUES(11,2,1599,5,'深证ETF');
INSERT INTO CodeRuleType VALUES(12,1,510,5,'上证ETF');
INSERT INTO CodeRuleType VALUES(13,1,500,4,'上证基金');
INSERT INTO CodeRuleType VALUES(14,2,10,6,'深证国债');
INSERT INTO CodeRuleType VALUES(15,1,0,6,'上证国债');
INSERT INTO CodeRuleType VALUES(16,1,12,7,'上证债券（不含国债）');
INSERT INTO CodeRuleType VALUES(17,2,11,7,'深证债券（不含国债）');
INSERT INTO CodeRuleType VALUES(18,2,300,8,'创业板');

CREATE TABLE "LastDate" (
	id INTEGER NOT NULL, 
	date INTEGER, 
	PRIMARY KEY (id)
);

CREATE TABLE Market (marketid INTEGER PRIMARY KEY, market VARCHAR(10), name VARCHAR(60), description VARCHAR(100), code VARCHAR(20), lastDate INTEGER);
INSERT INTO Market VALUES(1,'SH','上海证劵交易所','上海市场',000001,20130115);
INSERT INTO Market VALUES(2,'SZ','深圳证劵交易所','深圳市场',399001,20130115);

CREATE TABLE Stock (stockid INTEGER PRIMARY KEY, marketid INTEGER, code VARCHAR(20), name VARCHAR(60), type INTEGER, valid INTEGER, startDate INTEGER, endDate INTEGER);

CREATE TABLE StockTypeInfo (precision INTEGER, id INTEGER PRIMARY KEY, type INTEGER, description VARCHAR(100), tick NUMERIC, minTradeNumber INTEGER, maxTradeNumber INTEGER);
INSERT INTO StockTypeInfo VALUES(2,0,0,'Block',100,100,1000000);
INSERT INTO StockTypeInfo VALUES(2,1,1,'A股',0.01,100,1000000);
INSERT INTO StockTypeInfo VALUES(3,2,2,'指数',0.001,1,1000000);
INSERT INTO StockTypeInfo VALUES(3,3,3,'B股',0.001,100,1000000);
INSERT INTO StockTypeInfo VALUES(3,4,4,'基金（不含ETF）',0.001,100,1000000);
INSERT INTO StockTypeInfo VALUES(3,5,5,'ETF',0.001,1000,1000000);
INSERT INTO StockTypeInfo VALUES(2,6,6,'国债',0.01,10,10000);
INSERT INTO StockTypeInfo VALUES(2,7,7,'其他债券',0.01,10,10000);
INSERT INTO StockTypeInfo VALUES(2,8,8,'创业板',0.01,100,1000000);

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

CREATE INDEX "index_stkWeight" ON "stkWeight" (stockid, date);
CREATE INDEX "ix_BlockInfo_blockid" ON "BlockInfo" (blockid);
CREATE INDEX "ix_BlockInfo_stockid" ON "BlockInfo" (stockid);
CREATE INDEX ix_Block_blockid ON Block(blockid ASC);
CREATE INDEX "ix_stkWeight_date" ON "stkWeight" (date);
CREATE INDEX "ix_stkWeight_stockid" ON "stkWeight" (stockid);
COMMIT;
