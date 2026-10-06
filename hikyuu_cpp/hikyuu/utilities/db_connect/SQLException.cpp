/*
 *  Copyright(c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-06
 *      Author: fasiondog
 */

#include "hikyuu/utilities/db_connect/SQLException.h"

namespace hku {

// Defined inside the library so that this class has a key function: the vtable and the typeinfo
// are emitted once here and referenced by every other translation unit, which keeps
// catch (const SQLException &) working across a shared library boundary
SQLException::~SQLException() noexcept = default;

}  // namespace hku
