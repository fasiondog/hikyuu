/*
 * PartRegistry.h
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_PART_REGISTRY_H_
#define HKU_PART_REGISTRY_H_

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include "hikyuu/utilities/Parameter.h"
#include "Part.h"
#include "PartMeta.h"

namespace hku {

/**
 * Unified part registry (singleton).
 *
 * - Scalar params go through Parameter; part-typed params (Indicator, System, ...) go through
 *   the PartParamMap, since Parameter does not support Indicator (neither the serialization
 *   format nor the type set may be changed for compatibility).
 * - PartParamMap holds a PartList per param name: a non-list part param takes exactly 1 element,
 *   a list param (e.g. SystemList) takes 0~N elements; both share the same channel.
 * - All member functions are defined in PartRegistry.cpp (not inline) and exported with HKU_API;
 *   see the cross-DLL lesson recorded in IndicatorImp.h.
 * - Read-only lookups are lock-free O(1)/O(log n); registration must be finished before the
 *   calculation hot path starts.
 */
class HKU_API PartRegistry {
public:
    using PartList = ::hku::PartList;
    using PartParamMap = ::hku::PartParamMap;
    /** scalars via Parameter, part-typed params via PartParamMap */
    using Creator = std::function<Part(const Parameter&, const PartParamMap&)>;

    /**
     * Process-wide singleton, lazily allocated and intentionally never destroyed automatically
     * (same as StockManager): it avoids the static destruction order fiasco and the leak-check
     * false positives caused by destructing a static object at exit.
     */
    static PartRegistry& instance();

    /**
     * Release the singleton, called from GlobalInitializer::clean() (same as
     * DataDriverFactory::release()); instance() recreates it afterwards (tests / controlled
     * shutdown).
     */
    static void release();

    // ===== register / unregister =====
    /**
     * Register a part
     * @param meta part meta info, full_name must be unique
     * @param creator factory translating (scalars, part params) into a Part
     * @param overwrite allow replacing an existing registration of the same full_name
     */
    void registerPart(const PartMeta& meta, Creator creator, bool overwrite = false);

    /**
     * Attach a lazily executed introspector that fills the meta on the first access.
     *
     * Building a prototype may touch the data layer (e.g. a market info lookup), which is not
     * ready while the global initializers run, so the introspection is deferred until the first
     * getMeta/create call. The introspector runs at most once; a throwing one leaves the meta as
     * it was registered.
     */
    void setIntrospector(const string& full_name, std::function<void(PartMeta&)> introspector);
    bool unregister(const string& full_name);
    /** Unregister all parts of the given module (plugins must call this before unload) */
    void unregisterByModule(const string& module_hint);
    void unregisterBySource(PartMeta::Source source);
    /** Remove all registrations (tests and resets only) */
    void clear();

    // ===== introspection =====
    bool has(const string& full_name) const;
    /** Throws when full_name is not registered */
    const PartMeta& getMeta(const string& full_name) const;
    bool tryGetMeta(const string& full_name, PartMeta& out) const;
    /** All full_names, sorted by full_name */
    StringList getNameList() const;
    StringList getNameList(PartType type) const;
    std::vector<PartMeta> getMetaList(PartType type) const;
    /** Keyword search over full_name/doc/label/category */
    std::vector<PartMeta> search(const string& keyword, std::optional<PartType> type = std::nullopt,
                                 const string& label = "") const;

    // ===== create =====
    /** Create with all params at their defaults; throws when a required param is missing */
    Part create(const string& full_name) const;
    /** Provided scalars override the defaults */
    Part create(const string& full_name, const Parameter& scalars) const;
    /** Provided scalars and part-typed params override the defaults */
    Part create(const string& full_name, const Parameter& scalars, const PartParamMap& parts) const;
    /**
     * Parse "Name(k1=v1,k2=v2)" or "part.Name(k1=v1)" and create.
     * Part-typed params are resolved recursively (e.g. "SG_Band(ind=MA(n=10),lower=10,upper=20)"),
     * up to 32 levels of nesting; list-typed part params are not supported here, use the
     * programming interface instead.
     */
    Part createFromString(const string& expr) const;

    // ===== diagnostics =====
    /** JSON dump of the metas; type == std::nullopt dumps all categories */
    string toJson(std::optional<PartType> type = std::nullopt) const;
    size_t size() const {
        return m_entries.size();
    }

private:
    PartRegistry() = default;

    /** Max nesting depth of the part-typed params in a createFromString expression */
    static constexpr size_t MAX_EXPR_NESTING_DEPTH = 32;

    struct Entry {
        PartMeta meta;
        Creator creator;
        std::function<void(PartMeta&)> introspector;  // consumed on the first meta access
    };

    /** Validate the inputs against the meta, fill defaults, then invoke the creator */
    Part createWithMeta(const PartMeta& meta, const Creator& creator, const Parameter& scalars,
                        const PartParamMap& parts) const;

    /** createFromString with the current nesting depth of the part-typed params */
    Part createFromStringImpl(const string& expr, size_t depth) const;

    /** Run the pending introspector of the given part, if any */
    void ensureMeta(const string& full_name) const;
    /** Run the pending introspectors of all parts */
    void ensureAllMeta() const;

private:
    static PartRegistry* m_registry;

    mutable std::map<string, Entry> m_entries;  // key: full_name
    std::map<PartType, StringList> m_by_type;   // full_name index by category, registered order
    mutable std::recursive_mutex m_introspect_mutex;  // serializes the lazy introspection;
                                                      // recursive so an introspector may query
                                                      // the registry for other parts
    mutable std::atomic<size_t> m_pending_introspect{0};  // number of pending introspectors
};

}  // namespace hku

#endif /* HKU_PART_REGISTRY_H_ */
