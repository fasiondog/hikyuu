/*
 * part_convert.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "part_convert.h"

#include <hikyuu/trade_manage/TradeCostBase.h>
#include <hikyuu/trade_manage/TradeManagerBase.h>
#include <hikyuu/trade_sys/allocatefunds/AllocateFundsBase.h>
#include <hikyuu/trade_sys/condition/ConditionBase.h>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include <hikyuu/trade_sys/moneymanager/MoneyManagerBase.h>
#include <hikyuu/trade_sys/multifactor/MultiFactorBase.h>
#include <hikyuu/trade_sys/profitgoal/ProfitGoalBase.h>
#include <hikyuu/trade_sys/selector/SelectorBase.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/slippage/SlippageBase.h>
#include <hikyuu/trade_sys/stoploss/StoplossBase.h>
#include <hikyuu/trade_sys/system/System.h>
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>

#include "hikyuu/utilities/Log.h"

namespace py = pybind11;

namespace hku {
namespace pywrap {

py::object partToPython(const Part& part) {
    return visitPart(
      part, [](const Indicator& ind) { return py::cast(ind); },
      [](const SignalPtr& p) { return py::cast(p); }, [](const MMPtr& p) { return py::cast(p); },
      [](const STPtr& p) { return py::cast(p); }, [](const CNPtr& p) { return py::cast(p); },
      [](const EVPtr& p) { return py::cast(p); }, [](const PGPtr& p) { return py::cast(p); },
      [](const SPPtr& p) { return py::cast(p); }, [](const PFPtr& p) { return py::cast(p); },
      [](const SEPtr& p) { return py::cast(p); }, [](const AFPtr& p) { return py::cast(p); },
      [](const MFPtr& p) { return py::cast(p); }, [](const SYSPtr& p) { return py::cast(p); },
      [](const TradeManagerPtr& p) { return py::cast(p); },
      [](const TradeCostPtr& p) { return py::cast(p); });
}

Part partFromPython(const py::object& obj) {
    if (py::isinstance<Indicator>(obj)) {
        return py::cast<Indicator>(obj);
    }
    if (py::isinstance<SignalBase>(obj)) {
        return py::cast<SignalPtr>(obj);
    }
    if (py::isinstance<MoneyManagerBase>(obj)) {
        return py::cast<MMPtr>(obj);
    }
    if (py::isinstance<StoplossBase>(obj)) {
        return py::cast<STPtr>(obj);
    }
    if (py::isinstance<ConditionBase>(obj)) {
        return py::cast<CNPtr>(obj);
    }
    if (py::isinstance<EnvironmentBase>(obj)) {
        return py::cast<EVPtr>(obj);
    }
    if (py::isinstance<ProfitGoalBase>(obj)) {
        return py::cast<PGPtr>(obj);
    }
    if (py::isinstance<SlippageBase>(obj)) {
        return py::cast<SPPtr>(obj);
    }
    if (py::isinstance<MultiSystem>(obj)) {
        return py::cast<PFPtr>(obj);
    }
    if (py::isinstance<SelectorBase>(obj)) {
        return py::cast<SEPtr>(obj);
    }
    if (py::isinstance<AllocateFundsBase>(obj)) {
        return py::cast<AFPtr>(obj);
    }
    if (py::isinstance<MultiFactorBase>(obj)) {
        return py::cast<MFPtr>(obj);
    }
    if (py::isinstance<System>(obj)) {
        return py::cast<SYSPtr>(obj);
    }
    if (py::isinstance<TradeManagerBase>(obj)) {
        return py::cast<TradeManagerPtr>(obj);
    }
    if (py::isinstance<TradeCostBase>(obj)) {
        return py::cast<TradeCostPtr>(obj);
    }

    HKU_THROW(
      "the python factory must return a part (Indicator, Signal, MoneyManager, ...), "
      "got {}",
      py::str(py::type::of(obj)).cast<std::string>());
}

}  // namespace pywrap
}  // namespace hku
