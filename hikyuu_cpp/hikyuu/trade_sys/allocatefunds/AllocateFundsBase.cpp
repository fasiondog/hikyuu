/*
 * AllocateMoney.cpp
 *
 *  Created on: 2018年1月30日
 *      Author: fasiondog
 */

#include <unordered_set>
#include <functional>
#include "AllocateFundsBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const AllocateFundsBase& af) {
    os << "AllocateFunds(" << af.name() << ", " << af.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const AFPtr& af) {
    if (af) {
        os << *af;
    } else {
        os << "AllocateFunds(NULL)";
    }

    return os;
}

AllocateFundsBase::AllocateFundsBase() : m_name("AllocateMoneyBase"), m_reserve_percent(0) {
    // 是否调整之前已经持仓策略的持仓。不调整时，仅使用总账户当前剩余资金进行分配，否则将使用总市值进行分配
    // 注意：无论是否调整已持仓策略，权重比例都是相对于总资产，不是针对剩余现金余额
    //  adjust_running_sys: True - 主动根据资产分配对已持仓策略进行增减仓
    //  adjust_running_sys: False - 不会根据当前分配权重对已持仓策略进行强制加减仓
    setParam<bool>("adjust_running_sys", false);
    setParam<int>("max_sys_num", 1000000);             // 允许运行的最大系统实例数
    setParam<double>("weight_unit", 0.0001);           // 最小权重单位
    setParam<double>("default_reserve_percent", 0.0);  // 默认保留不参与重分配的资产比例
    setParam<bool>("trace", false);                    // 打印跟踪
}

AllocateFundsBase::AllocateFundsBase(const string& name)
: m_name("AllocateMoneyBase"), m_reserve_percent(0) {
    setParam<bool>("adjust_running_sys", false);
    setParam<int>("max_sys_num", 100000);              // 最大系统实例数
    setParam<double>("weight_unit", 0.0001);           // 最小权重单位
    setParam<double>("default_reserve_percent", 0.0);  // 默认保留不参与重分配的资产比例
    setParam<bool>("trace", false);                    // 打印跟踪
}

AllocateFundsBase::~AllocateFundsBase() {}

void AllocateFundsBase::reset() {
    // 参数检查
    HKU_CHECK(getParam<int>("max_sys_num") > 0, R"(AF param["max_sys_num"]({}) need > 0!)",
              getParam<int>("max_sys_num"));
    HKU_CHECK(getParam<double>("weight_unit") > 0.0, R"(AF param[{}]) need > 0!)",
              getParam<double>("weight_unit"));

    double default_reserve_percent = getParam<double>("default_reserve_percent");
    HKU_CHECK(default_reserve_percent >= 0.0 && default_reserve_percent < 1.0,
              R"(AF param(default_reserve_percent)({}) >= 1.0, No asset adjustments will be made!)",
              default_reserve_percent);

    m_reserve_percent = default_reserve_percent;
    _reset();
}

