ALTER TABLE `hku_base`.`stocktypeinfo` 
UPDATE `minTradeNumber` = CASE
    WHEN `type` = 9 AND `description` = '科创板' THEN 200
    WHEN `type` = 11 AND `description` = '北交所' THEN 100
    ELSE `minTradeNumber`
END
WHERE (`type` = 9 AND `description` = '科创板') 
   OR (`type` = 11 AND `description` = '北交所');
ALTER TABLE `hku_base`.`version` UPDATE `version`=2 WHERE `id`=0;