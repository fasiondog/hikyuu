BEGIN TRANSACTION;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880006', '通达信大盘涨跌停数', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880006'
);
UPDATE `version` set `version` = 25;
COMMIT;