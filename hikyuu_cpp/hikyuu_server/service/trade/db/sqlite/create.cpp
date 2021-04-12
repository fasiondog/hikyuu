/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-08
 *     Author: fasiondog
 */

#include "sqlitedb.h"

namespace hku {

const char *g_sqlite_create_db{
  R"(
CREATE TABLE "td_account" (
	"id"	INTEGER NOT NULL UNIQUE,
	"account"	TEXT NOT NULL UNIQUE,
	"name"	TEXT NOT NULL UNIQUE,
	"type"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE UNIQUE INDEX "ix_td_account_on_account" ON "td_account" (
	"account"	ASC
);
CREATE UNIQUE INDEX "ix_td_account_on_name" ON "td_account" (
	"name"	ASC
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
CREATE TABLE "td_enum" (
	"id"	INTEGER NOT NULL UNIQUE,
	"table"	TEXT NOT NULL,
	"field"	TEXT NOT NULL,
	"value"	INTEGER NOT NULL,
	"string"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
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
CREATE TABLE "td_sta_account" (
	"id"	INTEGER NOT NULL UNIQUE,
	"td_id"	INTEGER NOT NULL,
	"account"	TEXT NOT NULL,
	"name"	TEXT NOT NULL,
	"path"	TEXT NOT NULL,
	"status"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE UNIQUE INDEX "ix_td_sta_account_on_account" ON "td_sta_account" (
	"account"	ASC
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
CREATE TABLE "td_account_xq" (
	"id"	INTEGER NOT NULL UNIQUE,
	"td_id"	INTEGER NOT NULL,
	"cookies"	TEXT NOT NULL,
	"portfolio_code"	TEXT NOT NULL,
	"portfolio_market"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);

)"};

}