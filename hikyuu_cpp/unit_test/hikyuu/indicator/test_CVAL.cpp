/*
 * test_CVAL.cpp
 *
 *  Created on: 2017-6-25
 *      Author: Administrator
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_CVAL test_indicator_CVAL
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_CVAL") {
    Indicator result;

    PriceList d;
    d.push_back(6063);
    d.push_back(6041);
    d.push_back(6065);
    d.push_back(6078);
    d.push_back(6114);
    d.push_back(6121);
    d.push_back(6106);
    d.push_back(6101);
    d.push_back(6166);
    d.push_back(6169);
    d.push_back(6195);
    d.push_back(6222);
    d.push_back(6186);
    d.push_back(6214);
    d.push_back(6185);
    d.push_back(6209);
    d.push_back(6221);
    d.push_back(6278);
    d.push_back(6326);
    d.push_back(6347);

    /** @arg operator(ind) */
    Indicator ind = PRICELIST(d);
    CHECK_EQ(ind.size(), 20);
    result = CVAL(ind, 100);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.getResultNumber(), 1);
    for (size_t i = 0; i < ind.size(); ++i) {
        CHECK_EQ(result[i], 100);
    }

    /** @arg operator() */
    result = CVAL(100);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result[0], 100);

    /** @arg Test the discard, the ind discard is not given (=2) */
    result = CVAL(100, 2);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.getParam<int>("discard"), 2);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_UNARY(std::isnan(result[0]));

    /** @arg Test the discard, the ind discard=2 */
    ind = PRICELIST(d);
    CHECK_EQ(ind.size(), 20);
    result = CVAL(ind, 100, 2);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.getParam<int>("discard"), 2);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 2);
    CHECK_EQ(result.getResultNumber(), 1);
    for (size_t i = result.discard(); i < ind.size(); ++i) {
        CHECK_EQ(result[i], 100);
    }
}

/** @par Test points */
TEST_CASE("test_CVAL_nested_size_propagation") {
    // Before the fix: one=CVAL(10) inside CVAL(one, 10) -> goes through Indicator::operator()
    //   alike (a new empty shell ICval and the calculated CVAL10) is true -> short-circuits and
    //   returns a clone of the empty shell m_imp (m_imp was not calculated, size==0) -> violating
    //   the CVAL.h contract "its length equals that of the input ind"
    // After the fix: the calculated ind is reused and the size is propagated correctly

    Indicator one = CVAL(10);
    CHECK_EQ(one.size(), 1);
    CHECK_EQ(one[0], 10);
    CHECK_EQ(one.discard(), 0);

    // A nested call with alike==true (the same value) -> triggers the fixed return ind
    Indicator two = CVAL(one, 10);
    CHECK_EQ(two.size(), one.size());  // 0 before the fix and 1 after
    CHECK_EQ(two.discard(), one.discard());
    CHECK_EQ(two[0], 10);
    // A white box assertion: verify that the return ind path (a pointer reuse) is taken instead of
    // a cloned empty shell
    CHECK_EQ(two.getImp().get(), one.getImp().get());

    // A nested call with alike==false (different values 5!=10) -> goes through the normal branch B
    // (clone + calculate)
    Indicator three = CVAL(two, 5);
    CHECK_EQ(three.size(), 1);
    CHECK_EQ(three.discard(), 0);
    CHECK_EQ(three[0], 5);
    // A white box assertion: verify that the short-circuit branch (an independent clone) is not
    // taken
    CHECK_NE(three.getImp().get(), two.getImp().get());
}

/** @par Test points */
TEST_CASE("test_CVAL_nested_state_sharing") {
    // The fix returns ind (sharing the underlying node) when alike==true instead of an independent
    // clone of m_imp. This is a semantic change introduced by the fix: the returned value shares
    // the same IndicatorImp with ind. Under the immutable parameter semantics of hikyuu, alike has
    // verified that m_params are equal, and setParam triggers an in-place recalculation (without
    // changing the buffer immediately), so the sharing is safe. This case fixes that sharing
    // behavior as an expected invariant.

    Indicator base = CVAL(100);
    CHECK_EQ(base.size(), 1);
    CHECK_EQ(base[0], 100);

    // A self nesting: base.operator()(base) -> alike (this==other) true -> return ind (i.e. base)
    Indicator result = base(base);
    CHECK_EQ(result.size(), base.size());
    // The pointers are exactly the same: the AST trimming reuses the argument directly
    CHECK_EQ(result.getImp().get(), base.getImp().get());

    // The state sharing case: modifying the parameters of result also affects base (the same imp)
    result.setParam<double>("value", 999.0);
    CHECK_EQ(base.getParam<double>("value"), 999.0);
    CHECK_EQ(result.getParam<double>("value"), 999.0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_CVAL_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/CVAL.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = CVAL(CLOSE(kdata), 100.0);
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
