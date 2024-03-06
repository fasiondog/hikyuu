CREATE TABLE IF NOT EXISTS `hku_base`.`stkfinance` (
	`id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`stockid` INT UNSIGNED NOT NULL, 
	`updated_date` INT NOT NULL, 
	`ipo_date` INT NOT NULL,       -- 42.上市日期
	`province` DOUBLE NOT NULL, 
    `industry` DOUBLE NOT NULL,
	`zongguben` DOUBLE NOT NULL,      -- 1.总股本(股)
    `liutongguben` DOUBLE NOT NULL,   -- 7.流通A股（股）
	`guojiagu` DOUBLE NOT NULL,       -- 2.国家股（股）
	`faqirenfarengu` DOUBLE NOT NULL, -- 3.发起人法人股（股）
	`farengu` DOUBLE NOT NULL,        -- 4.法人股（股）
	`bgu` DOUBLE NOT NULL,            -- 5.B股（股）
	`hgu` DOUBLE NOT NULL,            -- 6.H股（股）
	`zhigonggu` DOUBLE NOT NULL,      -- 8.职工股（股）
	`zongzichan` DOUBLE NOT NULL,     -- 10.总资产（元）
	`liudongzichan` DOUBLE NOT NULL,  -- 11.流动资产（元）
    `gudingzichan` DOUBLE NOT NULL,   -- 12.固定资产（元）
	`wuxingzichan` DOUBLE NOT NULL,   -- 13.无形资产（元）
	`gudongrenshu` DOUBLE NOT NULL,   -- 股东人数
	`liudongfuzhai` DOUBLE NOT NULL,  -- 15.流动负债
	`changqifuzhai` DOUBLE NOT NULL,  -- 16.长期负债
	`zibengongjijin` DOUBLE NOT NULL, -- 17.资本公积金
	`jingzichan` DOUBLE NOT NULL,     -- 净资产（元）
	`zhuyingshouru` DOUBLE NOT NULL,  -- 20.主营收入
	`zhuyinglirun` DOUBLE NOT NULL,   -- 21.主营利润
	`yingshouzhangkuan` DOUBLE NOT NULL, -- 应收账款
	`yingyelirun` DOUBLE NOT NULL,    -- 23.营业利润
	`touzishouyu` DOUBLE NOT NULL,    -- 投资收益
	`jingyingxianjinliu` DOUBLE NOT NULL, -- 经营现金流
	`zongxianjinliu` DOUBLE NOT NULL,     -- 总现金流
	`cunhuo` DOUBLE NOT NULL,             -- 存货
	`lirunzonghe` DOUBLE NOT NULL,    -- 28.利润总额
	`shuihoulirun` DOUBLE NOT NULL,   -- 29.税后利润
	`jinglirun` DOUBLE NOT NULL,      -- 30.净利润
	`weifenpeilirun` DOUBLE NOT NULL, -- 31.未分配利润
	`meigujingzichan` DOUBLE NOT NULL, -- 34.每股净资产
	`baoliu2` DOUBLE NOT NULL,
	PRIMARY KEY (`id`), 
   FOREIGN KEY(`stockid`) REFERENCES `hku_base`.`stock` (`stockid`),
   INDEX `ix_stkfinance_date` (`updated_date`),
   INDEX `ix_stkfinance_stock_date` (`stockid`, `updated_date`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;

UPDATE `hku_base`.`version` set `version` = 9;