
#include "hikyuu/utilities/config.h"

#include "AsyncMySQLConnect.cpp"
#include "AsyncMySQLStatement.cpp"

#if HKU_DISABLE_LIBMYSQLCLIENT
#include "MySQLConnect.cpp"
#include "MySQLStatement.cpp"
#else
#include "libmysqlclient/MySQLConnect.cpp"
#include "libmysqlclient/MySQLStatement.cpp"
#endif
