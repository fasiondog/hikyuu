/*
 * KQuery.cpp
 *
 *  Created on: 2012-9-23
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "KQuery.h"

namespace hku {

const string KQuery::MIN("MIN");
const string KQuery::MIN5("MIN5");
const string KQuery::MIN15("MIN15");
const string KQuery::MIN30("MIN30");
const string KQuery::MIN60("MIN60");
const string KQuery::DAY("DAY");
const string KQuery::WEEK("WEEK");
const string KQuery::MONTH("MONTH");
const string KQuery::QUARTER("QUARTER");
const string KQuery::HALFYEAR("HALFYEAR");
const string KQuery::YEAR("YEAR");
const string KQuery::MIN3("MIN3");
const string KQuery::HOUR2("HOUR2");
const string KQuery::HOUR4("HOUR4");
const string KQuery::HOUR6("HOUR5");
const string KQuery::HOUR12("HOUR12");
// const string KQuery::INVALID_KTYPE("Z");

KQuery::KQuery(Datetime start, Datetime end, KType ktype, RecoverType recoverType)
: m_start(start == Null<Datetime>() ? (int64_t)start.number()
                                    : (int64_t)(start.number() * 100 + start.second())),
  m_end(end == Null<Datetime>() ? (int64_t)end.number()
                                : (int64_t)(end.number() * 100 + end.second())),
  m_queryType(KQuery::DATE),
  m_dataType(ktype),
  m_recoverType(recoverType) {
    to_upper(m_dataType);
}

Datetime KQuery::startDatetime() const {
    HKU_IF_RETURN(m_queryType != DATE || (uint64_t)m_start == Null<uint64_t>(), Null<Datetime>());
    uint64_t number = (uint64_t)(m_start / 100);
    Datetime d(number);
    return Datetime(d.year(), d.month(), d.day(), d.hour(), d.minute(), m_start - number * 100);
}

Datetime KQuery::endDatetime() const {
    HKU_IF_RETURN(m_queryType != DATE || (uint64_t)m_end == Null<uint64_t>(), Null<Datetime>());
    uint64_t number = (uint64_t)(m_end / 100);
    Datetime d(number);
    return Datetime(d.year(), d.month(), d.day(), d.hour(), d.minute(), m_end - number * 100);
}

string KQuery::getQueryTypeName(QueryType queryType) {
    switch (queryType) {
        case INDEX:
            return "INDEX";
        case DATE:
            return "DATE";
        default:
            return "INVALID";
    }
}

KQuery::QueryType KQuery::getQueryTypeEnum(const string& arg) {
    string name(arg);
    to_upper(name);
    HKU_IF_RETURN("INDEX" == name, INDEX);
    HKU_IF_RETURN("DATE" == name, DATE);
    return INVALID;
}

string KQuery::getKTypeName(KType dataType) {
    string result(dataType);
    to_upper(result);
    return result;
}

KQuery::KType KQuery::getKTypeEnum(const string& arg) {
    string name(arg);
    to_upper(name);
    return name;
}

string KQuery::getRecoverTypeName(RecoverType recoverType) {
    switch (recoverType) {
        case NO_RECOVER:
            return "NO_RECOVER";
        case FORWARD:
            return "FORWARD";
        case BACKWARD:
            return "BACKWARD";
        case EQUAL_FORWARD:
            return "EQUAL_FORWARD";
        case EQUAL_BACKWARD:
            return "EQUAL_BACKWARD";
        default:
            return "INVALID_RECOVER_TYPE";
    }
}

KQuery::RecoverType KQuery::getRecoverTypeEnum(const string& arg) {
    string name(arg);
    to_upper(name);
    HKU_IF_RETURN("NO_RECOVER" == name, NO_RECOVER);
    HKU_IF_RETURN("FORWARD" == name, FORWARD);
    HKU_IF_RETURN("BACKWARD" == name, BACKWARD);
    HKU_IF_RETURN("EQUAL_FORWARD" == name, EQUAL_FORWARD);
    HKU_IF_RETURN("EQUAL_BACKWARD" == name, EQUAL_BACKWARD);
    return INVALID_RECOVER_TYPE;
}

HKU_API std::ostream& operator<<(std::ostream& os, const KQuery& query) {
    string strip(", ");
    if (query.queryType() == KQuery::INDEX) {
        os << "KQuery(" << query.start() << strip << query.end() << strip
           << KQuery::getQueryTypeName(query.queryType()) << strip
           << KQuery::getKTypeName(query.kType()) << strip
           << KQuery::getRecoverTypeName(query.recoverType()) << ")";
    } else {
        os << "KQueryByDate(" << query.startDatetime() << strip << query.endDatetime() << strip
           << KQuery::getQueryTypeName(query.queryType()) << strip
           << KQuery::getKTypeName(query.kType()) << strip
           << KQuery::getRecoverTypeName(query.recoverType()) << ")";
    }
    return os;
}

}  // namespace hku
