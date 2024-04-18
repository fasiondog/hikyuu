BEGIN TRANSACTION;

UPDATE `Stock` set `type`=11 where `marketid`=3;
UPDATE `CodeRuleType` set `type`=11 where `marketid`=3;
UPDATE `version` set `version` = 15;

COMMIT;