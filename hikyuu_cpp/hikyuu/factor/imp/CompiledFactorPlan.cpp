/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-15
 *      Author: woleigegg
 *
 *  Internal compiled execution plan for stock-local factor formulas.
 */

#include <algorithm>

#include "hikyuu/indicator/IndicatorImp.h"
#include "CompiledFactorPlan.h"

namespace hku {
namespace detail {

IndicatorImpPtr CompiledFactorPlan::cloneNode(const IndicatorImpPtr& src, CloneMap& clones) {
    if (!src) {
        return {};
    }

    auto iter = clones.find(src.get());
    if (iter != clones.end()) {
        return iter->second;
    }

    // Indicator2InImp::_clone() currently clones its private reference graph independently. The
    // executor still rebinds that graph correctly, but sharing inside it is not preserved yet.
    IndicatorImpPtr dst = src->_clone();
    clones.emplace(src.get(), dst);

    dst->m_params = src->m_params;
    dst->m_name = src->m_name;
    dst->m_is_python_object = src->m_is_python_object;
    dst->m_need_self_alike_compare = src->m_need_self_alike_compare;
    dst->m_is_serial = src->m_is_serial;
    dst->m_discard = src->m_discard;
    dst->m_result_num = src->m_result_num;
    dst->m_context = src->m_context;
    dst->m_old_context = src->m_old_context;
    dst->m_need_calculate = src->m_need_calculate;
    dst->m_param_changed = src->m_param_changed;
    dst->m_optype = src->m_optype;
    dst->m_parent = nullptr;

    dst->_readyBuffer(src->size(), src->m_result_num);
    for (size_t i = 0; i < src->m_result_num; ++i) {
        if (src->m_pBuffer[i]) {
            std::copy(src->m_pBuffer[i]->begin(), src->m_pBuffer[i]->end(),
                      dst->m_pBuffer[i]->begin());
        }
    }

    dst->m_left = cloneNode(src->m_left, clones);
    dst->m_right = cloneNode(src->m_right, clones);
    dst->m_three = cloneNode(src->m_three, clones);
    if (dst->m_left) {
        dst->m_left->m_parent = dst.get();
    }
    if (dst->m_right) {
        dst->m_right->m_parent = dst.get();
    }
    if (dst->m_three) {
        dst->m_three->m_parent = dst.get();
    }

    dst->m_ind_params.clear();
    for (const auto& [name, value] : src->m_ind_params) {
        dst->m_ind_params[name] = cloneNode(value, clones);
    }

    return dst;
}

IndicatorImpPtr CompiledFactorPlan::canonicalizeNode(const IndicatorImpPtr& node,
                                                     CanonicalMap& canonical,
                                                     vector<IndicatorImpPtr>& unique_nodes) {
    if (!node) {
        return {};
    }

    auto iter = canonical.find(node.get());
    if (iter != canonical.end()) {
        return iter->second;
    }

    node->m_left = canonicalizeNode(node->m_left, canonical, unique_nodes);
    node->m_right = canonicalizeNode(node->m_right, canonical, unique_nodes);
    node->m_three = canonicalizeNode(node->m_three, canonical, unique_nodes);
    for (auto& [_, value] : node->m_ind_params) {
        value = canonicalizeNode(value, canonical, unique_nodes);
    }

    for (const auto& candidate : unique_nodes) {
        if (candidate->alike(*node)) {
            canonical.emplace(node.get(), candidate);
            return candidate;
        }
    }

    unique_nodes.emplace_back(node);
    canonical.emplace(node.get(), node);
    return node;
}

void CompiledFactorPlan::prepareNode(const IndicatorImpPtr& node, const KData& kdata,
                                     std::unordered_set<IndicatorImp*>& visited) {
    if (!node || !visited.emplace(node.get()).second) {
        return;
    }

    node->onlySetContext(kdata);
    node->m_old_context = KData();
    node->m_need_calculate = true;
    node->m_param_changed = true;
    node->m_discard = 0;

    // CONTEXT owns a deliberately independent input context. Its implementation is responsible
    // for rebinding that private subgraph when the outer context changes.
    if (node->name() == "CONTEXT") {
        return;
    }

    prepareNode(node->m_left, kdata, visited);
    prepareNode(node->m_right, kdata, visited);
    prepareNode(node->m_three, kdata, visited);
    for (const auto& [_, value] : node->m_ind_params) {
        prepareNode(value, kdata, visited);
        value->calculate();
    }

    vector<IndicatorImpPtr> inner_nodes;
    node->getSelfInnerNodesWithInputConext(inner_nodes);
    for (const auto& inner : inner_nodes) {
        prepareNode(inner, kdata, visited);
    }
}

bool CompiledFactorPlan::isEligibleNode(const IndicatorImpPtr& node,
                                        std::unordered_set<const IndicatorImp*>& visited) {
    if (!node || !visited.emplace(node.get()).second) {
        return true;
    }

    // Python implementations can retain arbitrary state that the C++ executor cannot reset.
    // CONTEXT contains a private independently-bound graph and stays on the proven legacy path.
    if (!node->supportBatchReuse() || node->name() == "CONTEXT") {
        return false;
    }

    if (!isEligibleNode(node->m_left, visited) || !isEligibleNode(node->m_right, visited) ||
        !isEligibleNode(node->m_three, visited)) {
        return false;
    }
    for (const auto& [_, value] : node->m_ind_params) {
        if (!isEligibleNode(value, visited)) {
            return false;
        }
    }

    vector<IndicatorImpPtr> inner_nodes;
    node->getSelfInnerNodesWithInputConext(inner_nodes);
    for (const auto& inner : inner_nodes) {
        if (!isEligibleNode(inner, visited)) {
            return false;
        }
    }
    return true;
}

void CompiledFactorPlan::scrubTemplateNode(const IndicatorImpPtr& node,
                                           std::unordered_set<IndicatorImp*>& visited) {
    if (!node || !visited.emplace(node.get()).second) {
        return;
    }

    node->m_context = KData();
    node->m_old_context = KData();
    node->m_need_calculate = true;
    node->m_param_changed = true;
    if (!node->isLeaf() || node->isNeedContext()) {
        node->_clearBuffer();
        node->m_discard = 0;
    }

    scrubTemplateNode(node->m_left, visited);
    scrubTemplateNode(node->m_right, visited);
    scrubTemplateNode(node->m_three, visited);
    for (const auto& [_, value] : node->m_ind_params) {
        scrubTemplateNode(value, visited);
    }

    vector<IndicatorImpPtr> inner_nodes;
    node->getSelfInnerNodesWithInputConext(inner_nodes);
    for (const auto& inner : inner_nodes) {
        scrubTemplateNode(inner, visited);
    }
}

void CompiledFactorPlan::normalizeParents(const IndicatorList& roots) {
    using ParentInfo = std::pair<size_t, IndicatorImp*>;
    std::unordered_map<IndicatorImp*, ParentInfo> parents;
    std::unordered_set<IndicatorImp*> visited;
    vector<IndicatorImpPtr> stack;

    for (const auto& root : roots) {
        if (root.getImp()) {
            auto [iter, inserted] = parents.emplace(root.getImp().get(), ParentInfo{1, nullptr});
            if (!inserted) {
                ++iter->second.first;
            }
            root.getImp()->m_parent = nullptr;
            stack.emplace_back(root.getImp());
        }
    }

    while (!stack.empty()) {
        IndicatorImpPtr node = std::move(stack.back());
        stack.pop_back();
        if (!node || !visited.emplace(node.get()).second) {
            continue;
        }

        auto record_child = [&](const IndicatorImpPtr& child) {
            if (!child) {
                return;
            }
            auto [iter, inserted] = parents.emplace(child.get(), ParentInfo{1, node.get()});
            if (!inserted) {
                ++iter->second.first;
            }
            stack.emplace_back(child);
        };
        record_child(node->m_left);
        record_child(node->m_right);
        record_child(node->m_three);
        for (const auto& [_, value] : node->m_ind_params) {
            record_child(value);
        }
    }

    for (const auto& [node, info] : parents) {
        node->m_parent = info.first == 1 ? info.second : nullptr;
    }
}

CompiledFactorPlan::CompiledFactorPlan(const IndicatorList& formulas) {
    std::unordered_set<const IndicatorImp*> visited;
    for (const auto& formula : formulas) {
        if (!isEligibleNode(formula.getImp(), visited)) {
            m_reusable = false;
            return;
        }
    }

    m_roots.reserve(formulas.size());
    CloneMap clones;
    for (const auto& formula : formulas) {
        m_roots.emplace_back(cloneNode(formula.getImp(), clones));
    }
    normalizeParents(m_roots);

    std::unordered_set<IndicatorImp*> scrubbed;
    for (const auto& root : m_roots) {
        scrubTemplateNode(root.getImp(), scrubbed);
    }

    CanonicalMap canonical;
    vector<IndicatorImpPtr> unique_nodes;
    for (auto& root : m_roots) {
        root = Indicator(canonicalizeNode(root.getImp(), canonical, unique_nodes));
    }
    normalizeParents(m_roots);

    // Private derived-class graphs are intentionally left to the existing indicator hook. They
    // remain correct but cannot participate in plan-wide CSE until their ownership is exposed.
    for (const auto& root : m_roots) {
        if (root.getImp()) {
            root.getImp()->repeatSeparateKTypeLeafALikeNodes();
        }
    }
    normalizeParents(m_roots);
}

IndicatorList CompiledFactorPlan::cloneRoots() const {
    IndicatorList roots;
    roots.reserve(m_roots.size());

    CloneMap clones;
    for (const auto& root : m_roots) {
        roots.emplace_back(cloneNode(root.getImp(), clones));
    }

    normalizeParents(roots);

    return roots;
}

FactorPlanExecutor CompiledFactorPlan::createExecutor() const {
    return FactorPlanExecutor(cloneRoots());
}

IndicatorList FactorPlanExecutor::executeValues(const KData& kdata) {
    if (kdata.empty()) {
        return IndicatorList(m_roots.size());
    }

    std::unordered_set<IndicatorImp*> visited;
    for (const auto& root : m_roots) {
        CompiledFactorPlan::prepareNode(root.getImp(), kdata, visited);
    }

    std::unordered_set<IndicatorImp*> executed_roots;
    for (auto& root : m_roots) {
        if (root.getImp() && executed_roots.emplace(root.getImp().get()).second) {
            root.getImp()->calculate();
        }
    }

    IndicatorList result;
    result.reserve(m_roots.size());
    for (const auto& root : m_roots) {
        result.emplace_back(root.getResult(0));
    }
    return result;
}

}  // namespace detail
}  // namespace hku
