/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Internal compiled execution plan for stock-local factor formulas.
 */

#pragma once

#include <unordered_map>
#include <unordered_set>

#include "hikyuu/indicator/Indicator.h"

namespace hku {
namespace detail {

class FactorPlanExecutor;

class HKU_API CompiledFactorPlan {
public:
    explicit CompiledFactorPlan(const IndicatorList& formulas);

    bool isReusable() const noexcept {
        return m_reusable;
    }

    FactorPlanExecutor createExecutor() const;

    size_t size() const noexcept {
        return m_roots.size();
    }

private:
    friend class FactorPlanExecutor;
    using CloneMap = std::unordered_map<const IndicatorImp*, IndicatorImpPtr>;
    using CanonicalMap = std::unordered_map<const IndicatorImp*, IndicatorImpPtr>;

    static IndicatorImpPtr cloneNode(const IndicatorImpPtr& src, CloneMap& clones);
    static IndicatorImpPtr canonicalizeNode(const IndicatorImpPtr& node, CanonicalMap& canonical,
                                            vector<IndicatorImpPtr>& unique_nodes);
    static bool isEligibleNode(const IndicatorImpPtr& node,
                               std::unordered_set<const IndicatorImp*>& visited);
    static void scrubTemplateNode(const IndicatorImpPtr& node,
                                  std::unordered_set<IndicatorImp*>& visited);
    static void prepareNode(const IndicatorImpPtr& node, const KData& kdata,
                            std::unordered_set<IndicatorImp*>& visited);
    static void normalizeParents(const IndicatorList& roots);
    IndicatorList cloneRoots() const;

private:
    IndicatorList m_roots;
    bool m_reusable{true};
};

class HKU_API FactorPlanExecutor {
public:
    FactorPlanExecutor(const FactorPlanExecutor&) = delete;
    FactorPlanExecutor& operator=(const FactorPlanExecutor&) = delete;
    FactorPlanExecutor(FactorPlanExecutor&&) noexcept = default;
    FactorPlanExecutor& operator=(FactorPlanExecutor&&) noexcept = default;

    IndicatorList executeValues(const KData& kdata);

private:
    friend class CompiledFactorPlan;
    explicit FactorPlanExecutor(IndicatorList roots) : m_roots(std::move(roots)) {}

private:
    IndicatorList m_roots;
};

}  // namespace detail
}  // namespace hku
