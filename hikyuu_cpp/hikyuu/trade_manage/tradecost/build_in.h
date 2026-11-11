/*
 * build_in.h
 *
 *  Created on: 2026-11-5
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADECOST_BUILD_IN_H_
#define TRADECOST_BUILD_IN_H_

#include "crt/TC_FixedA.h"
#include "crt/TC_FixedAPre2015.h"
#include "crt/TC_FixedA2015.h"
#include "crt/TC_FixedA2017.h"
#include "crt/TC_Fixed.h"
#include "crt/TC_FixedETF.h"
#include "crt/TC_HK.h"
#include "crt/TC_MarginInterest.h"
#include "crt/TC_Zero.h"

namespace hku {

void registerTradeCostParts();

}  // namespace hku

#endif /* TRADECOST_BUILD_IN_H_ */
