/*
 * test_PositionRecord.cpp
 *
 *  Created on: 2026-10-9
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/config.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/PositionRecord.h>

#if HKU_SUPPORT_SERIALIZATION
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <sstream>
#endif

using namespace hku;

/**
 * @defgroup test_PositionRecord test_PositionRecord
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par Test points: totalProfit is NaN for unclosed records, exact for closed ones */
TEST_CASE("test_PositionRecord_totalProfit") {
    PositionRecord p;
    p.buyMoney = 1000.0;
    p.sellMoney = 1200.0;
    p.totalCost = 20.0;

    /** @arg an unclosed record returns Null (NaN), not 0 */
    p.cleanDatetime = Null<Datetime>();
    CHECK_UNARY(std::isnan(p.totalProfit()));

    /** @arg a genuine zero profit is still distinguishable (0, not NaN) */
    p.cleanDatetime = Datetime(199911170000);
    p.sellMoney = p.buyMoney + p.totalCost;
    CHECK_UNARY(!std::isnan(p.totalProfit()));
    CHECK_EQ(p.totalProfit(), 0.0);

    /** @arg a closed record: sell - buy - cost */
    p.sellMoney = 1200.0;
    CHECK_EQ(p.totalProfit(), 180.0);
}

/** @par Test points: the settlement basis starts at the opening notional and travels with the
 * record */
TEST_CASE("test_PositionRecord_settleBasis") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg A default record starts with both bases at zero */
    PositionRecord empty;
    CHECK_EQ(empty.openBasis, 0.0);
    CHECK_EQ(empty.settleBasis, 0.0);

    /** @arg The constructor seeds the settlement basis with the opening notional */
    PositionRecord opened(stock, Datetime(200001040000), Null<Datetime>(), 100.0, 0.0, 0.0, 100.0,
                          1000.0, 0.0, 0.0, 0.0);
    CHECK_EQ(opened.openBasis, 1000.0);
    CHECK_EQ(opened.settleBasis, 1000.0);

    /** @arg A settlement rolls it away from the opening notional */
    opened.settleBasis = 900.0;

    /** @arg The copy carries it */
    PositionRecord copied(opened);
    CHECK_EQ(copied.openBasis, 1000.0);
    CHECK_EQ(copied.settleBasis, 900.0);

    /** @arg The move constructor carries it */
    PositionRecord moved(std::move(copied));
    CHECK_EQ(moved.settleBasis, 900.0);

    /** @arg The move assignment carries it */
    PositionRecord assigned;
    assigned = std::move(moved);
    CHECK_EQ(assigned.settleBasis, 900.0);

    /** @arg Two records differing only in the settlement basis are not equal */
    PositionRecord other(opened);
    other.settleBasis = 950.0;
    CHECK_UNARY(!(opened == other));
    other.settleBasis = opened.settleBasis;
    CHECK_UNARY(opened == other);

    /** @arg The string form reports it */
    CHECK_UNARY(opened.str().find("900.00") != std::string::npos);

#if HKU_SUPPORT_SERIALIZATION
    /** @arg The field survives an archive round trip */
    std::string archive;
    {
        std::ostringstream oss;
        {
            boost::archive::xml_oarchive oa(oss);
            oa << BOOST_SERIALIZATION_NVP(opened);
        }
        archive = oss.str();
    }

    PositionRecord restored;
    {
        std::istringstream iss(archive);
        boost::archive::xml_iarchive ia(iss);
        ia >> BOOST_SERIALIZATION_NVP(restored);
    }
    CHECK_UNARY(restored == opened);

    /** @arg An archive of the previous version loads and falls back to the opening notional */
    std::string old(archive);
    const std::string open_tag("<settleBasis>");
    const std::string close_tag("</settleBasis>");
    size_t begin = old.find(open_tag);
    REQUIRE_NE(begin, std::string::npos);
    size_t end = old.find(close_tag, begin);
    REQUIRE_NE(end, std::string::npos);

    // Downgrade by hand: drop the node and report the class version that did not carry it yet
    size_t ver_pos = old.rfind("version=\"3\"", begin);
    REQUIRE_NE(ver_pos, std::string::npos);
    old.replace(ver_pos, 11, "version=\"2\"");
    old.erase(begin, end + close_tag.size() - begin);
    REQUIRE_EQ(old.find("settleBasis"), std::string::npos);

    PositionRecord upgraded;
    {
        std::istringstream iss(old);
        boost::archive::xml_iarchive ia(iss);
        ia >> BOOST_SERIALIZATION_NVP(upgraded);
    }
    CHECK_EQ(upgraded.openBasis, 1000.0);
    CHECK_EQ(upgraded.settleBasis, upgraded.openBasis);
#endif
}

/** @} */
