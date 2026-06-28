BEGIN TRANSACTION;
DELETE FROM `Stock` WHERE `marketid`=1 and `code`='880006';
DELETE FROM `Stock` WHERE `marketid`=1 and `code`='880008';
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880006', '通达信停板家数', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880006'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880008', '通达信全A等权', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880008'
);
UPDATE `version` set `version` = 32;
COMMIT;