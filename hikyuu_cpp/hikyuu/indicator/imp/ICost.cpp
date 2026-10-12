/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-12-08
 *     Author: fasiondog
 */

#include "ICost.h"
#include "StockWeightFill.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICost)
#endif

namespace hku {

ICost::ICost() : IndicatorImp("COST", 1) {
    m_need_context = true;
    setParam<double>("percent", 10.0);
}

ICost::~ICost() {}

void ICost::_checkParam(const string& name) const {
    if (name == "percent") {
        double percent = getParam<double>("percent");
        HKU_CHECK(percent >= 0.0 && percent <= 100.0, "Invalid param percent: {}", percent);
    }
}

// Assume the cost distribution: DMA(x, HSL=A) = A*X+(1-A)*Y'
// The actual algorithm: DMA(CLOSE() + (HIGH() - LOW()) * x / 100.0, HSL());
void ICost::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    // transCount is in lots on the daily line and above but in shares below it; freeCount is in
    // units of 10 thousand shares. The factor normalizes both to the turnover ratio (x 100 = %).
    bool day_or_above =
      KQuery::getKTypeInSeconds(k.getQuery().kType()) >= KQuery::getKTypeInSeconds(KQuery::DAY);
    value_t hsl_factor = day_or_above ? 0.01 : 0.0001;

    _readyBuffer(total, 1);

    value_t percent = getParam<double>("percent") * 0.01;
    auto* dst = this->data();
    // DMA state decoupled from the buffer: seed with x until the first recursive value exists
    bool has_pre_cost = false;
    value_t pre_cost = 0.0;
    fill_by_weight_series(
      k, 0, [](const StockWeight& w) { return w.freeCount(); },
      [&](size_t pos, const KRecord& krecord, price_t pre_free_count) {
          value_t x = krecord.closePrice + (krecord.highPrice - krecord.lowPrice) * percent;
          value_t a = krecord.transCount / pre_free_count * hsl_factor;
          dst[pos] = has_pre_cost ? a * x + (1 - a) * pre_cost : x;
          has_pre_cost = true;
          pre_cost = dst[pos];
      });

    updateDiscard(true);
}

Indicator HKU_API COST(double x) {
    auto p = make_shared<ICost>();
    p->setParam<double>("percent", x);
    return Indicator(p);
}

Indicator HKU_API COST(const KData& k, double x) {
    auto p = make_shared<ICost>();
    p->setParam<double>("percent", x);
    p->setContext(k);
    return Indicator(p);
}

}  // namespace hku
