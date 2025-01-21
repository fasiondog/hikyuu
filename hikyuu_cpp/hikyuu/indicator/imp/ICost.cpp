/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-12-08
 *     Author: fasiondog
 */

#include "../crt/COST.h"
#include "../crt/DMA.h"
#include "../crt/COST.h"

#include "ICost.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICost)
#endif

namespace hku {

ICost::ICost() : IndicatorImp("COST", 1) {
    setParam<double>("percent", 10.0);
}

ICost::~ICost() {}

ICost::ICost(const KData& k, double percent) : IndicatorImp("COST", 1) {
    setParam<KData>("kdata", k);
    setParam<double>("percent", percent);
    ICost::_calculate(Indicator());
}

void ICost::_checkParam(const string& name) const {
    if (name == "percent") {
        double percent = getParam<double>("percent");
        HKU_CHECK(percent >= 0.0 && percent <= 100.0, "Invalid param percent: {}", percent);
    }
}

// 实际算法：DMA(CLOSE() + (HIGH() - LOW()) * x / 100.0, HSL());
void ICost::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);

    Stock stock = k.getStock();
    StockWeightList sw_list = stock.getWeight();
    if (sw_list.size() == 0) {
        return;
    }

    value_t percent = getParam<double>("percent") * 0.01;

    auto* kdata = k.data();
    auto* dst = this->data();

    size_t pos = 0;
    auto sw_iter = sw_list.begin();
    price_t pre_free_count = sw_iter->freeCount();
    value_t a = 0.0, x = 0.0;
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t free_count = sw_iter->freeCount();
        if (free_count == 0) {
            continue;  // 忽略流通盘为0的权息
        }

        while (pos < total && kdata[pos].datetime < sw_iter->datetime()) {
            const KRecord& krecord = kdata[pos];
            x = krecord.closePrice + (krecord.highPrice - krecord.lowPrice) * percent;
            a = krecord.transCount / pre_free_count;
            dst[pos] = pos >= 1 ? a * x + (1 - a) * dst[pos - 1] : a * x;
            pos++;
        }

        pre_free_count = free_count;
        if (pos >= total) {
            break;
        }
    }

    for (; pos < total; pos++) {
        const KRecord& krecord = kdata[pos];
        x = krecord.closePrice + (krecord.highPrice - krecord.lowPrice) * percent;
        a = krecord.transCount / pre_free_count;
        dst[pos] = a * x + (1 - a) * dst[pos - 1];
    }
}

Indicator HKU_API COST(double x) {
    auto p = make_shared<ICost>();
    p->setParam<double>("percent", x);
    return Indicator(p);
}

Indicator HKU_API COST(const KData& k, double x) {
    return Indicator(make_shared<ICost>(k, x));
}

Indicator HKU_API COST2(double x) {
    Indicator ind = DMA(CLOSE() + (HIGH() - LOW()) * x / 100.0, COST());
    ind.name("COST");
    ind.setParam<double>("x", x);
    return ind;
}

Indicator HKU_API COST2(const KData& k, double x) {
    Indicator ind = COST(x);
    ind.setContext(k);
    return ind;
}

}  // namespace hku