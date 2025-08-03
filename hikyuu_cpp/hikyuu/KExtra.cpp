/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-03
 *      Author: fasiondog
 */

#include "KQuery.h"
#include "KExtra.h"

namespace hku {

static std::unordered_map<string, KExtra> g_ktype_extra;

KExtra::KExtra(KExtra&& rhs)
: ktype(std::move(rhs.ktype)),
  basetype(std::move(rhs.basetype)),
  getPhaseEnd(std::move(rhs.getPhaseEnd)),
  getMinutes(std::move(rhs.getMinutes)) {}

KExtra& KExtra::operator=(KExtra&& rhs) {
    if (this != &rhs) {
        ktype = std::move(rhs.ktype);
        basetype = std::move(rhs.basetype);
        getPhaseEnd = std::move(rhs.getPhaseEnd);
        getMinutes = std::move(rhs.getMinutes);
    }
    return *this;
}

KRecordList KExtra::getExtraKRecordList(const KRecordList& kdata) const {
    KRecordList ret;
    HKU_IF_RETURN(kdata.empty(), ret);

    KRecord k0 = kdata[0];
    k0.datetime = getPhaseEnd(k0.datetime);
    ret.push_back(std::move(k0));

    for (size_t i = 1, len = kdata.size(); i < len; i++) {
        const KRecord& k = kdata[i];
        KRecord& retk = ret.back();
        Datetime enddate = getPhaseEnd(k.datetime);
        if (enddate != retk.datetime) {
            ret.emplace_back(enddate, k.openPrice, k.highPrice, k.lowPrice, k.closePrice,
                             k.transAmount, k.transCount);
        } else {
            retk.closePrice = k.closePrice;
            if (k.highPrice > retk.highPrice) {
                retk.highPrice = k.highPrice;
            }
            if (k.lowPrice < retk.lowPrice) {
                retk.lowPrice = k.lowPrice;
            }
            retk.transAmount += k.transAmount;
            retk.transCount += k.transCount;
        }
    }

    return ret;
}

void HKU_API registerKTypeExtra(const string& ktype, const string& basetype,
                                const std::function<Datetime(const Datetime&)>& getPhaseEnd,
                                const std::function<int32_t()>& getMinutes) {
    HKU_CHECK(getPhaseEnd && getMinutes, "Invalid function getPhaseEnd or getMinutes!");

    string nktype(ktype);
    to_upper(nktype);
    HKU_CHECK(!KQuery::isBaseKType(nktype), _tr("Invalid ktype: {}! It's bastktype."), ktype);

    string nbasetype(basetype);
    to_upper(nbasetype);
    HKU_CHECK(KQuery::isBaseKType(nbasetype), _tr("Invalid basetype: {}!"), basetype);

    auto iter = g_ktype_extra.find(nktype);
    HKU_CHECK(iter == g_ktype_extra.end(), _tr("ktype: {} is already registered!"), ktype);

    KExtra extra;
    extra.ktype = nktype;
    extra.basetype = nbasetype;
    extra.getPhaseEnd = getPhaseEnd;
    extra.getMinutes = getMinutes;
    g_ktype_extra.insert(std::make_pair(nktype, std::move(extra)));
}

bool HKU_API isExtraKType(const string& ktype) {
    string nktype(ktype);
    to_upper(nktype);
    auto iter = g_ktype_extra.find(nktype);
    return iter != g_ktype_extra.end();
}

const KExtra& getKExtra(const string& ktype) {
    string nktype(ktype);
    to_upper(nktype);
    auto iter = g_ktype_extra.find(nktype);
    HKU_CHECK(iter != g_ktype_extra.end(), _tr("Not register extra ktype: {}!"), ktype);
    return iter->second;
}

void HKU_API releaseKExtra() {
    g_ktype_extra.clear();
}

}  // namespace hku