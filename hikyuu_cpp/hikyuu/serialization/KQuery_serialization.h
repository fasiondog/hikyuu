/*
 * KQuery.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef SERIALIZATION_KQUERY_H_
#define SERIALIZATION_KQUERY_H_

#include "../KQuery.h"
#include "../config.h"

#if HKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {

template <class Archive>
void save(Archive& ar, const hku::KQuery& query, unsigned int version) {
    hku::string queryType, kType, recoverType;
    queryType = hku::KQuery::getQueryTypeName(query.queryType());
    kType = hku::KQuery::getKTypeName(query.kType());
    recoverType = hku::KQuery::getRecoverTypeName(query.recoverType());
    ar& BOOST_SERIALIZATION_NVP(queryType);
    ar& BOOST_SERIALIZATION_NVP(kType);
    ar& BOOST_SERIALIZATION_NVP(recoverType);

    if (query.queryType() == hku::KQuery::INDEX) {
        hku::int64_t start = query.start();
        hku::int64_t end = query.end();
        ar& BOOST_SERIALIZATION_NVP(start);
        ar& BOOST_SERIALIZATION_NVP(end);
    } else if (query.queryType() == hku::KQuery::DATE) {
        hku::uint64_t start = query.startDatetime().number();
        hku::uint64_t end = query.endDatetime().number();
        ar& BOOST_SERIALIZATION_NVP(start);
        ar& BOOST_SERIALIZATION_NVP(end);
    } else {
        // 非法忽略
    }
}

template <class Archive>
void load(Archive& ar, hku::KQuery& query, unsigned int version) {
    hku::string queryType, kType, recoverType;
    ar& BOOST_SERIALIZATION_NVP(queryType);
    ar& BOOST_SERIALIZATION_NVP(kType);
    ar& BOOST_SERIALIZATION_NVP(recoverType);

    hku::KQuery::QueryType enum_query = hku::KQuery::getQueryTypeEnum(queryType);
    hku::KQuery::KType enmu_ktype = hku::KQuery::getKTypeEnum(kType);
    hku::KQuery::RecoverType enum_recover = hku::KQuery::getRecoverTypeEnum(recoverType);

    if (enum_query == hku::KQuery::INDEX) {
        hku::int64_t start, end;
        ar& BOOST_SERIALIZATION_NVP(start);
        ar& BOOST_SERIALIZATION_NVP(end);
        query = hku::KQuery(start, end, enmu_ktype, enum_recover);
    } else if (enum_query == hku::KQuery::DATE) {
        hku::uint64_t start, end;
        ar& BOOST_SERIALIZATION_NVP(start);
        ar& BOOST_SERIALIZATION_NVP(end);
        query =
          hku::KQueryByDate(hku::Datetime(start), hku::Datetime(end), enmu_ktype, enum_recover);
    } else {
        // 非法忽略
    }
}

}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::KQuery)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* SERIALIZATION_KQUERY_H_ */
