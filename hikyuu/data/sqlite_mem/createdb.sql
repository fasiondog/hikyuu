BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS "sh_day" (
	"code"	TEXT NOT NULL,
	"datetime"	NUMERIC NOT NULL,
	"open"	REAL NOT NULL,
	"high"	REAL NOT NULL,
	"low"	REAL NOT NULL,
	"close"	REAL NOT NULL,
	"amount"	REAL NOT NULL,
	"volumn"	REAL NOT NULL,
	"bid1"	REAL NOT NULL,
	"bid1_amount"	REAL NOT NULL,
	"bid2"	REAL NOT NULL,
	"bid2_amount"	REAL NOT NULL,
	"bid3"	REAL NOT NULL,
	"bid3_amount"	REAL NOT NULL,
	"bid4"	REAL NOT NULL,
	"bid4_amount"	REAL NOT NULL,
	"bid5"	REAL NOT NULL,
	"bid5_amount"	REAL NOT NULL,
	"ask1"	REAL NOT NULL,
	"ask1_amount"	REAL NOT NULL,
	"ask2"	REAL NOT NULL,
	"ask2_amount"	REAL NOT NULL,
	"ask3"	REAL NOT NULL,
	"ask3_amount"	REAL NOT NULL,
	"ask4"	REAL NOT NULL,
	"ask4_amount"	REAL NOT NULL,
	"ask5"	REAL NOT NULL,
	"ask5_amount"	REAL NOT NULL,
	PRIMARY KEY("code","datetime")
);
CREATE TABLE IF NOT EXISTS "sz_day" (
	"code"	TEXT NOT NULL,
	"datetime"	NUMERIC NOT NULL,
	"open"	REAL NOT NULL,
	"high"	REAL NOT NULL,
	"low"	REAL NOT NULL,
	"close"	REAL NOT NULL,
	"amount"	REAL NOT NULL,
	"volumn"	REAL NOT NULL,
	"bid1"	REAL NOT NULL,
	"bid1_amount"	REAL NOT NULL,
	"bid2"	REAL NOT NULL,
	"bid2_amount"	REAL NOT NULL,
	"bid3"	REAL NOT NULL,
	"bid3_amount"	REAL NOT NULL,
	"bid4"	REAL NOT NULL,
	"bid4_amount"	REAL NOT NULL,
	"bid5"	REAL NOT NULL,
	"bid5_amount"	REAL NOT NULL,
	"ask1"	REAL NOT NULL,
	"ask1_amount"	REAL NOT NULL,
	"ask2"	REAL NOT NULL,
	"ask2_amount"	REAL NOT NULL,
	"ask3"	REAL NOT NULL,
	"ask3_amount"	REAL NOT NULL,
	"ask4"	REAL NOT NULL,
	"ask4_amount"	REAL NOT NULL,
	"ask5"	REAL NOT NULL,
	"ask5_amount"	REAL NOT NULL,
	PRIMARY KEY("code","datetime")
);
CREATE TABLE IF NOT EXISTS "sh_min" (
	"code"	TEXT NOT NULL,
	"datetime"	NUMERIC NOT NULL,
	"open"	REAL NOT NULL,
	"high"	REAL NOT NULL,
	"low"	REAL NOT NULL,
	"close"	REAL NOT NULL,
	"amount"	REAL NOT NULL,
	"volumn"	REAL NOT NULL,
	"bid1"	REAL NOT NULL,
	"bid1_amount"	REAL NOT NULL,
	"bid2"	REAL NOT NULL,
	"bid2_amount"	REAL NOT NULL,
	"bid3"	REAL NOT NULL,
	"bid3_amount"	REAL NOT NULL,
	"bid4"	REAL NOT NULL,
	"bid4_amount"	REAL NOT NULL,
	"bid5"	REAL NOT NULL,
	"bid5_amount"	REAL NOT NULL,
	"ask1"	REAL NOT NULL,
	"ask1_amount"	REAL NOT NULL,
	"ask2"	REAL NOT NULL,
	"ask2_amount"	REAL NOT NULL,
	"ask3"	REAL NOT NULL,
	"ask3_amount"	REAL NOT NULL,
	"ask4"	REAL NOT NULL,
	"ask4_amount"	REAL NOT NULL,
	"ask5"	REAL NOT NULL,
	"ask5_amount"	REAL NOT NULL,
	PRIMARY KEY("code","datetime")
);
CREATE TABLE IF NOT EXISTS "sz_min" (
	"code"	TEXT NOT NULL,
	"datetime"	NUMERIC NOT NULL,
	"open"	REAL NOT NULL,
	"high"	REAL NOT NULL,
	"low"	REAL NOT NULL,
	"close"	REAL NOT NULL,
	"amount"	REAL NOT NULL,
	"volumn"	REAL NOT NULL,
	"bid1"	REAL NOT NULL,
	"bid1_amount"	REAL NOT NULL,
	"bid2"	REAL NOT NULL,
	"bid2_amount"	REAL NOT NULL,
	"bid3"	REAL NOT NULL,
	"bid3_amount"	REAL NOT NULL,
	"bid4"	REAL NOT NULL,
	"bid4_amount"	REAL NOT NULL,
	"bid5"	REAL NOT NULL,
	"bid5_amount"	REAL NOT NULL,
	"ask1"	REAL NOT NULL,
	"ask1_amount"	REAL NOT NULL,
	"ask2"	REAL NOT NULL,
	"ask2_amount"	REAL NOT NULL,
	"ask3"	REAL NOT NULL,
	"ask3_amount"	REAL NOT NULL,
	"ask4"	REAL NOT NULL,
	"ask4_amount"	REAL NOT NULL,
	"ask5"	REAL NOT NULL,
	"ask5_amount"	REAL NOT NULL,
	PRIMARY KEY("code","datetime")
);
COMMIT;