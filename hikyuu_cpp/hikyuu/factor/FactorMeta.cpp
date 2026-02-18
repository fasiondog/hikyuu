/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-12
 *      Author: fasiondog
 */

#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <xxhash.h>
#include "hikyuu/indicator/imp/IPriceList.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "FactorMeta.h"

namespace hku {

// 数据库关键字集合，包含SQLite、MySQL、ClickHouse的关键字
static const std::unordered_set<std::string> DATABASE_KEYWORDS = {
  // SQLite关键字 (147个)
  "ABORT", "ACTION", "ADD", "AFTER", "ALL", "ALTER", "ALWAYS", "ANALYZE", "AND", "AS", "ASC",
  "ATTACH", "AUTOINCREMENT", "BEFORE", "BEGIN", "BETWEEN", "BY", "CASCADE", "CASE", "CAST", "CHECK",
  "COLLATE", "COLUMN", "COMMIT", "CONFLICT", "CONSTRAINT", "CREATE", "CROSS", "CURRENT",
  "CURRENT_DATE", "CURRENT_TIME", "CURRENT_TIMESTAMP", "DATABASE", "DEFAULT", "DEFERRABLE",
  "DEFERRED", "DELETE", "DESC", "DETACH", "DISTINCT", "DO", "DROP", "EACH", "ELSE", "END", "ESCAPE",
  "EXCEPT", "EXCLUDE", "EXCLUSIVE", "EXISTS", "EXPLAIN", "FAIL", "FILTER", "FIRST", "FOLLOWING",
  "FOR", "FOREIGN", "FROM", "FULL", "GENERATED", "GLOB", "GROUP", "GROUPS", "HAVING", "IF",
  "IGNORE", "IMMEDIATE", "IN", "INDEX", "INDEXED", "INITIALLY", "INNER", "INSERT", "INSTEAD",
  "INTERSECT", "INTO", "IS", "ISNULL", "JOIN", "KEY", "LAST", "LEFT", "LIKE", "LIMIT", "MATCH",
  "MATERIALIZED", "NATURAL", "NO", "NOT", "NOTHING", "NOTNULL", "NULL", "NULLS", "OF", "OFFSET",
  "ON", "OR", "ORDER", "OTHERS", "OUTER", "OVER", "PARTITION", "PLAN", "PRAGMA", "PRECEDING",
  "PRIMARY", "QUERY", "RAISE", "RANGE", "RECURSIVE", "REFERENCES", "REGEXP", "REINDEX", "RELEASE",
  "RENAME", "REPLACE", "RESTRICT", "RETURNING", "RIGHT", "ROLLBACK", "ROW", "ROWS", "SAVEPOINT",
  "SELECT", "SET", "TABLE", "TEMP", "TEMPORARY", "THEN", "TIES", "TO", "TRANSACTION", "TRIGGER",
  "UNBOUNDED", "UNION", "UNIQUE", "UPDATE", "USING", "VACUUM", "VALUES", "VIEW", "VIRTUAL", "WHEN",
  "WHERE", "WINDOW", "WITH", "WITHOUT",

  // MySQL关键字 (部分常用保留字)
  "ACCESSIBLE", "ACCOUNT", "ACTION", "ACTIVE", "ADD", "ADMIN", "AFTER", "AGAINST", "AGGREGATE",
  "ALGORITHM", "ALL", "ALTER", "ALWAYS", "ANALYZE", "AND", "ANY", "AS", "ASC", "ASCII",
  "ASENSITIVE", "AT", "AUTHENTICATION", "AUTO_INCREMENT", "AUTOEXTEND_SIZE", "AVG",
  "AVG_ROW_LENGTH", "BACKUP", "BEFORE", "BEGIN", "BETWEEN", "BIGINT", "BINARY", "BINLOG", "BIT",
  "BLOB", "BLOCK", "BOOL", "BOOLEAN", "BOTH", "BTREE", "BY", "BYTE", "CACHE", "CALL", "CASCADE",
  "CASCADED", "CASE", "CATALOG_NAME", "CHAIN", "CHANGE", "CHANGED", "CHANNEL", "CHAR", "CHARACTER",
  "CHARSET", "CHECK", "CHECKSUM", "CIPHER", "CLASS_ORIGIN", "CLIENT", "CLONE", "CLOSE", "COALESCE",
  "CODE", "COLLATE", "COLLATION", "COLUMN", "COLUMNS", "COLUMN_FORMAT", "COLUMN_NAME", "COMMENT",
  "COMMIT", "COMMITTED", "COMPACT", "COMPLETION", "COMPONENT", "COMPRESSED", "COMPRESSION",
  "CONCURRENT", "CONDITION", "CONNECTION", "CONSISTENT", "CONSTRAINT", "CONSTRAINT_CATALOG",
  "CONSTRAINT_NAME", "CONSTRAINT_SCHEMA", "CONTAINS", "CONTEXT", "CONTINUE", "CONVERT", "CPU",
  "CREATE", "CROSS", "CUBE", "CUME_DIST", "CURRENT", "CURRENT_DATE", "CURRENT_TIME",
  "CURRENT_TIMESTAMP", "CURRENT_USER", "CURSOR", "CURSOR_NAME", "DATA", "DATABASE", "DATABASES",
  "DATAFILE", "DATE", "DATETIME", "DAY", "DAY_HOUR", "DAY_MICROSECOND", "DAY_MINUTE", "DAY_SECOND",
  "DEALLOCATE", "DEC", "DECIMAL", "DECLARE", "DEFAULT", "DEFAULT_AUTH", "DEFINER", "DELAYED",
  "DELAY_KEY_WRITE", "DELETE", "DENSE_RANK", "DESC", "DESCRIBE", "DETERMINISTIC", "DIAGNOSTICS",
  "DIRECTORY", "DISABLE", "DISCARD", "DISK", "DISTINCT", "DISTINCTROW", "DIV", "DO", "DOUBLE",
  "DROP", "DUAL", "DUMPFILE", "DUPLICATE", "DYNAMIC", "EACH", "ELSE", "ELSEIF", "EMPTY", "ENABLE",
  "ENCLOSED", "ENCRYPTION", "END", "ENDS", "ENGINE", "ENGINES", "ENUM", "ERROR", "ERRORS", "ESCAPE",
  "ESCAPED", "EVENT", "EVENTS", "EVERY", "EXCEPT", "EXCHANGE", "EXECUTE", "EXISTS", "EXIT",
  "EXPANSION", "EXPIRE", "EXPLAIN", "EXPORT", "EXTENDED", "EXTENT_SIZE", "FALSE", "FAST", "FAULTS",
  "FETCH", "FIELDS", "FILE", "FILE_BLOCK_SIZE", "FILTER", "FIRST", "FIRST_VALUE", "FIXED", "FLOAT",
  "FLOAT4", "FLOAT8", "FLUSH", "FOLLOWING", "FOLLOWS", "FOR", "FORCE", "FOREIGN", "FORMAT", "FOUND",
  "FROM", "FULL", "FULLTEXT", "FUNCTION", "GENERAL", "GENERATED", "GEOMCOLLECTION", "GEOMETRY",
  "GEOMETRYCOLLECTION", "GET", "GET_FORMAT", "GLOBAL", "GRANT", "GRANTS", "GROUP", "GROUPING",
  "GROUPS", "GROUP_REPLICATION", "HANDLER", "HASH", "HAVING", "HELP", "HIGH_PRIORITY", "HOST",
  "HOSTS", "HOUR", "HOUR_MICROSECOND", "HOUR_MINUTE", "HOUR_SECOND", "IDENTIFIED", "IF", "IGNORE",
  "IGNORE_SERVER_IDS", "IMPORT", "IN", "INDEX", "INDEXES", "INFILE", "INITIAL_SIZE", "INNER",
  "INOUT", "INSENSITIVE", "INSERT", "INSERT_METHOD", "INSTALL", "INSTANCE", "INT", "INT1", "INT2",
  "INT3", "INT4", "INT8", "INTEGER", "INTERSECT", "INTERVAL", "INTO", "INVISIBLE", "INVOKER", "IO",
  "IO_AFTER_GTIDS", "IO_BEFORE_GTIDS", "IO_THREAD", "IPC", "IS", "ISOLATION", "ISSUER", "ITERATE",
  "JOIN", "JSON", "JSON_TABLE", "KEY", "KEYS", "KEY_BLOCK_SIZE", "KILL", "LAG", "LANGUAGE", "LAST",
  "LAST_VALUE", "LATERAL", "LEAD", "LEADING", "LEAVE", "LEAVES", "LEFT", "LESS", "LEVEL", "LIKE",
  "LIMIT", "LINEAR", "LINES", "LINESTRING", "LIST", "LOAD", "LOCAL", "LOCALTIME", "LOCALTIMESTAMP",
  "LOCK", "LOCKED", "LOCKS", "LOGFILE", "LOGS", "LONG", "LONGBLOB", "LONGTEXT", "LOOP",
  "LOW_PRIORITY", "MASTER", "MASTER_AUTO_POSITION", "MASTER_BIND", "MASTER_CONNECT_RETRY",
  "MASTER_DELAY", "MASTER_HEARTBEAT_PERIOD", "MASTER_HOST", "MASTER_LOG_FILE", "MASTER_LOG_POS",
  "MASTER_PASSWORD", "MASTER_PORT", "MASTER_RETRY_COUNT", "MASTER_SERVER_ID", "MASTER_SSL",
  "MASTER_SSL_CA", "MASTER_SSL_CAPATH", "MASTER_SSL_CERT", "MASTER_SSL_CIPHER", "MASTER_SSL_CRL",
  "MASTER_SSL_CRLPATH", "MASTER_SSL_KEY", "MASTER_SSL_VERIFY_SERVER_CERT", "MASTER_TLS_VERSION",
  "MASTER_USER", "MATCH", "MAXVALUE", "MAX_CONNECTIONS_PER_HOUR", "MAX_QUERIES_PER_HOUR",
  "MAX_ROWS", "MAX_SIZE", "MAX_STATEMENT_TIME", "MAX_UPDATES_PER_HOUR", "MAX_USER_CONNECTIONS",
  "MEDIUM", "MEDIUMBLOB", "MEDIUMINT", "MEDIUMTEXT", "MEMORY", "MERGE", "MESSAGE_TEXT",
  "MICROSECOND", "MIDDLEINT", "MIGRATE", "MINUTE", "MINUTE_MICROSECOND", "MINUTE_SECOND",
  "MIN_ROWS", "MOD", "MODE", "MODIFIES", "MODIFY", "MONTH", "MULTILINESTRING", "MULTIPOINT",
  "MULTIPOLYGON", "MUTEX", "MYSQL_ERRNO", "NAME", "NAMES", "NATIONAL", "NATURAL", "NCHAR", "NDB",
  "NDBCLUSTER", "NESTED", "NETWORK_NAMESPACE", "NEVER", "NEW", "NEXT", "NO", "NODEGROUP", "NONE",
  "NOT", "NOWAIT", "NO_WAIT", "NO_WRITE_TO_BINLOG", "NTH_VALUE", "NTILE", "NULL", "NULLS", "NUMBER",
  "NUMERIC", "NVARCHAR", "OF", "OFF", "OFFSET", "OJ", "OLD", "ON", "ONE", "ONLY", "OPEN",
  "OPTIMIZE", "OPTIMIZER_COSTS", "OPTION", "OPTIONAL", "OPTIONALLY", "OPTIONS", "OR", "ORDER",
  "ORDINALITY", "ORGANIZATION", "OTHERS", "OUT", "OUTER", "OUTFILE", "OVER", "OWNER", "PACK_KEYS",
  "PAGE", "PARSE_GCOL_EXPR", "PARSER", "PARTIAL", "PARTITION", "PARTITIONING", "PARTITIONS",
  "PASSWORD", "PATH", "PERCENT_RANK", "PERSIST", "PERSIST_ONLY", "PHASE", "PLUGIN", "PLUGINS",
  "PLUGIN_DIR", "POINT", "POLYGON", "PORT", "PRECEDES", "PRECEDING", "PRECISION", "PREPARE",
  "PRESERVE", "PREV", "PRIMARY", "PRIVILEGES", "PROCEDURE", "PROCESS", "PROCESSLIST", "PROFILE",
  "PROFILES", "PROXY", "PURGE", "QUARTER", "QUERY", "QUICK", "RANGE", "RANK", "READ", "READS",
  "READ_ONLY", "READ_WRITE", "REAL", "REBUILD", "RECOVER", "RECURSIVE", "REDOFILE",
  "REDO_BUFFER_SIZE", "REDUNDANT", "REFERENCE", "REFERENCES", "REGEXP", "RELAY", "RELAYLOG",
  "RELAY_LOG_FILE", "RELAY_LOG_POS", "RELAY_THREAD", "RELEASE", "RELOAD", "REMOVE", "RENAME",
  "REORGANIZE", "REPAIR", "REPEAT", "REPEATABLE", "REPLACE", "REPLICA", "REPLICAS",
  "REPLICATE_DO_DB", "REPLICATE_DO_TABLE", "REPLICATE_IGNORE_DB", "REPLICATE_IGNORE_TABLE",
  "REPLICATE_REWRITE_DB", "REPLICATE_WILD_DO_TABLE", "REPLICATE_WILD_IGNORE_TABLE", "REPLICATION",
  "REQUIRE", "RESET", "RESIGNAL", "RESOURCE", "RESPECT", "RESTART", "RESTORE", "RESTRICT", "RESUME",
  "RETURN", "RETURNED_SQLSTATE", "RETURNING", "RETURNS", "REVERSE", "REVOKE", "RIGHT", "RLIKE",
  "ROLE", "ROLLBACK", "ROLLUP", "ROTATE", "ROUTINE", "ROW", "ROWS", "ROW_COUNT", "ROW_FORMAT",
  "ROW_NUMBER", "RTREE", "SAVEPOINT", "SCHEDULE", "SCHEMA", "SCHEMAS", "SCHEMA_NAME", "SECOND",
  "SECOND_MICROSECOND", "SECURITY", "SELECT", "SENSITIVE", "SEPARATOR", "SERIAL", "SERIALIZABLE",
  "SERVER", "SESSION", "SET", "SHARE", "SHOW", "SHUTDOWN", "SIGNAL", "SIGNED", "SIMPLE", "SKIP",
  "SLAVE", "SLOW", "SMALLINT", "SNAPSHOT", "SOCKET", "SOME", "SONAME", "SOUNDS", "SOURCE",
  "SPATIAL", "SPECIFIC", "SQL", "SQLEXCEPTION", "SQLSTATE", "SQLWARNING", "SQL_AFTER_GTIDS",
  "SQL_AFTER_MTS_GAPS", "SQL_BEFORE_GTIDS", "SQL_BIG_RESULT", "SQL_BUFFER_RESULT", "SQL_CACHE",
  "SQL_CALC_FOUND_ROWS", "SQL_NO_CACHE", "SQL_SMALL_RESULT", "SQL_THREAD", "SQL_TSI_DAY",
  "SQL_TSI_HOUR", "SQL_TSI_MINUTE", "SQL_TSI_MONTH", "SQL_TSI_QUARTER", "SQL_TSI_SECOND",
  "SQL_TSI_WEEK", "SQL_TSI_YEAR", "SRID", "SSL", "STACKED", "START", "STARTING", "STARTS",
  "STATS_AUTO_RECALC", "STATS_PERSISTENT", "STATS_SAMPLE_PAGES", "STATUS", "STOP", "STORAGE",
  "STORED", "STRAIGHT_JOIN", "STRING", "SUBCLASS_ORIGIN", "SUBJECT", "SUBPARTITION",
  "SUBPARTITIONS", "SUPER", "SUSPEND", "SWAPS", "SWITCHES", "SYSTEM", "TABLE", "TABLES",
  "TABLESPACE", "TABLE_CHECKSUM", "TABLE_NAME", "TEMPORARY", "TEMPTABLE", "TERMINATED", "TEXT",
  "THAN", "THEN", "THREAD_PRIORITY", "TIES", "TIME", "TIMESTAMP", "TIMESTAMPADD", "TIMESTAMPDIFF",
  "TINYBLOB", "TINYINT", "TINYTEXT", "TO", "TRAILING", "TRANSACTION", "TRIGGER", "TRIGGERS", "TRUE",
  "TRUNCATE", "TYPE", "TYPES", "UNBOUNDED", "UNCOMMITTED", "UNDEFINED", "UNDO", "UNDOFILE",
  "UNDO_BUFFER_SIZE", "UNICODE", "UNINSTALL", "UNION", "UNIQUE", "UNKNOWN", "UNLOCK", "UNSIGNED",
  "UNTIL", "UPDATE", "UPGRADE", "USAGE", "USE", "USER", "USER_RESOURCES", "USE_FRM", "USING",
  "UTC_DATE", "UTC_TIME", "UTC_TIMESTAMP", "VALIDATION", "VALUE", "VALUES", "VARBINARY", "VARCHAR",
  "VARCHARACTER", "VARIABLES", "VARYING", "VCPU", "VIEW", "VIRTUAL", "VISIBLE", "WAIT", "WARNINGS",
  "WEEK", "WEIGHT_STRING", "WHEN", "WHERE", "WHILE", "WINDOW", "WITH", "WITHOUT", "WORK", "WRAPPER",
  "WRITE", "X509", "XA", "XID", "XML", "XOR", "YEAR", "YEAR_MONTH", "ZEROFILL",

  // ClickHouse关键字 (常用关键字)
  "SELECT", "FROM", "WHERE", "GROUP", "BY", "ORDER", "LIMIT", "HAVING", "UNION", "ALL", "DISTINCT",
  "AS", "ON", "USING", "JOIN", "INNER", "LEFT", "RIGHT", "FULL", "CROSS", "NATURAL", "OUTER",
  "CREATE", "TABLE", "DATABASE", "VIEW", "MATERIALIZED", "DICTIONARY", "FUNCTION", "DROP", "ALTER",
  "RENAME", "ATTACH", "DETACH", "CHECK", "OPTIMIZE", "FINAL", "SAMPLE", "ARRAY", "TUPLE", "DATE",
  "DATETIME", "STRING", "FIXEDSTRING", "UINT8", "UINT16", "UINT32", "UINT64", "INT8", "INT16",
  "INT32", "INT64", "FLOAT32", "FLOAT64", "DECIMAL", "DECIMAL32", "DECIMAL64", "DECIMAL128", "UUID",
  "ENUM8", "ENUM16", "LOWCARDINALITY", "NULLABLE", "NESTED", "AGGREGATEFUNCTION",
  "SIMPLEAGGREGATEFUNCTION", "INSERT", "INTO", "VALUES", "UPDATE", "DELETE", "SET", "SHOW",
  "DESCRIBE", "DESC", "EXISTS", "KILL", "PROCESSLIST", "SYSTEM", "RELOAD", "SHUTDOWN", "SYNC",
  "RESTART", "REPLICA", "REPLICAS", "FREEZE", "FETCH", "MOVE", "SETTINGS", "FORMAT", "INPUT",
  "OUTPUT", "TEMPORARY", "GLOBAL", "LOCAL", "PREWHERE", "ARRAYJOIN", "WITH", "TOTALS", "ROLLUP",
  "CUBE", "WINDOW", "PARTITION", "ORDER", "PRIMARY", "KEY", "SAMPLE", "SETTINGS", "TTL", "CODEC",
  "DEFAULT", "MATERIALIZED", "ALIAS", "COMMENT", "ENGINE", "POPULATE", "IF", "NOT", "IN", "AND",
  "OR", "LIKE", "ILIKE", "REGEXP", "MATCH", "EXTRACT", "SUBSTRING", "REPLACE", "POSITION", "LENGTH",
  "CHAR_LENGTH", "BIT_LENGTH", "OCTET_LENGTH", "LOWER", "UPPER", "LTRIM", "RTRIM", "TRIM", "CONCAT",
  "CONCAT_WS", "FORMAT", "TO_STRING", "TO_FIXED_STRING", "TO_DATE", "TO_DATETIME",
  "TO_UNIX_TIMESTAMP", "NOW", "TODAY", "YESTERDAY", "TIME_SLOT", "TO_START_OF_SECOND",
  "TO_START_OF_MINUTE", "TO_START_OF_FIVE_MINUTES", "TO_START_OF_TEN_MINUTES",
  "TO_START_OF_FIFTEEN_MINUTES", "TO_START_OF_HOUR", "TO_START_OF_DAY", "TO_START_OF_WEEK",
  "TO_START_OF_MONTH", "TO_START_OF_QUARTER", "TO_START_OF_YEAR", "TO_START_OF_ISO_YEAR",
  "TO_RELATIVE_SECOND_NUM", "TO_RELATIVE_MINUTE_NUM", "TO_RELATIVE_HOUR_NUM", "TO_RELATIVE_DAY_NUM",
  "TO_RELATIVE_WEEK_NUM", "TO_RELATIVE_MONTH_NUM", "TO_RELATIVE_QUARTER_NUM",
  "TO_RELATIVE_YEAR_NUM", "ADD_SECONDS", "ADD_MINUTES", "ADD_HOURS", "ADD_DAYS", "ADD_WEEKS",
  "ADD_MONTHS", "ADD_QUARTERS", "ADD_YEARS", "SUBTRACT_SECONDS", "SUBTRACT_MINUTES",
  "SUBTRACT_HOURS", "SUBTRACT_DAYS", "SUBTRACT_WEEKS", "SUBTRACT_MONTHS", "SUBTRACT_QUARTERS",
  "SUBTRACT_YEARS", "DATE_DIFF", "TIMESTAMP_ADD", "TIMESTAMP_SUB", "DATE_TRUNC", "DATE_PART",
  "EXTRACT", "COUNT", "SUM", "AVG", "MIN", "MAX", "ANY", "ANY_LAST", "STDDEV_POP", "STDDEV_SAMP",
  "VAR_POP", "VAR_SAMP", "COVAR_POP", "COVAR_SAMP", "CORR", "MEDIAN", "QUANTILE", "QUANTILES",
  "UNIQ", "UNIQ_COMBINED", "UNIQ_EXACT", "GROUP_ARRAY", "GROUP_UNIQ_ARRAY", "GROUP_BIT_AND",
  "GROUP_BIT_OR", "GROUP_BIT_XOR", "ARG_MIN", "ARG_MAX", "ARG_MIN_MT", "ARG_MAX_MT", "COUNT_IF",
  "SUM_IF", "AVG_IF", "MIN_IF", "MAX_IF", "ANY_IF", "STDDEV_POP_IF", "STDDEV_SAMP_IF", "VAR_POP_IF",
  "VAR_SAMP_IF", "COVAR_POP_IF", "COVAR_SAMP_IF", "CORR_IF", "MEDIAN_IF", "QUANTILE_IF", "UNIQ_IF",
  "GROUP_ARRAY_IF", "GROUP_UNIQ_ARRAY_IF", "GROUP_BIT_AND_IF", "GROUP_BIT_OR_IF",
  "GROUP_BIT_XOR_IF", "ROW_NUMBER", "RANK", "DENSE_RANK", "NTILE", "LAG", "LEAD", "FIRST_VALUE",
  "LAST_VALUE", "NTH_VALUE"};

static bool isValidName(const string& name) {
    // 名称规则: 英文字母、数字、_ 组成，且首字母不能为数字

    // 检查名称是否为空
    if (name.empty()) {
        return false;
    }

    // 检查首字符是否为字母或下划线
    if (!isalpha(name[0]) && name[0] != '_') {
        return false;
    }

    // 检查其余字符是否都是字母、数字或下划线
    for (size_t i = 1; i < name.length(); ++i) {
        if (!isalnum(name[i]) && name[i] != '_') {
            return false;
        }
    }

    // 转换为大写进行关键字检查
    std::string upper_name = name;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);

