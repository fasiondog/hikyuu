INSERT INTO `hku_base`.`market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`) VALUES (3, "BJ", "北京证券交易所", "北京市场", "830799", 20200727, 930, 1130, 1300, 1500);
INSERT INTO `hku_base`.`coderuletype` (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (35, 3, "43", 1, "北证A股");
INSERT INTO `hku_base`.`coderuletype` (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (36, 3, "83", 1, "北证A股");
INSERT INTO `hku_base`.`coderuletype` (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (37, 3, "87", 1, "北证A股");
INSERT INTO `hku_base`.`coderuletype` (`id`, `marketid`, `codepre`, `type`, `description`) VALUES (38, 3, "88", 1, "北证A股");
UPDATE `hku_base`.`version` set `version` = 8;