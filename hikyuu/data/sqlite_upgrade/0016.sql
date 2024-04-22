BEGIN TRANSACTION;

INSERT INTO `coderuletype` (marketid,codepre,type,description) VALUES (3,'92',11,'北证A股');
UPDATE `version` set `version` = 16;

COMMIT;