UPDATE `hku_base`.`stocktypeinfo` SET `minTradeNumber`=1 where `id`=9;
INSERT INTO `hku_base`.`stocktypeinfo` (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (11, 11, 2, 0.01, 0.01, 1, 1000000, '北交所');
UPDATE `hku_base`.`stock` set `type`=1 where `marketid`=3;

UPDATE `hku_base`.`version` set `version` = 12;