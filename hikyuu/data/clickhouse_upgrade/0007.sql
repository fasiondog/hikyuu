-- Float64 K-line tables for non-A-share markets (market not in SH/SZ/BJ).
-- A-share markets keep using the original fixed-point tables.

CREATE TABLE IF NOT EXISTS hku_data.day_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY market
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date)
SETTINGS index_granularity = 2048;
CREATE TABLE IF NOT EXISTS hku_data.week_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY market
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);
CREATE TABLE IF NOT EXISTS hku_data.month_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY market
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);
CREATE TABLE IF NOT EXISTS hku_data.quarter_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY market
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);
CREATE TABLE IF NOT EXISTS hku_data.halfyear_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY market
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);
CREATE TABLE IF NOT EXISTS hku_data.year_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY market
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);
CREATE TABLE IF NOT EXISTS hku_data.min_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY (market, toYear(date)-toYear(date)%10)
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);
CREATE TABLE IF NOT EXISTS hku_data.min5_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY (market, toYear(date)-toYear(date)%10)
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);
CREATE TABLE IF NOT EXISTS hku_data.min15_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY (market, toYear(date)-toYear(date)%10)
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);
CREATE TABLE IF NOT EXISTS hku_data.min30_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY (market, toYear(date)-toYear(date)%10)
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);
CREATE TABLE IF NOT EXISTS hku_data.min60_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY market
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);
CREATE TABLE IF NOT EXISTS hku_data.hour2_k_f64 (
    `market` String,
    `code` String,
    `date` DateTime,
    `open` Float64,
    `high` Float64,
    `low` Float64,
    `close` Float64,
    `amount` Float64,
    `volume` Float64
)
ENGINE = MergeTree()
PARTITION BY market
PRIMARY KEY (market, code, date)
ORDER BY (market, code, date);

ALTER TABLE `hku_base`.`version` UPDATE `version` = 7 WHERE `id` = 0 SETTINGS mutations_sync = 1;
