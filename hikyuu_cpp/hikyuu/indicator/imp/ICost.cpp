/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-12-08
 *     Author: fasiondog
 */

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

// 假设成本分布：DMA(x, HSL=A) = A*X+(1-A)*Y'
// 实际算法：DMA(CLOSE() + (HIGH() - LOW()) * x / 100.0, HSL());
void ICost::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    // 先将 discard 设为全部，后续更新
    m_discard = total;

    Stock stock = k.getStock();
    auto* kdata = k.data();
    Datetime lastdate = kdata[total - 1].datetime.startOfDay();

    StockWeightList sw_list = stock.getWeight(Datetime::min(), lastdate + Days(1));
    HKU_IF_RETURN(sw_list.empty(), void());

    // 寻找第一个流通盘不为0的权息
    price_t pre_free_count = 0.0;
    Datetime pre_sw_date;
    auto sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        if (sw_iter->freeCount() > 0) {
            pre_free_count = sw_iter->freeCount();
            pre_sw_date = sw_iter->datetime();
            break;
        }
    }

    // 没有流通盘相关权息数据, 或者该权息日期大于最后一根K线日期, 直接返回
    HKU_IF_RETURN(sw_iter == sw_list.end() || pre_sw_date > lastdate, void());

    auto* dst = this->data();

    value_t percent = getParam<double>("percent") * 0.01;
    size_t pos = 0;
    value_t x, a;
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t free_count = sw_iter->freeCount();
        Datetime cur_sw_date = sw_iter->datetime();
        if (free_count <= 0.0) {
            continue;  // 忽略流通盘为0的权息
        }

        while (pos < total && kdata[pos].datetime < cur_sw_date) {
            const KRecord& krecord = kdata[pos];
            if (krecord.datetime >= pre_sw_date) {
                x = krecord.closePrice + (krecord.highPrice - krecord.lowPrice) * percent;
                // transCount 为手数，流通股为万股
                a = krecord.transCount / pre_free_count * 0.01;
                dst[pos] = pos > 0 ? a * x + (1 - a) * dst[pos - 1] : x;
            }
            pos++;
        }

        pre_free_count = free_count;
        pre_sw_date = cur_sw_date;
        if (pos >= total) {
            break;
        }
    }

    if (pos == 0) {
        const KRecord& krecord = kdata[pos];
        x = krecord.closePrice + (krecord.highPrice - krecord.lowPrice) * percent;
        dst[pos] = x;
        pos++;
    }

    for (; pos < total; pos++) {
        const KRecord& krecord = kdata[pos];
        x = krecord.closePrice + (krecord.highPrice - krecord.lowPrice) * percent;
        a = krecord.transCount / pre_free_count * 0.01;
        dst[pos] = a * x + (1 - a) * dst[pos - 1];
    }

    // 更新 discard
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(dst[i])) {
            m_discard = i;
            break;
        }
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

}  // namespace hku