/*
 * test_PRICELIST.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_PRICELIST test_indicator_PRICELIST
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_PRICELIST") {
    PriceList tmp_list;
    Indicator result;

    /** @arg The PriceList is empty */
    result = PRICELIST(tmp_list);
    CHECK_EQ(result.size(), tmp_list.size());
    CHECK_EQ(result.empty(), true);
    /** @arg The PriceList is not empty */
    for (size_t i = 0; i < 10; ++i) {
        tmp_list.push_back(i);
    }
    result = PRICELIST(tmp_list);
    CHECK_EQ(result.size(), tmp_list.size());
    CHECK_EQ(result.empty(), false);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], tmp_list[i]);
    }

    /** @arg The array pointer is null */
    price_t* p_tmp = NULL;
    result = PRICELIST(p_tmp, 10);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.empty(), true);

    /** @arg The array pointer is not null */
    price_t tmp[10];
    for (size_t i = 0; i < 10; ++i) {
        tmp[i] = i;
    }
    result = PRICELIST(tmp, 10);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], tmp[i]);
    }

    /** @arg Constructed from a PriceList */
    result = PRICELIST(tmp_list);
    CHECK_EQ(result.size(), 10);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], tmp_list[i]);
    }

    /** @arg From a PriceList with discard=1 */
    result = PRICELIST(tmp_list, 1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    for (size_t i = 1; i < 10; ++i) {
        CHECK_EQ(result[i], tmp_list[i]);
    }
}

/**
 * @par Test points
 * A PRICELIST carrying its own date list must be laid out by the dates of the context, not by the
 * position in the given list.
 *
 * Background: the align branch copied from the raw data instead of the aligned result, so the
 * values stayed at their original positions and the copy read past the end of the list whenever the
 * context was longer than it.
 */
TEST_CASE("test_PRICELIST_align_by_date") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k = stock.getKData(KQuery(0, 20));
    CHECK_EQ(k.size(), 20);

    // The values belong to every second bar of the context
    PriceList values;
    DatetimeList dates;
    for (size_t i = 1; i < 20; i += 2) {
        values.push_back(double(i));
        dates.push_back(k[i].datetime);
    }
    CHECK_EQ(values.size(), 10);

    Indicator r = PRICELIST(values, dates);
    r.setContext(k);

    /** @arg the result keeps the context length */
    CHECK_EQ(r.size(), 20);
    /** @arg the first context bar carries no value of its own */
    CHECK_UNARY(std::isnan(r[0]));
    /** @arg every value lands on the bar of its own date, the gaps stay null */
    for (size_t i = 1; i < 20; ++i) {
        if (i % 2 == 1) {
            CHECK_EQ(r[i], doctest::Approx(values[i / 2]));
        } else {
            CHECK_UNARY(std::isnan(r[i]));
        }
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_PRICELIST_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/PRICELIST.xml";

    PriceList d;
    for (size_t i = 0; i < 20; ++i) {
        d.push_back(i);
    }

    Indicator ma1 = PRICELIST(d);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ma1);
    }

    Indicator ma2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ma2);
    }

    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = 0; i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
