/*
 * PartMeta.h
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_PART_META_H_
#define HKU_PART_META_H_

#include "Part.h"

namespace hku {

/** Meta info of a single parameter */
struct HKU_API ParamMeta {
    string name;           // parameter name
    string type;           // scalar types follow Parameter::type(): "int"/"int64"/"double"/
                           // "bool"/"string"/"Datetime"/"Stock"/"Block"/"KQuery"/"KData"/
                           // "PriceList"/"DatetimeList"; part-typed params use the alternative
                           // name, e.g. "Indicator"/"System"
    string default_value;  // default value as string; empty means required
    string desc;           // from the doxygen @param comment
    bool required{false};
    bool is_ind_param{false};   // the IndParam (indicator-as-parameter) form is supported
    bool is_part_param{false};  // held by the PartParamMap instead of Parameter
    bool is_list{false};        // part-typed list param (e.g. SystemList); 0~N elements,
                                // while a non-list part param takes exactly 1 element
};

/** Meta info of a registered part */
struct HKU_API PartMeta {
    PartType type{PartType::IND};
    string name;       // "MA" / "SG_Band"
    string full_name;  // "core." + partTypeName(type) + "." + name, e.g. "core.ind.ma" /
                       // "core.sg.band"; the C++ name is kept verbatim, the type segment is
                       // lowercase; the hub name mapping is produced by the generation script
    string author;
    string version;
    string doc;
    string label;
    string category;

    vector<ParamMeta> params;  // in the real signature order

    // indicator only
    size_t result_num{0};
    bool support_dynamic_param{false};
    bool support_increment{false};

    // runtime semantics
    enum class Source { BUILTIN, PLUGIN, PYTHON } source{Source::BUILTIN};
    bool support_batch_reuse{true};  // must be false for python parts (see the design doc)
    string module_hint;              // plugin/module name, for unregister and diagnostics
};

}  // namespace hku

#endif /* HKU_PART_META_H_ */
