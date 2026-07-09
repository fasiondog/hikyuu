INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880001' AS code, '通达信总市值' AS name, 2 AS type, 1 AS valid, 20160126 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880001');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880002' AS code, '通达信流通市值' AS name, 2 AS type, 1 AS valid, 20160126 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880002');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880003' AS code, '通达信平均股价' AS name, 2 AS type, 1 AS valid, 20160126 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880003');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880004' AS code, '通达信成交均价' AS name, 2 AS type, 1 AS valid, 20160126 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880004');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880005' AS code, '通达信涨跌家数' AS name, 2 AS type, 1 AS valid, 20160126 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880005');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880006' AS code, '通达信停板家数' AS name, 2 AS type, 1 AS valid, 19901220 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880006');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880008' AS code, '通达信全A等权' AS name, 2 AS type, 1 AS valid, 19901220 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880008');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880011' AS code, '通达信主板总值' AS name, 2 AS type, 1 AS valid, 19901220 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880011');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880031' AS code, '通达信创业总值' AS name, 2 AS type, 1 AS valid, 19901220 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880031');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880041' AS code, '通达信科创总值' AS name, 2 AS type, 1 AS valid, 19901220 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880041');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880098' AS code, '通达信可转债指数' AS name, 2 AS type, 1 AS valid, 19901220 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880098');

INSERT INTO hku_base.stock (market, code, name, type, valid, startDate, endDate)
SELECT *
FROM (SELECT 'SH' AS market, '880099' AS code, '通达信逆回购' AS name, 2 AS type, 1 AS valid, 19901220 AS startDate, 99999999 AS endDate)
WHERE NOT EXISTS (SELECT * FROM hku_base.stock WHERE market='SH' AND code='880099');