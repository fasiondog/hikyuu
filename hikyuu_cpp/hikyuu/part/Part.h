/*
 * Part.h
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_PART_H_
#define HKU_PART_H_

#include <map>
#include <optional>
#include <utility>
#include <variant>
#include "hikyuu/indicator/Indicator.h"

namespace hku {

class SignalBase;
class MoneyManagerBase;
class StoplossBase;
class ConditionBase;
class EnvironmentBase;
class ProfitGoalBase;
class SlippageBase;
class SelectorBase;
class AllocateFundsBase;
class MultiFactorBase;
class System;
class MultiSystem;
class TradeManagerBase;
class TradeCostBase;

/**
 * Part category, aligned with the part dictionary of hikyuu/hub.py (the hub's "other" is
 * excluded). TM/TC are hikyuu extensions beyond the hub part dictionary (TM currently has
 * only the crtTM factory, TC has the TC_* factories); both may be extended in the future.
 */
enum class PartType { IND, SG, MM, ST, CN, EV, PG, SP, PF, SE, AF, MF, SYS, TM, TC };

/** Get the string name of the part category: "ind" / "sg" / ... / "tc" */
HKU_API const char* partTypeName(PartType type);

/** Parse the part category from its string name; returns std::nullopt if invalid */
HKU_API std::optional<PartType> parsePartType(const string& name);

/**
 * Unified part handle.
 * - Indicator is value semantics (holds a shared_ptr internally, copy cost equals a
 *   shared_ptr copy); the others are shared_ptr semantics.
 *
 * The aliases below are identical to the canonical typedefs of each base class header
 * (redundant typedefs of the same type are allowed), so this header stays free of the heavy
 * base class headers; the alternatives only need to be complete where they are actually used
 * (e.g. in PartRegistry.cpp).
 */
using SignalPtr = std::shared_ptr<SignalBase>;              // Same as SignalBase.h
using MMPtr = std::shared_ptr<MoneyManagerBase>;            // Same as MoneyManagerBase.h
using STPtr = std::shared_ptr<StoplossBase>;                // Same as StoplossBase.h
using CNPtr = std::shared_ptr<ConditionBase>;               // Same as ConditionBase.h
using EVPtr = std::shared_ptr<EnvironmentBase>;             // Same as EnvironmentBase.h
using PGPtr = std::shared_ptr<ProfitGoalBase>;              // Same as ProfitGoalBase.h
using SPPtr = std::shared_ptr<SlippageBase>;                // Same as SlippageBase.h
using PFPtr = std::shared_ptr<MultiSystem>;                 // Same as MultiSystem.h
using SEPtr = std::shared_ptr<SelectorBase>;                // Same as SelectorBase.h
using AFPtr = std::shared_ptr<AllocateFundsBase>;           // Same as AllocateFundsBase.h
using MFPtr = std::shared_ptr<MultiFactorBase>;             // Same as MultiFactorBase.h
using SYSPtr = std::shared_ptr<System>;                     // Same as System.h
using TradeManagerPtr = std::shared_ptr<TradeManagerBase>;  // Same as TradeManagerBase.h
using TradeCostPtr = std::shared_ptr<TradeCostBase>;        // Same as TradeCostBase.h

using Part = std::variant<Indicator, SignalPtr, MMPtr, STPtr, CNPtr, EVPtr, PGPtr, SPPtr, PFPtr,
                          SEPtr, AFPtr, MFPtr, SYSPtr, TradeManagerPtr, TradeCostPtr>;

/** Part list for list-typed params (e.g. SystemList, IndicatorList) */
using PartList = std::vector<Part>;

/**
 * Part-typed param channel shared by single and list params: a non-list param holds exactly
 * 1 element, a list param 0~N elements (Parameter does not support Indicator, see the design
 * doc).
 */
using PartParamMap = std::map<string, PartList>;

/** Get the part category of the handle */
HKU_API PartType partTypeOf(const Part& part);

/**
 * Get the name of the held part (every alternative provides name()).
 * Note: for TM the name() is the mutable account name ("SYS" by default), not the factory
 * identity; the registry keys on full_name and is unaffected by this.
 */
HKU_API string partName(const Part& part);

/**
 * Type-safe access wrapping std::visit; a missing branch is a compile-time error.
 * It is recommended to always provide a [](const auto&) fallback so that adding a new
 * alternative does not break existing call sites.
 * @code
 *   visitPart(part,
 *             [](const Indicator& ind) {},
 *             [](const SignalPtr& sg) {},
 *             [](const auto&) {});
 * @endcode
 */
template <typename... Handlers>
struct PartVisitor : Handlers... {
    using Handlers::operator()...;
};

template <typename... Handlers>
PartVisitor(Handlers...) -> PartVisitor<Handlers...>;

template <typename... Handlers>
decltype(auto) visitPart(const Part& part, Handlers&&... handlers) {
    return std::visit(PartVisitor{std::forward<Handlers>(handlers)...}, part);
}

template <typename... Handlers>
decltype(auto) visitPart(Part& part, Handlers&&... handlers) {
    return std::visit(PartVisitor{std::forward<Handlers>(handlers)...}, part);
}

/** Try to get the alternative of type T; returns std::nullopt on type mismatch (no exception) */
template <typename T>
std::optional<T> tryGetAs(const Part& part) {
    if (const T* p = std::get_if<T>(&part)) {
        return *p;
    }
    return std::nullopt;
}

}  // namespace hku

#endif /* HKU_PART_H_ */
