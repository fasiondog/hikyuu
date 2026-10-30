/*
 * PartRegister.h
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_PART_REGISTER_H_
#define HKU_PART_REGISTER_H_

#include <functional>
#include "hikyuu/indicator/Indicator.h"
#include "PartRegistry.h"

namespace hku {

/**
 * Register an indicator part by introspecting its prototype.
 *
 * Param names, types and default values come from the Parameter of the prototype returned by
 * factory (i.e. the factory call with its default arguments); doc/label stay empty for the doc
 * pipeline to fill in. Idempotent: a repeated call for an already registered full_name is a
 * no-op, so the module aggregation functions can be called again after a registry reset.
 *
 * @param full_name hub style full name, e.g. "core.ind.ma"
 * @param factory returns the prototype and serves as the creator as well
 */
void HKU_API registerIndicatorPart(const string& full_name,
                                   const std::function<Indicator()>& factory);

/**
 * Register an indicator part with a hand written meta, for the parts whose prototype can not be
 * built without a context (e.g. it requires a KData or an Indicator).
 *
 * @param full_name hub style full name; it overrides meta.name/meta.full_name/meta.type
 * @param meta hand written meta, its params must be in the real signature order
 * @param creator factory translating (scalars, part params) into the indicator
 */
void HKU_API registerIndicatorPart(const string& full_name, const PartMeta& meta,
                                   const PartRegistry::Creator& creator);

/**
 * Read a part-typed param from the PartParamMap, falling back to the given default when the
 * caller did not provide it (the param has a default in the real signature).
 */
template <typename T>
T partOr(const PartParamMap& parts, const string& name, const T& default_value) {
    auto iter = parts.find(name);
    if (iter == parts.end() || iter->second.empty()) {
        return default_value;
    }
    return std::get<T>(iter->second.at(0));
}

}  // namespace hku

#endif /* HKU_PART_REGISTER_H_ */
