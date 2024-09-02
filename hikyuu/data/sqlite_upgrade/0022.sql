BEGIN TRANSACTION;
UPDATE `market` SET `closeTime1`=1130, `closeTime2`=1500 WHERE `marketid`=1;
UPDATE `market` SET `closeTime1`=1130, `closeTime2`=1500 WHERE `marketid`=2;
UPDATE `market` SET `closeTime1`=1130, `closeTime2`=1500 WHERE `marketid`=3;
UPDATE `version` set `version` = 22;
COMMIT;