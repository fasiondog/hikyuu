BEGIN TRANSACTION;
DELETE FROM `stkWeight`;
UPDATE `coderuletype` SET `codepre`=51 WHERE `codepre`=510 AND `marketid`=1;
UPDATE `coderuletype` SET `codepre`=50 WHERE `codepre`=500 AND `marketid`=1;
UPDATE `coderuletype` SET `codepre`=20 WHERE `codepre`=200 AND `marketid`=2;
UPDATE `version` set `version` = 23;
COMMIT;