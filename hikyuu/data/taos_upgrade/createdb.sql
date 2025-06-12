CREATE DATABASE IF NOT EXISTS hku_base;
CREATE TABLE IF NOT EXISTS hku_base.version (
	id timestamp,
    version INT 
);
insert into hku_base.version values(NOW, 1);


-- CREATE TABLE hku_base.stock (stockid timestamp, marketid INTEGER, code VARCHAR(20), name VARCHAR(60), type INTEGER, valid INTEGER, startDate INTEGER, endDate INTEGER);
