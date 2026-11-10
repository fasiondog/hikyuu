/*
 * test_TradeRecord.cpp
 *
 *  Created on: 2026-11-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <string>
#include <hikyuu/config.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/TradeRecord.h>

#if HKU_SUPPORT_SERIALIZATION
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <sstream>
#endif

using namespace hku;

/**
 * @defgroup test_TradeRecord test_TradeRecord
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par Test points: the locked margin travels with the record, off the books it stays zero */
TEST_CASE("test_TradeRecord_margin") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg A default record locks nothing */
    TradeRecord empty;
    CHECK_EQ(empty.margin, 0.0);

    /** @arg The constructor leaves the margin to the trade path */
    TradeRecord opened(stock, Datetime(200001040000), BUSINESS_BUY, 10.0, 10.0, 0.0, 100.0,
                       CostRecord(), 0.0, 9000.0, PART_INVALID);
    CHECK_EQ(opened.margin, 0.0);

    /** @arg The copy, the move and the assignment carry it */
    opened.margin = 1000.0;
    TradeRecord copied(opened);
    CHECK_EQ(copied.margin, 1000.0);
    TradeRecord moved(std::move(copied));
    CHECK_EQ(moved.margin, 1000.0);
    TradeRecord assigned;
    assigned = std::move(moved);
    CHECK_EQ(assigned.margin, 1000.0);

    /** @arg Two records differing only in the margin are not equal */
    TradeRecord other(opened);
    other.margin = 900.0;
    CHECK_UNARY(!(opened == other));
    other.margin = opened.margin;
    CHECK_UNARY(opened == other);

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

    TradeRecord restored;
    {
        std::istringstream iss(archive);
        boost::archive::xml_iarchive ia(iss);
        ia >> BOOST_SERIALIZATION_NVP(restored);
    }
    CHECK_EQ(restored.margin, 1000.0);
    CHECK_UNARY(restored == opened);

    /** @arg An archive of the previous version loads and falls back to the cash books */
    std::string old(archive);
    const std::string tag("<opened ");
    size_t tag_pos = old.find(tag);
    REQUIRE_NE(tag_pos, std::string::npos);
    size_t ver_pos = old.find("version=\"1\"", tag_pos);
    REQUIRE_NE(ver_pos, std::string::npos);

    const std::string open_tag("<margin>");
    const std::string close_tag("</margin>");
    size_t begin = old.find(open_tag);
    REQUIRE_NE(begin, std::string::npos);
    size_t end = old.find(close_tag, begin);
    REQUIRE_NE(end, std::string::npos);
    old.erase(begin, end + close_tag.size() - begin);

    // Downgrade by hand: report the class version that did not carry the field yet
    old.replace(ver_pos, 11, "version=\"0\"");
    REQUIRE_EQ(old.find("margin"), std::string::npos);

    TradeRecord upgraded;
    {
        std::istringstream iss(old);
        boost::archive::xml_iarchive ia(iss);
        ia >> BOOST_SERIALIZATION_NVP(upgraded);
    }
    CHECK_EQ(upgraded.margin, 0.0);
    CHECK_EQ(upgraded.cash, 9000.0);
    CHECK_EQ(upgraded.number, 100.0);
#endif
}

/** @par Test points: the settlement and the interest businesses name and map back */
TEST_CASE("test_getBusinessName") {
    /** @arg Each new business maps onto its own name and back */
    CHECK_EQ(getBusinessName(BUSINESS_SETTLEMENT), "SETTLEMENT");
    CHECK_EQ(getBusinessEnum("SETTLEMENT"), BUSINESS_SETTLEMENT);
    CHECK_EQ(getBusinessName(BUSINESS_INTEREST), "INTEREST");
    CHECK_EQ(getBusinessEnum("INTEREST"), BUSINESS_INTEREST);

    /** @arg The name lookup ignores the case like the other businesses */
    CHECK_EQ(getBusinessEnum("settlement"), BUSINESS_SETTLEMENT);

    /** @arg The invalid type keeps its own name and stays the target of an unknown one */
    CHECK_EQ(getBusinessName(BUSINESS_INVALID), "UNKNOWN");
    CHECK_EQ(getBusinessEnum("NO_SUCH_BUSINESS"), BUSINESS_INVALID);
}

/** @} */