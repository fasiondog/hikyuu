/*
 * PartRegistry.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "hikyuu/GlobalInitializer.h"
#include "hikyuu/part/PartRegistry.h"

#include <algorithm>
#include <cstdio>
#include <boost/lexical_cast.hpp>
#include <fmt/format.h>

#include "hikyuu/utilities/Log.h"

namespace hku {

namespace {

/** Map a part-typed param type name to its PartType; std::nullopt skips the element check */
std::optional<PartType> partTypeFromParamType(const string& type) {
    if (type == "Indicator") {
        return PartType::IND;
    } else if (type == "Signal") {
        return PartType::SG;
    } else if (type == "MoneyManager") {
        return PartType::MM;
    } else if (type == "Stoploss") {
        return PartType::ST;
    } else if (type == "Condition") {
        return PartType::CN;
    } else if (type == "Environment") {
        return PartType::EV;
    } else if (type == "ProfitGoal") {
        return PartType::PG;
    } else if (type == "Slippage") {
        return PartType::SP;
    } else if (type == "Portfolio") {
        return PartType::PF;
    } else if (type == "Selector") {
        return PartType::SE;
    } else if (type == "AllocateFunds") {
        return PartType::AF;
    } else if (type == "MultiFactor") {
        return PartType::MF;
    } else if (type == "System") {
        return PartType::SYS;
    } else if (type == "TradeManager") {
        return PartType::TM;
    }
    return std::nullopt;
}

/** Fill the missing scalar param with its declared string default; unsupported types are left
 *  to the creator (via tryGet) */
void injectDefault(Parameter& sc, const ParamMeta& p) {
    const string& v = p.default_value;
    try {
        if (p.type == "int") {
            sc.set<int>(p.name, boost::lexical_cast<int>(v));
        } else if (p.type == "int64") {
            sc.set<int64_t>(p.name, boost::lexical_cast<int64_t>(v));
        } else if (p.type == "double") {
            sc.set<double>(p.name, boost::lexical_cast<double>(v));
        } else if (p.type == "bool") {
            sc.set<bool>(p.name, v == "true" || v == "1");
        } else if (p.type == "string") {
            sc.set<string>(p.name, v);
        } else {
            // Datetime/Stock/Block/... have no textual default to inject: the creator must fall
            // back to its own default (sc.have(...) ? ... : <default>), otherwise it reads a
            // param that was never set
            HKU_WARN(
              "The default value of the param \"{}\" (type: {}) can not be injected, "
              "handle it inside the creator",
              p.name, p.type);
        }
    } catch (const boost::bad_lexical_cast& e) {
        HKU_THROW_EXCEPTION(std::invalid_argument,
                            "Invalid default value \"{}\" for param \"{}\": {}", v, p.name,
                            e.what());
    }
}

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        return string();
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string toLower(const string& s) {
    string out(s);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return out;
}

/** Set a scalar param from a string expression, with the expression as error context */
void setFromString(Parameter& sc, const ParamMeta& p, const string& v, const string& expr) {
    try {
        if (p.type == "int") {
            sc.set<int>(p.name, boost::lexical_cast<int>(v));
        } else if (p.type == "int64") {
            sc.set<int64_t>(p.name, boost::lexical_cast<int64_t>(v));
        } else if (p.type == "double") {
            sc.set<double>(p.name, boost::lexical_cast<double>(v));
        } else if (p.type == "bool") {
            sc.set<bool>(p.name, v == "true" || v == "1");
        } else if (p.type == "string") {
            sc.set<string>(p.name, v);
        } else {
            HKU_THROW_EXCEPTION(
              std::invalid_argument,
              "Param \"{}\" of type {} is not supported in a string expression: {}", p.name, p.type,
              expr);
        }
    } catch (const boost::bad_lexical_cast&) {
        HKU_THROW_EXCEPTION(std::invalid_argument,
                            "Param \"{}\" expects type {}, invalid value \"{}\": {}", p.name,
                            p.type, v, expr);
    }
}

string escapeJson(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    out += buf;
                } else {
                    out += c;
                }
                break;
        }
    }
    return out;
}

const char* sourceName(PartMeta::Source source) {
    switch (source) {
        case PartMeta::Source::BUILTIN:
            return "builtin";
        case PartMeta::Source::PLUGIN:
            return "plugin";
        case PartMeta::Source::PYTHON:
            return "python";
    }
    return "unknown";
}

}  // namespace

