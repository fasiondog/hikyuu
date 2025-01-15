BEGIN TRANSACTION;
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880001', '通达信总市值', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880001'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880002', '通达信流通市值', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880002'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880003', '通达信平均股价', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880003'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880004', '通达信成交均价', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880004'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880005', '通达信涨跌家数', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880005'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880006', '通达信停板家数', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880006'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880006', '通达信全A等权', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880008'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880011', '通达信主板总值', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880011'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880031', '通达信创业总值', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880031'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880041', '通达信科创总值', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880041'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880098', '通达信可转债指数', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880098'
);
INSERT INTO `Stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 1, '880099', '通达信逆回购', 2, 1, 20160126, 99999999
WHERE NOT EXISTS (
    SELECT stockid FROM `Stock` WHERE `marketid`=1 and `code`='880099'
);
UPDATE `version` set `version` = 25;
COMMIT;