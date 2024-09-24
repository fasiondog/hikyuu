BEGIN TRANSACTION;
DROP TABLE `stkWeight`;
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
    "suogu" NUMBER,
	PRIMARY KEY (id AUTOINCREMENT),
	 FOREIGN KEY(stockid) REFERENCES "Stock" (stockid)
);
CREATE INDEX "ix_stkWeight_stockid" ON "stkWeight" (stockid);
CREATE INDEX "ix_stkWeight_date" ON "stkWeight" (date);
CREATE INDEX "ix_stkWeight" ON "stkWeight" (stockid, date);
UPDATE `version` set `version` = 24;
COMMIT;