PartRegistry* PartRegistry::m_registry = nullptr;

PartRegistry& PartRegistry::instance() {
    if (!m_registry) {
        m_registry = new PartRegistry();
    }
    return (*m_registry);
}

void PartRegistry::release() {
    if (m_registry) {
        delete m_registry;
        m_registry = nullptr;
    }
}

void PartRegistry::registerPart(const PartMeta& meta, Creator creator, bool overwrite) {
    HKU_CHECK_THROW(!meta.full_name.empty(), std::invalid_argument, "Empty part full_name!");

    // The registry keys on lower-case full names: normalize here so that a mixed-case caller
    // (e.g. a future dynamic registration) cannot create an entry the lookups never find
    string full_name = toLower(meta.full_name);
    HKU_CHECK_THROW(creator != nullptr, std::invalid_argument, "Null creator for part \"{}\"!",
                    full_name);

    // Meta sanity: duplicate param names would make the validation ambiguous
    for (size_t i = 0; i < meta.params.size(); ++i) {
        for (size_t j = i + 1; j < meta.params.size(); ++j) {
            HKU_CHECK_THROW(meta.params[i].name != meta.params[j].name, std::invalid_argument,
                            "Duplicate param \"{}\" of part \"{}\"", meta.params[i].name,
                            full_name);
        }
    }

    auto iter = m_entries.find(full_name);
    if (iter != m_entries.end()) {
        HKU_CHECK_THROW(overwrite, std::invalid_argument, "Part \"{}\" is already registered!",
                        full_name);
        if (iter->second.introspector) {
            // the pending introspector is dropped together with the replaced entry
            m_pending_introspect.fetch_sub(1, std::memory_order_acq_rel);
        }
        if (iter->second.meta.type != meta.type) {
            auto& names = m_by_type[iter->second.meta.type];
            names.erase(std::remove(names.begin(), names.end(), full_name), names.end());
            m_by_type[meta.type].push_back(full_name);
        }
    } else {
        m_by_type[meta.type].push_back(full_name);
    }
    PartMeta normalized(meta);
    normalized.full_name = full_name;
    m_entries[full_name] = Entry{std::move(normalized), std::move(creator), nullptr};
}

void PartRegistry::setIntrospector(const string& full_name,
                                   std::function<void(PartMeta&)> introspector) {
    string key = toLower(full_name);
    auto iter = m_entries.find(key);
    HKU_CHECK_THROW(iter != m_entries.end(), std::out_of_range, "Part not registered: {}",
                    full_name);
    iter->second.introspector = std::move(introspector);
    m_pending_introspect.fetch_add(1, std::memory_order_acq_rel);
}

void PartRegistry::ensureMeta(const string& full_name) const {
    // Fast path: the atomic pending counter keeps the common case lock free without racing with
    // the thread that consumes the introspector
    if (m_pending_introspect.load(std::memory_order_acquire) == 0) {
        return;
    }

    std::function<void(PartMeta&)> fn;
    PartMeta* target = nullptr;
    {
        std::lock_guard<std::mutex> guard(m_introspect_mutex);
        auto iter = m_entries.find(full_name);
        if (iter == m_entries.end() || !iter->second.introspector) {
            return;
        }
        fn = std::move(iter->second.introspector);
        iter->second.introspector = nullptr;
        m_pending_introspect.fetch_sub(1, std::memory_order_acq_rel);
        target = &iter->second.meta;
    }

    // A throwing introspector must not break the caller: the meta keeps what was registered
    try {
        fn(*target);
    } catch (const std::exception& e) {
        HKU_WARN("Failed to introspect the part {}: {}", full_name, e.what());
    } catch (...) {
        HKU_WARN("Failed to introspect the part {}", full_name);
    }
}

void PartRegistry::ensureAllMeta() const {
    StringList names;
    for (const auto& [full_name, entry] : m_entries) {
        if (entry.introspector) {
            names.push_back(full_name);
        }
    }
    for (const auto& name : names) {
        ensureMeta(name);
    }
}

bool PartRegistry::unregister(const string& full_name) {
    string key = toLower(full_name);
    auto iter = m_entries.find(key);
    if (iter == m_entries.end()) {
        return false;
    }
    auto& names = m_by_type[iter->second.meta.type];
    names.erase(std::remove(names.begin(), names.end(), key), names.end());
    if (iter->second.introspector) {
        // the pending introspector goes away with the entry
        m_pending_introspect.fetch_sub(1, std::memory_order_acq_rel);
    }
    m_entries.erase(iter);
    return true;
}

