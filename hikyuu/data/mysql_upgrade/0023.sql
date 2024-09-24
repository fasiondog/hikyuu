DELETE FROM `hku_base`.`stkweight`;
alter table `hku_base`.`stkweight` add `suogu` DOUBLE not null default 0;
ALTER TABLE `hku_base`.`stkweight` AUTO_INCREMENT = 1;
UPDATE `hku_base`.`version` set `version` = 23;