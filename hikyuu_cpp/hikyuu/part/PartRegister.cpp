/*
 * PartRegister.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "hikyuu/part/PartRegister.h"

#include <algorithm>
#include <fmt/format.h>

#include "hikyuu/utilities/Log.h"

namespace hku {

namespace {

/** Param types that can be expressed as text, i.e. set through a Parameter */
const StringList TEXTUAL_TYPES{"int", "int64", "double", "bool", "string"};

/** Render one introspected param as its default value string; empty means "not representable" */
string paramToString(const Parameter& param, const string& name) {
    string type = param.type(name);
    if (type == "int" || type == "int64") {
        return fmt::format("{}", param.get<int64_t>(name));
    }
    if (type == "double") {
        return fmt::format("{:g}", param.get<double>(name));
    }
    if (type == "bool") {
        return param.get<bool>(name) ? "true" : "false";
    }
    if (type == "string") {
        return param.get<string>(name);
    }
    // Datetime/Stock/Block/... are not used by the indicator params; an empty default means the
    // caller has to pass it, which is what the meta says anyway
    return string();
}

/** Apply one introspected scalar onto the prototype; unknown types are left untouched */
void applyParam(Indicator& ind, const Parameter& sc, const string& name) {
    string type = sc.type(name);
    if (type == "int" || type == "int64") {
        ind.setParam<int64_t>(name, sc.get<int64_t>(name));
    } else if (type == "double") {
        ind.setParam<double>(name, sc.get<double>(name));
    } else if (type == "bool") {
        ind.setParam<bool>(name, sc.get<bool>(name));
    } else if (type == "string") {
        ind.setParam<string>(name, sc.get<string>(name));
    }
}

}  // namespace

void registerIndicatorPart(const string& full_name, const std::function<Indicator()>& factory) {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has(full_name), void());

    PartMeta meta;
    meta.type = PartType::IND;
    meta.full_name = full_name;
    meta.name = full_name.substr(full_name.rfind('.') + 1);

    reg.registerPart(meta, [factory](const Parameter& sc, const PartParamMap&) -> Part {
        Indicator ind = factory();
        for (auto iter = sc.begin(); iter != sc.end(); ++iter) {
            applyParam(ind, sc, iter->first);
        }
        return ind;
    });

    // Building the prototype may reach into the data layer, which is not ready while the global
    // initializers run, so the introspection is deferred to the first meta access
    reg.setIntrospector(full_name, [factory](PartMeta& m) {
        Indicator proto = factory();
        HKU_IF_RETURN(!proto.getImp(), void());

        m.name = proto.name();
        m.result_num = proto.getResultNumber();

        const Parameter& param = proto.getImp()->getParameter();
        for (auto iter = param.begin(); iter != param.end(); ++iter) {
            const string& pname = iter->first;
            ParamMeta pm;
            pm.name = pname;
            pm.type = param.type(pname);
            if (pm.type == "int64") {
                // Indicator params carry int semantics; Parameter stores them as int64
                pm.type = "int";
            }
            pm.default_value = paramToString(param, pname);
            // A param whose type has no textual form (KQuery/Stock/...) can only be set in code;
            // an empty default value of a text form (e.g. a string default of "") is fine: it
            // simply falls back to the factory default
            pm.required =
              std::find(TEXTUAL_TYPES.begin(), TEXTUAL_TYPES.end(), pm.type) == TEXTUAL_TYPES.end();
            m.params.push_back(std::move(pm));
        }
    });
}

void registerIndicatorPart(const string& full_name, const PartMeta& meta,
                           const PartRegistry::Creator& creator) {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has(full_name), void());

    PartMeta m(meta);
    m.type = PartType::IND;
    m.full_name = full_name;
    m.name = full_name.substr(full_name.rfind('.') + 1);
    reg.registerPart(m, creator);
}

}  // namespace hku
