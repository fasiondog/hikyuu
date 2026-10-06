/*
 *  Copyright(c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-06
 *      Author: fasiondog
 */

#include "hikyuu/utilities/db_connect/SQLStatementBase.h"

namespace hku {

// Defined inside the library so that this class has a key function: the vtable and the typeinfo
// are emitted once here and referenced by every other translation unit, which keeps
// catch (const null_blob_exception &) working across a shared library boundary. The drivers
// throw it in their own translation units, while the getColumn templates of the header catch it
// in the binary of the caller
null_blob_exception::~null_blob_exception() noexcept = default;

}  // namespace hku
