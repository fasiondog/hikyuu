/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-08
 *     Author: fasiondog
 */

#include "sqlitedb.h"

namespace hku {

const char *g_sqlite_create_td_db{
  R"(
CREATE TABLE "td_account" (
	"id"	INTEGER NOT NULL UNIQUE,
	"td_id"	INTEGER NOT NULL UNIQUE,
	"name"	TEXT NOT NULL UNIQUE,
	"type"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE UNIQUE INDEX "ix_td_account_on_td_id" ON "td_account" (
	"td_id"
);
CREATE UNIQUE INDEX "ix_td_account_on_name" ON "td_account" (
	"name"
);
CREATE TABLE "td_funds" (
	"id"	INTEGER NOT NULL UNIQUE,
	"td_id"	INTEGER NOT NULL,
	"currency"	TEXT NOT NULL,
	"balance"	REAL NOT NULL,
	"holde"	REAL NOT NULL,
	"available"	REAL NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_td_funds_on_td_id" ON "td_funds" (
	"td_id"
);
CREATE TABLE "td_enum" (
	"id"	INTEGER NOT NULL UNIQUE,
	"table_name"	TEXT NOT NULL,
	"field_name"	TEXT NOT NULL,
	"value_type"	TEXT NOT NULL,
	"value"	TEXT NOT NULL,
	"desc"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_td_enum_on_tabel_field" ON "td_enum" (
	"table_name",
	"field_name",
	"value"
);
INSERT INTO "td_enum" ("id", "table_name", "field_name", "value_type", "value", "desc") VALUES ('1', 'td_account', 'type', 'str', 'xq', '雪球');
CREATE TABLE "td_positions" (
	"id"	INTEGER NOT NULL UNIQUE,
	"td_id"	INTEGER NOT NULL,
	"sta_id"	INTEGER NOT NULL,
	"market"	TEXT NOT NULL,
	"code"	TEXT NOT NULL,
	"name"	TEXT,
	"amount"	REAL NOT NULL,
	"available_amount"	INTEGER,
	"buy_hold_amount"	REAL NOT NULL,
	"sell_hold_amount"	REAL NOT NULL,
	"ref_market_value"	REAL NOT NULL,
	"ref_market_price"	REAL NOT NULL,
	"ref_cost_price"	REAL NOT NULL,
	"ref_profit"	REAL NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_td_positions_on_td_id_sta_id" ON "td_positions" (
	"td_id",
	"sta_id"
);
CREATE TABLE "td_orders" (
	"id"	INTEGER NOT NULL UNIQUE,
	"td_id"	INTEGER NOT NULL,
	"datetime"	INTEGER NOT NULL,
	"order_seq"	TEXT NOT NULL,
	"sta_id"	INTEGER NOT NULL,
	"market"	TEXT NOT NULL,
	"code"	TEXT NOT NULL,
	"name"	INTEGER,
	"business"	INTEGER NOT NULL,
	"price"	REAL NOT NULL,
	"amount"	REAL NOT NULL,
	"type"	INTEGER NOT NULL,
	"status"	INTEGER NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_td_orders_on_td_id_datetime" ON "td_orders" (
	"td_id",
	"datetime" DESC
);
CREATE INDEX "ix_td_orders_on_td_id_sta_id_datetime" ON "td_orders" (
	"td_id",
	"sta_id",
	"datetime" DESC
);
CREATE TABLE "td_fills" (
	"id"	INTEGER NOT NULL UNIQUE,
	"td_id"	INTEGER NOT NULL,
	"sta_id"	INTEGER NOT NULL,
	"datetime"	INTEGER NOT NULL,
	"market"	TEXT NOT NULL,
	"code"	TEXT NOT NULL,
	"name"	TEXT,
	"amount"	REAL NOT NULL,
	"price"	REAL NOT NULL,
	"order_seq"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_td_fills_on_td_id_datetime" ON "td_fills" (
	"td_id",
	"datetime"	DESC
);
CREATE INDEX "ix_td_fills_on_td_id_sta_id_datetime" ON "td_fills" (
	"td_id",
	"sta_id",
	"datetime" DESC
);
CREATE TABLE "td_sta_account" (
	"id"	INTEGER NOT NULL UNIQUE,
	"sta_id"	INTEGER NOT NULL UNIQUE,
	"td_id"	INTEGER NOT NULL,
	"name"	TEXT NOT NULL,
	"path"	TEXT NOT NULL,
	"status"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE UNIQUE INDEX "ix_td_sta_account_on_sta_id" ON "td_sta_account" (
	"sta_id"	ASC
);
CREATE TABLE "td_sta_funds" (
	"id"	INTEGER NOT NULL UNIQUE,
	"sta_id"	INTEGER NOT NULL,
	"currency"	TEXT NOT NULL,
	"balance"	REAL NOT NULL,
	"hold"	REAL NOT NULL,
	"available"	REAL NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_td_sta_funds_on_sta_id" ON "td_sta_funds" (
	"sta_id"
);
CREATE TABLE "td_account_xq" (
	"id"	INTEGER NOT NULL UNIQUE,
	"td_id"	INTEGER NOT NULL,
	"cookies"	TEXT NOT NULL,
	"portfolio_code"	TEXT NOT NULL,
	"portfolio_market"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_td_account_xq_on_td_id" ON "td_account_xq" (
	"td_id"	ASC
);

)"};

}