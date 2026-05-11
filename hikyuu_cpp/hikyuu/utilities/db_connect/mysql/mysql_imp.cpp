
#include "hikyuu/utilities/config.h"

#include "AsyncMySQLConnect.cpp"
#include "AsyncMySQLStatement.cpp"

#if HKU_DISABLE_LIBMYSQLCLIENT
#include "MySQLConnect_boost.cpp"
#include "MySQLStatement_boost.cpp"
#else
#include "MySQLConnect_libmysqlclient.cpp"
#include "MySQLStatement_libmysqlclient.cpp"
#endif

