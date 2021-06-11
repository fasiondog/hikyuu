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
CREATE TABLE "admin_enum" (
	"id"	INTEGER NOT NULL UNIQUE,
	"table_name"	TEXT NOT NULL,
	"field_name"	TEXT NOT NULL,
	"value_type"	TEXT NOT NULL,
	"value"	TEXT NOT NULL,
	"desc"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_admin_enum_on_table_name_field_name" ON "admin_enum" (
	"table_name",
	"field_name"
);
INSERT INTO "admin_enum" ("id", "table_name", "field_name", "value_type", "value", "desc") VALUES ('1', 'admin_user', 'status', 'int', '1', 'normal');
INSERT INTO "admin_enum" ("id", "table_name", "field_name", "value_type", "value", "desc") VALUES ('2', 'admin_user', 'status', 'int', '99', 'deleted');
CREATE TABLE "admin_user" (
	"id"	INTEGER NOT NULL UNIQUE,
	"userid"	INTEGER NOT NULL UNIQUE,
	"name"	TEXT NOT NULL UNIQUE,
	"password"	TEXT NOT NULL,
	"start_time"	INTEGER NOT NULL,
	"end_time"	INTEGER,
	"status"	INTEGER NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_admin_user_on_userid" ON "admin_user" (
	"userid"
);
INSERT INTO "admin_user" ("id", "userid", "name", "password", "start_time", "end_time", "status") VALUES ('1', '4374148134604800', 'admin', '$2b$12$QyvJTRPKRSDKTxLGPhfBKOBNwyeIOAwJi0yvvyqWaoELCvV/ONjEC', '202104250141', '999912310000', '1');
CREATE TABLE "admin_token" (
	"id"	INTEGER NOT NULL UNIQUE,
	"userid"	INTEGER NOT NULL UNIQUE,	
	"token"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE INDEX "ix_admin_token_on_token" ON "admin_token" (
	"token"
);
CREATE INDEX "ix_admin_token_on_userid" ON "admin_token" (
	"userid"
);
)"};

}