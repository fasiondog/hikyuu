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
 * GlobalInitializer does not have to pull in all the crt headers. They are internal to the
 * library: registerAllBuiltinParts() is the only exported entry point, and the hosts and the
 * tests call the modules through it.
 */
void registerIndicatorParts();
void registerSignalParts();
void registerConditionParts();
void registerEnvironmentParts();
void registerMoneyManagerParts();
void registerStoplossParts();
void registerProfitGoalParts();
void registerSlippageParts();
void registerPortfolioParts();
void registerSelectorParts();
void registerAllocateFundsParts();
void registerMultiFactorParts();
void registerSystemParts();
void registerTradeManageParts();

/** Register every built-in part; called from GlobalInitializer::init() */
HKU_API void registerAllBuiltinParts();

}  // namespace hku

#endif /* HKU_PART_REGISTRATION_H_ */
