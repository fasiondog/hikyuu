BEGIN TRANSACTION;
CREATE TABLE "sh_day" (
	"code"	TEXT NOT NULL,
	"datetime"	NUMERIC NOT NULL,
	"open"	REAL NOT NULL,
	"high"	REAL NOT NULL,
	"low"	REAL NOT NULL,
	"close"	REAL NOT NULL,
	"amount"	REAL NOT NULL,
	"volumn"	REAL NOT NULL,
	PRIMARY KEY("code","datetime")
);
CREATE TABLE "sz_day" (
	"code"	TEXT NOT NULL,
	"datetime"	NUMERIC NOT NULL,
	"open"	REAL NOT NULL,
	"high"	REAL NOT NULL,
	"low"	REAL NOT NULL,
	"close"	REAL NOT NULL,
	"amount"	REAL NOT NULL,
	"volumn"	REAL NOT NULL,
	PRIMARY KEY("code","datetime")
);
CREATE TABLE "sh_min" (
	"code"	TEXT NOT NULL,
	"datetime"	NUMERIC NOT NULL,
	"open"	REAL NOT NULL,
	"high"	REAL NOT NULL,
	"low"	REAL NOT NULL,
	"close"	REAL NOT NULL,
	"amount"	REAL NOT NULL,
	"volumn"	REAL NOT NULL,
	PRIMARY KEY("code","datetime")
);
CREATE TABLE "sz_min" (
	"code"	TEXT NOT NULL,
	"datetime"	NUMERIC NOT NULL,
	"open"	REAL NOT NULL,
	"high"	REAL NOT NULL,
	"low"	REAL NOT NULL,
	"close"	REAL NOT NULL,
	"amount"	REAL NOT NULL,
	"volumn"	REAL NOT NULL,
	PRIMARY KEY("code","datetime")
);
COMMIT;
