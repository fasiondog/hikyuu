BEGIN TRANSACTION;
update `stocktypeinfo` set `minTradeNumber`=100 where `type`=5 and `description`='ETF';
UPDATE `version` set `version` = 29;
COMMIT;