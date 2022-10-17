BEGIN TRANSACTION;
INSERT INTO `coderuletype` (`marketid`, `codepre`, `type`, `description`) VALUES (1, "56", 5, "上证ETF");
INSERT INTO `coderuletype` (`marketid`, `codepre`, `type`, `description`) VALUES (1, "58", 5, "上证ETF");
UPDATE `version` set `version` = 10;
COMMIT;