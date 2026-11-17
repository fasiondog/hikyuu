BEGIN TRANSACTION;
INSERT INTO `coderuletype` (`marketid`, `codepre`, `type`, `description`) SELECT 1, "52", 5, "上证ETF" WHERE NOT EXISTS (SELECT 1 FROM `coderuletype` WHERE `marketid`=1 AND `codepre`="52");
INSERT INTO `coderuletype` (`marketid`, `codepre`, `type`, `description`) SELECT 1, "53", 5, "上证ETF" WHERE NOT EXISTS (SELECT 1 FROM `coderuletype` WHERE `marketid`=1 AND `codepre`="53");
INSERT INTO `coderuletype` (`marketid`, `codepre`, `type`, `description`) SELECT 1, "55", 5, "上证ETF" WHERE NOT EXISTS (SELECT 1 FROM `coderuletype` WHERE `marketid`=1 AND `codepre`="55");
INSERT INTO `coderuletype` (`marketid`, `codepre`, `type`, `description`) SELECT 2, "158", 5, "深证ETF" WHERE NOT EXISTS (SELECT 1 FROM `coderuletype` WHERE `marketid`=2 AND `codepre`="158");
UPDATE `version` set `version` = 33;
COMMIT;
