/*
 * AllocateMoney.cpp
 *
 *  Created on: 2018-1-30
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

AllocateFundsBase::AllocateFundsBase() : m_name("AllocateMoneyBase") {
    initParam();
}

AllocateFundsBase::AllocateFundsBase(const string& name) : m_name("AllocateMoneyBase") {
    initParam();
}

AllocateFundsBase::~AllocateFundsBase() {}

void AllocateFundsBase::initParam() {
    // Whether to adjust the positions of the strategies already held; when not, only the current
    // remaining funds of the total account are allocated, otherwise the total market value is
    // allocated Note: whether or not the held strategies are adjusted, the weight ratios are
    // relative to the total assets rather than to the remaining cash balance
    //       adjusting against the remaining cash ratio only is meaningless, and the allocation may
    //       not even complete a real trade due to the trade cost
    //  adjust_running_sys: True - actively increase or reduce the positions of the held strategies
    //  according to the asset allocation adjust_running_sys: False - the held strategies are not
    //  forcibly increased or reduced according to the current allocation weights
    setParam<bool>("adjust_running_sys", true);

    // Automatically adjust the weights; in this case the passed weights are regarded as the mutual
    // ratios of the securities (see the ignore_zero_weight description) otherwise the passed
    // weights are taken as the given weights without adjustment (in this case every passed weight
    // must be less than 1)
    setParam<bool>("auto_adjust_weight", true);

    // This parameter takes effect when auto_adjust_weight is used
    // Whether to filter out the 0 values (including the negative ones) and the nan values in the
    // ratio weight list returned by the subclass For example, when the subclass returns the weight
    // ratio list [6, 2, 0, 0, 0]:
    //   with the 0 values filtered out, the actually adjusted weights are Xi / sum(Xi): [6/8, 2/8]
    //   without filtering, m is the number of the non-zero elements and n is the number of all the
    //   elements, (Xi / Sum(Xi)) * (m / n):
    //           [(6/8)*(2/5), (2/8)*(2/5), 0, 0, 0]
    //          i.e. the total is divided into 5 parts and the relative ratio is kept within the 2
    //          parts only
    setParam<bool>("ignore_zero_weight", false);

    // Ignore the systems whose score is null in the selected system list,
    // Note: some SEs (such as SE_MultiFactor) may have a similar control themselves
    setParam<bool>("ignore_se_score_is_null", false);

    // Ignore the systems whose score is not greater than 0 in the selected system list
    setParam<bool>("ignore_se_score_lt_zero", false);

    setParam<double>("reserve_percent", 0.0);  // Ratio of the assets reserved from the reallocation
    setParam<bool>("trace", false);            // Print the trace
}

void AllocateFundsBase::baseCheckParam(const string& name) const {
    if ("reserve_percent" == name) {
        double reserve_percent = getParam<double>(name);
        HKU_ASSERT(reserve_percent >= 0.0 && reserve_percent < 1.0);
    }
}

void AllocateFundsBase::paramChanged() {}

void AllocateFundsBase::reset() {
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
    p->m_is_python_object = m_is_python_object;
    p->m_query = m_query;

    /* m_tm and m_cash_tm are given by PF at runtime, no clone is needed
    if (m_tm)
        p->m_tm = m_tm->clone();
    if (m_cash_tm)
        p->m_cash_tm = m_cash_tm->clone();*/
    return p;
}

SystemWeightList AllocateFundsBase::adjustFunds(const Datetime& date,
                                                const SystemWeightList& se_list,
                                                const std::unordered_set<SYSPtr>& running_list) {
    bool ignore_se_score_is_null = getParam<bool>("ignore_se_score_is_null");
    bool ignore_se_score_lt_zero = getParam<bool>("ignore_se_score_lt_zero");
    SystemWeightList filtered_se_list;
    for (auto iter = se_list.begin(); iter != se_list.end(); ++iter) {
        if (ignore_se_score_is_null && std::isnan(iter->weight)) {
            continue;
        }
        if (ignore_se_score_lt_zero && iter->weight <= 0.0) {
            continue;
        }
        filtered_se_list.emplace_back(*iter);
    }

    SystemWeightList result;
    if (getParam<bool>("adjust_running_sys")) {
        result = _adjust_with_running(date, filtered_se_list, running_list);
    } else {
        _adjust_without_running(date, filtered_se_list, running_list);
    }
    return result;
}

