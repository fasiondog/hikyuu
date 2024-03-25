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
    //       仅针对剩余现金比例调整没有意义，即使分配由于交易成本原因可能也无法完成实际交易
    //  adjust_running_sys: True - 主动根据资产分配对已持仓策略进行增减仓
    //  adjust_running_sys: False - 不会根据当前分配权重对已持仓策略进行强制加减仓
    setParam<bool>("adjust_running_sys", false);

    // 是否过滤子类返回的比例权重列表中的 0 值（包含小于0）和 nan 值
    // 如：子类返回权重比例列表 [6, 2, 0, 0, 0], 则
    //   过滤 0 值，则实际调整后的权重为 Xi / sum(Xi)：[6/8, 2/8]
    //   不过滤，m 设为非零元素个数，n为总元素个数，(Xi / Sum(Xi)) * (m / n)：
    //           [(6/8)*(2/5), (2/8)*(2/5), 0, 0, 0]
    //          即，保留分为5份后，仅在2份中保持相对比例
    setParam<bool>("filter_zero_weight", false);

    setParam<double>("default_reserve_percent", 0.0);  // 默认保留不参与重分配的资产比例
    setParam<bool>("trace", false);                    // 打印跟踪
}

AllocateFundsBase::AllocateFundsBase(const string& name)
: m_name("AllocateMoneyBase"), m_reserve_percent(0) {
    setParam<bool>("adjust_running_sys", false);
    setParam<bool>("filter_zero_weight", false);
    setParam<double>("default_reserve_percent", 0.0);  // 默认保留不参与重分配的资产比例
    setParam<bool>("trace", false);                    // 打印跟踪
}

AllocateFundsBase::~AllocateFundsBase() {}

void AllocateFundsBase::baseCheckParam(const string& name) const {}
void AllocateFundsBase::paramChanged() {}

