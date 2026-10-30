#include "build_in.h"

namespace hku {

void registerSelectorParts() {
    registerPart_SE_Fixed();
    registerPart_SE_MultiFactor();
    registerPart_SE_MultiFactor2();
    registerPart_SE_MaxFundsOptimal();
    registerPart_SE_Signal();
}

}  // namespace hku