    // 检查是否为数据库关键字
    if (DATABASE_KEYWORDS.find(upper_name) != DATABASE_KEYWORDS.end()) {
        return false;
    }

    return true;
}

HKU_API std::ostream& operator<<(std::ostream& os, const FactorMeta& factor) {
    string strip("  \n");
    os << "FactorMeta(";
    os << strip << "name: " << factor.m_data->name << strip << "ktype: " << factor.m_data->ktype
       << strip << "is_active: " << factor.m_data->is_active << strip
       << "create_at: " << factor.m_data->create_at.str() << strip
       << "update_at: " << factor.m_data->update_at.str() << strip
       << "formula: " << factor.m_data->formula() << strip << "brief: " << factor.m_data->brief
       << strip << "detail: " << factor.m_data->details << ")";
    return os;
}

string FactorMeta::str() const {
    std::ostringstream os;
    os << *this;
    return os.str();
}

FactorMeta::FactorMeta() : m_data(make_shared<Data>()) {}

FactorMeta::FactorMeta(const string& name, const Indicator& formula, const KQuery::KType& ktype,
                       const string& brief, const string& details)
: FactorMeta() {
    HKU_CHECK(isValidName(name),
              htr("Illegal name! Naming rules: Only letters, digits and underscores (_) allowed; "
                  "cannot start with a digit."));
    Indicator ind = formula.clone();
    IndicatorImp* imp_ptr = ind.getImp().get();
    HKU_CHECK(imp_ptr, "Invalid formula indicator!");
    ind.setContext(KData());

    IPriceList* tmp_ptr = dynamic_cast<IPriceList*>(imp_ptr);
    if (tmp_ptr) {
        ind = PRICELIST();
    }

    m_data->name = name;
    m_data->ktype = ktype;
    m_data->brief = brief;
    m_data->details = details;
    m_data->formula = ind;
    m_data->create_at = Datetime::now();
    m_data->is_active = true;
}

