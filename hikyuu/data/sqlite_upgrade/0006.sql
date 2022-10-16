BEGIN TRANSACTION;
INSERT INTO `stocktypeinfo` (tickValue,precision,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.01,2,9,'科创板',0.01,100,1000000);
INSERT INTO `coderuletype` (marketid,codepre,type,description) VALUES (1,'688',9,'科创板');
UPDATE `version` set `version` = 6;
COMMIT;