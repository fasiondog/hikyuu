BEGIN TRANSACTION;
INSERT INTO `coderuletype` (marketid,codepre,type,description) VALUES (1,'688',9,'科创板');
UPDATE `version` set `version` = 31;
COMMIT;