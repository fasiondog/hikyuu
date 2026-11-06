-- ClickHouse 的 holiday 以 date 为主键（MergeTree 排序键），同一 date 无法容纳多市场，
-- 故重建表并把排序键改为 (market, date)；存量行按 9 个共享 CN 日历市场展开。
-- 先清理上次中断可能留下的表，保证脚本可重跑（MergeTree 不去重，残留会翻倍）
DROP TABLE IF EXISTS hku_base.holiday_new;
DROP TABLE IF EXISTS hku_base.holiday_old;

CREATE TABLE IF NOT EXISTS hku_base.holiday_new (
    market String,
    date Int32,
    PRIMARY KEY (market, date)
) ENGINE = MergeTree();

-- SELECT DISTINCT：中断后重跑时源表已含多市场行，去重避免同一市场重复展开
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'SH', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'SZ', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'BJ', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'SHF', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'DCE', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'CZC', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'CFE', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'GFE', date FROM hku_base.holiday;
INSERT INTO hku_base.holiday_new (market, date) SELECT DISTINCT 'INE', date FROM hku_base.holiday;

-- RENAME 是原子的；SELECT DISTINCT 保证中断后重跑可安全收敛，版本推进紧跟 RENAME
RENAME TABLE hku_base.holiday TO hku_base.holiday_old, hku_base.holiday_new TO hku_base.holiday;
ALTER TABLE `hku_base`.`version` UPDATE `version` = 6 WHERE `id` = 0 SETTINGS mutations_sync = 1;
DROP TABLE IF EXISTS hku_base.holiday_old;
