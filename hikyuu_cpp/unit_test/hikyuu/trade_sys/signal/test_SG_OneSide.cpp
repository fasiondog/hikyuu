/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-13
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include "hikyuu/StockManager.h"
#include "hikyuu/indicator/crt/KDATA.h"
#include "hikyuu/indicator/crt/REF.h"
#include "hikyuu/trade_sys/signal/crt/SG_Logic.h"
#include "hikyuu/trade_sys/signal/crt/SG_Bool.h"
#include "hikyuu/trade_sys/signal/crt/SG_OneSide.h"

using namespace hku;

/**
 * @defgroup test_Signal test_Signal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SG_OneSide") {
    auto k = getKData("sh000001", KQuery(-30));
    Indicator ind;
    SignalPtr sg;

    /** @arg The input ind is empty */
    sg = SG_OneSide(ind, true);
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());

    sg = SG_OneSide(ind, false);
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());

    /** @arg A one-sided buy signal */
    ind = CLOSE() > REF(CLOSE(), 1);
    auto sg_buy = SG_OneSide(ind, true);
    sg_buy->setTO(k);
    auto expect = ind(k);
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        if (expect[i] > 0.0) {
            CHECK_UNARY(sg_buy->shouldBuy(k[i].datetime));
            CHECK_UNARY(!sg_buy->shouldSell(k[i].datetime));
        } else {
            CHECK_UNARY(!sg_buy->shouldBuy(k[i].datetime));
            CHECK_UNARY(!sg_buy->shouldSell(k[i].datetime));
        }
    }

    /** @arg A one-sided sell signal */
    ind = CLOSE() < REF(CLOSE(), 1);
    auto sg_sell = SG_OneSide(ind, false);
    sg_sell->setTO(k);
    expect = ind(k);
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        if (expect[i] > 0.0) {
            CHECK_UNARY(sg_sell->shouldSell(k[i].datetime));
            CHECK_UNARY(!sg_sell->shouldBuy(k[i].datetime));
        } else {
            CHECK_UNARY(!sg_sell->shouldSell(k[i].datetime));
            CHECK_UNARY(!sg_sell->shouldBuy(k[i].datetime));
        }
    }

    /** @arg Try to change the alternate parameter */
    sg_sell->setParam<bool>("alternate", false);
    CHECK_THROWS(sg_sell->setParam<bool>("alternate", true));

    /** @arg A one-sided buy + a one-sided sell */
    sg = sg_buy + sg_sell;
    sg->setParam<bool>("alternate", true);
    sg->setTO(k);
    auto sg_expect = SG_Bool(CLOSE() > REF(CLOSE(), 1), CLOSE() < REF(CLOSE(), 1));
    sg_expect->setTO(k);
    for (size_t i = 0; i < k.size(); i++) {
        CHECK_EQ(sg->shouldBuy(k[i].datetime), sg_expect->shouldBuy(k[i].datetime));
        CHECK_EQ(sg->shouldSell(k[i].datetime), sg_expect->shouldSell(k[i].datetime));
    }

    sg->setParam<bool>("alternate", false);
    sg_expect = SG_Bool(CLOSE() > REF(CLOSE(), 1), CLOSE() < REF(CLOSE(), 1), false);
    sg->setTO(k);
    sg_expect->setTO(k);
    for (size_t i = 0; i < k.size(); i++) {
        CHECK_EQ(sg->shouldBuy(k[i].datetime), sg_expect->shouldBuy(k[i].datetime));
        CHECK_EQ(sg->shouldSell(k[i].datetime), sg_expect->shouldSell(k[i].datetime));
    }
}

/** @} */
