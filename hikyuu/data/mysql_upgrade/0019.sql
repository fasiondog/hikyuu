UPDATE `hku_base`.`coderuletype` SET `codepre`=510 WHERE `codepre`=51 AND `marketid`=1;
UPDATE `hku_base`.`coderuletype` SET `codepre`=500 WHERE `codepre`=50 AND `marketid`=1;
UPDATE `hku_base`.`coderuletype` SET `codepre`=200 WHERE `codepre`=20 AND `marketid`=2;
INSERT INTO `hku_base`.`coderuletype` (`marketid`, `codepre`, `type`, `description`) VALUES (1, "56", 5, "上证ETF");
INSERT INTO `hku_base`.`coderuletype` (`marketid`, `codepre`, `type`, `description`) VALUES (1, "58", 5, "上证ETF");
UPDATE `hku_base`.`version` set `version` = 19;