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
