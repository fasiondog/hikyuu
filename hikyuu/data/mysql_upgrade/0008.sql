INSERT INTO `hku_base`.`Market` (`marketid`, `market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`) VALUES (3, "BJ", "北京证券交易所", "北京市场", "830799", 20200727, 930, 1130, 1300, 1500);
INSERT INTO `hku_base`.`CodeRuleType` (`marketid`, `codepre`, `type`, `description`) VALUES (3, "43", 1, "北证A股");
INSERT INTO `hku_base`.`CodeRuleType` (`marketid`, `codepre`, `type`, `description`) VALUES (3, "83", 1, "北证A股");
INSERT INTO `hku_base`.`CodeRuleType` (`marketid`, `codepre`, `type`, `description`) VALUES (3, "87", 1, "北证A股");
INSERT INTO `hku_base`.`CodeRuleType` (`marketid`, `codepre`, `type`, `description`) VALUES (3, "88", 1, "北证A股");
UPDATE `hku_base`.`version` set `version` = 8;