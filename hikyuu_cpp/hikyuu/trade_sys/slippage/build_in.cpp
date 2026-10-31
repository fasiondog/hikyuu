#include "build_in.h"

namespace hku {

void registerSlippageParts() {
    registerPart_SP_FixedPercent();
    registerPart_SP_FixedValue();
    registerPart_SP_LogNormal();
    registerPart_SP_Normal();
    registerPart_SP_TruncNormal();
    registerPart_SP_Uniform();
}

}  // namespace hku
