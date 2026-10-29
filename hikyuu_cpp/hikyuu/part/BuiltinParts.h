/*
 * BuiltinParts.h
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_BUILTIN_PARTS_H_
#define HKU_BUILTIN_PARTS_H_

#include "hikyuu/part/PartRegistry.h"

namespace hku {

/**
 * Explicitly register the hand-written builtin sample parts (core.ind.MA / core.sg.SG_Band /
 * core.tm.crtTM), validating the registry end to end. The full list will be produced by the
 * generation script later and replace this file.
 * No global dynamic initialization is involved (static initialization order fiasco safe);
 * the call must happen before any create. Idempotent: repeated calls are no-ops.
 */
HKU_API void registerBuiltinParts();

}  // namespace hku

#endif /* HKU_BUILTIN_PARTS_H_ */
