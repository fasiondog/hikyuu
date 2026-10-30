/*
 * test_PartRegistry.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <set>
#include <hikyuu/part/PartRegistration.h>
#include <hikyuu/part/PartRegistry.h>
#include <nlohmann/json.hpp>

using namespace hku;

/**
 * @defgroup test_part_PartRegistry test_part_PartRegistry
 * @ingroup test_hikyuu_part_suite
 * @{
 */

namespace {

PartMeta makeMeta(const string& full_name, PartType type, PartMeta::Source source,
                  const string& module_hint = "") {
    PartMeta meta;
    meta.type = type;
    meta.name = full_name;
    meta.full_name = full_name;
    meta.source = source;
    meta.module_hint = module_hint;
    meta.doc = "dummy part " + full_name;
    meta.label = "test";
    return meta;
}

}  // namespace

/** @par Test points */
TEST_CASE("test_PartRegistry_instance") {
    /** @arg instance always returns the same singleton */
    CHECK(&PartRegistry::instance() == &PartRegistry::instance());

    /** @arg release() releases the singleton and the next instance() re-creates it empty */
    PartRegistry::release();
    CHECK(PartRegistry::instance().size() == 0);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_registerPart") {
    auto& reg = PartRegistry::instance();
    string name = "testhub.ind.register";
    reg.unregister(name);

    /** @arg a normal registration is visible via has/size/getMeta */
    reg.registerPart(
      makeMeta(name, PartType::IND, PartMeta::Source::BUILTIN),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });
    CHECK_UNARY(reg.has(name));

    /** @arg an empty full_name is rejected */
    CHECK_THROWS_AS(reg.registerPart(makeMeta("", PartType::IND, PartMeta::Source::BUILTIN),
                                     [](const Parameter&, const PartRegistry::PartParamMap&) {
                                         return Part(Indicator());
                                     }),
                    std::invalid_argument);

    /** @arg a null creator is rejected */
    CHECK_THROWS_AS(
      reg.registerPart(makeMeta(name, PartType::IND, PartMeta::Source::BUILTIN), nullptr),
      std::invalid_argument);

    /** @arg a duplicate registration without overwrite is rejected */
    CHECK_THROWS_AS(reg.registerPart(makeMeta(name, PartType::IND, PartMeta::Source::BUILTIN),
                                     [](const Parameter&, const PartRegistry::PartParamMap&) {
                                         return Part(Indicator());
                                     }),
                    std::invalid_argument);

    /** @arg duplicate param names in the meta are rejected */
    PartMeta dup_meta = makeMeta("testhub.ind.dupparam", PartType::IND, PartMeta::Source::BUILTIN);
    ParamMeta dup_param;
    dup_param.name = "n";
    dup_meta.params.push_back(dup_param);
    dup_meta.params.push_back(dup_param);
    CHECK_THROWS_AS(reg.registerPart(dup_meta,
                                     [](const Parameter&, const PartRegistry::PartParamMap&) {
                                         return Part(Indicator());
                                     }),
                    std::invalid_argument);
    CHECK_UNARY_FALSE(reg.has("testhub.ind.dupparam"));

    /** @arg a duplicate registration with overwrite replaces the meta */
    PartMeta replaced = makeMeta(name, PartType::SG, PartMeta::Source::BUILTIN);
    reg.registerPart(
      replaced,
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(SignalPtr()); }, true);
    CHECK(reg.getMeta(name).type == PartType::SG);

    /** @arg cleanup */
    CHECK_UNARY(reg.unregister(name));
}

/** @par Test points */
TEST_CASE("test_PartRegistry_unregister") {
    auto& reg = PartRegistry::instance();
    string name = "testhub.ind.unregister";
    reg.registerPart(
      makeMeta(name, PartType::IND, PartMeta::Source::BUILTIN),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });
    CHECK_UNARY(reg.has(name));

    /** @arg an existing registration is removed */
    CHECK_UNARY(reg.unregister(name));
    CHECK_UNARY_FALSE(reg.has(name));

    /** @arg unregistering a non-existing name returns false */
    CHECK_UNARY_FALSE(reg.unregister(name));
}

