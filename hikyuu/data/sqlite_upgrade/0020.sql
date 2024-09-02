BEGIN TRANSACTION;
UPDATE `coderuletype` SET `codepre`=150 WHERE `codepre`=15 AND `marketid`=2;
UPDATE `version` set `version` = 20;
COMMIT;