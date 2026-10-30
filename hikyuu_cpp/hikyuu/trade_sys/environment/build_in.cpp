#include "build_in.h"

namespace hku {

void registerEnvironmentParts() {
    registerPart_EV_Bool();
    registerPart_EV_Manual();
    registerPart_EV_TwoLine();
}

}  // namespace hku