/** @par Test points */
TEST_CASE("test_PartRegistry_unregisterByModule") {
    auto& reg = PartRegistry::instance();
    string m1 = "testhub.ind.module1";
    string m2 = "testhub.ind.module2";
    reg.registerPart(
      makeMeta(m1, PartType::IND, PartMeta::Source::PLUGIN, "plugin_a"),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });
    reg.registerPart(
      makeMeta(m2, PartType::IND, PartMeta::Source::PLUGIN, "plugin_b"),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });

    /** @arg only the parts of the given module are removed */
    reg.unregisterByModule("plugin_a");
    CHECK_UNARY_FALSE(reg.has(m1));
    CHECK_UNARY(reg.has(m2));

    /** @arg an unknown module hint removes nothing */
    reg.unregisterByModule("plugin_x");
    CHECK_UNARY(reg.has(m2));

    reg.unregister(m2);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_unregisterBySource") {
    auto& reg = PartRegistry::instance();
    string py_name = "testhub.ind.bypython";
    string builtin_name = "testhub.ind.bybuiltin";
    reg.registerPart(
      makeMeta(py_name, PartType::IND, PartMeta::Source::PYTHON),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });
    reg.registerPart(
      makeMeta(builtin_name, PartType::IND, PartMeta::Source::BUILTIN),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });

    /** @arg only the parts of the given source are removed */
    reg.unregisterBySource(PartMeta::Source::PYTHON);
    CHECK_UNARY_FALSE(reg.has(py_name));
    CHECK_UNARY(reg.has(builtin_name));

    reg.unregister(builtin_name);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_getMeta") {
    auto& reg = PartRegistry::instance();
    string name = "testhub.ind.getmeta";
    reg.registerPart(
      makeMeta(name, PartType::IND, PartMeta::Source::BUILTIN),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });

    /** @arg getMeta returns the registered meta */
    const PartMeta& meta = reg.getMeta(name);
    CHECK(meta.full_name == name);
    CHECK(meta.type == PartType::IND);
    CHECK(meta.source == PartMeta::Source::BUILTIN);

    /** @arg tryGetMeta copies the meta on hit */
    PartMeta out;
    CHECK_UNARY(reg.tryGetMeta(name, out));
    CHECK(out.full_name == name);

    /** @arg getMeta throws and tryGetMeta returns false on a missing name */
    CHECK_THROWS_AS(reg.getMeta("testhub.ind.nosuch"), std::out_of_range);
    CHECK_UNARY_FALSE(reg.tryGetMeta("testhub.ind.nosuch", out));

    reg.unregister(name);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_getNameList") {
    auto& reg = PartRegistry::instance();
    string ind_name = "testhub.ind.namelist";
    string sg_name = "testhub.sg.namelist";
    reg.registerPart(
      makeMeta(ind_name, PartType::IND, PartMeta::Source::BUILTIN),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });
    reg.registerPart(
      makeMeta(sg_name, PartType::SG, PartMeta::Source::BUILTIN),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(SignalPtr()); });

    /** @arg the full list contains both names in sorted order */
    StringList all = reg.getNameList();
    auto i_ind = std::find(all.begin(), all.end(), ind_name);
    auto i_sg = std::find(all.begin(), all.end(), sg_name);
    CHECK_UNARY(i_ind != all.end());
    CHECK_UNARY(i_sg != all.end());
    CHECK(i_ind < i_sg);

    /** @arg the per-category list only holds the names of that category */
    StringList inds = reg.getNameList(PartType::IND);
    CHECK_UNARY(std::find(inds.begin(), inds.end(), ind_name) != inds.end());
    CHECK_UNARY(std::find(inds.begin(), inds.end(), sg_name) == inds.end());

    /** @arg a part is listed only in its own category */
    StringList mfs = reg.getNameList(PartType::MF);
    CHECK_UNARY(std::find(mfs.begin(), mfs.end(), ind_name) == mfs.end());

    /** @arg getMetaList mirrors getNameList with metas */
    auto metas = reg.getMetaList(PartType::SG);
    CHECK(metas.size() >= 1);
    CHECK(metas.back().full_name == sg_name);

    reg.unregister(ind_name);
    reg.unregister(sg_name);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_search") {
    auto& reg = PartRegistry::instance();
    string name = "testhub.ind.searchme";
    PartMeta meta = makeMeta(name, PartType::IND, PartMeta::Source::BUILTIN);
    meta.label = "special_label";
    meta.category = "trend";
    reg.registerPart(
      meta, [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });

    /** @arg keyword matches full_name (case insensitive) */
    CHECK(reg.search("SEARCHME").size() == 1);
    CHECK(reg.search("testhub").size() >= 1);

    /** @arg keyword matches doc/label/category */
    CHECK(reg.search("dummy part").size() >= 1);
    CHECK(reg.search("trend").size() == 1);

    /** @arg keyword matches meta.name even when the full_name does not contain it */
    PartMeta named_meta =
      makeMeta("testhub.ind.nameonly", PartType::IND, PartMeta::Source::BUILTIN);
    named_meta.name = "NameOnlyPart";
    named_meta.doc = "no keywords here";
    reg.registerPart(named_meta, [](const Parameter&, const PartRegistry::PartParamMap&) {
        return Part(Indicator());
    });
    CHECK(reg.search("NameOnlyPart").size() == 1);
    reg.unregister("testhub.ind.nameonly");

    /** @arg the type filter restricts the result */
    CHECK(reg.search("searchme", PartType::SG).empty());
    CHECK(reg.search("searchme", PartType::IND).size() == 1);

    /** @arg the label filter restricts the result */
    CHECK(reg.search("testhub", std::nullopt, "special_label").size() == 1);
    CHECK(reg.search("testhub", std::nullopt, "other_label").empty());

    /** @arg no match yields an empty list */
    CHECK_UNARY(reg.search("no_such_keyword").empty());

    reg.unregister(name);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_clear") {
    auto& reg = PartRegistry::instance();
    reg.registerPart(
      makeMeta("testhub.ind.clearme", PartType::IND, PartMeta::Source::BUILTIN),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });

    /** @arg clear removes all registrations */
    reg.clear();
    CHECK(reg.size() == 0);
    CHECK_UNARY(reg.getNameList().empty());
    CHECK_UNARY(reg.getNameList(PartType::IND).empty());

    /** @arg the registry stays usable after clear */
    reg.registerPart(
      makeMeta("testhub.ind.afterclear", PartType::IND, PartMeta::Source::BUILTIN),
      [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });
    CHECK(reg.size() == 1);
    reg.clear();
    CHECK(reg.size() == 0);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_toJson") {
    auto& reg = PartRegistry::instance();
    string name = "testhub.ind.jsonme";
    reg.unregister(name);

    PartMeta meta = makeMeta(name, PartType::IND, PartMeta::Source::BUILTIN);
    meta.doc = "doc with \"quotes\", \\backslash and a \bcontrol char";
    meta.label = "json_label";
    ParamMeta param;
    param.name = "n";
    param.type = "int";
    param.default_value = "22";
    param.desc = "the \"n\" window";
    meta.params.push_back(param);
    reg.registerPart(
      meta, [](const Parameter&, const PartRegistry::PartParamMap&) { return Part(Indicator()); });

    string json = reg.toJson();

    /** @arg the output is parseable JSON with the registered part and its params */
    nlohmann::json parsed = nlohmann::json::parse(json);
    CHECK_UNARY(parsed.is_array());
    bool found = false;
    for (const auto& item : parsed) {
        if (item.at("full_name") == name) {
            found = true;
            CHECK(item.at("type") == "ind");
            CHECK(item.at("source") == "builtin");
            CHECK(item.at("params").size() == 1);
            CHECK(item.at("params")[0].at("name") == "n");
            CHECK(item.at("params")[0].at("default") == "22");
            CHECK_UNARY(item.at("params")[0].at("is_part_param") == false);
        }
    }
    CHECK_UNARY(found);

    /** @arg quotes and backslashes in texts are escaped, not breaking the JSON structure */
    CHECK(json.find("\\\"quotes\\\"") != string::npos);
    CHECK(json.find("\\\\backslash") != string::npos);
    CHECK(json.find("the \\\"n\\\" window") != string::npos);

    /** @arg every control character is escaped as \u00XX */
    CHECK(json.find("\\u0008") != string::npos);
    CHECK(json.find('\b') == string::npos);

    /** @arg the type filter restricts the output to the given category */
    nlohmann::json sg_parsed = nlohmann::json::parse(reg.toJson(PartType::SG));
    for (const auto& item : sg_parsed) {
        CHECK(item.at("type") == "sg");
        CHECK_UNARY(item.at("full_name") != name);
    }

    reg.unregister(name);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_meta_consistency") {
    registerAllBuiltinParts();
    auto& reg = PartRegistry::instance();

    /** @arg every registered meta is self consistent */
    const std::set<string> part_type_names = {
      "Indicator",     "Signal",      "MoneyManager", "Stoploss",     "Condition",
      "Environment",   "ProfitGoal",  "Slippage",     "Portfolio",    "Selector",
      "AllocateFunds", "MultiFactor", "System",       "TradeManager", "TradeCost"};

    vector<string> problems;
    for (auto type : {PartType::IND, PartType::SG, PartType::MM, PartType::ST, PartType::PG,
                      PartType::SP, PartType::AF, PartType::CN, PartType::EV, PartType::MF,
                      PartType::PF, PartType::SE, PartType::SYS, PartType::TM, PartType::TC}) {
        for (const auto& m : reg.getMetaList(type)) {
            std::set<string> names;
            for (const auto& p : m.params) {
                if (!names.insert(p.name).second) {
                    problems.push_back(
                      fmt::format("{}: duplicate param \"{}\"", m.full_name, p.name));
                }
                if (p.required && !p.default_value.empty()) {
                    problems.push_back(fmt::format("{}: param \"{}\" is required but has a default",
                                                   m.full_name, p.name));
                }
                if (p.is_list && !p.is_part_param) {
                    problems.push_back(fmt::format("{}: param \"{}\" is a list but not part-typed",
                                                   m.full_name, p.name));
                }
                if (p.is_part_param && part_type_names.count(p.type) == 0) {
                    problems.push_back(fmt::format("{}: param \"{}\" has unknown part type \"{}\"",
                                                   m.full_name, p.name, p.type));
                }
            }
        }
    }
    string joined;
    for (const auto& problem : problems) {
        joined += problem + "\n";
    }
    INFO(joined);
    CHECK_UNARY(problems.empty());
}

/** @} */