void PartRegistry::unregisterByModule(const string& module_hint) {
    StringList names;
    for (const auto& [full_name, entry] : m_entries) {
        if (entry.meta.module_hint == module_hint) {
            names.push_back(full_name);
        }
    }
    for (const auto& name : names) {
        unregister(name);
    }
}

void PartRegistry::unregisterBySource(PartMeta::Source source) {
    StringList names;
    for (const auto& [full_name, entry] : m_entries) {
        if (entry.meta.source == source) {
            names.push_back(full_name);
        }
    }
    for (const auto& name : names) {
        unregister(name);
    }
}

void PartRegistry::clear() {
    m_entries.clear();
    m_by_type.clear();
}

bool PartRegistry::has(const string& full_name) const {
    return m_entries.count(toLower(full_name)) != 0;
}

const PartMeta& PartRegistry::getMeta(const string& full_name) const {
    string key = toLower(full_name);
    auto iter = m_entries.find(key);
    HKU_CHECK_THROW(iter != m_entries.end(), std::out_of_range, "Part not registered: {}",
                    full_name);
    ensureMeta(key);
    return iter->second.meta;
}

bool PartRegistry::tryGetMeta(const string& full_name, PartMeta& out) const {
    string key = toLower(full_name);
    auto iter = m_entries.find(key);
    if (iter == m_entries.end()) {
        return false;
    }
    ensureMeta(key);
    out = iter->second.meta;
    return true;
}

StringList PartRegistry::getNameList() const {
    StringList names;
    names.reserve(m_entries.size());
    for (const auto& [full_name, entry] : m_entries) {
        names.push_back(full_name);
    }
    return names;
}

StringList PartRegistry::getNameList(PartType type) const {
    auto iter = m_by_type.find(type);
    return iter == m_by_type.end() ? StringList() : iter->second;
}

vector<PartMeta> PartRegistry::getMetaList(PartType type) const {
    vector<PartMeta> metas;
    auto iter = m_by_type.find(type);
    if (iter == m_by_type.end()) {
        return metas;
    }
    metas.reserve(iter->second.size());
    for (const auto& name : iter->second) {
        ensureMeta(name);
        metas.push_back(m_entries.at(name).meta);
    }
    return metas;
}

vector<PartMeta> PartRegistry::search(const string& keyword, std::optional<PartType> type,
                                      const string& label) const {
    vector<PartMeta> result;
    string lower_keyword = toLower(trim(keyword));
    for (const auto& [full_name, entry] : m_entries) {
        const PartMeta& meta = entry.meta;
        if (type && meta.type != *type) {
            continue;
        }
        if (!label.empty() && meta.label != label) {
            continue;
        }
        if (!lower_keyword.empty()) {
            string text = toLower(full_name + " " + meta.name + " " + meta.doc + " " + meta.label +
                                  " " + meta.category);
            if (text.find(lower_keyword) == string::npos) {
                continue;
            }
        }
        // only the matched parts need their meta introspected
        ensureMeta(full_name);
        result.push_back(entry.meta);
    }
    return result;
}

Part PartRegistry::create(const string& full_name) const {
    string key = toLower(full_name);
    auto iter = m_entries.find(key);
    HKU_CHECK_THROW(iter != m_entries.end(), std::out_of_range, "Part not registered: {}",
                    full_name);
    ensureMeta(key);
    return createWithMeta(iter->second.meta, iter->second.creator, Parameter(), PartParamMap());
}

Part PartRegistry::create(const string& full_name, const Parameter& scalars) const {
    string key = toLower(full_name);
    auto iter = m_entries.find(key);
    HKU_CHECK_THROW(iter != m_entries.end(), std::out_of_range, "Part not registered: {}",
                    full_name);
    ensureMeta(key);
    return createWithMeta(iter->second.meta, iter->second.creator, scalars, PartParamMap());
}

Part PartRegistry::create(const string& full_name, const Parameter& scalars,
                          const PartParamMap& parts) const {
    string key = toLower(full_name);
    auto iter = m_entries.find(key);
    HKU_CHECK_THROW(iter != m_entries.end(), std::out_of_range, "Part not registered: {}",
                    full_name);
    ensureMeta(key);
    return createWithMeta(iter->second.meta, iter->second.creator, scalars, parts);
}

