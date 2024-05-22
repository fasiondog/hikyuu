BEGIN TRANSACTION;
DELETE FROM `stkWeight`;
UPDATE `version` set `version` = 19;
COMMIT;