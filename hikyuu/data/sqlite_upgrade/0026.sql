BEGIN TRANSACTION;
INSERT INTO `coderuletype` (marketid,codepre,type,description) VALUES (3,'899',2,'北证指数');
UPDATE `market` SET `code`='899050' WHERE `marketid`=3;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 3, '899050', '北证50', 2, 1, 20220429, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=3 and `code`='899050'
);
UPDATE `version` set `version` = 26;
COMMIT;