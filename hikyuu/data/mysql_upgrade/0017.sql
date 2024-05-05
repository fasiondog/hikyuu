delete from `hku_base`.`stkweight` where 1=1;
alter table `hku_base`.`stkweight` AUTO_INCREMENT=1;
alter table `hku_base`.`stkweight` modify column `countAsGift` DOUBLE not null default 0;
alter table `hku_base`.`stkweight` modify column `countForSell` DOUBLE not null default 0;
UPDATE `hku_base`.`version` set `version` = 17;
