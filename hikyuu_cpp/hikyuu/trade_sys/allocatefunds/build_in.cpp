#include "build_in.h"

namespace hku {

void registerAllocateFundsParts() {
    registerPart_AF_EqualWeight();
    registerPart_AF_FixedAmount();
    registerPart_AF_FixedWeight();
    registerPart_AF_FixedWeightList();
    registerPart_AF_MultiFactor();
}

}  // namespace hku
