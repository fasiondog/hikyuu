#include "build_in.h"

namespace hku {

void registerMoneyManagerParts() {
    registerPart_MM_FixedCapital();
    registerPart_MM_FixedCapitalFunds();
    registerPart_MM_FixedCount();
    registerPart_MM_FixedCountTps();
    registerPart_MM_FixedPercent();
    registerPart_MM_FixedRisk();
    registerPart_MM_FixedUnits();
    registerPart_MM_Nothing();
    registerPart_MM_WilliamsFixedRisk();
}

}  // namespace hku
