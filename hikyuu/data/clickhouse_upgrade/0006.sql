-- ClickHouse's holiday uses date as the primary key (MergeTree sort key), so one date cannot
-- hold multiple markets; rebuild the table with the sort key (market, date) and expand existing
-- rows over the 9 shared CN-calendar markets.
-- Drop tables possibly left by a previous interruption first, so the script is re-runnable
-- (MergeTree does not dedup; leftovers would double the rows)
DROP TABLE IF EXISTS hku_base.holiday_new;
DROP TABLE IF EXISTS hku_base.holiday_old;

CREATE TABLE IF NOT EXISTS hku_base.holiday_new (
    market String,
    date Int32,
    PRIMARY KEY (market, date)
) ENGINE = MergeTree();

-- SELECT DISTINCT: after an interruption the source already holds multi-market rows; dedup avoids re-expanding the same market
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'SH', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'SZ', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'BJ', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'SHF', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'DCE', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'CZC', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'CFE', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'GFE', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'INE', date FROM hku_base.holiday;

-- RENAME is atomic; SELECT DISTINCT makes a rerun after interruption converge safely, the version bump follows the RENAME
RENAME TABLE hku_base.holiday TO hku_base.holiday_old, hku_base.holiday_new TO hku_base.holiday;
ALTER TABLE `hku_base`.`version` UPDATE `version` = 6 WHERE `id` = 0 SETTINGS mutations_sync = 1;
DROP TABLE IF EXISTS hku_base.holiday_old;
