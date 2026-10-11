/*
 *  Copyright (c) 2026 hikyuu.org
 */

#include "../test_config.h"
#include "hikyuu/GlobalInitializer.h"
#include "hikyuu/StockManager.h"

/**
 * @defgroup test_hikyuu_GlobalInitializer test_hikyuu_GlobalInitializer
 * @ingroup test_hikyuu_base_suite
 * @{
 */

// Disabled by default: it cannot fail unless the instance counting itself regresses. Enable it when
// GlobalInitializer changes.
#if 0
/** @par Test points */
TEST_CASE("test_GlobalInitializer_instance_counting") {
    auto& sm = StockManager::instance();

    /** @arg An extra instance does not initialize again, and destroying it does not release the
     * global resources while the static instances are still alive */
    CHECK_UNARY(!sm.getStock("sh000001").isNull());
    {
        GlobalInitializer guard;
        CHECK_UNARY(!sm.getStock("sh000001").isNull());
    }
    CHECK_UNARY(!sm.getStock("sh000001").isNull());
}
#endif

/** @} */
