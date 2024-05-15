BEGIN TRANSACTION;
DELETE FROM `stkWeight`;
UPDATE sqlite_sequence SET seq = 0 WHERE name='stkWeight';
UPDATE `version` set `version` = 18;
COMMIT;