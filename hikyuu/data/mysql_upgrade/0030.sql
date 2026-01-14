update `hku_base`.`stocktypeinfo` set `minTradeNumber`=200 where `type`=9 and `description`='科创板';
update `hku_base`.`stocktypeinfo` set `minTradeNumber`=100 where `type`=11 and `description`='北交所';
UPDATE `hku_base`.`version` set `version` = 30;