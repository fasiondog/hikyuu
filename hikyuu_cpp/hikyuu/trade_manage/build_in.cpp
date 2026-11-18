#include "build_in.h"

namespace hku {

void registerTradeManageParts() {
    registerPart_crtTM();
    registerMarginParts();
    registerTradeCostParts();
}

}  // namespace hku
