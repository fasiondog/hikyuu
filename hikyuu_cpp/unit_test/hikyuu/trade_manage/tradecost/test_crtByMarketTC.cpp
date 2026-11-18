/*
 * test_crtByMarketTC.cpp
 *
 *  Created on: 2026-11-12
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/config.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_ByMarket.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_HK.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_MarginInterest.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_Zero.h>
#include <hikyuu/trade_manage/tradecost/imp/ByMarketTradeCost.h>

#if HKU_SUPPORT_SERIALIZATION
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_crtByMarketTC test_crtByMarketTC
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** A model that only bills the Hong Kong stamp duty: 10,000 of turnover costs 10 */
static TradeCostPtr makeStampCost() {
    return TC_HK(0.0, 0.0, 0.001, 0.0, 0.0);
}

/** A model that bills a flat fee per trade: the record tells the models apart whatever the
 * contract multiplier of the security */
static TradeCostPtr makeFlatCost(price_t fee) {
    return TC_HK(0.0, 0.0, 0.0, 0.0, fee);
}

namespace {

/** A model answering a distinguishing record, optionally flagged as a Python defined one */
class StubCost final : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    explicit StubCost(bool python = false) : TradeCostBase("StubCost") {
        m_is_python_object = python;
    }

    CostRecord getBuyCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord(1.0, 0.0, 0.0, 0.0, 1.0);
    }

    CostRecord getSellCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord(1.0, 0.0, 0.0, 0.0, 1.0);
    }

    TradeCostPtr _clone() override {
        return make_shared<StubCost>(m_is_python_object);
    }
};

}  // namespace

/** @par Test points: a query is answered by the model of the market, then of the security type,
 * then the default model, and the market wins when both are registered */
TEST_CASE("test_TC_ByMarket_route") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    Stock sz = sm.getStock("sz000001");
    Stock rb = sm.getStock("SHFRB0000");
    REQUIRE_UNARY(!sh.isNull());
    REQUIRE_UNARY(!sz.isNull());
    REQUIRE_UNARY(!rb.isNull());
    const Datetime dt(201001010000);

    auto costs = std::static_pointer_cast<ByMarketTradeCost>(TC_ByMarket(TC_Zero()));
    REQUIRE_UNARY(costs);

    /** @arg An empty table bills nothing through the default model */
    CHECK_EQ(costs->name(), "TC_ByMarket");
    CHECK_EQ(costs->keys().size(), 0);
    CHECK_EQ(costs->getBuyCost(dt, sh, 10.0, 1000).total, 0.0);
    CHECK_EQ(costs->getBuyCost(dt, rb, 4000.0, 10).total, 0.0);

    /** @arg A registered market is answered by its own model, on both sides */
    costs->add("SH", makeStampCost());
    CHECK_EQ(costs->getBuyCost(dt, sh, 10.0, 1000).stamptax, 10.0);
    CHECK_EQ(costs->getSellCost(dt, sh, 10.0, 1000).total, 10.0);

    /** @arg An unregistered market falls back to the default model */
    CHECK_EQ(costs->getBuyCost(dt, sz, 10.0, 1000).total, 0.0);

    /** @arg A type key answers every market of that type (type 12: futures) */
    costs->add("type:12", makeFlatCost(13.0));
    CHECK_EQ(costs->getBuyCost(dt, rb, 4000.0, 10).total, 13.0);

    /** @arg The market match wins over the type match */
    costs->add("SHF", makeFlatCost(17.0));
    CHECK_EQ(costs->getBuyCost(dt, rb, 4000.0, 10).total, 17.0);

    /** @arg The keys are normalized to upper case and sorted */
    auto keys = costs->keys();
    REQUIRE_EQ(keys.size(), 3);
    CHECK_EQ(keys[0], "SH");
    CHECK_EQ(keys[1], "SHF");
    CHECK_EQ(keys[2], "TYPE:12");

    /** @arg get() is case insensitive and answers only a registered key */
    CHECK_UNARY(!costs->get("hk"));
    CHECK_EQ(costs->get("sh")->name(), "TC_HK");
    CHECK_EQ(costs->getDefault()->name(), "TC_Zero");

    /** @arg Re-registering a key replaces its model */
    costs->add("SH", TC_Zero());
    CHECK_EQ(costs->getBuyCost(dt, sh, 10.0, 1000).total, 0.0);

    /** @arg setDefault replaces the fallback, a null pointer restores the zero-cost model */
    costs->setDefault(makeFlatCost(5.0));
    CHECK_EQ(costs->getBuyCost(dt, sz, 10.0, 1000).total, 5.0);
    costs->setDefault(TradeCostPtr());
    CHECK_EQ(costs->getBuyCost(dt, sz, 10.0, 1000).total, 0.0);

    /** @arg An empty key or a type key without the type number is rejected, a null model is
     * ignored */
    auto count = costs->keys().size();
    CHECK_THROWS_AS(costs->add("", makeFlatCost(1.0)), hku::exception);
    CHECK_THROWS_AS(costs->add("type:", makeFlatCost(1.0)), hku::exception);
    CHECK_THROWS_AS(costs->add("type:x", makeFlatCost(1.0)), hku::exception);
    CHECK_EQ(costs->keys().size(), count);
    costs->add("HK", TradeCostPtr());
    CHECK_EQ(costs->keys().size(), count);
}

/** @par Test points: the securities lending hooks follow the same routing, while the hooks that
 * carry no security are answered by the default model */
