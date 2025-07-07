CREATE DATABASE IF NOT EXISTS hku_base;
CREATE TABLE IF NOT EXISTS hku_base.version (
    id Int32,
	version Int32,
	PRIMARY KEY (id)
);
INSERT INTO hku_base.version (id, version) VALUES (0, 0);

CREATE TABLE IF NOT EXISTS hku_base.market (
    market String,
    name String,
    description String,
    code String,
    lastDate Int64,
    openTime1 Int32,
    closeTime1 Int32,
    openTime2 Int32,
    closeTime2 Int32,
    PRIMARY KEY (market)
)ENGINE = MergeTree();
INSERT INTO `hku_base`.`market` (`market`, `name`, `description`, `code`, `lastDate`, `openTime1`, `closeTime1`, `openTime2`, `closeTime2`) VALUES 
('SH', '上海证券交易所', '上海市场', '000001', 20250704, 930, 1130, 1300, 1500),
('SZ', '深圳证券交易所', '深圳市场', '399001', 20250704, 930, 1130, 1300, 1500),
('BJ', '北京证券交易所', '北京市场', '899050', 20250430, 930, 1130, 1300, 1500);

CREATE TABLE IF NOT EXISTS hku_base.stock (
    market String,
    code String,
    name String,
    type Int32,
    valid Int32,
    startDate Int32,
    endDate Int32,
    PRIMARY KEY (market, code)
)ENGINE = MergeTree();

CREATE TABLE IF NOT EXISTS hku_base.stkweight (
    market String,
    code String,
    date Int32,
    countAsGift Float64,
    countForSell Float64,
    priceForSell Float64,
    bonus Float64,
    countOfIncreasement Float64,
    totalCount Float64,
    freeCount Float64,
    suogu Float64,
    PRIMARY KEY (market, code, date)
)ENGINE = MergeTree();

CREATE TABLE IF NOT EXISTS hku_base.historyfinance (
    file_date Int32,
    market String,
    code String,
    report_date Int32,
    `values` String,
    PRIMARY KEY (market, code, report_date)
) ENGINE = MergeTree();

CREATE TABLE IF NOT EXISTS hku_base.stkfinance (
    market String,
    code String,
	`updated_date` Int32, 
	`ipo_date` Int32,       --42.上市日期
	`province` DOUBLE, 
    `industry` DOUBLE,
	`zongguben` DOUBLE,      --1.总股本(股)
    `liutongguben` DOUBLE,   --7.流通A股（股）
	`guojiagu` DOUBLE,       --2.国家股（股）
	`faqirenfarengu` DOUBLE, --3.发起人法人股（股）
	`farengu` DOUBLE,        --4.法人股（股）
	`bgu` DOUBLE,            --5.B股（股）
	`hgu` DOUBLE,            --6.H股（股）
	`zhigonggu` DOUBLE,      --8.职工股（股）
	`zongzichan` DOUBLE,     --10.总资产（元）
	`liudongzichan` DOUBLE,  --11.流动资产（元）
    `gudingzichan` DOUBLE,   --12.固定资产（元）
	`wuxingzichan` DOUBLE,   --13.无形资产（元）
	`gudongrenshu` DOUBLE,   --股东人数
	`liudongfuzhai` DOUBLE,  --15.流动负债
	`changqifuzhai` DOUBLE,  --16.长期负债
	`zibengongjijin` DOUBLE, --17.资本公积金
	`jingzichan` DOUBLE,     --净资产（元）
	`zhuyingshouru` DOUBLE,  --20.主营收入
	`zhuyinglirun` DOUBLE,   --21.主营利润
	`yingshouzhangkuan` DOUBLE, --应收账款
	`yingyelirun` DOUBLE,    --23.营业利润
	`touzishouyu` DOUBLE,    --投资收益
	`jingyingxianjinliu` DOUBLE, --经营现金流
	`zongxianjinliu` DOUBLE,     --总现金流
	`cunhuo` DOUBLE,             --存货
	`lirunzonghe` DOUBLE,    --28.利润总额
	`shuihoulirun` DOUBLE,   --29.税后利润
	`jinglirun` DOUBLE,      --30.净利润
	`weifenpeilirun` DOUBLE, --31.未分配利润
	`meigujingzichan` DOUBLE, --34.每股净资产
	`baoliu2` DOUBLE,
    PRIMARY KEY (market, code)
) ENGINE = MergeTree();

