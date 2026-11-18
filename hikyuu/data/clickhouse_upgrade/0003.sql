INSERT INTO `hku_base`.`coderuletype` (`id`, `market`, `codepre`, `type`, `description`)
SELECT * FROM (SELECT 47 AS id, 'SH' AS market, '689' AS codepre, 9 AS type, '科创板' AS description)
WHERE NOT EXISTS (SELECT * FROM `hku_base`.`coderuletype` WHERE `id`=47);
ALTER TABLE `hku_base`.`version` UPDATE `version`=3 WHERE `id`=0 SETTINGS mutations_sync = 1;