FactorMeta::FactorMeta(const FactorMeta& other) : FactorMeta() {
    m_data = other.m_data;
}

FactorMeta::FactorMeta(const FactorMeta&& other) : FactorMeta() {
    m_data = std::move(other.m_data);
}

FactorMeta& FactorMeta::operator=(const FactorMeta& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data->name = other.m_data->name;
    m_data->ktype = other.m_data->ktype;
    m_data->brief = other.m_data->brief;
    m_data->details = other.m_data->details;
    m_data->formula = other.m_data->formula;
    m_data->create_at = other.m_data->create_at;
    m_data->update_at = other.m_data->update_at;
    m_data->is_active = other.m_data->is_active;
    return *this;
}

FactorMeta& FactorMeta::operator=(const FactorMeta&& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data->name = std::move(other.m_data->name);
    m_data->ktype = std::move(other.m_data->ktype);
    m_data->brief = std::move(other.m_data->brief);
    m_data->details = std::move(other.m_data->details);
    m_data->formula = std::move(other.m_data->formula);
    m_data->create_at = std::move(other.m_data->create_at);
    m_data->update_at = std::move(other.m_data->update_at);
    m_data->is_active = other.m_data->is_active;
    other.m_data->is_active = false;
    return *this;
}

uint64_t FactorMeta::hash() const noexcept {
    XXH64_state_t* state = XXH64_createState();
    HKU_IF_RETURN(!state, 0);

    uint64_t seed = 0;
    XXH64_reset(state, seed);

    XXH64_update(state, m_data->name.data(), m_data->name.size());
    XXH64_update(state, m_data->ktype.data(), m_data->ktype.size());

    uint64_t result = XXH64_digest(state);
    XXH64_freeState(state);
    return result;
}

Indicator FactorMeta::getIndicator(const Stock& stk, const KQuery& query) const {
    HKU_CHECK(!stk.isNull(), "Stock is null!");

    Indicator ret = m_data->formula.clone();
    auto imp = ret.getImp();
    HKU_CHECK(imp, "Invalid formula indicator!");

    auto k = stk.getKData(query);
    imp->onlySetContext(k);
    imp->_readyBuffer(k.size(), 1);

    return ret;
}

}  // namespace hku