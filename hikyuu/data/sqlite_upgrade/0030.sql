BEGIN TRANSACTION;
update `stocktypeinfo` set `minTradeNumber`=200 where `type`=9 and `description`='科创板';
update `stocktypeinfo` set `minTradeNumber`=100 where `type`=11 and `description`='北交所';
UPDATE `version` set `version` = 30;
COMMIT;