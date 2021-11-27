BEGIN TRANSACTION;
INSERT INTO `CodeRuleType` (marketid,codepre,type,description) VALUES (2,'301',8,'创业板');
UPDATE `version` set `version` = 7;
COMMIT;