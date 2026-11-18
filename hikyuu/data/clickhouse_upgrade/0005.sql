INSERT INTO `hku_base`.`coderuletype` (`id`, `market`, `codepre`, `type`, `description`)
SELECT *
FROM (SELECT 48 AS id, 'SH' AS market, '52' AS codepre, 5 AS type, '上证ETF' AS description)
WHERE NOT EXISTS (SELECT * FROM `hku_base`.`coderuletype` WHERE `market`='SH' AND `codepre`='52');

INSERT INTO `hku_base`.`coderuletype` (`id`, `market`, `codepre`, `type`, `description`)
SELECT *
FROM (SELECT 49 AS id, 'SH' AS market, '53' AS codepre, 5 AS type, '上证ETF' AS description)
WHERE NOT EXISTS (SELECT * FROM `hku_base`.`coderuletype` WHERE `market`='SH' AND `codepre`='53');

INSERT INTO `hku_base`.`coderuletype` (`id`, `market`, `codepre`, `type`, `description`)
SELECT *
FROM (SELECT 50 AS id, 'SH' AS market, '55' AS codepre, 5 AS type, '上证ETF' AS description)
WHERE NOT EXISTS (SELECT * FROM `hku_base`.`coderuletype` WHERE `market`='SH' AND `codepre`='55');

INSERT INTO `hku_base`.`coderuletype` (`id`, `market`, `codepre`, `type`, `description`)
SELECT *
FROM (SELECT 51 AS id, 'SZ' AS market, '158' AS codepre, 5 AS type, '深证ETF' AS description)
WHERE NOT EXISTS (SELECT * FROM `hku_base`.`coderuletype` WHERE `market`='SZ' AND `codepre`='158');

ALTER TABLE `hku_base`.`version` UPDATE `version`=5 WHERE `id`=0 SETTINGS mutations_sync = 1;
