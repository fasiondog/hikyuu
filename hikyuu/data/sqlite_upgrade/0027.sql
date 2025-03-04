BEGIN TRANSACTION;
INSERT INTO `CodeRuleType` (`marketid`, `codepre`, `type`, `description`) 
SELECT 2, '302', 8, '创业板'
WHERE NOT EXISTS (
    SELECT id FROM `CodeRuleType` WHERE `codepre`=302
);
UPDATE `version` set `version` = 27;
COMMIT;