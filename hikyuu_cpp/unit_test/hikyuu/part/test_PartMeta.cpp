/*
 * test_PartMeta.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/part/PartMeta.h>

using namespace hku;

/**
 * @defgroup test_part_PartMeta test_part_PartMeta
 * @ingroup test_hikyuu_part_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ParamMeta_default") {
    /** @arg the default constructed ParamMeta is an optional scalar param */
    ParamMeta p;
    CHECK(p.name.empty());
    CHECK(p.type.empty());
    CHECK(p.default_value.empty());
    CHECK(p.desc.empty());
    CHECK_UNARY_FALSE(p.required);
    CHECK_UNARY_FALSE(p.is_ind_param);
    CHECK_UNARY_FALSE(p.is_part_param);
    CHECK_UNARY_FALSE(p.is_list);
}

/** @par Test points */
TEST_CASE("test_PartMeta_default") {
    /** @arg the default constructed PartMeta is a builtin indicator */
    PartMeta m;
    CHECK(m.type == PartType::IND);
    CHECK(m.name.empty());
    CHECK(m.full_name.empty());
    CHECK(m.params.empty());
    CHECK(m.result_num == 0);
    CHECK_UNARY_FALSE(m.support_dynamic_param);
    CHECK_UNARY_FALSE(m.support_increment);
    CHECK(m.source == PartMeta::Source::BUILTIN);
    CHECK_UNARY(m.support_batch_reuse);
    CHECK(m.module_hint.empty());
}

/** @par Test points */
TEST_CASE("test_PartMeta_source") {
    /** @arg the three sources are distinguishable */
    CHECK(PartMeta::Source::BUILTIN != PartMeta::Source::PLUGIN);
    CHECK(PartMeta::Source::PLUGIN != PartMeta::Source::PYTHON);
    CHECK(PartMeta::Source::BUILTIN != PartMeta::Source::PYTHON);
}

/** @} */
