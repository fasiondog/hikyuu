#include "build_in.h"

namespace hku {

void registerTradeCostParts() {
    registerPart_TC_FixedA();
    registerPart_TC_FixedA2015();
    registerPart_TC_FixedA2017();
    registerPart_TC_Fixed();
    registerPart_TC_FixedAPre2015();
    registerPart_TC_FixedETF();
    registerPart_TC_HK();
    registerPart_TC_MarginInterest();
    registerPart_TC_Zero();
}

}  // namespace hku
