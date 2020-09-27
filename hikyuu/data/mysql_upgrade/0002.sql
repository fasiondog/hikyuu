alter table `hku_base`.`stocktypeinfo` modify column `minTradeNumber` double not null default 1;
alter table `hku_base`.`stocktypeinfo` modify column `maxTradeNumber` double not null default 1;
INSERT INTO `hku_base`.`version` (version) VALUES (2);
