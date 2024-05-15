/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-07
 *      Author: fasiondog
 */

#include "../test_config.h"
#include "hikyuu/StrategyContext.h"

/**
 * @defgroup test_hikyuu_StrategyContext test_hikyuu_StrategyContext
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_StrategyContext") {
    StrategyContext sc;
    sc.setKTypeList(
      {KQuery::MONTH, KQuery::MIN5, KQuery::DAY, KQuery::MIN, KQuery::WEEK, KQuery::MIN60});

    vector<KQuery::KType> expect{KQuery::MIN, KQuery::MIN5, KQuery::MIN60,
                                 KQuery::DAY, KQuery::WEEK, KQuery::MONTH};
    const auto ktypes = sc.getKTypeList();
    for (size_t i = 0, len = ktypes.size(); i < len; i++) {
        CHECK_EQ(ktypes[i], expect[i]);
    }
}

/** @} */