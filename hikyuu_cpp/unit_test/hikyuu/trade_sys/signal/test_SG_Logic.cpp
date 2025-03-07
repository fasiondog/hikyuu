/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include "hikyuu/trade_sys/signal/crt/SG_Logic.h"
#include "hikyuu/trade_sys/signal/crt/SG_Manual.h"

using namespace hku;

/**
 * @defgroup test_Signal test_Signal_logic
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

static void reset_expect(std::map<Datetime, double>& expect) {
    for (auto& iter : expect) {
        iter.second = 0.0;
    }
}

static void check_result(SignalPtr sg, const std::map<Datetime, double>& expect) {
    for (auto iter = expect.begin(); iter != expect.end(); ++iter) {
        // HKU_INFO("{}: {}, {}", iter->first, sg->getValue(iter->first), iter->second);
        CHECK_EQ(sg->getValue(iter->first), iter->second);
    }
}

/** @par 检测点 */
TEST_CASE("test_SG_Add") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111125)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        expect[k[i].datetime] = 0.0;
    }

    /** @arg 输入的 sg 都为空 */
    SGPtr sg1, sg2;
    auto ret = sg1 + sg2;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 其中一个 sg 为空 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    ret = sg1 + sg2;
    ret->setTO(k);

    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111115)] = -1.0;
    check_result(ret, expect);

    ret = sg2 + sg1;
    ret->setTO(k);
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 不存在重叠 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    sg2 = SG_Manual();
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 + sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);

    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = -1.0;
    expect[Datetime(20111116)] = 0.0;
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 不存在重叠 */
    ret = sg1 + sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);

    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 1.0;
    expect[Datetime(20111115)] = -1.0;
    expect[Datetime(20111116)] = -1.0;
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    REQUIRE(sg1->getValue(Datetime(20111115)) == -1.0);
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    reset_expect(expect);
    expect[Datetime(20111110)] = 2.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = -1.0;
    expect[Datetime(20111116)] = 0.0;

    ret = sg1 + sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    REQUIRE(sg1->getValue(Datetime(20111115)) == -1.0);
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));
    REQUIRE(sg2->getValue(Datetime(20111116)) == -1.0);

    reset_expect(expect);
    expect[Datetime(20111110)] = 2.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = -1.0;
    expect[Datetime(20111116)] = -1.0;

    ret = sg1 + sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    check_result(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_SG_Sub") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111125)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        expect[k[i].datetime] = 0.0;
    }

    /** @arg 输入的 sg 都为空 */
    SGPtr sg1, sg2;
    auto ret = sg1 - sg2;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 其中一个 sg 为空 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    REQUIRE(!sg2);
    ret = sg1 - sg2;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111115)] = -1.0;
    check_result(ret, expect);

    ret = sg2 - sg1;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = -1.0;
    expect[Datetime(20111115)] = 1.0;
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 不存在重叠 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    sg2 = SG_Manual();
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 - sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);

    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = -1.0;
    expect[Datetime(20111115)] = 0.0;
    expect[Datetime(20111116)] = 1.0;
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 不存在重叠 */
    ret = sg1 - sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);

    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = -1.0;
    expect[Datetime(20111115)] = -1.0;
    expect[Datetime(20111116)] = 1.0;
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    REQUIRE(sg1->getValue(Datetime(20111115)) == -1.0);
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 - sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 0.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = 0.0;
    expect[Datetime(20111116)] = 1.0;
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    REQUIRE(sg1->getValue(Datetime(20111115)) == -1.0);
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));
    REQUIRE(sg2->getValue(Datetime(20111116)) == -1.0);

    ret = sg1 - sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 0.0;
    expect[Datetime(20111114)] = -2.0;
    expect[Datetime(20111115)] = -1.0;
    expect[Datetime(20111116)] = 1.0;
    check_result(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_SG_Mul") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111125)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        expect[k[i].datetime] = 0.0;
    }

    /** @arg 输入的 sg 都为空 */
    SGPtr sg1, sg2;
    auto ret = sg1 * sg2;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 其中一个 sg 为空 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    REQUIRE(!sg2);
    ret = sg1 * sg2;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg2 * sg1;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 不存在重叠 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    sg2 = SG_Manual();
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 * sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);

    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 不存在重叠 */
    ret = sg1 * sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    REQUIRE(sg1->getValue(Datetime(20111115)) == -1.0);
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 * sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = 0.0;
    expect[Datetime(20111116)] = 0.0;
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    sg1->_addSellSignal(Datetime(20111116));
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 * sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = 0.0;
    expect[Datetime(20111116)] = -1.0;
    check_result(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_SG_Div") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111125)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        expect[k[i].datetime] = 0.0;
    }

    /** @arg 输入的 sg 都为空 */
    SGPtr sg1, sg2;
    auto ret = sg1 / sg2;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 其中一个 sg 为空 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110), 2.0);
    sg1->_addSellSignal(Datetime(20111115), -3.0);

    REQUIRE(!sg2);
    ret = sg1 / sg2;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg2 / sg1;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 不存在重叠 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    sg2 = SG_Manual();
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 / sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);

    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 不存在重叠 */
    ret = sg1 / sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    REQUIRE(sg1->getValue(Datetime(20111115)) == -1.0);
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 / sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = 0.0;
    expect[Datetime(20111116)] = 0.0;
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110), 6.0);
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    sg1->_addSellSignal(Datetime(20111116), -6.0);
    sg2->_addBuySignal(Datetime(20111110), 2.0);
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116), -2.0);

    ret = sg1 / sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 3.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = 0.0;
    expect[Datetime(20111116)] = -3.0;
    check_result(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_SG_AddValue") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111125)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        expect[k[i].datetime] = 0.0;
    }

    /** @arg 输入的 sg 为空 */
    SGPtr sg;
    auto ret = sg + 1.0;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = 1.0 + sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg sg + number */
    sg = SG_Manual();
    sg->_addBuySignal(Datetime(20111110));
    sg->_addSellSignal(Datetime(20111115));

    ret = sg + 0.0;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111115)] = -1.0;
    check_result(ret, expect);

    ret = sg + Null<double>();
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111115)] = -1.0;
    check_result(ret, expect);

    ret = sg + 1.0;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 2.0;
    expect[Datetime(20111115)] = -2.0;
    check_result(ret, expect);

    /** @arg number + sg */
    ret = 0.0 + sg;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111115)] = -1.0;
    check_result(ret, expect);

    ret = Null<double>() + sg;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111115)] = -1.0;
    check_result(ret, expect);

    ret = 1.0 + sg;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 2.0;
    expect[Datetime(20111115)] = -2.0;
    check_result(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_SG_MulValue") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111125)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        expect[k[i].datetime] = 0.0;
    }

    /** @arg 输入的 sg 为空 */
    SGPtr sg;
    auto ret = sg * 1.0;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = 1.0 * sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = Null<double>() * sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg * Null<double>();
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg sg * number */
    sg = SG_Manual();
    sg->_addBuySignal(Datetime(20111110));
    sg->_addSellSignal(Datetime(20111115));

    ret = sg * 0.0;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg * Null<double>();
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg * 2.0;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 2.0;
    expect[Datetime(20111115)] = -2.0;
    check_result(ret, expect);

    /** @arg number * sg */
    ret = 0.0 * sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = Null<double>() * sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = 2.0 * sg;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 2.0;
    expect[Datetime(20111115)] = -2.0;
    check_result(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_SG_SubValue") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111125)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        expect[k[i].datetime] = 0.0;
    }

    /** @arg 输入的 sg 为空 */
    SGPtr sg;
    auto ret = sg - 1.0;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = 1.0 - sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = Null<double>() - sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg - Null<double>();
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg sg - number */
    sg = SG_Manual();
    sg->_addBuySignal(Datetime(20111110), 2.0);
    sg->_addSellSignal(Datetime(20111115), -3.0);

    ret = sg - 0.0;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 2.0;
    expect[Datetime(20111115)] = -3.0;
    check_result(ret, expect);

    ret = sg - Null<double>();
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 2.0;
    expect[Datetime(20111115)] = -3.0;
    check_result(ret, expect);

    ret = sg - 2.0;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg - 2.0;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111115)] = -5.0;
    check_result(ret, expect);

    /** @arg number - sg */
    ret = 0.0 - sg;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111115)] = 3.0;
    check_result(ret, expect);

    ret = 0.0 - sg;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = -2.0;
    expect[Datetime(20111115)] = 3.0;
    check_result(ret, expect);

    ret = Null<double>() - sg;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111115)] = 3.0;
    check_result(ret, expect);

    ret = Null<double>() - sg;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = -2.0;
    expect[Datetime(20111115)] = 3.0;
    check_result(ret, expect);

    ret = 2.0 - sg;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111115)] = 5.0;
    check_result(ret, expect);

    ret = 2.0 - sg;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111115)] = 5.0;
    check_result(ret, expect);

    ret = -2.0 - sg;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111115)] = 1.0;
    check_result(ret, expect);

    ret = -2.0 - sg;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = -4.0;
    expect[Datetime(20111115)] = 1.0;
    check_result(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_SG_DivValue") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111125)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        expect[k[i].datetime] = 0.0;
    }

    /** @arg 输入的 sg 为空 */
    SGPtr sg;
    auto ret = sg / 1.0;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = 1.0 / sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = Null<double>() / sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg / Null<double>();
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg sg / number */
    sg = SG_Manual();
    sg->_addBuySignal(Datetime(20111110));
    sg->_addSellSignal(Datetime(20111115));

    ret = sg / 0.0;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg / Null<double>();
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg / 2.0;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 0.5;
    expect[Datetime(20111115)] = -0.5;
    check_result(ret, expect);

    /** @arg number / sg */
    ret = 0.0 / sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = Null<double>() / sg;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = 2.0 / sg;
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 2.0;
    expect[Datetime(20111115)] = -2.0;
    check_result(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_SG_Or") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111125)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        expect[k[i].datetime] = 0.0;
    }

    /** @arg 输入的 sg 都为空 */
    SGPtr sg1, sg2;
    auto ret = sg1 | sg2;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 其中一个 sg 为空 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    ret = sg1 | sg2;
    ret->setTO(k);

    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111115)] = -1.0;
    check_result(ret, expect);

    ret = sg2 | sg1;
    ret->setTO(k);
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 不存在重叠 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    sg2 = SG_Manual();
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 | sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);

    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = -1.0;
    expect[Datetime(20111116)] = 0.0;
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 不存在重叠 */
    ret = sg1 | sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);

    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 1.0;
    expect[Datetime(20111115)] = -1.0;
    expect[Datetime(20111116)] = -1.0;
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    REQUIRE(sg1->getValue(Datetime(20111115)) == -1.0);
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = -1.0;
    expect[Datetime(20111116)] = 0.0;

    ret = sg1 | sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    REQUIRE(sg1->getValue(Datetime(20111115)) == -1.0);
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));
    REQUIRE(sg2->getValue(Datetime(20111116)) == -1.0);

    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = -1.0;
    expect[Datetime(20111116)] = -1.0;

    ret = sg1 | sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    check_result(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_SG_And") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111125)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        expect[k[i].datetime] = 0.0;
    }

    /** @arg 输入的 sg 都为空 */
    SGPtr sg1, sg2;
    auto ret = sg1 & sg2;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 其中一个 sg 为空 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    REQUIRE(!sg2);
    ret = sg1 & sg2;
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    ret = sg2 & sg1;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 不存在重叠 */
    sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111115));

    sg2 = SG_Manual();
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 & sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);

    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 不存在重叠 */
    ret = sg1 & sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    check_result(ret, expect);

    /** @arg 交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    REQUIRE(sg1->getValue(Datetime(20111115)) == -1.0);
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 & sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = 0.0;
    expect[Datetime(20111116)] = 0.0;
    check_result(ret, expect);

    /** @arg 非交替模式，sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->setParam<bool>("alternate", false);
    sg2->setParam<bool>("alternate", false);
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg1->_addSellSignal(Datetime(20111115));
    sg1->_addSellSignal(Datetime(20111116));
    sg2->_addBuySignal(Datetime(20111110));
    sg2->_addBuySignal(Datetime(20111114));
    sg2->_addSellSignal(Datetime(20111116));

    ret = sg1 & sg2;
    ret->setParam<bool>("alternate", false);
    ret->setTO(k);
    reset_expect(expect);
    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = 0.0;
    expect[Datetime(20111115)] = 0.0;
    expect[Datetime(20111116)] = -1.0;
    check_result(ret, expect);
}

/** @} */