AFPtr AllocateFundsBase::clone() {
    AFPtr p;
    try {
        p = _clone();
    } catch (...) {
        HKU_ERROR("Subclass _clone failed!");
        p = AFPtr();
    }

    if (!p || p.get() == this) {
        HKU_WARN("Failed clone! Will use self-ptr!");
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_query = m_query;
    p->m_reserve_percent = m_reserve_percent;

    /* m_tm, m_shadow_tm 由 PF 运行时指定，不需要 clone
    if (m_tm)
        p->m_tm = m_tm->clone();
    if (m_shadow_tm)
        p->m_shadow_tm = m_shadow_tm->clone();*/
    return p;
}

void AllocateFundsBase::setReservePercent(double percent) {
    HKU_CHECK_THROW(percent >= 0.0 && percent <= 1.0, std::out_of_range,
                    "percent ({}) is out of range [0, 1]!");
    m_reserve_percent = percent;
}

void AllocateFundsBase::_check_weight(const SystemWeightList& sw_list) {
    price_t sum_weight = 0.0;
    for (const auto& sw : sw_list) {
        HKU_CHECK(!std::isnan(sw.weight) && sw.weight >= 0.0 && sw.weight <= 1.0,
                  "Invalid weight ({}, {})", sw.sys->name(), sw.weight);
        sum_weight += sw.weight;
    }
    HKU_CHECK(sum_weight <= 1.001, "The cumulative weight exceeds 1! sum_weight: {}", sum_weight);
}

SystemWeightList AllocateFundsBase::adjustFunds(const Datetime& date,
                                                const SystemWeightList& se_list,
                                                const std::unordered_set<SYSPtr>& running_list) {
    SystemWeightList result;
    if (getParam<bool>("adjust_running_sys")) {
        result = _adjust_with_running(date, se_list, running_list);
    } else {
        _adjust_without_running(date, se_list, running_list);
    }
    return result;
}

void AllocateFundsBase::_adjust_without_running(const Datetime& date,
                                                const SystemWeightList& se_list,
                                                const std::unordered_set<SYSPtr>& running_set) {
    HKU_IF_RETURN(se_list.size() == 0, void());

    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "[AF] {} _adjust_without_running", date);

    // 计算不包含运行中系统的子系统列表
    SystemWeightList pure_se_list;
    for (auto& sw : se_list) {
        if (running_set.find(sw.sys) == running_set.end()) {
            pure_se_list.push_back(sw);
        }
    }

    // 获取当前总资产市值，计算需保留的资产与剩余可分配权重
    int precision = m_shadow_tm->getParam<int>("precision");
    FundsRecord funds = m_tm->getFunds(date, m_query.kType());
    price_t total_funds =
      funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;
    HKU_ERROR_IF(m_reserve_percent < 0.0 || m_reserve_percent >= 1.0,
                 "Invalid reserve_percent({}) in AF, Calculations that will cause errors!",
                 m_reserve_percent);
    price_t reserve_funds = roundDown(total_funds * m_reserve_percent, precision);
    double can_allocate_weight = funds.cash / total_funds;
    HKU_INFO_IF("can_allocate_weight: {:<.4f}", can_allocate_weight);
    HKU_IF_RETURN(can_allocate_weight <= 0., void());

    // 获取计划分配的资产权重
    SystemWeightList sw_list =
      _allocateWeight(date, pure_se_list, running_set.size(), can_allocate_weight);
    HKU_IF_RETURN(sw_list.size() == 0, void());
    if (trace) {
        _check_weight(sw_list);
    }

    // 按权重升序排序（注意：无法保证等权重的相对顺序，即使用stable_sort也一样，后面要倒序遍历）
    std::sort(
      sw_list.begin(), sw_list.end(),
      std::bind(std::less<double>(), std::bind(&SystemWeight::weight, std::placeholders::_1),
                std::bind(&SystemWeight::weight, std::placeholders::_2)));

    // 检测是否有信号发生，过滤掉：
    //  1. 没有发生信号的系统
    //  2. 权重小于等于 0 的系统
    //  2. 累积权重>可分配的总权重之后的系统
    price_t can_allocate_sum_weight = 1.0 - m_reserve_percent;
    price_t sum_weight = 0.0;
    SystemWeightList new_sw_list;
    for (auto sw_iter = sw_list.rbegin(); sw_iter != sw_list.rend(); ++sw_iter) {
        // 如果当前系统权重小于等于0 或者 累积权重以及大于等于1 终止循环
        if (sw_iter->weight <= 0.0 || sum_weight >= can_allocate_sum_weight) {
            break;
        }

        if (sw_iter->sys->getSG()->shouldBuy(date)) {
            sum_weight += sw_iter->weight;
            // 如果累积权重大于1，则调整最后的系统权重
            if (sum_weight > can_allocate_sum_weight) {
                sw_iter->weight = sum_weight - can_allocate_sum_weight;
                sum_weight = can_allocate_sum_weight;
            }
            new_sw_list.emplace_back(*sw_iter);
        }
    }

    if (trace) {
        for (auto iter = new_sw_list.begin(); iter != new_sw_list.end(); ++iter) {
            HKU_INFO("[AF] ({}, {}, weight: {:<.4f}) ", iter->sys->name(),
                     iter->sys->getStock().market_code(), iter->weight);
        }
    }

    // 计算可用于分配的现金, 小于等于需保留的资产，则直接返回
    price_t current_cash = m_shadow_tm->cash(date, m_query.kType());
    price_t can_allocate_cash = roundDown(current_cash - reserve_funds, precision);
    if (can_allocate_cash <= 0.0) {
        HKU_WARN_IF(
          trace,
          "Insufficient available cash! can_allocate_cash: {}, current_cash: {}, reserve_funds: {}",
          can_allocate_cash, current_cash, reserve_funds);
        return;
    }

    // 根据账户精度微调总资产，尽可能消除由于四舍五入后导致的精度问题
    total_funds = total_funds - std::pow(0.1, precision) * 0.5 * new_sw_list.size();

    // 再次遍历选中子系统列表，并将剩余现金按权重比例转入子账户
    int max_num = getParam<int>("max_sys_num");
    size_t can_run_count = 0;
    for (auto iter = new_sw_list.begin(), end_iter = new_sw_list.end(); iter != end_iter; ++iter) {
        // 该系统期望分配的资金
        price_t will_cash = roundUp(total_funds * iter->weight, precision);
        if (will_cash <= 0.0) {
            continue;
        }

        // 计算子账户实际可获取的的资金
        price_t need_cash = will_cash <= can_allocate_cash ? will_cash : can_allocate_cash;

        // 尝试从总账户中取出资金存入子账户
        TMPtr sub_tm = iter->sys->getTM();
        if (m_shadow_tm->checkout(date, need_cash)) {
            sub_tm->checkin(date, need_cash);
            HKU_INFO_IF(trace, "[AF] {} fetched cash: {}", iter->sys->name(), need_cash);

            // 如果超出允许运行的最大系统数，跳出循环
            can_run_count++;

            // 计算剩余的可用于分配的资金
            can_allocate_cash = roundDown(can_allocate_cash - need_cash, precision);

        } else {
            HKU_DEBUG_IF(trace, "[AF] {} failed to fetch cash from total account!",
                         iter->sys->name());
        }

        if (can_run_count >= max_num) {
            break;
        }

        // (不需要小于0，小于一个特定的值即可，这里用1)
        if (can_allocate_cash <= 1.0) {
            break;
        }
    }
}

