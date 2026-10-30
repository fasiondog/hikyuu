/*
 * PartRegistration.h
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_PART_REGISTRATION_H_
#define HKU_PART_REGISTRATION_H_

#include "PartMeta.h"

namespace hku {

/**
 * Module aggregation functions of the built-in part registration.
 *
 * Each one is defined in the build_in.cpp of its own module and registers every part of that
 * module; GlobalInitializer::init() calls them all in turn. They are idempotent, so they can be
 * called again after a registry reset.
 *
 * The declarations are collected here (instead of each module build_in.h) so that
 * GlobalInitializer does not have to pull in all the crt headers. They are exported (unlike the
 * per part registerPart_XXX()) because the unit tests and the hosts may trigger them explicitly.
 */
HKU_API void registerIndicatorParts();
HKU_API void registerSignalParts();
HKU_API void registerConditionParts();
HKU_API void registerEnvironmentParts();
HKU_API void registerMoneyManagerParts();
HKU_API void registerStoplossParts();
HKU_API void registerProfitGoalParts();
HKU_API void registerSlippageParts();
HKU_API void registerPortfolioParts();
HKU_API void registerSelectorParts();
HKU_API void registerAllocateFundsParts();
HKU_API void registerMultiFactorParts();
HKU_API void registerSystemParts();
HKU_API void registerTradeManageParts();

/** Register every built-in part; called from GlobalInitializer::init() */
HKU_API void registerAllBuiltinParts();

}  // namespace hku

#endif /* HKU_PART_REGISTRATION_H_ */
