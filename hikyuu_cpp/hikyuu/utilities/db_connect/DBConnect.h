/*
 * DBConnect.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-11
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DB_CONNECT_H
#define HIKYUU_DB_CONNECT_H

#include "SQLResultSet.h"
#include "AsyncSQLResultSet.h"
#include "DBConnectBase.h"
#include "SQLStatementBase.h"
#include "AsyncSQLStatementBase.h"
#include "AutoTransAction.h"
#include "AsyncTransAction.h"
#include "TableMacro.h"
#include "DBUpgrade.h"

#include "hikyuu/utilities/config.h"
#if HKU_ENABLE_MYSQL
#include "mysql/MySQLConnect.h"
#include "mysql/AsyncMySQLConnect.h"
#endif

#if HKU_ENABLE_SQLITE
#include "sqlite/SQLiteConnect.h"
#include "sqlite/AsyncSQLiteConnect.h"
#include "sqlite/SQLiteUtil.h"
#endif

#endif /* HIKYUU_DB_CONNECT_H */