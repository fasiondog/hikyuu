/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-21
 *      Author: fasiondog
 *
 *  Output validation of MultiFactorBase: _calculate()/normalize() return values are read by the
 *  subscript downstream, so a size mismatch must raise a catchable exception, not crash.
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/trade_sys/multifactor/MultiFactorBase.h>
#include <hikyuu/trade_sys/multifactor/NormalizeBase.h>
#include <hikyuu/trade_sys/multifactor/buildin_norm.h>

using namespace hku;

/**
 * @defgroup test_MF_output_validation test_MF_output_validation
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

namespace {

// Output mode of the test MF; simulates a derived class returning a malformed result
enum OutMode {
    OUT_OK = 0,      // The normal output: stk_count indicators, each of days_total in size
    OUT_FEW_FACTOR,  // One factor is missing
    OUT_SHORT_DAYS,  // Every factor is one day short
    OUT_EMPTY_IND,   // An empty (default constructed) indicator
};

class TestOutMF : public MultiFactorBase {
    MULTIFACTOR_IMP(TestOutMF)

public:
    TestOutMF() : MultiFactorBase("TestOutMF") {}
    explicit TestOutMF(int mode) : MultiFactorBase("TestOutMF"), m_mode(mode) {}

    int mode() const {
        return m_mode;
    }
    void mode(int m) {
        m_mode = m;
    }

private:
    int m_mode{OUT_OK};
};

IndicatorList TestOutMF::_calculate(const vector<IndicatorList>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    IndicatorList ret;
    switch (m_mode) {
        case OUT_FEW_FACTOR:
            ret.resize(stk_count - 1);
            for (auto& ind : ret) {
                ind = PRICELIST(PriceList(days_total, 1.0));
            }
            break;
        case OUT_SHORT_DAYS:
            ret.resize(stk_count);
            for (auto& ind : ret) {
                ind = PRICELIST(PriceList(days_total - 1, 1.0));
            }
            break;
        case OUT_EMPTY_IND:
            ret.resize(stk_count);
            break;
        default:
            ret.resize(stk_count);
            for (auto& ind : ret) {
                ind = PRICELIST(PriceList(days_total, 1.0));
            }
            break;
    }
    return ret;
}

/** A test standardizer returning a list one element short */
class ShortListNorm : public NormalizeBase {
    NORMALIZE_IMP(ShortListNorm)

public:
    ShortListNorm() : NormalizeBase("ShortListNorm") {}
};

PriceList ShortListNorm::normalize(const PriceList& data) {
    return PriceList(data.begin(), data.end() - 1);
}

/** Two factors so the _calculate branch is taken, not the single-factor passthrough */
std::shared_ptr<TestOutMF> makeTestMF(int mode) {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    KQuery query = KQuery(-30);

    auto mf = std::make_shared<TestOutMF>(mode);
    mf->setStockList(stks);
    mf->setQuery(query);
    mf->setRefStock(sm["sh000001"]);
    IndicatorList inds{MA(CLOSE(), 5), EMA(CLOSE(), 10)};
    mf->setRefFactorSet(FactorSet(inds, KQuery::DAY));
    return mf;
}

}  // namespace

/** @par Test points */
TEST_CASE("test_MF_output_validation") {
    /** @arg The normal output passes the verification and the calculation succeeds */
    auto mf = makeTestMF(OUT_OK);
    CHECK_NOTHROW(mf->calculate());

    /** @arg One factor is missing: an exception is thrown instead of an out of bounds read */
    mf = makeTestMF(OUT_FEW_FACTOR);
    CHECK_THROWS_AS(mf->calculate(), std::exception);

    /** @arg Every factor is one day short: an exception is thrown instead of an out of bounds read
     */
    mf = makeTestMF(OUT_SHORT_DAYS);
    CHECK_THROWS_AS(mf->calculate(), std::exception);

    /** @arg An empty indicator is returned: an exception is thrown instead of a null pointer
     * dereference */
    mf = makeTestMF(OUT_EMPTY_IND);
    CHECK_THROWS_AS(mf->calculate(), std::exception);

    /** @arg The instance stays uncalculated after a failed verification, and the calculation
     * succeeds after the output is corrected */
    mf->mode(OUT_OK);
    CHECK_NOTHROW(mf->calculate());
    CHECK_EQ(mf->getDatetimeList().size(), mf->getAllScores().size());
}

/** @par Test points */
TEST_CASE("test_MF_normalize_output_validation") {
    /** @arg The standardizer returns a list one element short: an exception is thrown before the
     * writeback instead of an out of bounds write */
    auto mf = makeTestMF(OUT_OK);
    mf->setNormalize(std::make_shared<ShortListNorm>());
    CHECK_THROWS_AS(mf->calculate(), std::exception);

    /** @arg The built-in standardizer output has the normal size: no false failure */
    mf = makeTestMF(OUT_OK);
    mf->setNormalize(NORM_Zscore());
    CHECK_NOTHROW(mf->calculate());

    /** @arg No standardizer is configured: no false failure */
    mf = makeTestMF(OUT_OK);
    CHECK_NOTHROW(mf->calculate());
}

/** @} */
