DELETE FROM `hku_base`.`stkweight`;
UPDATE `hku_base`.`coderuletype` SET `codepre`=51 WHERE `codepre`=510 AND `marketid`=1;
UPDATE `hku_base`.`coderuletype` SET `codepre`=50 WHERE `codepre`=500 AND `marketid`=1;
UPDATE `hku_base`.`coderuletype` SET `codepre`=20 WHERE `codepre`=200 AND `marketid`=2;
UPDATE `hku_base`.`version` set `version` = 22;