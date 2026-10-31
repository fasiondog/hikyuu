/*
 * Part.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "hikyuu/part/Part.h"
#include "hikyuu/trade_manage/TradeCostBase.h"
#include "hikyuu/trade_manage/TradeManagerBase.h"
#include "hikyuu/trade_sys/allocatefunds/AllocateFundsBase.h"
#include "hikyuu/trade_sys/condition/ConditionBase.h"
#include "hikyuu/trade_sys/environment/EnvironmentBase.h"
#include "hikyuu/trade_sys/moneymanager/MoneyManagerBase.h"
#include "hikyuu/trade_sys/multifactor/MultiFactorBase.h"
#include "hikyuu/trade_sys/profitgoal/ProfitGoalBase.h"
#include "hikyuu/trade_sys/selector/SelectorBase.h"
#include "hikyuu/trade_sys/signal/SignalBase.h"
#include "hikyuu/trade_sys/slippage/SlippageBase.h"
#include "hikyuu/trade_sys/stoploss/StoplossBase.h"
#include "hikyuu/trade_sys/system/System.h"
#include "hikyuu/trade_sys/system/imp/MultiSystem.h"

namespace hku {

const char* partTypeName(PartType type) {
    switch (type) {
        case PartType::IND:
            return "ind";
        case PartType::SG:
            return "sg";
        case PartType::MM:
            return "mm";
        case PartType::ST:
            return "st";
        case PartType::CN:
            return "cn";
        case PartType::EV:
            return "ev";
        case PartType::PG:
            return "pg";
        case PartType::SP:
            return "sp";
        case PartType::PF:
            return "pf";
        case PartType::SE:
            return "se";
        case PartType::AF:
            return "af";
        case PartType::MF:
            return "mf";
        case PartType::SYS:
            return "sys";
        case PartType::TM:
            return "tm";
        case PartType::TC:
            return "tc";
    }
    return "unknown";
}

std::optional<PartType> parsePartType(const string& name) {
    if (name == "ind") {
        return PartType::IND;
    } else if (name == "sg") {
        return PartType::SG;
    } else if (name == "mm") {
        return PartType::MM;
    } else if (name == "st") {
        return PartType::ST;
    } else if (name == "cn") {
        return PartType::CN;
    } else if (name == "ev") {
        return PartType::EV;
    } else if (name == "pg") {
        return PartType::PG;
    } else if (name == "sp") {
        return PartType::SP;
    } else if (name == "pf") {
        return PartType::PF;
    } else if (name == "se") {
        return PartType::SE;
    } else if (name == "af") {
        return PartType::AF;
    } else if (name == "mf") {
        return PartType::MF;
    } else if (name == "sys") {
        return PartType::SYS;
    } else if (name == "tm") {
        return PartType::TM;
    } else if (name == "tc") {
        return PartType::TC;
    }
    return std::nullopt;
}

namespace {

/** Type-keyed PartType mapping; independent of the Part alternative declaration order, so
 *  adding or reordering alternatives cannot silently break the mapping */
template <typename T>
struct PartTypeOf;

template <>
struct PartTypeOf<Indicator> {
    static constexpr PartType value = PartType::IND;
};
template <>
struct PartTypeOf<SignalPtr> {
    static constexpr PartType value = PartType::SG;
};
template <>
struct PartTypeOf<MMPtr> {
    static constexpr PartType value = PartType::MM;
};
template <>
struct PartTypeOf<STPtr> {
    static constexpr PartType value = PartType::ST;
};
template <>
struct PartTypeOf<CNPtr> {
    static constexpr PartType value = PartType::CN;
};
template <>
struct PartTypeOf<EVPtr> {
    static constexpr PartType value = PartType::EV;
};
template <>
struct PartTypeOf<PGPtr> {
    static constexpr PartType value = PartType::PG;
};
template <>
struct PartTypeOf<SPPtr> {
    static constexpr PartType value = PartType::SP;
};
template <>
struct PartTypeOf<PFPtr> {
    static constexpr PartType value = PartType::PF;
};
template <>
struct PartTypeOf<SEPtr> {
    static constexpr PartType value = PartType::SE;
};
template <>
struct PartTypeOf<AFPtr> {
    static constexpr PartType value = PartType::AF;
};
template <>
struct PartTypeOf<MFPtr> {
    static constexpr PartType value = PartType::MF;
};
template <>
struct PartTypeOf<SYSPtr> {
    static constexpr PartType value = PartType::SYS;
};
template <>
struct PartTypeOf<TradeManagerPtr> {
    static constexpr PartType value = PartType::TM;
};
template <>
struct PartTypeOf<TradeCostPtr> {
    static constexpr PartType value = PartType::TC;
};

/** All alternatives must map to distinct PartType values */
template <size_t... I>
constexpr bool partTypesAreDistinct(std::index_sequence<I...>) {
    constexpr PartType types[] = {PartTypeOf<std::variant_alternative_t<I, Part>>::value...};
    for (size_t i = 0; i < sizeof...(I); ++i) {
        for (size_t j = i + 1; j < sizeof...(I); ++j) {
            if (types[i] == types[j]) {
                return false;
            }
        }
    }
    return true;
}

static_assert(partTypesAreDistinct(std::make_index_sequence<std::variant_size_v<Part>>{}),
              "Part alternatives must map to distinct PartType values");

}  // namespace

PartType partTypeOf(const Part& part) {
    return visitPart(part,
                     [](const auto& p) { return PartTypeOf<std::decay_t<decltype(p)>>::value; });
}

string partName(const Part& part) {
    return visitPart(
      part, [](const Indicator& ind) -> string { return ind.name(); },
      [](const auto& p) -> string { return p ? p->name() : string(); });
}

}  // namespace hku