void AllocateFundsBase::reset() {
    // 参数检查
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

    /* m_tm, m_cash_tm 由 PF 运行时指定，不需要 clone
    if (m_tm)
        p->m_tm = m_tm->clone();
    if (m_cash_tm)
        p->m_cash_tm = m_cash_tm->clone();*/
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

// 降序排列 SystemWeightList，并将权重调整为总权重为 1
static void adjustWeight(SystemWeightList& sw_list, double can_allocate_weight, bool filter_zero) {
    std::sort(sw_list.begin(), sw_list.end(), [](const SystemWeight& a, const SystemWeight& b) {
        if (std::isnan(a.weight) && std::isnan(b.weight)) {
            return false;
        } else if (!std::isnan(a.weight) && std::isnan(b.weight)) {
            return true;
        } else if (std::isnan(a.weight) && !std::isnan(b.weight)) {
            return false;
        }
        return a.weight > b.weight;
    });

    SystemWeightList new_list;
    new_list.reserve(sw_list.size());
    price_t sum = 0.0;
    for (size_t i = 0, total = sw_list.size(); i < total; i++) {
        const auto& item = sw_list[i];
        if (std::isnan(item.weight) || item.weight <= 0.0) {
            break;
        }
        sum += item.weight;
        new_list.emplace_back(item);
    }

    double per_weight =
      filter_zero ? 1.0 / sum : (new_list.size() * can_allocate_weight) / (sum * sw_list.size());
    for (size_t i = 0, total = new_list.size(); i < total; i++) {
        new_list[i].weight = new_list[i].weight * per_weight;
    }

    sw_list.swap(new_list);
}

// 只对现金余额进行分配，此时的权重是针对余额的
void AllocateFundsBase::_adjust_without_running(const Datetime& date,
                                                const SystemWeightList& se_list,
                                                const std::unordered_set<SYSPtr>& running_set) {
    HKU_CHECK(m_reserve_percent >= 0.0 && m_reserve_percent < 1.0,
              "Invalid reserve_percent({}) in AF, Calculations that will cause errors!",
              m_reserve_percent);

    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "[AF] {} _adjust_without_running", date);

    // 获取计划分配的资产权重，因为不调整已运行系统实现占位，将所有运行中的系统以0比例权重加在选中系统之前
    SystemWeightList new_se_list;
    for (const auto& running_sw : running_set) {
        new_se_list.emplace_back(running_sw, 0.0);
    }

    for (const auto& sw : se_list) {
        if (running_set.find(sw.sys) == running_set.end()) {
            new_se_list.emplace_back(sw);
        }
    }

    HKU_IF_RETURN(new_se_list.size() == 0, void());
    SystemWeightList sw_list = _allocateWeight(date, new_se_list);
    HKU_IF_RETURN(sw_list.size() == 0, void());

    // 获取当前总资产市值，计算剩余可分配权重与现金
    int precision = m_tm->getParam<int>("precision");
    FundsRecord funds = m_tm->getFunds(date, m_query.kType());  // 总资产从总账户获取
    price_t total_funds =
      funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;
    price_t reserve_funds = total_funds * m_reserve_percent;
    price_t can_allocate_cash = m_cash_tm->currentCash();  // 可分配资金从资金账户中获取
    if (can_allocate_cash + reserve_funds > total_funds) {
        can_allocate_cash = roundDown(total_funds - reserve_funds, precision);
    }
    double can_allocate_weight = 1.0 - m_reserve_percent;
    HKU_INFO_IF(trace,
                "can_allocate_weight: {:<.4f}, can_allocate_cash: {:<.2f}, current cash: {:<.2f}, "
                "total funds: {:<.2f}, "
                "reserved funds: {:<.2f}",
                can_allocate_weight, can_allocate_cash, funds.cash, total_funds, reserve_funds);
    HKU_IF_RETURN(can_allocate_cash <= 1.0, void());

    // 调整权重（累积权重和归一）并按降序排列, 并过滤掉 0 值和 Nan 值
    adjustWeight(sw_list, can_allocate_weight, getParam<bool>("filter_zero_weight"));

    // 遍历选中子系统列表，并将剩余现金按权重比例转入子账户
    double sum_weight = 0.0;  // 由于不调整已运行系统，已运行系统实际占用比例可能和要求的比例不一致
    for (auto iter = sw_list.begin(), end_iter = sw_list.end(); iter != end_iter; ++iter) {
        if (can_allocate_cash <= 1.0 || sum_weight >= can_allocate_weight) {
            break;
        }

        // 如果是运行中系统，不使用计算的权重，更新累积权重和
        if (running_set.find(iter->sys) != running_set.cend()) {
            FundsRecord sub_funds = m_tm->getFunds(date, m_query.kType());
            price_t sub_total_funds = sub_funds.cash + sub_funds.market_value +
                                      sub_funds.borrow_asset - sub_funds.short_market_value;
            sum_weight += sub_total_funds / total_funds;
            continue;
        }

        // 计算实际可用的权重
        price_t current_weight =
          iter->weight + sum_weight > 1.0 ? iter->weight + sum_weight - 1.0 : iter->weight;

        // 该系统期望分配的资金
        price_t will_cash = roundUp(total_funds * current_weight, precision);
        if (will_cash <= 0.0) {
            continue;
        }

        // 计算子账户实际可获取的的资金
        price_t need_cash = will_cash <= can_allocate_cash ? will_cash : can_allocate_cash;

        // 如果需要的资金连一手都买不了，直接忽略跳过
        KRecord krecord =
          iter->sys->getStock().getKRecord(date, iter->sys->getTO().getQuery().kType());
        if (krecord.isValid() &&
            need_cash < (krecord.closePrice * iter->sys->getStock().minTradeNumber())) {
            continue;
        }

        // 尝试从总账户中取出资金存入子账户
        TMPtr sub_tm = iter->sys->getTM();
        if (m_cash_tm->checkout(date, need_cash)) {
            sub_tm->checkin(date, need_cash);
            HKU_INFO_IF(trace, "[AF] ({}, {}, weight: {:<.4f}) fetched cash: {}", iter->sys->name(),
                        iter->sys->getStock().market_code(), current_weight, need_cash);

            // 计算剩余的可用于分配的资金
            can_allocate_cash = roundDown(can_allocate_cash - need_cash, precision);
            sum_weight += current_weight;

        } else {
            HKU_DEBUG_IF(trace, "[AF] {} failed to fetch cash from total account ({})!",
                         iter->sys->name(), m_cash_tm->currentCash());
        }
    }
}