Part PartRegistry::createFromString(const string& expr) const {
    // Format: "Name" | "Name(k1=v1,k2=v2)" | "part.Name(...)" | "hub.part.Name(...)"
    string src = trim(expr);
    HKU_CHECK_THROW(!src.empty(), std::invalid_argument, "Empty part expression: {}", expr);

    string name;
    string args_str;
    bool has_args = false;
    size_t lpos = src.find('(');
    if (lpos == string::npos) {
        name = src;
    } else {
        HKU_CHECK_THROW(src.back() == ')', std::invalid_argument,
                        "Unbalanced parentheses in part expression: {}", expr);
        name = trim(src.substr(0, lpos));
        args_str = src.substr(lpos + 1, src.size() - lpos - 2);
        has_args = true;
    }
    HKU_CHECK_THROW(!name.empty(), std::invalid_argument, "Empty part name in expression: {}",
                    expr);

    // Resolve the name (case insensitive): exact full_name first, then the short C++ name
    // (meta.name), then a unique ".name" suffix of the full_names; all matches must be unique
    string key = toLower(name);
    auto iter = m_entries.find(key);
    if (iter == m_entries.end()) {
        for (const auto& [full_name, entry] : m_entries) {
            if (toLower(entry.meta.name) == key) {
                HKU_CHECK_THROW(iter == m_entries.end(), std::invalid_argument,
                                "Ambiguous part name \"{}\" in expression: {}", name, expr);
                iter = m_entries.find(full_name);
            }
        }
    }
    if (iter == m_entries.end()) {
        string suffix = "." + key;
        for (const auto& [full_name, entry] : m_entries) {
            if (full_name.size() > suffix.size() &&
                full_name.compare(full_name.size() - suffix.size(), suffix.size(), suffix) == 0) {
                HKU_CHECK_THROW(iter == m_entries.end(), std::invalid_argument,
                                "Ambiguous part name \"{}\" in expression: {}", name, expr);
                iter = m_entries.find(full_name);
            }
        }
        HKU_CHECK_THROW(iter != m_entries.end(), std::out_of_range, "Part not registered: {} ({})",
                        name, expr);
    }

    ensureMeta(iter->first);
    const PartMeta& meta = iter->second.meta;
    Parameter sc;
    if (has_args && !args_str.empty()) {
        size_t start = 0;
        while (start <= args_str.size()) {
            size_t pos = args_str.find(',', start);
            string item =
              trim(args_str.substr(start, pos == string::npos ? string::npos : pos - start));
            if (!item.empty()) {
                size_t eq = item.find('=');
                HKU_CHECK_THROW(eq != string::npos, std::invalid_argument,
                                "Invalid argument \"{}\" (expected k=v) in expression: {}", item,
                                expr);
                string k = trim(item.substr(0, eq));
                string v = trim(item.substr(eq + 1));
                const ParamMeta* pm = nullptr;
                for (const auto& p : meta.params) {
                    if (p.name == k) {
                        pm = &p;
                        break;
                    }
                }
                HKU_CHECK_THROW(pm != nullptr, std::invalid_argument,
                                "Unknown param \"{}\" for part \"{}\": {}", k, meta.full_name,
                                expr);
                HKU_CHECK_THROW(!pm->is_part_param, std::invalid_argument,
                                "Part-typed param \"{}\" cannot be set in a string expression: {}",
                                k, expr);
                setFromString(sc, *pm, v, expr);
            }
            if (pos == string::npos) {
                break;
            }
            start = pos + 1;
        }
    }
    return createWithMeta(meta, iter->second.creator, sc, PartParamMap());
}

