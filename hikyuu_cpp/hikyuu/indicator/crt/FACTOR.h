/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-15
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"
#include "hikyuu/factor/Factor.h"

namespace hku {

/**
 * Factor to indicator conversion
 * @details Convert a Factor object into an Indicator, so that it can be used in the indicator
 *          system.
 *          This indicator needs a K-line context to be calculated.
 * @note Two Factors are considered the same by their names only
 * @param factor the factor object
 * @return the Indicator object
 * @ingroup Factor
 */
Indicator HKU_API FACTOR(const Factor& factor);

/**
 * Factor to indicator conversion (convenience version)
 * @details Create an Indicator by the factor name, so that it can be used in the indicator system.
 *          This indicator needs a K-line context to be calculated.
 * @note Two Factors are considered the same by their names only
 * @param name factor name
 * @return the Indicator object
 * @ingroup Factor
 */
inline Indicator FACTOR(const string& name) {
    return FACTOR(Factor(name));
}

}  // namespace hku