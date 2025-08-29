/*
 * test_PRICELIST.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SLICE.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_SLICE test_indicator_SLICE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SLICE") {
    PriceList tmp_list;
    Indicator result;

    /** @arg PriceList 为空 */
    result = SLICE(tmp_list, 0, tmp_list.size());
    CHECK_EQ(result.size(), tmp_list.size());
    CHECK_EQ(result.empty(), true);

    /** @arg PriceList 非空, 获取全部数据 */
    for (size_t i = 0; i < 10; ++i) {
        tmp_list.push_back(i);
    }
    result = SLICE(tmp_list, 0, tmp_list.size());
    CHECK_EQ(result.size(), tmp_list.size());
    CHECK_EQ(result.empty(), false);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], tmp_list[i]);
    }

    /** @arg PriceList 非空, 正索引获取部分数据 */
    result = SLICE(tmp_list, 2, tmp_list.size());
    CHECK_EQ(result.size(), tmp_list.size() - 2);
    CHECK_EQ(result.empty(), false);
    for (size_t i = 0; i < 8; ++i) {
        CHECK_EQ(result[i], tmp_list[i + 2]);
    }

    result = SLICE(tmp_list, 3, 4);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result[0], tmp_list[3]);

    /** @arg PriceList 非空, 负索引获取部分数据 */
    result = SLICE(tmp_list, -2, -1);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result[0], tmp_list[8]);

    /** @arg PriceList 非空, 正负索引获取部分数据 */
    result = SLICE(tmp_list, 1, -7);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result[0], tmp_list[1]);
    CHECK_EQ(result[1], tmp_list[2]);

    /** @arg PriceList 非空, 索引不在范围内 */
    result = SLICE(tmp_list, 10, 11);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.empty(), true);

    /** @arg 待转化数据为Indicator，Indicator为空 */
    result = SLICE(Indicator(), 0, 1);
    CHECK_EQ(result.size(), 0);

    /** @arg 待转化数据为Indicator, result_num=0 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KQuery query(0, 30);
    KData kdata = stock.getKData(query);
    Indicator ikdata = KDATA(kdata);
    CHECK_EQ(ikdata.size(), 30);
    result = SLICE(ikdata, 0, 30, 0);
    CHECK_EQ(result.size(), ikdata.size());
    CHECK_EQ(result.discard(), ikdata.discard());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], ikdata[i]);
    }

    result = SLICE(ikdata, 2, 5, 0);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], ikdata[2]);
    CHECK_EQ(result[1], ikdata[3]);
    CHECK_EQ(result[2], ikdata[4]);

    /** @arg 待转化数据为Indicator, result_num=1 */
    result = SLICE(ikdata, 0, 30, 1);
    CHECK_EQ(result.size(), ikdata.size());
    CHECK_EQ(result.discard(), ikdata.discard());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], ikdata.get(i, 1));
    }

    /** @arg 待转化数据为Indicator, result_num=-1 */
    result = SLICE(ikdata, 0, 30, -1);
    CHECK_EQ(result.getResultNumber(), ikdata.getResultNumber());
    CHECK_EQ(result.size(), ikdata.size());
    CHECK_EQ(result.discard(), ikdata.discard());
    for (size_t r = 0; r < result.getResultNumber(); ++r) {
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result.get(i, r), ikdata.get(i, r));
        }
    }

    result = SLICE(ikdata, 2, 5, -1);
    CHECK_EQ(result.getResultNumber(), ikdata.getResultNumber());
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], ikdata[2]);
    CHECK_EQ(result[1], ikdata[3]);
    CHECK_EQ(result[2], ikdata[4]);
    CHECK_EQ(result.get(0, 1), ikdata.get(2, 1));
    CHECK_EQ(result.get(1, 1), ikdata.get(3, 1));
    CHECK_EQ(result.get(2, 1), ikdata.get(4, 1));
    CHECK_EQ(result.get(0, 2), ikdata.get(2, 2));
    CHECK_EQ(result.get(1, 2), ikdata.get(3, 2));
    CHECK_EQ(result.get(2, 2), ikdata.get(4, 2));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
// TEST_CASE("test_SLICE_export") {
//     StockManager& sm = StockManager::instance();
//     string filename(sm.tmpdir());
//     filename += "/SLICE.xml";

//     PriceList d;
//     for (size_t i = 0; i < 20; ++i) {
//         d.push_back(i);
//     }

//     Indicator ma1 = PRICELIST(d);
//     {
//         std::ofstream ofs(filename);
//         boost::archive::xml_oarchive oa(ofs);
//         oa << BOOST_SERIALIZATION_NVP(ma1);
//     }

//     Indicator ma2;
//     {
//         std::ifstream ifs(filename);
//         boost::archive::xml_iarchive ia(ifs);
//         ia >> BOOST_SERIALIZATION_NVP(ma2);
//     }

//     CHECK_EQ(ma1.size(), ma2.size());
//     CHECK_EQ(ma1.discard(), ma2.discard());
//     CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
//     for (size_t i = 0; i < ma1.size(); ++i) {
//         CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
//     }
// }
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