Part PartRegistry::createWithMeta(const PartMeta& meta, const Creator& creator,
                                  const Parameter& scalars, const PartParamMap& parts) const {
    Parameter sc(scalars);
    PartParamMap pp(parts);

    // Reject keys that are not declared in the meta; silently dropping them would hide caller
    // typos and yield parts with unexpected defaults
    std::map<string, const ParamMeta*> declared;
    for (const auto& p : meta.params) {
        declared.emplace(p.name, &p);
    }
    for (const auto& [key, value] : sc) {
        auto iter = declared.find(key);
        HKU_CHECK_THROW(iter != declared.end(), std::invalid_argument,
                        "Unknown param \"{}\" of part \"{}\"", key, meta.full_name);
        HKU_CHECK_THROW(!iter->second->is_part_param, std::invalid_argument,
                        "Param \"{}\" of part \"{}\" is part-typed, pass it via the PartParamMap",
                        key, meta.full_name);
    }
    for (const auto& [key, value] : pp) {
        auto iter = declared.find(key);
        HKU_CHECK_THROW(iter != declared.end(), std::invalid_argument,
                        "Unknown param \"{}\" of part \"{}\"", key, meta.full_name);
        HKU_CHECK_THROW(iter->second->is_part_param, std::invalid_argument,
                        "Param \"{}\" of part \"{}\" is a scalar param, pass it via Parameter", key,
                        meta.full_name);
    }

    for (const auto& p : meta.params) {
        if (p.is_part_param) {
            auto iter = pp.find(p.name);
            bool provided = iter != pp.end() && !iter->second.empty();
            HKU_CHECK_THROW(provided || !p.required, std::invalid_argument,
                            "Missing required param \"{}\" of part \"{}\"", p.name, meta.full_name);
            if (provided) {
                if (!p.is_list) {
                    HKU_CHECK_THROW(iter->second.size() == 1, std::invalid_argument,
                                    "Param \"{}\" of part \"{}\" takes exactly 1 element, got {}",
                                    p.name, meta.full_name, iter->second.size());
                }
                auto expected = partTypeFromParamType(p.type);
                for (const auto& elem : iter->second) {
                    PartType actual = partTypeOf(elem);
                    HKU_CHECK_THROW(!expected || actual == *expected, std::invalid_argument,
                                    "Param \"{}\" of part \"{}\" expects type {}, got {}", p.name,
                                    meta.full_name, p.type, partTypeName(actual));
                }
            }
        } else {
            if (sc.have(p.name)) {
                // Parameter stores int as int64_t internally, so "int" and "int64" are
                // equivalent here
                string actual = sc.type(p.name);
                bool type_ok = actual == p.type || (p.type == "int" && actual == "int64");
                HKU_CHECK_THROW(type_ok, std::invalid_argument,
                                "Param \"{}\" of part \"{}\" expects type {}, got {}", p.name,
                                meta.full_name, p.type, actual);
            } else if (!p.default_value.empty()) {
                injectDefault(sc, p);
            } else {
                HKU_CHECK_THROW(!p.required, std::invalid_argument,
                                "Missing required param \"{}\" of part \"{}\"", p.name,
                                meta.full_name);
            }
        }
    }
    return creator(sc, pp);
}

string PartRegistry::toJson(std::optional<PartType> type) const {
    ensureAllMeta();
    string out = "[";
    bool first = true;
    auto append = [&](const PartMeta& m) {
        if (!first) {
            out += ",";
        }
        first = false;
        out += fmt::format(
          "{{\"type\":\"{}\",\"name\":\"{}\",\"full_name\":\"{}\",\"author\":\"{}\","
          "\"version\":\"{}\",\"doc\":\"{}\",\"label\":\"{}\",\"category\":\"{}\","
          "\"source\":\"{}\",\"support_batch_reuse\":{},\"params\":[",
          partTypeName(m.type), escapeJson(m.name), escapeJson(m.full_name), escapeJson(m.author),
          escapeJson(m.version), escapeJson(m.doc), escapeJson(m.label), escapeJson(m.category),
          sourceName(m.source), m.support_batch_reuse);
        bool pfirst = true;
        for (const auto& p : m.params) {
            if (!pfirst) {
                out += ",";
            }
            pfirst = false;
            out += fmt::format(
              "{{\"name\":\"{}\",\"type\":\"{}\",\"default\":\"{}\",\"desc\":\"{}\","
              "\"required\":{},\"is_part_param\":{},\"is_list\":{}}}",
              escapeJson(p.name), escapeJson(p.type), escapeJson(p.default_value),
              escapeJson(p.desc), p.required, p.is_part_param, p.is_list);
        }
        out += "]}";
    };

    if (type) {
        for (const auto& name : getNameList(*type)) {
            append(m_entries.at(name).meta);
        }
    } else {
        for (const auto& [full_name, entry] : m_entries) {
            append(entry.meta);
        }
    }
    out += "]";
    return out;
}

}  // namespace hku
