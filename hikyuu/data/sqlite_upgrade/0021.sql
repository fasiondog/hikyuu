BEGIN TRANSACTION;
UPDATE `market` SET `closeTime1`=1135, `closeTime2`=1505 WHERE `marketid`=1;
UPDATE `market` SET `closeTime1`=1135, `closeTime2`=1505 WHERE `marketid`=2;
UPDATE `market` SET `closeTime1`=1135, `closeTime2`=1505 WHERE `marketid`=3;
UPDATE `version` set `version` = 21;
COMMIT;