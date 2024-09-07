/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240907 added by fasiondog
 */

#include "hikyuu/trade_manage/Performance.h"
#include "optimize.h"

namespace hku {

static std::pair<double, SYSPtr> runSysWithOptimizeParam(
  const SYSPtr& sys, const Stock& stk, const KQuery& query, const Datetime& lastDate,
  const vector<std::pair<SystemPart, Parameter>>& params, const std::string& out_key) {
    for (const auto& combinate_param : params) {
        if (SystemPart::PART_ENVIRONMENT == combinate_param.first) {
            auto ev = sys->getEV();
            if (ev) {
                for (auto iter = combinate_param.second.begin();
                     iter != combinate_param.second.end(); ++iter) {
                    ev->setParam<boost::any>(iter->first, iter->second);
                }
            }
        } else if (SystemPart::PART_CONDITION == combinate_param.first) {
            auto cn = sys->getCN();
            if (cn) {
                for (auto iter = combinate_param.second.begin();
                     iter != combinate_param.second.end(); ++iter) {
                    cn->setParam<boost::any>(iter->first, iter->second);
                }
            }
        } else if (SystemPart::PART_SIGNAL == combinate_param.first) {
            auto sg = sys->getSG();
            if (sg) {
                for (auto iter = combinate_param.second.begin();
                     iter != combinate_param.second.end(); ++iter) {
                    sg->setParam<boost::any>(iter->first, iter->second);
                }
            }
        } else if (SystemPart::PART_STOPLOSS == combinate_param.first) {
            auto st = sys->getST();
            if (st) {
                for (auto iter = combinate_param.second.begin();
                     iter != combinate_param.second.end(); ++iter) {
                    st->setParam<boost::any>(iter->first, iter->second);
                }
            }
        } else if (SystemPart::PART_TAKEPROFIT == combinate_param.first) {
            auto tp = sys->getTP();
            if (tp) {
                for (auto iter = combinate_param.second.begin();
                     iter != combinate_param.second.end(); ++iter) {
                    tp->setParam<boost::any>(iter->first, iter->second);
                }
            }
        } else if (SystemPart::PART_MONEYMANAGER == combinate_param.first) {
            auto mm = sys->getMM();
            if (mm) {
                for (auto iter = combinate_param.second.begin();
                     iter != combinate_param.second.end(); ++iter) {
                    mm->setParam<boost::any>(iter->first, iter->second);
                }
            }
        } else if (SystemPart::PART_PROFITGOAL == combinate_param.first) {
            auto pg = sys->getPG();
            if (pg) {
                for (auto iter = combinate_param.second.begin();
                     iter != combinate_param.second.end(); ++iter) {
                    pg->setParam<boost::any>(iter->first, iter->second);
                }
            }
        } else {
            HKU_WARN("Not support part: {}", int(combinate_param.first));
        }
    }

    sys->run(stk, query);
    Performance per;
    per.statistics(sys->getTM(), lastDate);

    double val = per.get(out_key);

    return std::make_pair(val, sys);
}

SYSPtr HKU_API findOptimizeParam(const SYSPtr& sys, const Stock& stk, const KQuery& query,
                                 const OptimizeParamList& optParams, const string& sort_key) {
    HKU_ASSERT(sys && sys->getTM());
    HKU_ASSERT(!stk.isNull());
    HKU_ASSERT(!optParams.empty());

    string statistic_key = sort_key.empty() ? "帐户平均年收益率%" : sort_key;
    HKU_CHECK(Performance::exist(statistic_key),
              "Invalid sort key: {}! A statistical item does not exist!", statistic_key);

    auto date_list = StockManager::instance().getTradingCalendar(query);
    HKU_CHECK(!date_list.empty(),
              "Invalid query: {}! The statistical end date could not be determined!", query);
    Datetime last_datetime = date_list.back();

    sys->setNotSharedAll();
    sys->forceResetAll();

    ThreadPool tg;
    vector<std::future<std::pair<double, SYSPtr>>> tasks;
    vector<std::pair<SystemPart, Parameter>> sys_param;
    Parameter param;
    while (true) {
        size_t finished_count = 0;
        for (const auto& opt_param : optParams) {
            if (opt_param->type() == OptimizeParam::OPT_PARAM_INT) {
                OptimizeParamInt* p = dynamic_cast<OptimizeParamInt*>(opt_param.get());
                int value = p->getValue();
                if (value != Null<int>()) {
                    param.set<int>(p->name(), value);
                    sys_param.emplace_back(std::make_pair(opt_param->part(), std::move(param)));
                } else {
                    finished_count++;
                }
            } else if (opt_param->type() == OptimizeParam::OPT_PARAM_DOUBLE) {
                OptimizeParamDouble* p = dynamic_cast<OptimizeParamDouble*>(opt_param.get());
                double value = p->getValue();
                if (value != Null<double>()) {
                    param.set<double>(p->name(), value);
                    sys_param.emplace_back(std::make_pair(opt_param->part(), std::move(param)));
                } else {
                    finished_count++;
                }
            } else if (opt_param->type() == OptimizeParam::OPT_PARAM_BOOL) {
                OptimizeParamBool* p = dynamic_cast<OptimizeParamBool*>(opt_param.get());
                uint8_t value = p->getValue();
                if (value >= 0) {
                    param.set<>(p->name(), static_cast<bool>(value));
                    sys_param.emplace_back(std::make_pair(opt_param->part(), std::move(param)));
                } else {
                    finished_count++;
                }

            } else if (opt_param->type() == OptimizeParam::OPT_PARAM_STRING) {
                OptimizeParamString* p = dynamic_cast<OptimizeParamString*>(opt_param.get());
                string value = p->getValue();
                if (!value.empty()) {
                    param.set<string>(p->name(), value);
                    sys_param.emplace_back(std::make_pair(opt_param->part(), std::move(param)));
                } else {
                    finished_count++;
                }
            } else {
                finished_count++;
                HKU_WARN("Ignore unknown opt param type: {}", int(opt_param->type()));
                continue;
            }
        }

        if (!sys_param.empty()) {
            tasks.emplace_back(
              tg.submit([stk, query, &statistic_key, last_datetime, new_sys = sys->clone(),
                         new_params = std::move(sys_param)]() {
                  try {
                      return runSysWithOptimizeParam(new_sys, stk, query, last_datetime, new_params,
                                                     statistic_key);
                  } catch (const std::exception& e) {
                      HKU_ERROR(e.what());
                      return std::make_pair(std::numeric_limits<double>::min(), new_sys);
                  } catch (...) {
                      HKU_ERROR_UNKNOWN;
                      return std::make_pair(std::numeric_limits<double>::min(), new_sys);
                  }
              }));
        }

        if (finished_count >= optParams.size()) {
            break;
        }
    }

    vector<std::pair<double, SYSPtr>> all_result;
    for (auto& task : tasks) {
        all_result.emplace_back(task.get());
    }

    SYSPtr ret;
    if (!all_result.empty()) {
        std::sort(all_result.begin(), all_result.end(),
                  [](const std::pair<double, SYSPtr>& a, const std::pair<double, SYSPtr>& b) {
                      return a.first > b.first;
                  });
        ret = all_result.front().second;
    }

    return ret;
}

}  // namespace hku