/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "hikyuu/StockTypeInfo.h"
#include "hikyuu/indicator/crt/KDATA.h"
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/CVAL.h"
#include "hikyuu/indicator/crt/SLICE.h"
#include "IIndex.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIndex)
#endif

namespace hku {

IIndex::IIndex() : IndicatorImp("INDEX", 1) {
    setParam<bool>("fill_null", true);
    setParam<string>("kpart", "CLOSE");
    setParam<string>("market_code", "SH000001");
}

IIndex::IIndex(const string& kpart, bool fill_null) : IndicatorImp("INDEX", 1) {
    setParam<bool>("fill_null", fill_null);
    string part_name(kpart);
    to_upper(part_name);
    setParam<string>("kpart", part_name);
    setParam<string>("market_code", "SH000001");
}

IIndex::~IIndex() {}

void IIndex::_checkParam(const string& name) const {
    if ("kpart" == name) {
        string part = getParam<string>("kpart");
        HKU_ASSERT("OPEN" == part || "HIGH" == part || "LOW" == part || "CLOSE" == part ||
                   "AMO" == part || "VOL" == part);
    }
}

void IIndex::_calculate(const Indicator& ind) {
    auto k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    string market_code = getParam<string>("market_code");
    Stock stk = k.getStock();
    if (stk.type() == STOCKTYPE_A) {
        if (stk.market() == "SH") {
            market_code = "SH000001";
        } else if (stk.market() == "SZ") {
            market_code = "SZ399001";
        } else {
            HKU_WARN("Not known the index code, will use SH000001 as default.");
        }
    } else if (stk.type() == STOCKTYPE_A_BJ) {
        market_code = "BJ899050";
    } else if (stk.type() == STOCKTYPE_START) {
        market_code = "SH000688";
    } else if (stk.type() == STOCKTYPE_GEM) {
        market_code = "SZ399006";
    } else {
        HKU_WARN("Not known the index code, will use {} as default.", market_code);
    }

    // 调整 market_code 参数为当前指数 market_code
    setParam<string>("market_code", market_code);

    KQuery query = k.getQuery();
    auto minutes = KQuery::getKTypeInMin(query.kType());
    query = KQueryByDate(k[0].datetime, k[total - 1].datetime + Minutes(minutes), query.kType(),
                         query.recoverType());

    KData index_k = getKData(market_code, query);
    HKU_ASSERT(index_k.size() == total);
    Indicator index =
      ALIGN(KDATA_PART(index_k, getParam<string>("kpart")), k, getParam<bool>("fill_null"));

    HKU_ASSERT(index.size() == total);
    const auto* src = index.data();
    auto* dst = this->data();
    for (size_t i = 0; i < total; i++) {
        dst[i] = src[i];
    }
}

static Indicator INDEX(const string& kpart, bool fill_null) {
    IndicatorImpPtr p = make_shared<IIndex>(kpart, fill_null);
    if ("OPEN" == kpart) {
        p->name("INDEXO");
    } else if ("HIGH" == kpart) {
        p->name("INDEXH");
    } else if ("LOW" == kpart) {
        p->name("INDEXL");
    } else if ("CLOSE" == kpart) {
        p->name("INDEXC");
    } else if ("AMO" == kpart) {
        p->name("INDEXA");
    } else if ("VOL" == kpart) {
        p->name("INDEXV");
    }
    return p->calculate();
}

Indicator HKU_API INDEXO(bool fill_null) {
    return INDEX("OPEN", fill_null);
}

Indicator HKU_API INDEXH(bool fill_null) {
    return INDEX("HIGH", fill_null);
}

Indicator HKU_API INDEXL(bool fill_null) {
    return INDEX("LOW", fill_null);
}

Indicator HKU_API INDEXC(bool fill_null) {
    return INDEX("CLOSE", fill_null);
}

Indicator HKU_API INDEXA(bool fill_null) {
    return INDEX("AMO", fill_null);
}

Indicator HKU_API INDEXV(bool fill_null) {
    return INDEX("VOL", fill_null);
}

}  // namespace hku