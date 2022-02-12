BEGIN TRANSACTION;
ALTER TABLE `Market` ADD `openTime1` INTEGER;
ALTER TABLE `Market` ADD `closeTime1` INTEGER;
ALTER TABLE `Market` ADD `openTime2` INTEGER;
ALTER TABLE `Market` ADD `closeTime2` INTEGER;

UPDATE `Market` SET `openTime1`=930, `closeTime1`=1130, `openTime2`=1300, `closeTime2`=1500 WHERE `marketid`=1;
UPDATE `Market` SET `openTime1`=930, `closeTime1`=1130, `openTime2`=1300, `closeTime2`=1500 WHERE `marketid`=2;

UPDATE `version` set `version` = 4;
COMMIT;