SystemWeightList AllocateFundsBase::_adjust_with_running(
  const Datetime& date, const SystemWeightList& se_list,
  const std::unordered_set<SYSPtr>& running_set) {
    HKU_CHECK(m_reserve_percent >= 0.0 && m_reserve_percent < 1.0,
              "Invalid reserve_percent({}) in AF, Calculations that will cause errors!",
              m_reserve_percent);

    SystemWeightList delay_list;

    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "[AF] {} _adjust_with_running", date);
    HKU_IF_RETURN(se_list.size() == 0, delay_list);

    // 获取计划分配的资产权重
    SystemWeightList sw_list = _allocateWeight(date, se_list);
    HKU_IF_RETURN(sw_list.size() == 0, delay_list);

    // 按权重降序排列
    double can_allocate_weight = 1.0 - m_reserve_percent;
    adjustWeight(sw_list, can_allocate_weight, getParam<bool>("filter_zero_weight"));

    //-----------------------------------------------------------------
    // 先将已不在 sw_list 中的运行系统进行清仓，回收可分配资金
    //-----------------------------------------------------------------
    std::unordered_set<SYSPtr> running_in_sw_list;
    for (const auto& sw : sw_list) {
        if (running_set.find(sw.sys) != running_set.cend()) {
            running_in_sw_list.insert(sw.sys);
        }
    }

    for (const auto& sys : running_set) {
        if (running_in_sw_list.find(sys) == running_in_sw_list.cend()) {
            if (sys->getParam<bool>("buy_delay")) {
                delay_list.emplace_back(sys, MAX_DOUBLE);
            } else {
                // 非延迟卖出的系统，立即强制卖出并回收资金
                auto tr = sys->sellForceOnClose(date, MAX_DOUBLE, PART_ALLOCATEFUNDS);
                HKU_DEBUG_IF(trace && tr.isNull(), "[AF] failed to sell: {}", sys->name());
                if (!tr.isNull()) {
                    auto sub_tm = sys->getTM();
                    auto sub_cash = sub_tm->currentCash();
                    if (sub_tm->checkout(date, sub_cash)) {
                        m_cash_tm->checkin(date, sub_cash);
                        m_tm->addTradeRecord(tr);  // 向总账户加入交易记录
                        HKU_INFO_IF(trace, "[AF] Adjust sell: {}, recycle cash: {:<.2f}",
                                    sys->name(), sub_cash);
                    }
                }
            }
        }
    }

    //-----------------------------------------------------------------
    // 对于仍在选中系统中的运行系统，根据其权重进行减仓处理，回收可分配资金
    //-----------------------------------------------------------------
    // 获取当前总资产市值，计算需保留的资产
    int precision = m_cash_tm->getParam<int>("precision");
    FundsRecord funds = m_tm->getFunds(date, m_query.kType());
    price_t total_funds =
      funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;
    price_t reserve_funds = roundEx(total_funds * m_reserve_percent, precision);

    std::unordered_set<SYSPtr> reduced_running_set;  // 缓存已执行过减仓的运行中系统
    for (auto iter = sw_list.begin(), end_iter = sw_list.end(); iter != end_iter; ++iter) {
        // 如果当前系统是运行中的系统
        if (running_set.find(iter->sys) != running_set.cend()) {
            TMPtr sub_tm = iter->sys->getTM();
            const KQuery& query = iter->sys->getTO().getQuery();
            FundsRecord sub_funds = sub_tm->getFunds(date, query.kType());
            price_t sub_total_funds = sub_funds.cash + sub_funds.market_value +
                                      sub_funds.borrow_asset - sub_funds.short_market_value;
            price_t sub_will_funds = total_funds * iter->weight;
            if (sub_total_funds > sub_will_funds) {
                reduced_running_set.insert(iter->sys);  // 缓存执行了减仓的系统
                price_t need_back_funds = sub_total_funds - sub_will_funds;
                Stock stock = iter->sys->getStock();
                KRecord krecord = stock.getKRecord(date, query.kType());
                size_t need_back_shou =
                  size_t(need_back_funds / krecord.closePrice / stock.minTradeNumber());
                if (need_back_shou > 0) {
                    size_t need_back_num = need_back_shou * stock.minTradeNumber();
                    size_t hold_num = sub_tm->getHoldNumber(date, stock);
                    if (hold_num - need_back_num < stock.minTradeNumber()) {
                        need_back_num = hold_num;
                    }
                    if (iter->sys->getParam<bool>("buy_delay")) {
                        delay_list.emplace_back(iter->sys, need_back_num);
                    } else {
                        auto tr =
                          iter->sys->sellForceOnClose(date, need_back_num, PART_ALLOCATEFUNDS);
                        if (!tr.isNull()) {
                            auto sub_tm = iter->sys->getTM();
                            auto sub_cash = sub_tm->currentCash();
                            if (sub_tm->checkout(date, sub_cash)) {
                                m_cash_tm->checkin(date, sub_cash);
                                m_tm->addTradeRecord(tr);  // 向总账户加入交易记录
                            }
                        }
                    }
                }
            }
        }
    }

    //-----------------------------------------------------------------
    // 遍历当前选中系统，按指定权重分配资金
    //-----------------------------------------------------------------
    // 计算可用于分配的现金, 小于等于需保留的资产，则直接返回
    price_t current_cash = m_cash_tm->currentCash();
    price_t can_allocate_cash = roundDown(current_cash - reserve_funds, precision);

    HKU_INFO_IF(trace,
                "can_allocate_weight: {:<.4f}, can_allocate_cash: {:<.2f}, current cash: {:<.2f}, "
                "total funds: {:<.2f}, "
                "reserved funds: {:<.2f}",
                can_allocate_weight, can_allocate_cash, funds.cash, total_funds, reserve_funds);

    HKU_IF_RETURN(can_allocate_cash < 1.0, delay_list);

    // 遍历选中子系统列表，并调整资产
    price_t sum_weight = 0.0;
    for (auto iter = sw_list.begin(), end_iter = sw_list.end(); iter != end_iter; ++iter) {
        if (sum_weight >= can_allocate_weight || can_allocate_cash < 1.0) {
            break;
        }

        // 计算实际可用的权重
        price_t current_weight = iter->weight + sum_weight > can_allocate_weight
                                   ? iter->weight + sum_weight - can_allocate_weight
                                   : iter->weight;

        // 系统期望分配的资产额
        price_t will_funds = roundUp(total_funds * current_weight, precision);

        // 如果该系统是当前运行中系统
        if (running_set.find(iter->sys) != running_set.cend()) {
            auto sub_tm = iter->sys->getTM();
            const KQuery& query = iter->sys->getTO().getQuery();
            FundsRecord sub_funds = sub_tm->getFunds(date, query.kType());
            price_t sub_total_funds = sub_funds.cash + sub_funds.market_value +
                                      sub_funds.borrow_asset - sub_funds.short_market_value;

            // 如果是已经执行过减仓的系统
            if (reduced_running_set.find(iter->sys) != reduced_running_set.cend()) {
                // 剩余可分配资金不变，已占用权重按实际权重累积
                sum_weight += sub_total_funds / total_funds;

            } else {
                // 未执行过减仓的系统，需要予以相应资金分配
                if (sub_total_funds >= will_funds) {
                    sum_weight += sub_total_funds / total_funds;
                } else {
                    price_t need_cash = will_funds - sub_total_funds;
                    if (need_cash > can_allocate_cash) {
                        need_cash = can_allocate_cash;
                    }
                    // 如果期望的资金连一手都买不起，则跳过
                    const KQuery& query = iter->sys->getTO().getQuery();
                    auto krecord = iter->sys->getStock().getKRecord(date, query.kType());
                    if (krecord.isValid() &&
                        need_cash < krecord.closePrice * iter->sys->getStock().minTradeNumber()) {
                        continue;
                    }

                    if (m_cash_tm->checkout(date, need_cash)) {
                        sub_tm->checkin(date, need_cash);
                        can_allocate_cash = roundDown(can_allocate_cash - need_cash, precision);
                        // 更新已分配的累积权重
                        sum_weight += (sub_total_funds + need_cash) / total_funds;
                    }
                }
            }

        } else {
            // 非运行中的系统
            // 计算子账户实际可获取的的资金
            price_t need_cash = will_funds <= can_allocate_cash ? will_funds : can_allocate_cash;

            // 如果期望的资金连一手都买不起，则跳过
            const KQuery& query = iter->sys->getTO().getQuery();
            auto krecord = iter->sys->getStock().getKRecord(date, query.kType());
            if (krecord.isValid() &&
                need_cash < krecord.closePrice * iter->sys->getStock().minTradeNumber()) {
                continue;
            }

            // 尝试从资金账户中取出资金存入子账户
            TMPtr sub_tm = iter->sys->getTM();
            if (m_cash_tm->checkout(date, need_cash)) {
                sub_tm->checkin(date, need_cash);
                HKU_INFO_IF(trace, "[AF] {} fetched cash: {}", iter->sys->name(), need_cash);

                // 更新剩余可分配资金
                can_allocate_cash = roundDown(can_allocate_cash - need_cash, precision);

                // 更新已分配的累积权重
                sum_weight += iter->weight;

            } else {
                HKU_DEBUG_IF(trace, "[AF] {} failed to fetch cash from total account!",
                             iter->sys->name());
            }
        }
    }

    return delay_list;
}

} /* namespace hku */
