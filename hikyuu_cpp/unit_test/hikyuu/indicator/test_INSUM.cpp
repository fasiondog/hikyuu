/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2026-10-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <cmath>
#include <fstream>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/Block.h>
#include <hikyuu/indicator/crt/INSUM.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_INSUM test_indicator_INSUM
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_INSUM_inf_input") {
    StockManager& sm = StockManager::instance();
    Block blk;
    blk.add(sm["sh600004"]);
    blk.add(sm["sh600005"]);
    KData ref_k = sm["sh000001"].getKData(KQuery(-10));
    double inf = std::numeric_limits<double>::infinity();

    /** @arg All +-Inf input is excluded, so the sum yields no valid slot (never +-Inf) */
    Indicator ret = INSUM(blk, CVAL(inf), 0)(ref_k);
    CHECK_EQ(ret.name(), "INSUM");
    CHECK_EQ(ret.size(), ref_k.size());
    CHECK_EQ(ret.discard(), ret.size());
    for (size_t i = 0; i < ret.size(); ++i) {
        CHECK_UNARY(std::isnan(ret[i]));
    }

    /** @arg Normal aggregate: no output slot is ever +-Inf */
    ret = INSUM(blk, CLOSE(), 0)(ref_k);
    CHECK_EQ(ret.name(), "INSUM");
    CHECK_EQ(ret.size(), ref_k.size());
    for (size_t i = 0; i < ret.size(); ++i) {
        CHECK_UNARY(std::isnan(ret[i]) || std::isfinite(ret[i]));
    }

    /** @arg Rank mode with +-Inf input: no output slot is +-Inf */
    ret = INSUM(blk, CVAL(inf), 4)(ref_k);
    CHECK_EQ(ret.name(), "INSUM");
    CHECK_EQ(ret.size(), ref_k.size());
    for (size_t i = 0; i < ret.size(); ++i) {
        CHECK_UNARY(std::isnan(ret[i]) || std::isfinite(ret[i]));
    }
}

/** @par Test points */
TEST_CASE("test_INSUM_no_input") {
    StockManager& sm = StockManager::instance();
    Block blk;
    blk.add(sm["sh600004"]);
    blk.add(sm["sh600005"]);
    KData ref_k = sm["sh000001"].getKData(KQuery(-10));
    if (ref_k.empty()) {
        return;
    }

    /** @arg no input indicator (leaf mode): result fully discarded */
    Indicator ret = INSUM(blk, 0, true)(ref_k);
    CHECK_EQ(ret.name(), "INSUM");
    CHECK_EQ(ret.size(), ref_k.size());
    CHECK_EQ(ret.discard(), ret.size());
}

/** @par Test points
 * Regression: an empty input indicator must only discard the output for the rank modes
 * (4/5), because those modes use ind's own data as the current-stock baseline. The
 * aggregate modes (0/1/2/3) read only the per-stock results produced internally by
 * getAllIndicators, so they still yield valid values even when the execution context
 * kline is empty (i.e. ind.size() == 0). Extending the guard back to all modes would
 * turn INSUM(block, query, ISLIMITUP(), 0) into an all-NaN series.
 */
TEST_CASE("test_INSUM_empty_input_context") {
    StockManager& sm = StockManager::instance();
    Block blk;
    blk.add(sm["sh600004"]);
    blk.add(sm["sh600005"]);
    KQuery query(-10);
    KData empty_k;  // No context kline -> the input ind is executed against an empty KData

    /** @arg mode 0 (cum): block aggregation is still valid even though ind.size() == 0 */
    Indicator ret = INSUM(blk, query, CLOSE(), 0)(empty_k);
    CHECK_EQ(ret.name(), "INSUM");
    CHECK_UNARY(ret.size() > 0);
    CHECK_UNARY(ret.discard() < ret.size());
    bool has_valid = false;
    for (size_t i = 0; i < ret.size(); ++i) {
        if (std::isfinite(ret[i])) {
            has_valid = true;
            break;
        }
    }
    CHECK_UNARY(has_valid);

    /** @arg mode 1 (mean): same as mode 0 */
    ret = INSUM(blk, query, CLOSE(), 1)(empty_k);
    CHECK_UNARY(ret.discard() < ret.size());

    /** @arg mode 2 (max): same as mode 0 */
    ret = INSUM(blk, query, CLOSE(), 2)(empty_k);
    CHECK_UNARY(ret.discard() < ret.size());

    /** @arg mode 3 (min): same as mode 0 */
    ret = INSUM(blk, query, CLOSE(), 3)(empty_k);
    CHECK_UNARY(ret.discard() < ret.size());

    /** @arg mode 4 (rank desc): the current-stock baseline is missing -> fully discarded */
    ret = INSUM(blk, query, CLOSE(), 4)(empty_k);
    CHECK_EQ(ret.size(), ret.discard());

    /** @arg mode 5 (rank asc): the current-stock baseline is missing -> fully discarded */
    ret = INSUM(blk, query, CLOSE(), 5)(empty_k);
    CHECK_EQ(ret.size(), ret.discard());
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_INSUM_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/INSUM.xml";

    Block blk;
    blk.add(sm["sh600004"]);
    blk.add(sm["sh600005"]);
    KData ref_k = sm["sh000001"].getKData(KQuery(-10));
    Indicator x1 = INSUM(blk, CLOSE(), 0)(ref_k);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    Indicator x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
