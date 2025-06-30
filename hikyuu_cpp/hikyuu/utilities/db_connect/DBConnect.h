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
#include "DBConnectBase.h"
#include "SQLStatementBase.h"
#include "AutoTransAction.h"
#include "TableMacro.h"
#include "DBUpgrade.h"

#include "hikyuu/utilities/config.h"
#if HKU_ENABLE_MYSQL
#include "mysql/MySQLConnect.h"
#endif

#if HKU_ENABLE_SQLITE
#include "sqlite/SQLiteConnect.h"
#include "sqlite/SQLiteUtil.h"
#endif

#if HKU_ENABLE_TDENGINE
#include "tdengine/TDengineConnect.h"
#endif

#endif /* HIKYUU_DB_CONNECT_H */