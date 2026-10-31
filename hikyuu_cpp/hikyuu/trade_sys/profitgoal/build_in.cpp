#include "build_in.h"

namespace hku {

void registerProfitGoalParts() {
    registerPart_PG_FixedHoldDays();
    registerPart_PG_FixedPercent();
    registerPart_PG_NoGoal();
}

}  // namespace hku