SystemWeightList AllocateFundsBase::_adjust_with_running(
  const Datetime& date, const SystemWeightList& se_list,
  const std::unordered_set<SYSPtr>& running_set) {
    SystemWeightList result;
    HKU_IF_RETURN(se_list.size() == 0, result);

    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "[AF] {} _adjust_without_running", date);

    // 剩余可用于分配的权重
    HKU_ERROR_IF(m_reserve_percent < 0.0 || m_reserve_percent >= 1.0,
                 "Invalid reserve_percent({}) in AF, Calculations that will cause errors!",
                 m_reserve_percent);
    double can_allocate_weith = 1.0 - m_reserve_percent;

    // 获取计划分配的资产权重
    SystemWeightList sw_list =
      _allocateWeight(date, se_list, running_set.size(), can_allocate_weith);
    HKU_IF_RETURN(sw_list.size() == 0, result);
    if (trace) {
        _check_weight(sw_list);
    }

    // 按权重升序排序（注意：无法保证等权重的相对顺序，即使用stable_sort也一样，后面要倒序遍历）
    std::sort(
      sw_list.begin(), sw_list.end(),
      std::bind(std::less<double>(), std::bind(&SystemWeight::weight, std::placeholders::_1),
                std::bind(&SystemWeight::weight, std::placeholders::_2)));

    // 获取当前总资产市值，计算需保留的资产
    int precision = m_shadow_tm->getParam<int>("precision");
    FundsRecord funds = m_tm->getFunds(date, m_query.kType());
    price_t total_funds =
      funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;
    price_t reserve_funds = roundDown(total_funds * m_reserve_percent, precision);

    // 检测是否有信号发生，过滤掉：
    //  1. 不在当前运行系统集合中且没有发生信号的系统
    //  2. 权重小于等于 0 的系统，如果是运行中的系统，则强制清仓
    //  2. 累积权重>可分配的总权重之后的系统
    price_t can_allocate_sum_weight = 1.0 - m_reserve_percent;
    price_t sum_weight = 0.0;
    SystemWeightList new_sw_list;
    std::unordered_set<System*> new_sw_set;
    for (auto sw_iter = sw_list.rbegin(); sw_iter != sw_list.rend(); ++sw_iter) {
        // 如果当前系统权重小于等于0 或者 累积权重以及大于等于1 终止循环
        if (sw_iter->weight <= 0.0 || sum_weight >= can_allocate_sum_weight) {
            break;
        }

        // 如果系统在运行中，或者有信号发生
        if (running_set.count(sw_iter->sys) > 0 || sw_iter->sys->getSG()->shouldBuy(date)) {
            sum_weight += sw_iter->weight;
            // 如果累积权重大于1，则调整最后的系统权重
            if (sum_weight > can_allocate_sum_weight) {
                sw_iter->weight = sum_weight - can_allocate_sum_weight;
                sum_weight = can_allocate_sum_weight;
            }
            new_sw_list.emplace_back(*sw_iter);
            new_sw_set.insert(sw_iter->sys.get());
        }
    }

    // 如果调整运行中系统持仓，则：
    // 对已不在新的实际分配权重的系统集合中的运行中的系统，进行清仓操作
    for (auto& running_sys : running_set) {
        if (new_sw_set.count(running_sys.get()) == 0) {
            // 如果是延迟买入的系统，由于可能是在当天开盘刚刚买入，此时不能立刻清仓卖出,放入延迟卖出清单中
            if (running_sys->getParam<bool>("buy_delay")) {
                result.emplace_back(running_sys, MAX_DOUBLE);
            } else {
                auto tr = running_sys->sellForce(date, MAX_DOUBLE, PART_ALLOCATEFUNDS);
                if (!tr.isNull()) {
                    m_tm->addTradeRecord(tr);
                    auto sub_tm = running_sys->getTM();
                    auto sub_cash = sub_tm->currentCash();
                    if (sub_tm->checkout(date, sub_cash)) {
                        m_shadow_tm->checkin(date, sub_cash);
                    }
                }
            }
        }
    }

    if (trace) {
        for (auto iter = sw_list.begin(); iter != sw_list.end(); ++iter) {
            HKU_INFO("[AF] ({}, {}, weight: {:<.4f}) ", iter->sys->name(),
                     iter->sys->getStock().market_code(), iter->weight);
        }
    }

    // 计算可用于分配的现金, 小于等于需保留的资产，则直接返回
    price_t current_cash = m_shadow_tm->cash(date, m_query.kType());
    price_t can_allocate_cash = roundDown(current_cash - reserve_funds, precision);
    if (can_allocate_cash <= 0.0) {
        HKU_WARN_IF(
          trace,
          "Insufficient available cash! can_allocate_cash: {}, current_cash: {}, reserve_funds: {}",
          can_allocate_cash, current_cash, reserve_funds);
        return result;
    }

    // 根据账户精度微调总资产，尽可能消除由于四舍五入后导致的精度问题
    total_funds = total_funds - std::pow(0.1, precision) * 0.5 * new_sw_list.size();

    // 再次遍历选中子系统列表，并将剩余现金按权重比例转入子账户
    int max_num = getParam<int>("max_sys_num");
    size_t can_run_count = 0;
    for (auto iter = new_sw_list.begin(), end_iter = new_sw_list.end(); iter != end_iter; ++iter) {
        // 系统预期的分配资产额
        price_t will_funds = roundUp(total_funds * iter->weight, precision);

        // 如果该系统是当前运行中系统
        if (running_set.count(iter->sys) != 0) {
            can_run_count++;

            TMPtr sub_tm = iter->sys->getTM();
            const KQuery& query = iter->sys->getTO().getQuery();
            FundsRecord funds = sub_tm->getFunds(query.kType());
            price_t current_funds =
              funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;

            // 如果预期资产和当前资产相等，则转到下个系统
            if (current_funds == will_funds) {
                continue;
            }

            // 如果预期资产和当前资产之差不够买入或卖出最小交易数量的证券，则转至下一系统
            Stock stock = iter->sys->getStock();
            auto krecord = stock.getKRecord(date, query.kType());
            double min_num = stock.minTradeNumber();
            if (std::abs(current_funds < will_funds) < krecord.closePrice * min_num) {
                continue;
            }

            if (current_funds < will_funds) {
                // 已有资产小于预期资产，则分配新的资金
                price_t diff_price = roundUp(will_funds - current_funds, precision);
                if (can_allocate_cash > 0. && m_shadow_tm->checkout(date, diff_price)) {
                    sub_tm->checkin(date, diff_price);
                    can_allocate_cash = roundDown(can_allocate_cash - diff_price, precision);
                }

            } else {
                // 已有资产大于预期资产，尝试减仓
                price_t need_back_return = current_funds - will_funds;
                double need_back_num = need_back_return / krecord.closePrice;
                if (min_num > 1) {
                    need_back_num = static_cast<int64_t>(need_back_num / min_num) * min_num;
                }

                // 如果是延迟买入的系统，则将减仓卖出记录加入返回的延迟清单
                if (iter->sys->getParam<bool>("buy_delay")) {
                    result.emplace_back(iter->sys, need_back_num);
                    can_allocate_cash =
                      roundDown(can_allocate_cash - need_back_num * krecord.closePrice, precision);

                } else {
                    auto tr = iter->sys->sellForce(date, need_back_num, PART_ALLOCATEFUNDS);
                    if (!tr.isNull()) {
                        m_tm->addTradeRecord(tr);
                        auto sub_cash = sub_tm->currentCash();
                        if (sub_tm->checkout(date, sub_cash)) {
                            m_shadow_tm->checkin(date, sub_cash);
                            can_allocate_cash = roundDown(can_allocate_cash - sub_cash, precision);
                        }
                    }
                }
            }

        } else {
            // 非运行中的系统

            // 计算子账户实际可获取的的资金
            price_t need_cash = will_funds <= can_allocate_cash ? will_funds : can_allocate_cash;

            // 尝试从总账户中取出资金存入子账户
            TMPtr sub_tm = iter->sys->getTM();
            if (m_shadow_tm->checkout(date, need_cash)) {
                sub_tm->checkin(date, need_cash);
                HKU_INFO_IF(trace, "[AF] {} fetched cash: {}", iter->sys->name(), need_cash);

                // 如果超出允许运行的最大系统数，跳出循环
                can_run_count++;

                // 计算剩余的可用于分配的资金，如果小于1，退出循环
                // (不需要小于0，小于一个特定的值即可，这里用1)
                can_allocate_cash = roundUp(can_allocate_cash - need_cash, precision);

            } else {
                HKU_DEBUG_IF(trace, "[AF] {} failed to fetch cash from total account!",
                             iter->sys->name());
            }
        }

        if (can_run_count >= max_num) {
            break;
        }

        if (can_allocate_cash <= 1.0) {
            break;
        }
    }

    return result;
}

} /* namespace hku */
