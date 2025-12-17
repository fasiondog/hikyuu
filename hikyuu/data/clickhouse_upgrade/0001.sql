ALTER TABLE `hku_base`.`stocktypeinfo` UPDATE `minTradeNumber`=100 WHERE `type`=5 AND `description`='ETF';
ALTER TABLE `hku_base`.`version` UPDATE `version`=1 WHERE `id`=0;