CREATE SCHEMA `hku_base`;

CREATE TABLE `hku_base`.`market` (
	`marketid` INT(10) UNSIGNED NOT NULL,
	`market` VARCHAR(10) NULL DEFAULT NULL,
	`name` VARCHAR(60) NULL DEFAULT NULL,
	`description` VARCHAR(100) NULL DEFAULT NULL,
	`code` VARCHAR(20) NULL DEFAULT NULL,
	`lastDate` BIGINT(20) UNSIGNED NULL DEFAULT NULL,
	PRIMARY KEY (`marketid`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;
INSERT INTO `hku_base`.`market` (marketid,market,name,description,code,lastDate) VALUES (1,'SH','上海证券交易所','上海市场','000001',19901219);
INSERT INTO `hku_base`.`market` (marketid,market,name,description,code,lastDate) VALUES (2,'SZ','深圳证券交易所','深圳市场','399001',19901219);


CREATE TABLE `hku_base`.`stkweight` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`stockid` INT(10) UNSIGNED NOT NULL,
	`date` BIGINT(20) UNSIGNED NOT NULL,
	`countAsGift` DOUBLE UNSIGNED NOT NULL,
	`countForSell` DOUBLE UNSIGNED NOT NULL,
	`priceForSell` DOUBLE UNSIGNED NOT NULL,
	`bonus` DOUBLE UNSIGNED NOT NULL,
	`countOfIncreasement` DOUBLE UNSIGNED NOT NULL,
	`totalCount` DOUBLE UNSIGNED NOT NULL,
	`freeCount` DOUBLE UNSIGNED NOT NULL,
	PRIMARY KEY (`id`),
	INDEX `ix_stockid_date` (`stockid`, `date`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;

CREATE TABLE `hku_base`.`stock` (
	`stockid` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`marketid` INT(10) UNSIGNED NULL DEFAULT NULL,
	`code` VARCHAR(20) NULL DEFAULT NULL,
	`name` VARCHAR(60) NULL DEFAULT NULL,
	`type` INT(10) UNSIGNED NULL DEFAULT NULL,
	`valid` INT(10) UNSIGNED NULL DEFAULT NULL,
	`startDate` BIGINT(20) UNSIGNED NULL DEFAULT NULL,
	`endDate` BIGINT(20) UNSIGNED NULL DEFAULT NULL,
	PRIMARY KEY (`stockid`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;

CREATE TABLE `hku_base`.`stocktypeinfo` (
	`id` INT(10) UNSIGNED NOT NULL,
	`type` INT(10) UNSIGNED NULL DEFAULT NULL,
	`precision` INT(11) NULL DEFAULT NULL,
	`tick` DOUBLE NULL DEFAULT NULL,
	`tickValue` DOUBLE NULL DEFAULT NULL,
	`minTradeNumber` INT(32) UNSIGNED NULL DEFAULT NULL,
	`maxTradeNumber` INT(32) UNSIGNED NULL DEFAULT NULL,
	`description` VARCHAR(100) NULL DEFAULT NULL,
	PRIMARY KEY (`id`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;
INSERT INTO `hku_base`.`stocktypeinfo` (tickValue,`precision`,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (100,2,0,0,'Block',100,100,1000000);
INSERT INTO `hku_base`.`stocktypeinfo` (tickValue,`precision`,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.01,2,1,1,'A股',0.01,100,1000000);
INSERT INTO `hku_base`.`stocktypeinfo` (tickValue,`precision`,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.001,3,2,2,'指数',0.001,1,1000000);
INSERT INTO `hku_base`.`stocktypeinfo` (tickValue,`precision`,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.001,3,3,3,'B股',0.001,100,1000000);
INSERT INTO `hku_base`.`stocktypeinfo` (tickValue,`precision`,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.001,3,4,4,'基金（不含ETF）',0.001,100,1000000);
INSERT INTO `hku_base`.`stocktypeinfo` (tickValue,`precision`,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.001,3,5,5,'ETF',0.001,1000,1000000);
INSERT INTO `hku_base`.`stocktypeinfo` (tickValue,`precision`,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.01,2,6,6,'国债',0.01,10,10000);
INSERT INTO `hku_base`.`stocktypeinfo` (tickValue,`precision`,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.01,2,7,7,'其他债券',0.01,10,10000);
INSERT INTO `hku_base`.`stocktypeinfo` (tickValue,`precision`,id,type,description,tick,minTradeNumber,maxTradeNumber) VALUES (0.01,2,8,8,'创业板',0.01,100,1000000);

CREATE TABLE `hku_base`.`coderuletype` (
	`id` INT(11) NOT NULL,
	`marketid` INT(11) NULL DEFAULT NULL,
	`codepre` VARCHAR(20) NULL DEFAULT NULL,
	`TYPE` INT(11) NULL DEFAULT NULL,
	`description` VARCHAR(100) NULL DEFAULT NULL,
	PRIMARY KEY (`id`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (1,1,'000',2,'上证指数');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (2,1,'60',1,'上证A股');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (4,1,'900',3,'上证B股');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (5,2,'00',1,'深证A股');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (6,2,'20',3,'深证B股');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (7,2,'39',2,'深证指数');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (8,2,'150',4,'深证基金');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (9,2,'16',4,'深证基金');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (10,2,'18',4,'深证基金');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (11,2,'159',5,'深证ETF');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (12,1,'51',5,'上证ETF');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (13,1,'50',4,'上证基金');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (18,2,'300',8,'创业板');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (20,1,'519',4,'上证基金');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (21,1,'009',6,'国债');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (22,1,'010',6,'国债');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (23,1,'10',7,'可转债');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (24,1,'11',7,'可转债');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (25,1,'12',7,'其他债券');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (26,1,'13',7,'地方政府债');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (27,2,'100',6,'国债');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (28,2,'101',6,'国债');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (29,2,'108',7,'贴债');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (30,2,'109',7,'地方政府债');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (31,2,'11',7,'其他债券');
INSERT INTO `hku_base`.`coderuletype` (id,marketid,codepre,type,description) VALUES (32,2,'12',7,'其他债券');

