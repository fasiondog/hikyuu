alter table `hku_base`.`stocktypeinfo` modify column `minTradeNumber` double not null default 1;
alter table `hku_base`.`stocktypeinfo` modify column `maxTradeNumber` double not null default 1;
UPDATE `hku_base`.`version` set `version` = 2;
