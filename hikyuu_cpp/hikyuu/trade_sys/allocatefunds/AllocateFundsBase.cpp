/*
 * AllocateMoney.cpp
 *
 *  Created on: 2018年1月30日
 *      Author: fasiondog
 */

#include <boost/bind.hpp>
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

AllocateFundsBase::AllocateFundsBase()
: m_name("AllocateMoneyBase"), m_count(0), m_pre_date(Datetime::min()), m_reserve_percent(0) {
    //是否调整之前已经持仓策略的持仓。不调整时，仅使用总账户当前剩余资金进行分配，否则将使用总市值进行分配
    setParam<bool>("adjust_running_sys", false);
    setParam<int>("max_sys_num", 100000);  //最大系统实例数
}

AllocateFundsBase::AllocateFundsBase(const string& name)
: m_name("AllocateMoneyBase"), m_count(0), m_pre_date(Datetime::min()), m_reserve_percent(0) {
    setParam<bool>("adjust_running_sys", false);
    setParam<int>("max_sys_num", 100000);  //最大系统实例数
}

AllocateFundsBase::~AllocateFundsBase() {}

void AllocateFundsBase::reset() {
    m_count = 0;
    m_pre_date = Datetime::min();
    m_reserve_percent = 0;
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
        HKU_ERROR("Failed clone! Will use self-ptr!");
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_query = m_query;
    p->m_count = m_count;
    p->m_pre_date = m_pre_date;
    p->m_reserve_percent = m_reserve_percent;
    p->m_tm = m_tm;
    return p;
}

void AllocateFundsBase::setReserverPercent(double percent) {
    if (percent < 0) {
        m_reserve_percent = 0;
    } else if (m_reserve_percent > 1) {
        m_reserve_percent = 1;
    } else {
        m_reserve_percent = percent;
    }
}

void AllocateFundsBase ::adjustFunds(const Datetime& date, const SystemList& se_list,
                                     const std::list<SYSPtr>& running_list) {
    int max_num = getParam<int>("max_sys_num");
    if (max_num <= 0) {
        HKU_ERROR("param(max_sys_num) need > 0!");
        return;
    }

    if (getParam<bool>("adjust_running_sys")) {
        _adjust_with_running(date, se_list, running_list);
    } else {
        _adjust_without_running(date, se_list, running_list);
    }
}

