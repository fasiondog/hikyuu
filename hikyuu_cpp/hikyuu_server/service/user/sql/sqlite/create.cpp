/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-08
 *     Author: fasiondog
 */

#include "sqlitedb.h"

namespace hku {

const char *g_sqlite_create_usr_db{
  R"(
CREATE TABLE "user" (
	"id"	INTEGER NOT NULL UNIQUE,
	"user_id"	INTEGER NOT NULL UNIQUE,
	"name"	TEXT NOT NULL UNIQUE,
	"password"	TEXT NOT NULL,
	"create_time" INTEGER NOT NULL,
	"token"	TEXT,
	"expire_time"	INTEGER,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_on_user_by_user_id" ON "user" (
	"user_id"
);
INSERT INTO "main"."user" ("id", "user_id", "name", "password", "create_time") VALUES ('1', '4374148134604800', 'admin', 'admin', '202104250141');
)"};

}