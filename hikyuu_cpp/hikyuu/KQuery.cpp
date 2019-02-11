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
//const string KQuery::INVALID_KTYPE("Z");


KQuery KQueryByIndex(hku_int64 start, hku_int64 end,
        KQuery::KType dataType, KQuery::RecoverType recoverType) {
    return KQuery(start, end, dataType, recoverType, KQuery::INDEX);
}

KQuery KQueryByDate(const Datetime& start, const Datetime& end,
        KQuery::KType dataType, KQuery::RecoverType recoverType) {
    hku_int64 start_number = start == Null<Datetime>()
                              ? (hku_int64)start.number()
                              : (hku_int64)(start.number()*100 + start.second());
    hku_int64 end_number = end == Null<Datetime>()
                              ? (hku_int64)end.number()
                              : (hku_int64)(end.number()*100 + end.second());
    return KQuery(start_number, end_number,
                  dataType, recoverType, KQuery::DATE);
}


Datetime KQuery::startDatetime() const {
    if (m_queryType != DATE || (hku_uint64)m_start == Null<hku_uint64>()) {
        return Null<Datetime>();
    }

    hku_uint64 number = (hku_uint64)(m_start/100);
    Datetime d(number);
    return Datetime(d.year(), d.month(), d.day(), d.hour(),
                    d.minute(), m_start - number * 100);
}

Datetime KQuery::endDatetime() const {
    if (m_queryType != DATE || (hku_uint64)m_end == Null<hku_uint64>()) {
        return Null<Datetime>();
    }

    hku_uint64 number = (hku_uint64)(m_end/100);
    Datetime d(number);
    return Datetime(d.year(), d.month(), d.day(), d.hour(),
                    d.minute(), m_end - number * 100);
}

string KQuery::getQueryTypeName(QueryType queryType) {
    switch(queryType){
    case INDEX:
        return "INDEX";
    case DATE:
        return "DATE";
    default:
        return "INVALID";
    }
}

KQuery::QueryType KQuery::getQueryTypeEnum(const string& arg) {
    QueryType result;
    string name(arg);
    boost::to_upper(name);
    if ("INDEX" == name) {
        result = INDEX;
    } else if ("DATE" == name) {
        result = DATE;
    } else {
        result = INVALID;
    }
    return result;
}

string KQuery::getKTypeName(KType dataType) {
    return dataType;
    /*switch(dataType) {
    case MIN:
        return "MIN";
    case MIN5:
        return "MIN5";
    case MIN15:
        return "MIN15";
    case MIN30:
        return "MIN30";
    case MIN60:
        return "MIN60";
    case DAY:
        return "DAY";
    case WEEK:
        return "WEEK";
    case MONTH:
        return "MONTH";
    case QUARTER:
        return "QUARTER";
    case HALFYEAR:
        return "HALFYEAR";
    case YEAR:
        return "YEAR";

    //BTC扩展
    case MIN3:
        return "MIN3";
    case HOUR2:
        return "HOUR2";
    case HOUR4:
        return "HOUR4";
    case HOUR6:
        return "HOUR6";
    case HOUR12:
        return "HOUR12";

    default:
        return "INVALID_DATA_TYPE";
    }*/
}

KQuery::KType KQuery::getKTypeEnum(const string& arg) {
    string name(arg);
    boost::to_upper(name);
    return name;
   /* KType result;
    if ("MIN" == name) {
        result = MIN;
    } else if ("MIN5" == name) {
        result = MIN5;
    } else if ("MIN15" == name) {
        result = MIN15;
    } else if ("MIN30" == name) {
        result = MIN30;
    } else if ("MIN60" == name) {
        result = MIN60;
    } else if ("DAY" == name) {
        result = DAY;
    } else if ("WEEK" == name) {
        result = WEEK;
    } else if ("MONTH" == name) {
        result = MONTH;
    } else if ("QUARTER" == name) {
        result = QUARTER;
    } else if ("HALFYEAR" == name) {
        result = HALFYEAR;
    } else if ("YEAR" == name) {
        result = YEAR;

    //BTC扩展
    } else if ("MIN3" == name) {
        result = MIN3;
    } else if ("HOUR2" == name) {
        result = HOUR2;
    } else if ("HOUR4" == name) {
        result = HOUR4;
    } else if ("HOUR6" == name) {
        result = HOUR6;
    } else if ("HOUR12" == name) {
        result = HOUR12;
    } else {
        result = INVALID_KTYPE;
    }
    return result;*/
}

string KQuery::getRecoverTypeName(RecoverType recoverType) {
    switch(recoverType) {
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
    RecoverType result;
    string name(arg);
    boost::to_upper(name);
    if ("NO_RECOVER" == name) {
        result = NO_RECOVER;
    } else if ("FORWARD" == name) {
        result = FORWARD;
    } else if ("BACKWARD" == name) {
        result = BACKWARD;
    } else if ("EQUAL_FORWARD" == name) {
        result = EQUAL_FORWARD;
    } else if ("EQUAL_BACKWARD" == name) {
        result = EQUAL_BACKWARD;
    } else {
        result = INVALID_RECOVER_TYPE;
    }
    return result;
}


HKU_API std::ostream& operator <<(std::ostream &os, const KQuery& query){
    string strip(", ");
    if (query.queryType() == KQuery::INDEX) {
        os << "KQuery(" << query.start() << strip << query.end() << strip
           << KQuery::getQueryTypeName(query.queryType()) << strip
           << KQuery::getKTypeName(query.kType()) << strip
           << KQuery::getRecoverTypeName(query.recoverType()) << ")";
    } else {
        os << "KQueryByDate(" << query.startDatetime() << strip
           << query.endDatetime() << strip
           << KQuery::getQueryTypeName(query.queryType()) << strip
           << KQuery::getKTypeName(query.kType()) << strip
           << KQuery::getRecoverTypeName(query.recoverType()) << ")";
    }
    return os;
}

} /* namespace */


