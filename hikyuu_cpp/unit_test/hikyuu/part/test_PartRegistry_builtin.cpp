/*
 * test_PartRegistry_builtin.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <atomic>
#include <thread>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/part/PartRegistration.h>
#include <hikyuu/part/PartRegistry.h>

using namespace hku;

/**
 * @defgroup test_part_PartRegistry_builtin test_part_PartRegistry_builtin
 * @ingroup test_hikyuu_part_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_registerIndicatorParts") {
    PartRegistry::release();
    registerIndicatorParts();
    auto& reg = PartRegistry::instance();

    /** @arg all the built-in indicators are registered */
    CHECK_EQ(reg.getNameList(PartType::IND).size(), 138);

    /** @arg the meta is introspected from the prototype */
    const PartMeta& meta = reg.getMeta("core.ind.ma");
    CHECK_EQ(meta.name, "MA");

    /** @arg the hand-written C++ name of a part with the explicit meta is kept verbatim */
    CHECK_EQ(reg.getMeta("core.ind.avedev").name, "AVEDEV");
    CHECK_EQ(reg.getMeta("core.ind.icir").name, "ICIR");

    /** @arg the lookup is case insensitive */
    CHECK_EQ(reg.getMeta("core.ind.MA").full_name, "core.ind.ma");
    CHECK_UNARY(reg.has("CORE.IND.MA"));
    CHECK_UNARY(!reg.getMeta("Core.Ind.Ma").full_name.empty());
    CHECK_EQ(meta.result_num, 1);
    REQUIRE_EQ(meta.params.size(), 1);
    CHECK_EQ(meta.params[0].name, "n");
    CHECK_EQ(meta.params[0].type, "int");
    CHECK_EQ(meta.params[0].default_value, "22");

    /** @arg the part is created from its full name with the defaults */
    auto ind = std::get<Indicator>(reg.create("core.ind.ma"));
    CHECK_UNARY(ind.alike(MA()));

    /** @arg a provided scalar overrides the default */
    Parameter sc;
    sc.set<int>("n", 10);
    auto ind10 = std::get<Indicator>(reg.create("core.ind.ma", sc));
    CHECK_UNARY(ind10.alike(MA(10)));

    /** @arg the registration is idempotent */
    registerIndicatorParts();
    CHECK_EQ(reg.getNameList(PartType::IND).size(), 138);
}

/** @par Test points */
TEST_CASE("test_registerIndicatorParts_concurrent_introspection") {
    PartRegistry::release();
    registerIndicatorParts();

    /** @arg concurrent first-time meta access of the same parts does not race */
    std::atomic<int> ok{0};
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; i++) {
        threads.emplace_back([&ok]() {
            auto& reg = PartRegistry::instance();
            for (int j = 0; j < 50; j++) {
                PartMeta meta;
                if (!reg.tryGetMeta("core.ind.ma", meta)) {
                    return;
                }
                if (meta.params.size() != 1 || meta.params[0].name != "n") {
                    return;
                }
                auto ind = std::get<Indicator>(reg.create("core.ind.ma"));
                if (!ind.alike(MA())) {
                    return;
                }
            }
            ok.fetch_add(1);
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    CHECK_EQ(ok.load(), 8);

    /** @arg the registry keeps consistent after the concurrent access */
    auto& reg = PartRegistry::instance();
    CHECK_EQ(reg.getNameList(PartType::IND).size(), 138);
    const PartMeta& meta = reg.getMeta("core.ind.ma");
    CHECK_EQ(meta.name, "MA");
    CHECK_EQ(meta.params[0].default_value, "22");
}

/** @} */
