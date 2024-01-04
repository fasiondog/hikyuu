CREATE TABLE
    IF NOT EXISTS `hku_base`.`block` (
        `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
        `category` VARCHAR(100) NOT NULL,
        `name` VARCHAR(100) NOT NULL,
        `market_code` VARCHAR(30) NOT NULL,
        PRIMARY KEY (`id`),
        INDEX `ix_block` (`category`, `name`)
    ) COLLATE = 'utf8_general_ci' ENGINE = InnoDB;

UPDATE `hku_base`.`version` set `version` = 10;