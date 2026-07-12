#include "IAdx.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAdx)
#endif

namespace hku {

IAdx::IAdx() : IndicatorImp("ADX", 3) {
    m_need_context = true;
    setParam<int>("n", 14);
}

IAdx::~IAdx() {}

void IAdx::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 2);
    }
}

void IAdx::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 3);

    if (total < 2) {
        return;
    }

    auto* adx = this->data(0);
    auto* pdi = this->data(1);
    auto* mdi = this->data(2);

    int n = getParam<int>("n");
    size_t period = static_cast<size_t>(n);
    size_t adx_start = period + period - 1;

    if (total <= adx_start) {
        m_discard = total;
        return;
    }

    value_t smooth_factor = 1.0 / n;
    value_t prev_factor = 1.0 - smooth_factor;

    value_t tr_sum = 0.0;
    value_t pdm_sum = 0.0;
    value_t mdm_sum = 0.0;

    for (size_t i = 1; i <= period; i++) {
        const KRecord& r = k[i];
        const KRecord& prev_r = k[i - 1];

        value_t h = r.highPrice;
        value_t l = r.lowPrice;
        value_t prev_c = prev_r.closePrice;

        value_t tr1 = h - l;
        value_t tr2 = std::abs(h - prev_c);
        value_t tr3 = std::abs(l - prev_c);
        tr_sum += std::max({tr1, tr2, tr3});

        value_t up_move = h - prev_r.highPrice;
        value_t down_move = prev_r.lowPrice - l;

        if (up_move > down_move && up_move > 0) {
            pdm_sum += up_move;
        }
        if (down_move > up_move && down_move > 0) {
            mdm_sum += down_move;
        }
    }

    value_t tr_smooth = tr_sum / n;
    value_t pdm_smooth = pdm_sum / n;
    value_t mdm_smooth = mdm_sum / n;

    if (tr_smooth != 0.0) {
        pdi[period] = 100.0 * pdm_smooth / tr_smooth;
        mdi[period] = 100.0 * mdm_smooth / tr_smooth;
    } else {
        pdi[period] = 0.0;
        mdi[period] = 0.0;
    }

    for (size_t i = period + 1; i < adx_start; i++) {
        const KRecord& r = k[i];
        const KRecord& prev_r = k[i - 1];

        value_t h = r.highPrice;
        value_t l = r.lowPrice;
        value_t prev_c = prev_r.closePrice;

        value_t tr1 = h - l;
        value_t tr2 = std::abs(h - prev_c);
        value_t tr3 = std::abs(l - prev_c);
        value_t tr_i = std::max({tr1, tr2, tr3});

        value_t up_move = h - prev_r.highPrice;
        value_t down_move = prev_r.lowPrice - l;

        value_t pdm_i = (up_move > down_move && up_move > 0) ? up_move : 0.0;
        value_t mdm_i = (down_move > up_move && down_move > 0) ? down_move : 0.0;

        tr_smooth = tr_smooth * prev_factor + tr_i * smooth_factor;
        pdm_smooth = pdm_smooth * prev_factor + pdm_i * smooth_factor;
        mdm_smooth = mdm_smooth * prev_factor + mdm_i * smooth_factor;

        if (tr_smooth != 0.0) {
            pdi[i] = 100.0 * pdm_smooth / tr_smooth;
            mdi[i] = 100.0 * mdm_smooth / tr_smooth;
        } else {
            pdi[i] = 0.0;
            mdi[i] = 0.0;
        }
    }

    for (size_t i = adx_start; i < total; i++) {
        const KRecord& r = k[i];
        const KRecord& prev_r = k[i - 1];

        value_t h = r.highPrice;
        value_t l = r.lowPrice;
        value_t prev_c = prev_r.closePrice;

        value_t tr1 = h - l;
        value_t tr2 = std::abs(h - prev_c);
        value_t tr3 = std::abs(l - prev_c);
        value_t tr_i = std::max({tr1, tr2, tr3});

        value_t up_move = h - prev_r.highPrice;
        value_t down_move = prev_r.lowPrice - l;

        value_t pdm_i = (up_move > down_move && up_move > 0) ? up_move : 0.0;
        value_t mdm_i = (down_move > up_move && down_move > 0) ? down_move : 0.0;

        tr_smooth = tr_smooth * prev_factor + tr_i * smooth_factor;
        pdm_smooth = pdm_smooth * prev_factor + pdm_i * smooth_factor;
        mdm_smooth = mdm_smooth * prev_factor + mdm_i * smooth_factor;

        if (tr_smooth != 0.0) {
            pdi[i] = 100.0 * pdm_smooth / tr_smooth;
            mdi[i] = 100.0 * mdm_smooth / tr_smooth;
        } else {
            pdi[i] = 0.0;
            mdi[i] = 0.0;
        }
    }

    m_discard = adx_start;

    if (adx_start < total) {
        value_t dx_sum = 0.0;
        for (size_t i = period; i < adx_start; i++) {
            value_t sum = pdi[i] + mdi[i];
            dx_sum += (sum != 0.0) ? 100.0 * std::abs(pdi[i] - mdi[i]) / sum : 0.0;
        }
        adx[adx_start] = dx_sum / (n - 1);

        for (size_t i = adx_start + 1; i < total; i++) {
            value_t sum = pdi[i] + mdi[i];
            value_t dx = (sum != 0.0) ? 100.0 * std::abs(pdi[i] - mdi[i]) / sum : 0.0;
            adx[i] = adx[i - 1] * prev_factor + dx * smooth_factor;
        }
    }

    for (size_t i = 0; i < period; i++) {
        pdi[i] = Null<value_t>();
        mdi[i] = Null<value_t>();
    }
}

Indicator HKU_API ADX(int n) {
    auto p = make_shared<IAdx>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API ADX(const KData& kdata, int n) {
    auto p = make_shared<IAdx>();
    p->setParam<int>("n", n);
    p->setContext(kdata);
    return Indicator(p);
}

} /* namespace hku */