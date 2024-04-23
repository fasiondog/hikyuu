ALTER TABLE `hku_base`.`coderuletype` modify `id` int(11)  auto_increment;
INSERT INTO `hku_base`.`coderuletype` (`marketid`,`codepre`,`type`,`description`) VALUES (3,'92',11,'北证A股');
INSERT INTO `hku_base`.`coderuletype` (`marketid`,`codepre`,`type`,`description`) VALUES (1,'880',2,'通达信板块指数');
UPDATE `hku_base`.`version` set `version` = 15;