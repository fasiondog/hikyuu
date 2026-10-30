#include "build_in.h"

namespace hku {

void registerPortfolioParts() {
    registerPart_PF_SignalAggregate();
    registerPart_PF_Simple();
    registerPart_PF_WithoutAF();
}

}  // namespace hku