CREATE TABLE IF NOT EXISTS hku_base.stocktypeinfo (
    id Int32,
    type Int32,
    precision Int32,
    tickValue Float64,
    tick Float64,
    minTradeNumber Float64,
    maxTradeNumber Float64,
    description String,
    PRIMARY KEY (id)
) ENGINE = MergeTree();
INSERT INTO `hku_base`.`stocktypeinfo` (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES 
(1, 0, 2, 100, 100, 100, 1000000, 'Block'),
(2, 1, 2, 0.01, 0.01, 100, 1000000, 'A股'),
(3, 2, 3, 0.001, 0.001, 1, 1000000, '指数'),
(4, 3, 3, 0.001, 0.001, 100, 1000000, 'B股'),
(5, 4, 3, 0.001, 0.001, 100, 1000000, '基金（不含ETF）'),
(6, 5, 3, 0.001, 0.001, 1000, 1000000, 'ETF'),
(7, 6, 2, 0.01, 0.01, 10, 10000, '国债'),
(8, 7, 2, 0.01, 0.01, 10, 10000, '其他债券'),
(9, 8, 2, 0.01, 0.01, 100, 1000000, '创业板'),
(10, 9, 2, 0.01, 0.01, 1, 1000000, '科创板'),
(11, 11, 2, 0.01, 0.01, 1, 1000000, '北交所');

CREATE TABLE IF NOT EXISTS hku_base.coderuletype (
    id Int32,
    market String,
    codepre String,
    type Int32,
    description String,
    PRIMARY KEY (id)
) ENGINE = MergeTree();
INSERT INTO `hku_base`.`coderuletype` (`id`, `market`, `codepre`, `type`, `description`) VALUES 
(1, 'SH', '000', 2, '上证指数'),
(2, 'SH', '60', 1, '上证A股'),
(4, 'SH', '900', 3, '上证B股'),
(5, 'SZ', '00', 1, '深证A股'),
(6, 'SZ', '20', 3, '深证B股'),
(7, 'SZ', '39', 2, '深证指数'),
(8, 'SZ', '150', 4, '深证基金'),
(9, 'SZ', '16', 4, '深证基金'),
(10, 'SZ', '18', 4, '深证基金'),
(11, 'SZ', '159', 5, '深证ETF'),
(12, 'SH', '51', 5, '上证ETF'),
(13, 'SH', '50', 4, '上证基金'),
(18, 'SZ', '300', 8, '创业板'),
(21, 'SH', '009', 6, '国债'),
(22, 'SH', '010', 6, '国债'),
(23, 'SH', '10', 7, '可转债'),
(24, 'SH', '11', 7, '可转债'),
(25, 'SH', '12', 7, '其他债券'),
(26, 'SH', '13', 7, '地方政府债'),
(27, 'SZ', '100', 6, '国债'),
(28, 'SZ', '101', 6, '国债'),
(29, 'SZ', '108', 7, '贴债'),
(30, 'SZ', '109', 7, '地方政府债'),
(31, 'SZ', '11', 7, '其他债券'),
(32, 'SZ', '12', 7, '其他债券'),
(33, 'SH', '688', 9, '科创板'),
(34, 'SZ', '301', 8, '创业板'),
(35, 'BJ', '43', 11, '北证A股'),
(36, 'BJ', '83', 11, '北证A股'),
(37, 'BJ', '87', 11, '北证A股'),
(38, 'BJ', '88', 11, '北证A股'),
(40, 'BJ', '92', 11, '北证A股'),
(41, 'SH', '880', 2, '通达信板块指数'),
(43, 'SH', '56', 5, '上证ETF'),
(44, 'SH', '58', 5, '上证ETF'),
(45, 'BJ', '899', 2, '北证指数'),
(46, 'SZ', '302', 8, '创业板');

CREATE TABLE IF NOT EXISTS hku_base.block (
    category String,
    name String,
    market_code String,
    PRIMARY KEY (category, name)
) ENGINE = MergeTree();

CREATE TABLE IF NOT EXISTS hku_base.blockindex (
    category String,
    name String,
    index_code String,
    PRIMARY KEY (category, name)
) ENGINE = MergeTree();

CREATE TABLE IF NOT EXISTS hku_base.zh_bond10 (
    date Int32,
    value Int64,
    PRIMARY KEY (date)
) ENGINE = MergeTree();

CREATE TABLE IF NOT EXISTS hku_base.holiday (
    date Int32,
    PRIMARY KEY (date)
) ENGINE = MergeTree();
INSERT INTO `hku_base`.`holiday` (`date`) VALUES 
(20210101),
(20210211),
(20210212),
(20210215),
(20210216),
(20210217),
(20210405),
(20210501),
(20210503),
(20210504),
(20210505),
(20210614),
(20210920),
(20210921),
(20211001),
(20211004),
(20211005),
(20211006),
(20211007),
(20220103),
(20220131),
(20220201),
(20220202),
(20220203),
(20220204),
(20220205),
(20220404),
(20220405),
(20220502),
(20220503),
(20220504),
(20220603),
(20220912),
(20221003),
(20221004),
(20221005),
(20221006),
(20221007),
(20230102),
(20230123),
(20230124),
(20230125),
(20230126),
(20230127),
(20230405),
(20230501),
(20230502),
(20230503),
(20230622),
(20230623),
(20230929),
(20231002),
(20231003),
(20231004),
(20231005),
(20231006),
(20240101),
(20240212),
(20240213),
(20240214),
(20240215),
(20240216),
(20240404),
(20240405),
(20240501),
(20240502),
(20240503),
(20240610),
(20240916),
(20240917),
(20241001),
(20241002),
(20241003),
(20241004),
(20241007),
(20250101),
(20250128),
(20250129),
(20250130),
(20250131),
(20250203),
(20250204),
(20250404),
(20250501),
(20250502),
(20250505),
(20250602),
(20251001),
(20251002),
(20251003),
(20251006),
(20251007),
(20251008);
