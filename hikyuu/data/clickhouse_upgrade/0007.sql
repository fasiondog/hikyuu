DROP TABLE IF EXISTS hku_base.holiday_new;
DROP TABLE IF EXISTS hku_base.holiday_old;

CREATE TABLE IF NOT EXISTS hku_base.holiday_new (
    market String,
    date Int32,
    PRIMARY KEY (market, date)
) ENGINE = MergeTree();

INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'SH', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'SZ', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'BJ', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'SHF', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'DCE', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'CZC', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'CFE', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'GFE', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'INE', date FROM hku_base.holiday;

CREATE TABLE IF NOT EXISTS hku_base.stockparam (
    market String,
    code String,
    tick Float64,
    tickValue Float64,
    minTradeNumber Float64,
    maxTradeNumber Float64,
    precision Int32,
    PRIMARY KEY (market, code)
) ENGINE = ReplacingMergeTree();

RENAME TABLE hku_base.holiday TO hku_base.holiday_old, hku_base.holiday_new TO hku_base.holiday;
ALTER TABLE `hku_base`.`version` UPDATE `version` = 7 WHERE `id` = 0 SETTINGS mutations_sync = 1;
DROP TABLE IF EXISTS hku_base.holiday_old;
