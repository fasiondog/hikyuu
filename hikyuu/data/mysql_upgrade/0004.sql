CREATE TABLE IF NOT EXISTS `hku_base`.`holiday` (
	`id`	INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`date` BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY(`id`)
);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210101);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210211);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210212);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210215);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210216);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210217);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210405);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210501);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210503);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210504);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210505);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210614);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210920);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20210921);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20211001);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20211004);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20211005);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20211006);
INSERT INTO `hku_base`.`holiday` (`date`) VALUES (20211007);

UPDATE `hku_base`.`version` set `version` = 4;