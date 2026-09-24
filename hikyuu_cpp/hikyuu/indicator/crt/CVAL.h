/*
 * CVAL.h
 *
 *  Created on: 2017-6-25
 *      Author: fasiondog
 */

#pragma once
#ifndef CVAL_H_
#define CVAL_H_

#include "../Indicator.h"

namespace hku {

/**
 * Create a constant indicator with the given length

 * @param value the constant

 * @param discard the number to discard, 0 by default

 * @ingroup Indicator
 */
Indicator HKU_API CVAL(double value, size_t discard=0);

/**
 * Create a constant indicator whose length is the same as the input ind and whose value is fixed to
 * the given value

 * @param ind the data to be calculated

 * @param value the constant

 * @param discard the number to discard, 0 by default

 * @ingroup Indicator
 */
Indicator HKU_API CVAL(const Indicator& ind, double value = 0.0, int discard = 0);

} /* namespace */

#endif /* CVAL_H_ */
