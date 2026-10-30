#include "build_in.h"

namespace hku {

void registerMultiFactorParts() {
    registerPart_MF_EqualWeight();
    registerPart_MF_ICIRWeight();
    registerPart_MF_ICWeight();
    registerPart_MF_Weight();
}

}  // namespace hku
