BEGIN TRANSACTION;

UPDATE `StockTypeInfo` set `minTradeNumber`=1 where `id`=9;
INSERT INTO "StockTypeInfo" ("tickValue", "precision", "id", "type", "description", "tick", "minTradeNumber", "maxTradeNumber") VALUES ('0.01', '2', '11', '11', '北交所', '0.01', '1', '1000000');
UPDATE `stock` set `type`=11 where `marketid`=3;

UPDATE `version` set `version` = 13;

COMMIT;