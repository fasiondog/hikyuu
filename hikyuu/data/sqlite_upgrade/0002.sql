BEGIN TRANSACTION;
CREATE TABLE `stkfinance` (
	id INTEGER NOT NULL, 
	stockid INTEGER, 
	`updated_date` INTEGER, 
	`ipo_date` INTEGER,       --42.上市日期
	`province` INTEGER, 
    `industry` INTEGER,
	`zongguben` NUMERIC,      --1.总股本(股)
    `liutongguben` NUMERIC,   --7.流通A股（股）
	`guojiagu` NUMERIC,       --2.国家股（股）
	`faqirenfarengu` NUMERIC, --3.发起人法人股（股）
	`farengu` NUMERIC,        --4.法人股（股）
	`bgu` NUMERIC,            --5.B股（股）
	`hgu` NUMERIC,            --6.H股（股）
	`zhigonggu` NUMERIC,      --8.职工股（股）
	`zongzichan` NUMERIC,     --10.总资产（元）
	`liudongzichan` NUMERIC,  --11.流动资产（元）
    `gudingzichan` NUMERIC,   --12.固定资产（元）
	`wuxingzichan` NUMERIC,   --13.无形资产（元）
	`gudongrenshu` NUMERIC,   --股东人数
	`liudongfuzhai` NUMERIC,  --15.流动负债
	`changqifuzhai` NUMERIC,  --16.长期负债
	`zibengongjijin` NUMERIC, --17.资本公积金
	`jingzichan` NUMERIC,     --净资产（元）
	`zhuyingshouru` NUMERIC,  --20.主营收入
	`zhuyinglirun` NUMERIC,   --21.主营利润
	`yingshouzhangkuan` NUMERIC, --应收账款
	`yingyelirun` NUMERIC,    --23.营业利润
	`touzishouyu` NUMERIC,    --投资收益
	`jingyingxianjinliu` NUMERIC, --经营现金流
	`zongxianjinliu` NUMERIC,     --总现金流
	`cunhuo` NUMERIC,             --存货
	`lirunzonghe` NUMERIC,    --28.利润总额
	`shuihoulirun` NUMERIC,   --29.税后利润
	`jinglirun` NUMERIC,      --30.净利润
	`weifenpeilirun` NUMERIC, --31.未分配利润
	`meigujingzichan` NUMERIC, --34.每股净资产
	`baoliu2` NUMERIC,
	PRIMARY KEY (id), 
	 FOREIGN KEY(stockid) REFERENCES "Stock" (stockid)
);

CREATE INDEX "ix_stkfinance_date" ON "stkfinance" (updated_date);
CREATE INDEX "ix_stkfinance_stock_date" ON "stkfinance" (stockid, updated_date);

UPDATE `version` set `version` = 2;
COMMIT;
