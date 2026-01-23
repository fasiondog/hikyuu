/*
 * KQuery.cpp
 *
 *  Created on: 2012-9-23
 *      Author: fasiondog
 */

#include <boost/functional/hash.hpp>
#include <xxhash.h>
#include "plugin/hkuextra.h"
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

const string KQuery::DAY3{"DAY3"};
const string KQuery::DAY5{"DAY5"};
const string KQuery::DAY7{"DAY7"};
const string KQuery::MIN3("MIN3");
const string KQuery::HOUR2("HOUR2");
const string KQuery::HOUR4("HOUR4");
const string KQuery::HOUR6("HOUR6");
const string KQuery::HOUR12("HOUR12");
const string KQuery::TIMELINE("TIMELINE");  // 分时
const string KQuery::TRANS("TRANS");        // 分笔

// 所有基础K线类型（即有实际物理存储的K线类型）
static std::unordered_set<string> g_all_base_ktype{
  KQuery::MIN,  KQuery::MIN5,  KQuery::MIN15,    KQuery::MIN30,   KQuery::MIN60,
  KQuery::DAY,  KQuery::WEEK,  KQuery::MONTH,    KQuery::QUARTER, KQuery::HALFYEAR,
  KQuery::YEAR, KQuery::HOUR2, KQuery::TIMELINE, KQuery::TRANS};

static unordered_map<string, int32_t> g_ktype2min{
  {KQuery::MIN, 1},
  {KQuery::MIN3, 3},

  {KQuery::MIN5, 5},
  {KQuery::MIN15, 15},
  {KQuery::MIN30, 30},
  {KQuery::MIN60, 60},
  {KQuery::HOUR2, 60 * 2},

  {KQuery::DAY, 60 * 24},
  {KQuery::WEEK, 60 * 24 * 7},
  {KQuery::MONTH, 60 * 24 * 30},
  {KQuery::QUARTER, 60 * 24 * 30 * 3},
  {KQuery::HALFYEAR, 60 * 24 * 30 * 6},
  {KQuery::YEAR, 60 * 24 * 365},

  {KQuery::TIMELINE, 1},
  //   {KQuery::TRANS, 1}
};

static unordered_map<string, int64_t> g_ktype2sec{
  {KQuery::TRANS, 3},
};

// 获取所有的 KType
vector<KQuery::KType> KQuery::getBaseKTypeList() noexcept {
    vector<KQuery::KType> ret;
    for (const auto& v : g_all_base_ktype) {
        ret.push_back(v);
    }
    return ret;
}

vector<KQuery::KType> KQuery::getExtraKTypeList() {
    return hku::getExtraKTypeList();
}

int32_t KQuery::getKTypeInMin(const KType& ktype) {
    string nktype(ktype);
    to_upper(nktype);
    auto iter = g_ktype2min.find(nktype);
    HKU_IF_RETURN(iter != g_ktype2min.end(), iter->second);

    int32_t sec = getKTypeExtraMinutes(nktype);
    HKU_WARN_IF(sec <= 0, "Can't get KType in minutes: {}, will return 0", nktype);
    return sec;
}

int32_t KQuery::getBaseKTypeInMin(const KType& ktype) noexcept {
    string nktype(ktype);
    to_upper(nktype);
    auto iter = g_ktype2min.find(nktype);
    if (iter != g_ktype2min.end()) {
        return iter->second;
    }
    HKU_WARN("Can't get base KType in minutes: {}, will return 0", nktype);
    return 0;
}

int64_t KQuery::getKTypeInSeconds(const KType& ktype) {
    string nktype(ktype);
    to_upper(nktype);
    auto iter = g_ktype2min.find(nktype);
    HKU_IF_RETURN(iter != g_ktype2min.end(), iter->second * 60);

    auto sec = getKTypeExtraMinutes(nktype) * 60;
    if (sec <= 0) {
        auto sec_iter = g_ktype2sec.find(nktype);
        HKU_IF_RETURN(sec_iter != g_ktype2sec.end(), sec_iter->second);
        HKU_WARN("Can't get KType in seconds: {}, will return 0", nktype);
    }
    return sec;
}

bool KQuery::isValidKType(const string& ktype) {
    return isBaseKType(ktype) || hku::isExtraKType(ktype);
}

bool KQuery::isBaseKType(const string& ktype) noexcept {
    string nktype(ktype);
    to_upper(nktype);
    auto iter = g_all_base_ktype.find(nktype);
    return iter != g_all_base_ktype.end();
}

bool KQuery::isExtraKType(const string& ktype) {
    return hku::isExtraKType(ktype);
}

KQuery::KQuery(Datetime start, Datetime end, const KType& ktype, RecoverType recoverType)
: m_start(start.ymdhms()),
  m_end(end.ymdhms()),
  m_queryType(KQuery::DATE),
  m_dataType(ktype),
  m_recoverType(recoverType) {
    to_upper(m_dataType);
}

Datetime KQuery::startDatetime() const {
    HKU_IF_RETURN(m_queryType != DATE || (uint64_t)m_start == Null<uint64_t>(), Null<Datetime>());
    return Datetime(m_start);
}

Datetime KQuery::endDatetime() const {
    HKU_IF_RETURN(m_queryType != DATE || (uint64_t)m_end == Null<uint64_t>(), Null<Datetime>());
    return Datetime(m_end);
}

uint64_t KQuery::hash() const {
    XXH64_state_t* state = XXH64_createState();
    HKU_IF_RETURN(!state, 0);

    uint64_t seed = 0;
    XXH64_reset(state, seed);
    XXH64_update(state, &m_start, sizeof(m_start));
    XXH64_update(state, &m_end, sizeof(m_end));
    XXH64_update(state, &m_queryType, sizeof(m_queryType));
    XXH64_update(state, &m_recoverType, sizeof(m_recoverType));
    XXH64_update(state, m_dataType.data(), m_dataType.size());

    // 获取最终哈希值
    uint64_t result = XXH64_digest(state);
    XXH64_freeState(state);
    return result;
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

string KQuery::getKTypeName(const KType& dataType) {
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

bool HKU_API operator!=(const KQuery& q1, const KQuery& q2) noexcept {
    // cppcheck-suppress [mismatchingContainerExpression]
    HKU_IF_RETURN(q1.queryType() != q2.queryType(), true);
    if (q1.queryType() == KQuery::DATE) {
        return q1.kType() != q2.kType() || q1.recoverType() != q2.recoverType() ||
               q1.startDatetime() != q2.startDatetime() || q1.endDatetime() != q2.endDatetime();
    }
    return q1.kType() != q2.kType() || q1.recoverType() != q2.recoverType() ||
           q1.start() != q2.start() || q1.end() != q2.end();
}

bool HKU_API operator==(const KQuery& q1, const KQuery& q2) noexcept {
    // cppcheck-suppress [mismatchingContainerExpression]
    HKU_IF_RETURN(q1.queryType() != q2.queryType(), false);
    if (q1.queryType() == KQuery::DATE) {
        return q1.kType() == q2.kType() && q1.recoverType() == q2.recoverType() &&
               q1.startDatetime() == q2.startDatetime() && q1.endDatetime() == q2.endDatetime();
    }
    return q1.kType() == q2.kType() && q1.recoverType() == q2.recoverType() &&
           q1.start() == q2.start() && q1.end() == q2.end();
}

}  // namespace hku
