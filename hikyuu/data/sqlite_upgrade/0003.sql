BEGIN TRANSACTION;
alter table StockTypeInfo rename to StockTypeInfoOld;
CREATE TABLE `stocktypeinfo` (
	`tickValue`	NUMERIC,
	`precision`	INTEGER,
	`id`	INTEGER,
	`type`	INTEGER,
	`description`	VARCHAR(100),
	`tick`	NUMERIC,
	`minTradeNumber`	NUMERIC,
	`maxTradeNumber`	NUMERIC,
	PRIMARY KEY(`id`)
);
insert into StockTypeInfo (id, type, description, tick, tickValue, precision, minTradeNumber, maxTradeNumber) select id, type, description, tick, tickValue, precision, minTradeNumber, maxTradeNumber from StockTypeInfoOld;
drop table StockTypeInfoOld;

UPDATE `version` set `version` = 3;
COMMIT;