/*
 * Sort the SystemWeightList in the descending order
 * can_allocate_weight - the remaining total weight available for the allocation
 * auto_adjust - whether to adjust automatically by ratio, making the total weight 1
 */
void AllocateFundsBase::adjustWeight(SystemWeightList& sw_list, double can_allocate_weight,
                                     bool auto_adjust, bool ignore_zero) {
    // Sort in the descending order, keeping the nan at the end
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

    if (auto_adjust) {
        //   with the 0 values filtered out, the actually adjusted weights are Xi / sum(Xi): [6/8,
        //   2/8] without filtering, m is the number of the non-zero elements and n is the number of
        //   all the elements, (Xi / Sum(Xi)) * (m / n):
        //           [(6/8)*(2/5), (2/8)*(2/5), 0, 0, 0]
        //          i.e. the total is divided into 5 parts and the relative ratio is kept within the
        //          2 parts only
        double per_weight = ignore_zero
                              ? 1.0 / sum * can_allocate_weight
                              : (new_list.size() * can_allocate_weight) / (sum * sw_list.size());
        for (size_t i = 0, total = new_list.size(); i < total; i++) {
            new_list[i].weight = new_list[i].weight * per_weight;
        }
    }

    sw_list.swap(new_list);
}

// All the weight allocations are relative to the total assets rather than to the remaining cash
// The held strategies are not forcibly increased or reduced according to the current weights
void AllocateFundsBase::_adjust_without_running(const Datetime& date,
                                                const SystemWeightList& se_list,
                                                const std::unordered_set<SYSPtr>& running_set) {
    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "[AF] {} _adjust_without_running", date);

    // Get the planned asset allocation weights from the allocation algorithm
    SystemWeightList sw_list = _allocateWeight(date, se_list);
    HKU_IF_RETURN(sw_list.size() == 0, void());

    // Get the current total assets market value and calculate the remaining allocatable weight and
    // cash
    int precision = m_tm->getParam<int>("precision");
    FundsRecord funds =
      m_tm->getFunds(date, m_query.kType());  // The total assets come from the total account
    price_t total_funds =
      funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;
    double reserve_percent = getParam<double>("reserve_percent");
    price_t reserve_funds = total_funds * reserve_percent;
    price_t can_allocate_cash =
      m_cash_tm->currentCash();  // The allocatable funds come from the cash account
    if (can_allocate_cash + reserve_funds > total_funds) {
        can_allocate_cash = roundDown(total_funds - reserve_funds, precision);
    }
    double can_allocate_weight = 1.0 - reserve_percent;
    HKU_INFO_IF(trace,
                "can_allocate_weight: {:<.4f}, can_allocate_cash: {:<.2f}, current cash: {:<.2f}, "
                "total funds: {:<.2f}, "
                "reserved funds: {:<.2f}",
                can_allocate_weight, can_allocate_cash, funds.cash, total_funds, reserve_funds);
    HKU_IF_RETURN(can_allocate_cash <= 1.0, void());

    // Adjust the weights (accumulate and normalize them), sort them in the descending order and
    // filter out the 0 and NaN values
    adjustWeight(sw_list, can_allocate_weight, getParam<bool>("auto_adjust_weight"),
                 getParam<bool>("ignore_zero_weight"));

    // Traverse the selected subsystem list and transfer the remaining cash into the sub accounts by
    // the weight ratios
    double sum_weight =
      0.0;  // The running systems are not adjusted, so their actual ratio may differ
    for (auto iter = sw_list.begin(), end_iter = sw_list.end(); iter != end_iter; ++iter) {
        if (can_allocate_cash <= 1.0 || sum_weight >= can_allocate_weight) {
            break;
        }

        if (!iter->sys) {
            continue;
        }

        // For a running system the calculated weight is not used; the accumulated occupied weight
        // is updated with the actual assets of the subsystem Note: sub_tm must be used rather than
        // the total account m_tm (otherwise every running system would raise sum_weight to about 1)
        // kType uses the unified m_query of AF, guaranteeing the same valuation context as
        // total_funds
        if (running_set.find(iter->sys) != running_set.cend()) {
            TMPtr sub_tm = iter->sys->getTM();
            FundsRecord sub_funds = sub_tm->getFunds(date, m_query.kType());
            sum_weight += sub_funds.total_assets() / total_funds;
            continue;
        }

        // Calculate the actually available weight
        price_t current_weight = iter->weight + sum_weight > can_allocate_weight
                                   ? can_allocate_weight - sum_weight
                                   : iter->weight;

        // The funds this system expects to be allocated
        price_t will_cash = roundUp(total_funds * current_weight, precision);
        if (will_cash <= 0.0) {
            continue;
        }

        // Calculate the funds the sub account can actually get
        price_t need_cash = will_cash <= can_allocate_cash ? will_cash : can_allocate_cash;

        // Skip it when the needed funds cannot buy even one lot
        KRecord krecord =
          iter->sys->getStock().getKRecord(date, iter->sys->getTO().getQuery().kType());
        if (krecord.isValid() &&
            need_cash < (krecord.closePrice * iter->sys->getStock().minTradeNumber())) {
            continue;
        }

        // Try to withdraw the funds from the total account and deposit them into the sub account
        TMPtr sub_tm = iter->sys->getTM();
        if (m_cash_tm->checkout(date, need_cash)) {
            sub_tm->checkin(date, need_cash);
            HKU_INFO_IF(trace, "[AF] ({}, {}, weight: {:<.4f}) fetched cash: {}", iter->sys->name(),
                        iter->sys->getStock().market_code(), current_weight, need_cash);

            // Calculate the remaining funds available for the allocation
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
    SystemWeightList delay_list;

    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "[AF] {} _adjust_with_running", date);

    // When the selected list is empty everything must be liquidated, so it must not return here
    // HKU_IF_RETURN(se_list.size() == 0, delay_list);

    //-----------------------------------------------------------------
    // Recall the remaining funds of all the running systems for the reallocation
    //-----------------------------------------------------------------
    for (const auto& sys : running_set) {
        if (sys) {
            auto sub_tm = sys->getTM();
            auto sub_cash = sub_tm->currentCash();
            if (sub_cash > 0.0 && sub_tm->checkout(date, sub_cash)) {
                m_cash_tm->checkin(date, sub_cash);
                HKU_INFO_IF(trace, "[AF] Recycle cash: {:<.2f} from {}", sub_cash, sys->name());
            }
        }
    }

    // Get the planned asset allocation weights
    SystemWeightList sw_list = _allocateWeight(date, se_list);
    HKU_IF_RETURN(sw_list.size() == 0, delay_list);

    // Sort in the descending weight order
    double reserve_percent = getParam<double>("reserve_percent");
    double can_allocate_weight = 1.0 - reserve_percent;
    adjustWeight(sw_list, can_allocate_weight, getParam<bool>("auto_adjust_weight"),
                 getParam<bool>("ignore_zero_weight"));

    //-----------------------------------------------------------------
    // Force a liquidation of the running systems no longer in sw_list first, recalling the
    // allocatable funds The delayed buy systems need no distinction; whatever the type, the
    // liquidation is done immediately with the close price
    //-----------------------------------------------------------------
    std::unordered_set<SYSPtr> running_in_sw_set;
    for (const auto& sw : sw_list) {
        if (running_set.find(sw.sys) != running_set.cend()) {
            running_in_sw_set.insert(sw.sys);
        }
    }

    for (const auto& sys : running_set) {
        if (running_in_sw_set.find(sys) == running_in_sw_set.cend()) {
            PositionRecord position = sys->getTM()->getPosition(date, sys->getStock());
            if (position.takeDatetime >= date) {
                // When the buy date of the position is today, it is delayed to the open of the next
                // day
                delay_list.emplace_back(sys, position.number);
            } else {
                auto tr = sys->sellForceOnClose(date, position.number, PART_ALLOCATEFUNDS);
                if (!tr.isNull()) {
                    auto sub_tm = sys->getTM();
                    auto sub_cash = sub_tm->currentCash();
                    if (sub_tm->checkout(date, sub_cash)) {
                        m_cash_tm->checkin(date, sub_cash);
                        m_tm->addTradeRecord(tr);  // Add the trade record into the total account
                        HKU_INFO_IF(trace, "[AF] Clean position sell: {}, recycle cash: {:<.2f}",
                                    sys->name(), sub_cash);
                    }
                } else {
                    // A failed liquidation sell is also added into the delayed sell list so that it
                    // can be executed on the next trading day
                    if (position.number > 0.0) {
                        delay_list.emplace_back(sys, position.number);
                        HKU_INFO_IF(trace, "[AF] Clean delay {}", sys->name());
                    }
                }
            }
        }
    }

    //-----------------------------------------------------------------
    // For the running systems still in the selected system, reduce their positions according to
    // their weights and recall the allocatable funds
    //-----------------------------------------------------------------
    // Get the current total assets market value and calculate the assets to be reserved
    int precision = m_cash_tm->getParam<int>("precision");
    FundsRecord funds = m_tm->getFunds(date, m_query.kType());
    price_t total_funds = funds.total_assets();
    price_t reserve_funds = roundEx(total_funds * reserve_percent, precision);

    std::unordered_set<SYSPtr> reduced_running_set;  // Cache the running systems already reduced
    for (auto iter = sw_list.begin(), end_iter = sw_list.end(); iter != end_iter; ++iter) {
        if (!iter->sys) {
            continue;
        }

        // If the current system is a running system
        if (running_set.find(iter->sys) != running_set.cend()) {
            TMPtr sub_tm = iter->sys->getTM();
            const KQuery& query = iter->sys->getTO().getQuery();
            FundsRecord sub_funds = sub_tm->getFunds(date, query.kType());
            price_t sub_total_funds = sub_funds.total_assets();
            price_t sub_will_funds = total_funds * iter->weight;

            // If the position reduction needs to be executed
            if (sub_total_funds > sub_will_funds) {
                reduced_running_set.insert(
                  iter->sys);  // Cache the system whose position was reduced
                price_t need_back_funds = sub_total_funds - sub_will_funds;
                Stock stock = iter->sys->getStock();

                // Get the current last close price
                price_t last_close_price = stock.getMarketValue(date, query.kType());
                if (last_close_price <= 0.0) {
                    // The security is invalid and cannot be processed, all the assets are lost
                    HKU_WARN_IF(trace, "{} has been delisted!", iter->sys->name());
                    continue;
                }

                PositionRecord position = sub_tm->getPosition(date, stock);
                double hold_num = position.number;
                if (hold_num <= 0.0) {
                    // There is actually no position
                    continue;
                }

                // The quantity expected to be sold
                double min_num = stock.minTradeNumber();
                double need_back_num =
                  static_cast<int64_t>(need_back_funds / last_close_price / min_num) * min_num;
                if (hold_num - need_back_num < min_num) {
                    need_back_num = hold_num;
                }

                if (need_back_num == 0.0) {
                    continue;
                }

                if (position.takeDatetime >= date) {
                    // A trade bought today needs a delayed position adjustment
                    delay_list.emplace_back(iter->sys, need_back_num);
                    HKU_INFO_IF(trace, "[AF] Delay deduce position {}, need sell num: {}",
                                iter->sys->name(), need_back_num);
                } else {
                    auto tr = iter->sys->sellForceOnClose(date, need_back_num, PART_ALLOCATEFUNDS);
                    if (!tr.isNull()) {
                        auto sub_cash = sub_tm->currentCash();
                        if (sub_tm->checkout(date, sub_cash)) {
                            m_cash_tm->checkin(date, sub_cash);
                            m_tm->addTradeRecord(
                              tr);  // Add the trade record into the total account
                            HKU_INFO_IF(trace,
                                        "[AF] Deduce position {}, sell num: {}, recycle cash: {}",
                                        iter->sys->name(), need_back_num, sub_cash);
                        }
                    } else {
                        // A failed sell is also added into the delayed trade list
                        delay_list.emplace_back(iter->sys, need_back_num);
                        HKU_INFO_IF(trace, "[AF] Delay deduce position {}, need sell num: {}",
                                    iter->sys->name(), need_back_num);
                    }
                }
            }
        }
    }

    //-----------------------------------------------------------------
    // Traverse the currently selected systems and allocate the funds by the given weights
    //-----------------------------------------------------------------
    // Calculate the cash available for the allocation; return directly when it is not greater than
    // the assets to be reserved
    price_t current_cash = m_cash_tm->currentCash();
    price_t can_allocate_cash = roundDown(current_cash - reserve_funds, precision);

    HKU_INFO_IF(trace,
                "can_allocate_weight: {:<.4f}, can_allocate_cash: {:<.2f}, current cash: {:<.2f}, "
                "total funds: {:<.2f}, "
                "reserved funds: {:<.2f}",
                can_allocate_weight, can_allocate_cash, funds.cash, total_funds, reserve_funds);

    HKU_IF_RETURN(can_allocate_cash < 1.0, delay_list);

    // Traverse the selected subsystem list and adjust the assets
    price_t sum_weight = 0.0;
    for (auto iter = sw_list.begin(), end_iter = sw_list.end(); iter != end_iter; ++iter) {
        if (sum_weight >= can_allocate_weight || can_allocate_cash < 1.0) {
            break;
        }

        if (!iter->sys) {
            continue;
        }

        // The asset amount the system expects to be allocated
        price_t will_funds = roundUp(total_funds * iter->weight, precision);

        // If this system is a currently running system
        if (running_set.find(iter->sys) != running_set.cend()) {
            auto sub_tm = iter->sys->getTM();
            const KQuery& query = iter->sys->getTO().getQuery();
            FundsRecord sub_funds = sub_tm->getFunds(date, query.kType());
            price_t sub_total_funds = sub_funds.cash + sub_funds.market_value +
                                      sub_funds.borrow_asset - sub_funds.short_market_value;

            // If the position of this system has already been reduced
            if (reduced_running_set.find(iter->sys) != reduced_running_set.cend()) {
                // The remaining allocatable funds stay unchanged and the occupied weight is
                // accumulated with the actual weight
                sum_weight += sub_total_funds / total_funds;

            } else {
                // A system whose position has not been reduced needs the corresponding funds
                // allocation
                if (sub_total_funds >= will_funds) {
                    sum_weight += sub_total_funds / total_funds;

                } else {
                    price_t need_cash = will_funds - sub_total_funds;
                    if (need_cash > can_allocate_cash) {
                        need_cash = can_allocate_cash;
                    }

                    // Skip it when the expected funds cannot buy even one lot (including a
                    // delisting)
                    auto last_price = iter->sys->getStock().getMarketValue(date, query.kType());
                    if (need_cash < last_price * iter->sys->getStock().minTradeNumber()) {
                        continue;
                    }

                    if (m_cash_tm->checkout(date, need_cash)) {
                        sub_tm->checkin(date, need_cash);
                        HKU_INFO_IF(trace, "[AF] {} fetched cash: {}", iter->sys->name(),
                                    need_cash);

                        can_allocate_cash = roundDown(can_allocate_cash - need_cash, precision);
                        // Update the accumulated allocated weight
                        sum_weight += (sub_total_funds + need_cash) / total_funds;
                    }
                }
            }
        } else {
            // A system that is not running
            // Calculate the funds the sub account can actually get
            price_t need_cash = will_funds <= can_allocate_cash ? will_funds : can_allocate_cash;

            // Try to withdraw the funds from the cash account and deposit them into the sub account
            TMPtr sub_tm = iter->sys->getTM();
            if (m_cash_tm->checkout(date, need_cash)) {
                sub_tm->checkin(date, need_cash);
                HKU_INFO_IF(trace, "[AF] {} fetched cash: {}", iter->sys->name(), need_cash);

                // Update the remaining allocatable funds
                can_allocate_cash = roundDown(can_allocate_cash - need_cash, precision);

                // Update the accumulated allocated weight
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
