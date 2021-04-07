/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-08
 *     Author: fasiondog
 */

namespace hku {

const char *g_sqlite_create_db{
  R"(
CREATE TABLE "td_version" (
	"id"	INTEGER NOT NULL UNIQUE,
	"module"	TEXT NOT NULL,
	"version"	INTEGER NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);

)"};

}