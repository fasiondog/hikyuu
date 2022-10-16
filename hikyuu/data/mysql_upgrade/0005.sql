INSERT INTO `hku_base`.`stocktypeinfo` (`id`, `tickValue`,`precision`,`type`,`description`,`tick`,`minTradeNumber`,`maxTradeNumber`) VALUES (9, 0.01,2,9,'科创板',0.01,100,1000000);
INSERT INTO `hku_base`.`coderuletype` (`id`,`marketid`,`codepre`,`type`,`description`) VALUES (33, 1,'688',9,'科创板');
UPDATE `hku_base`.`version` set `version` = 5;