BEGIN TRANSACTION;

INSERT INTO `coderuletype` (marketid,codepre,type,description) VALUES (3,'92',11,'北证A股');
INSERT INTO `coderuletype` (marketid,codepre,type,description) VALUES (1,'880',2,'通达信板块指数');

UPDATE `version` set `version` = 16;

COMMIT;