void AllocateFundsBase::_adjust_with_running(const Datetime& date, const SystemList& se_list,
                                             const std::list<SYSPtr>& running_list) {
    //计算当前选中系统列表的权重
    SystemWeightList sw_list = _allocateWeight(date, se_list);
    if (sw_list.size() == 0) {
        return;
    }

    //构建实际分配权重大于零的的系统集合，同时计算总权重
    std::set<SYSPtr> selected_sets;
    price_t total_weight = 0.0;
    for (auto iter = sw_list.begin(); iter != sw_list.end(); ++iter) {
        if (iter->getWeight() > 0) {
            selected_sets.insert(iter->getSYS());
            total_weight += iter->getWeight();
        }
    }

    //如果当前持仓的系统不在实际的选中系统集合，则强制清仓卖出，如果账户有现金则同时回收现金
    for (auto iter = running_list.begin(); iter != running_list.end(); ++iter) {
        const SYSPtr& sys = *iter;
        if (selected_sets.find(sys) == selected_sets.end()) {
            KRecord record = sys->getTO().getKRecordByDate(date);
            sys->_sell(record, PART_ALLOCATEFUNDS);
            TMPtr tm = sys->getTM();
            price_t cash = tm->currentCash();
            if (cash > 0) {
                if (tm->checkout(date, cash)) {
                    m_tm->checkin(date, cash);
                } else {
                    HKU_ERROR("Can't checkout from sub_tm!");
                }
            }
        }
    }

    //获取当前总账户资产，计算每单位权重代表的资金，当前能分配的现金
    FundsRecord funds = m_tm->getFunds(m_query.kType());
    price_t total_funds =
      funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;
    price_t per_weight_funds = total_funds / total_weight;
    int precision = m_tm->getParam<int>("precision");
    price_t can_allocate_cash = m_tm->currentCash();

    // 按权重升序排序（注意：无法保证等权重的相对顺序，即使用stable_sort也一样，后面要倒序遍历）
    std::sort(sw_list.begin(), sw_list.end(),
              boost::bind(std::less<double>(), boost::bind(&SystemWeight::m_weight, _1),
                          boost::bind(&SystemWeight::m_weight, _2)));

    // 按分配的权重将现金转移至子账户
    std::list<std::pair<SYSPtr, price_t>> wait_list;
    for (auto iter = sw_list.rbegin(); iter != sw_list.rend(); ++iter) {
        // 如果可分配的现金不足或选中系统的分配权重已经小于等于0，则退出
        if (can_allocate_cash <= 0 || iter->getWeight() <= 0) {
            break;
        }

        // 获取系统账户的当前资产市值
        SYSPtr sys = iter->getSYS();
        TMPtr tm = sys->getTM();
        funds = tm->getFunds(m_query.kType());
        price_t funds_value =
          funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;

        price_t will_funds_value = roundDown(iter->getWeight() * per_weight_funds, precision);
        if (funds_value == will_funds_value) {
            // 如果当前资产已经等于期望分配的资产，则忽略
            continue;

        } else if (funds_value < will_funds_value) {
            // 如果当前资产小于期望分配的资产，则补充现金
            price_t will_cash = roundDown(will_funds_value - funds_value, precision);

            // 如果当前可用于分配的资金大于期望的资金，则尝试从总账户中将现金补充进子账户中
            if (will_cash <= can_allocate_cash) {
                if (m_tm->checkout(date, will_cash)) {
                    tm->checkin(date, will_cash);
                    can_allocate_cash = roundDown(can_allocate_cash - will_cash, precision);
                } else {
                    HKU_WARN("Can't checkout from m_tm!");
                }

            } else {
                // 如果当前可用于分配的资金已经不足，则先全部转入子账户，并调整该系统实例权重。
                // 同时，将该系统实例放入带重分配列表中，等有需要腾出仓位的系统卖出后，再重新分配补充现金
                if (m_tm->checkout(date, can_allocate_cash)) {
                    tm->checkin(date, can_allocate_cash);
                    can_allocate_cash = 0;
                } else {
                    HKU_WARN("Can't checkout from m_tm!");
                }

                wait_list.push_back(
                  std::make_pair(sys, roundDown(will_cash - can_allocate_cash, precision)));
            }

        } else {
            //如果当前资产大于期望分配的资产，则子账户是否有现金可以取出抵扣，否则卖掉部分股票
            price_t will_return_cash = roundDown(funds_value - will_funds_value, precision);
            price_t cash = tm->currentCash();
            if (cash >= will_return_cash) {
                if (tm->checkout(date, will_return_cash)) {
                    m_tm->checkin(date, will_return_cash);
                } else {
                    HKU_ERROR("Could not checkout from tm");
                }

            } else {
                // 将子账户的剩余资金都转出至总账户
                if (tm->checkout(date, cash)) {
                    m_tm->checkin(date, cash);
                } else {
                    HKU_ERROR("Could not checkout from tm");
                }

                // 还需要返还的资金
                price_t need_cash = will_return_cash - cash;

                // 计算并卖出部分股票以获取剩下需要返还的资金
                Stock stock = sys->getStock();
                KRecord k = sys->getTO().getKRecordByDate(date);
                PositionRecord position = tm->getPosition(stock);
                if (position.number > 0) {
                    double need_sell_num = need_cash / k.closePrice;
                    need_sell_num =
                      size_t(need_sell_num / stock.minTradeNumber()) * stock.minTradeNumber();
                    if (position.number >= need_sell_num) {
                        sys->_sellForce(k, need_sell_num, PART_ALLOCATEFUNDS);
                    }
                }

                // 卖出后，将资金取出转移至总账户
                cash = tm->currentCash();
                if (tm->checkout(date, cash)) {
                    m_tm->checkin(date, cash);
                }
            }
        }
    }

    // 部分调整仓位的股票被卖出后，再次将资金分配至等待资金的子系统
    can_allocate_cash = m_tm->currentCash();
    for (auto iter = wait_list.begin(); iter != wait_list.end(); ++iter) {
        // 如果可分配的现金不足或选中系统的分配权重已经小于等于0，则退出
        if (can_allocate_cash <= 0) {
            break;
        }

        // 获取系统账户的当前资产市值
        SYSPtr sys = iter->first;
        TMPtr tm = sys->getTM();

        price_t need_cash = iter->second;
        if (m_tm->checkout(date, need_cash)) {
            tm->checkin(date, need_cash);
            can_allocate_cash = roundDown(can_allocate_cash - need_cash, precision);
        }
    }
}

