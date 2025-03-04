INSERT INTO `hku_base`.`coderuletype` (`marketid`, `codepre`, `type`, `description`) 
SELECT 2, '302', 8, '创业板'
WHERE NOT EXISTS (
    SELECT id FROM `hku_base`.`coderuletype` WHERE `codepre`=302
);
UPDATE `hku_base`.`version` set `version` = 27;