/*
 * KQuery.cpp
 *
 *  Created on: 2012-9-23
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "KQuery.h"

namespace hku {

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
    switch(dataType) {
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

    //BTCÀ©Õ¹
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
    }
}

KQuery::KType KQuery::getKTypeEnum(const string& arg) {
    string name(arg);
    boost::to_upper(name);
    KType result;
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

    //BTCÀ©Õ¹
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
    return result;
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