void AllocateFundsBase::_adjust_without_running(const Datetime& date, const SystemList& se_list,
                                                const std::list<SYSPtr>& running_list) {
    if (se_list.size() == 0) {
        return;
    }

    //如果持仓的系统数已大于等于最大持仓系统数，直接输出已持仓系统列表，并返回
    int max_num = getParam<int>("max_sys_num");
    if (running_list.size() >= max_num) {
        return;
    }

    //从当前选中的系统列表中将运行中的子系统排除
    std::set<SYSPtr> hold_sets;
    for (auto iter = running_list.begin(); iter != running_list.end(); ++iter) {
        hold_sets.insert(*iter);
    }

    SystemList pure_se_list;  // 不包含运行中系统的子系统列表
    for (auto iter = se_list.begin(); iter != se_list.end(); ++iter) {
        if (hold_sets.find(*iter) == hold_sets.end()) {
            pure_se_list.push_back(*iter);
        }
    }

    //获取权重
    SystemWeightList sw_list = _allocateWeight(date, pure_se_list);
    if (sw_list.size() == 0) {
        return;
    }

    //按权重升序排序（注意：无法保证等权重的相对顺序，即使用stable_sort也一样，后面要倒序遍历）
    std::sort(sw_list.begin(), sw_list.end(),
              boost::bind(std::less<double>(), boost::bind(&SystemWeight::m_weight, _1),
                          boost::bind(&SystemWeight::m_weight, _2)));

    //倒序遍历，计算总权重，并在遇到权重为0或等于最大持仓时结束遍历
    size_t remain = max_num - running_list.size();
    price_t total_weight = 0.0;
    auto sw_iter = sw_list.rbegin();
    for (size_t count = 0; sw_iter != sw_list.rend(); ++sw_iter, count++) {
        if (sw_iter->getWeight() <= 0.0 || count >= remain)
            break;
        total_weight += sw_iter->getWeight();
    }

    if (total_weight <= 0.0) {
        return;
    }

    auto end_iter = sw_iter;

    int precision = m_tm->getParam<int>("precision");
    price_t per_cash = 0.0;

    per_cash = m_tm->currentCash() / total_weight;  // 每单位权重资金
    sw_iter = sw_list.rbegin();
    for (; sw_iter != end_iter; ++sw_iter) {
        // 该系统期望分配的资金
        price_t will_cash = roundDown(per_cash * sw_iter->getWeight(), precision);
        if (will_cash <= std::abs(roundDown(0.0, precision))) {
            break;
        }

        // 尝试从总账户中取出资金存入子账户
        SYSPtr sub_sys = sw_iter->getSYS();
        TMPtr sub_tm = sub_sys->getTM();
        if (m_tm->checkout(date, will_cash)) {
            sub_tm->checkin(date, will_cash);
        }
    }
}

} /* namespace hku */