TEST_CASE("test_TC_ByMarket_borrow") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    Stock rb = sm.getStock("SHFRB0000");
    REQUIRE_UNARY(!sh.isNull());
    REQUIRE_UNARY(!rb.isNull());
    const Datetime dt(201001010000);

    TradeCostPtr default_cost = TC_MarginInterest(TC_Zero(), 0.05, "", 0.10);
    auto costs = std::static_pointer_cast<ByMarketTradeCost>(TC_ByMarket(default_cost));
    REQUIRE_UNARY(costs);

    /** @arg The borrowed cash is not attributed to a market: the default model answers it */
    CHECK_EQ(costs->getBorrowCashRate(dt), 0.05);
    CHECK_EQ(costs->getBorrowCashCost(dt, 10000.0), CostRecord());
    CHECK_EQ(costs->getReturnCashCost(dt, dt, 10000.0), CostRecord());

    /** @arg The securities lending rate follows the routing of the security */
    CHECK_EQ(costs->getBorrowStockRate(dt, sh), 0.10);
    costs->add("type:12", TC_MarginInterest(TC_Zero(), 0.05, "", 0.20));
    CHECK_EQ(costs->getBorrowStockRate(dt, rb), 0.20);
    CHECK_EQ(costs->getBorrowStockRate(dt, sh), 0.10);

    /** @arg The lending and returning costs are forwarded to the routed model as well */
    CHECK_EQ(costs->getBorrowStockCost(dt, rb, 4000.0, 10), CostRecord());
    CHECK_EQ(costs->getReturnStockCost(dt, dt, rb, 4000.0, 10), CostRecord());
}

/** @par Test points: the clone copies the routing table and the models, and the copy can be
 * reconfigured without touching the original */
TEST_CASE("test_TC_ByMarket_clone") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    const Datetime dt(201001010000);

    auto costs = std::static_pointer_cast<ByMarketTradeCost>(TC_ByMarket(makeFlatCost(3.0)));
    costs->add("SH", makeStampCost());

    auto cloned = std::static_pointer_cast<ByMarketTradeCost>(costs->clone());
    REQUIRE_UNARY(cloned);

    /** @arg The clone keeps the name, the keys, the fees and the default model */
    CHECK_EQ(cloned->name(), "TC_ByMarket");
    REQUIRE_EQ(cloned->keys().size(), costs->keys().size());
    CHECK_EQ(cloned->keys()[0], costs->keys()[0]);
    CHECK_EQ(cloned->getBuyCost(dt, sh, 10.0, 1000).total, 10.0);

    /** @arg The models are cloned, not shared */
    CHECK_UNARY(cloned->get("SH").get() != costs->get("SH").get());
    CHECK_UNARY(cloned->getDefault().get() != costs->getDefault().get());

    /** @arg Reconfiguring the clone leaves the original untouched */
    cloned->add("SH", TC_Zero());
    CHECK_EQ(cloned->getBuyCost(dt, sh, 10.0, 1000).total, 0.0);
    CHECK_EQ(costs->getBuyCost(dt, sh, 10.0, 1000).total, 10.0);
}

/** @par Test points: the Python flag of the routed and of the default model is reported upward, so
 * a run never releases the GIL while a Python cost model is still being called */
TEST_CASE("test_TC_ByMarket_python_part") {
    /** @arg Built-in models on both sides keep the account pickable for a parallel run */
    auto costs = std::static_pointer_cast<ByMarketTradeCost>(TC_ByMarket(TC_Zero()));
    costs->add("SH", TC_Zero());
    CHECK_UNARY(!costs->hasPythonPart());

    /** @arg A Python defined routed model is reported upward, and the clone carries the flag */
    costs = std::static_pointer_cast<ByMarketTradeCost>(TC_ByMarket(TC_Zero()));
    costs->add("SH", make_shared<StubCost>(true));
    CHECK_UNARY(costs->hasPythonPart());
    CHECK_UNARY(costs->clone()->hasPythonPart());

    /** @arg A Python defined default model is reported as well */
    CHECK_UNARY(TC_ByMarket(make_shared<StubCost>(true))->hasPythonPart());

    /** @arg The flag belongs to the model, not to the routing: a plain stub stays off */
    costs = std::static_pointer_cast<ByMarketTradeCost>(TC_ByMarket(make_shared<StubCost>()));
    costs->add("SH", make_shared<StubCost>());
    CHECK_UNARY(!costs->hasPythonPart());
}

#if HKU_SUPPORT_SERIALIZATION
/** @par Test points: the serialization round trip keeps the routing table and the default model */
TEST_CASE("test_ByMarketTradeCost_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TC_ByMarket.xml";

    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    const Datetime dt(201001010000);

    auto costs = std::static_pointer_cast<ByMarketTradeCost>(
      TC_ByMarket(TC_MarginInterest(TC_Zero(), 0.05, "", 0.10)));
    costs->add("SH", makeStampCost());
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        TradeCostPtr base = costs;
        oa << BOOST_SERIALIZATION_NVP(base);
    }

    TradeCostPtr loaded;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(loaded);
    }

    auto costs2 = std::static_pointer_cast<ByMarketTradeCost>(loaded);
    REQUIRE_UNARY(costs2);

    /** @arg The name and the routing table survive the round trip */
    CHECK_EQ(costs2->name(), "TC_ByMarket");
    REQUIRE_EQ(costs2->keys().size(), 1);
    CHECK_EQ(costs2->keys()[0], "SH");

    /** @arg The routed fee and the default model survive the round trip */
    CHECK_EQ(costs2->getBuyCost(dt, sh, 10.0, 1000).total, 10.0);
    CHECK_EQ(costs2->getBorrowCashRate(dt), 0.05);
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
