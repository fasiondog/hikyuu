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

AllocateFundsBase::AllocateFundsBase()
: m_name("AllocateMoneyBase"), m_count(0), m_pre_date(Datetime::min()), m_reserve_percent(0) {
    //是否调整之前已经持仓策略的持仓。不调整时，仅使用总账户当前剩余资金进行分配，否则将使用总市值进行分配
    setParam<bool>("adjust_running_sys", false);
    setParam<int>("max_sys_num", 100000);     //最大系统实例数
    setParam<double>("weight_unit", 0.0001);  //最小权重单位
}

AllocateFundsBase::AllocateFundsBase(const string& name)
: m_name("AllocateMoneyBase"), m_count(0), m_pre_date(Datetime::min()), m_reserve_percent(0) {
    setParam<bool>("adjust_running_sys", false);
    setParam<int>("max_sys_num", 100000);     //最大系统实例数
    setParam<double>("weight_unit", 0.0001);  //最小权重单位
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
    /*if (m_tm)
        p->m_tm = m_tm->clone();
    if (m_shadow_tm)
        p->m_shadow_tm = m_shadow_tm->clone();*/
    return p;
}

void AllocateFundsBase::setReservePercent(double percent) {
    HKU_CHECK_THROW(percent >= 0 && percent <= 1, std::out_of_range,
                    "percent ({}) is out of range [0, 1]!");
    m_reserve_percent = percent;
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

price_t AllocateFundsBase::_getTotalFunds(const std::list<SYSPtr>& running_list) {
    price_t total_value = 0;

    // 计算运行中的子系统总资产净值
    for (auto& sub_sys : running_list) {
        TMPtr sub_tm = sub_sys->getTM();
        KQuery sub_query = sub_sys->getTO().getQuery();
        FundsRecord funds = sub_tm->getFunds(sub_query.kType());
        total_value +=
          funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;
    }

    // 加上当前总账户现金余额
    total_value =
      roundDown(total_value + m_shadow_tm->currentCash(), m_shadow_tm->getParam<int>("precision"));
    return total_value;
}

void AllocateFundsBase::_adjust_with_running(const Datetime& date, const SystemList& se_list,
                                             const std::list<SYSPtr>& running_list) {
    // 计算当前选中系统列表的权重
    SystemWeightList sw_list = _allocateWeight(date, se_list);
    if (sw_list.size() == 0) {
        return;
    }

    // 如果运行中的系统数已大于等于允许的最大系统数，直接返回
    int max_num = getParam<int>("max_sys_num");
    if (running_list.size() >= max_num) {
        return;
    }

    //构建实际分配权重大于零的的系统集合
    std::set<SYSPtr> selected_sets;
    for (auto iter = sw_list.begin(); iter != sw_list.end(); ++iter) {
        if (iter->getWeight() > 0) {
            selected_sets.insert(iter->getSYS());
        }
    }

    std::unordered_set<SYSPtr> selected_running_sets;

    // 如果当前持仓的系统不在实际的选中系统集合，则强制清仓卖出，如果账户有现金则同时回收现金
    for (auto iter = running_list.begin(); iter != running_list.end(); ++iter) {
        const SYSPtr& sys = *iter;
        if (selected_sets.find(sys) == selected_sets.end()) {
            KRecord record = sys->getTO().getKRecord(date);
            TradeRecord tr = sys->_sell(record, PART_ALLOCATEFUNDS);
            if (!tr.isNull()) {
                m_tm->addTradeRecord(tr);
            }
            TMPtr tm = sys->getTM();
            price_t cash = tm->currentCash();
            if (cash > 0) {
                if (tm->checkout(date, cash)) {
                    m_shadow_tm->checkin(date, cash);
                } else {
                    HKU_ERROR("Can't checkout from sub_tm!");
                }
            }
        } else {
            selected_running_sets.insert(*iter);
        }
    }

    // 按权重升序排序（注意：无法保证等权重的相对顺序，即使用stable_sort也一样，后面要倒序遍历）
    std::sort(
      sw_list.begin(), sw_list.end(),
      std::bind(std::less<double>(), std::bind(&SystemWeight::m_weight, std::placeholders::_1),
                std::bind(&SystemWeight::m_weight, std::placeholders::_2)));

    // 按权重从大到小遍历，构建不超过最大允许的运行子系统数的新权重列表（此时按从大到小顺序存放）
    // 同时，将超出最大允许的运行子系统数范围外的运行中子系统清仓回收资金
    std::list<SystemWeight> new_sw_list;  // 存放新的权重列表
    size_t count = 0;
    for (auto iter = sw_list.rbegin(); iter != sw_list.rend(); ++iter) {
        if (count < max_num) {
            // 小于最大允许运行数，保存至新的权重列表
            new_sw_list.push_back(*iter);

        } else if (selected_running_sets.find(iter->getSYS()) != selected_running_sets.end()) {
            // 超出最大允许运行数且属于正在运行的子系统，则尝试清仓并回收资金
            auto sys = iter->getSYS();
            KRecord record = sys->getTO().getKRecord(date);
            auto tr = sys->_sell(record, PART_ALLOCATEFUNDS);
            if (!tr.isNull()) {
                m_tm->addTradeRecord(tr);
            }
            TMPtr tm = sys->getTM();
            price_t cash = tm->currentCash();
            if (cash > 0) {
                if (tm->checkout(date, cash)) {
                    m_shadow_tm->checkin(date, cash);
                } else {
                    HKU_ERROR("Can't checkout from sub_tm!");
                }
            }
        }
        count++;
    }

    //获取当前总账户资产净值，并计算每单位权重代表的资金
    price_t total_funds = _getTotalFunds(running_list);

    // 计算需保留的资产
    price_t reserve_funds = total_funds * m_reserve_percent;

    // 计算每单位权重资产
    double weight_unit = getParam<double>("weight_unit");
    price_t per_weight_funds = total_funds * weight_unit;

    // 账户资金精度
    int precision = m_shadow_tm->getParam<int>("precision");

    // 计算可分配现金
    price_t can_allocate_cash =
      m_shadow_tm->currentCash() > reserve_funds ? m_shadow_tm->currentCash() - reserve_funds : 0;

    // 缓存需要进一步处理的系统及其待补充资金的列表
    std::list<std::pair<SYSPtr, price_t>> wait_list;

    // 按权重从大到小遍历
    // 1.如果子系统当前资产已经等于期望被分配的资产则不做处理
    // 2.如果子系统当前资产小于期望被分配的资产，则尝试补充资金，否则放入等待列表
    // 3.如果当前资产大于期望分配的资产，则子账户是否有现金可以取出抵扣，否则卖掉部分股票
    for (auto iter = new_sw_list.begin(); iter != new_sw_list.end(); ++iter) {
        // 选中系统的分配权重已经小于等于0，则退出
        if (iter->getWeight() <= 0) {
            break;
        }

        // 获取系统账户的当前资产市值
        SYSPtr sys = iter->getSYS();
        TMPtr tm = sys->getTM();
        FundsRecord funds = tm->getFunds(m_query.kType());
        price_t funds_value =
          funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;

        price_t will_funds_value =
          roundDown((iter->getWeight() / weight_unit) * per_weight_funds, precision);
        if (funds_value == will_funds_value) {
            // 如果当前资产已经等于期望分配的资产，则忽略
            continue;

        } else if (funds_value < will_funds_value) {
            // 如果当前资产小于期望分配的资产，则补充现金
            price_t will_cash = roundDown(will_funds_value - funds_value, precision);

            // 如果当前可用于分配的资金大于期望的资金，则尝试从总账户中将现金补充进子账户中
            if (will_cash <= can_allocate_cash) {
                if (m_shadow_tm->checkout(date, will_cash)) {
                    tm->checkin(date, will_cash);
                    can_allocate_cash = roundDown(can_allocate_cash - will_cash, precision);
                }

            } else {
                // 如果当前可用于分配的资金已经不足，则先全部转入子账户，并调整该系统实例权重。
                // 同时，将该系统实例放入带重分配列表中，等有需要腾出仓位的系统卖出后，再重新分配补充现金
                price_t reserve_cash = roundDown(will_cash - can_allocate_cash, precision);
                if (m_shadow_tm->checkout(date, can_allocate_cash)) {
                    tm->checkin(date, can_allocate_cash);
                    can_allocate_cash = 0;
                }

                // 缓存至等待列表，以便后续处理
                wait_list.push_back(std::make_pair(sys, reserve_cash));
            }

        } else {
            //如果当前资产大于期望分配的资产，则子账户是否有现金可以取出抵扣，否则卖掉部分股票
            price_t will_return_cash = roundDown(funds_value - will_funds_value, precision);
            price_t sub_cash = tm->currentCash();
            if (sub_cash >= will_return_cash) {
                if (tm->checkout(date, will_return_cash)) {
                    m_shadow_tm->checkin(date, will_return_cash);
                    can_allocate_cash = roundDown(can_allocate_cash + will_return_cash, precision);
                }

            } else {
                // 计算需要卖出股票换取的资金
                price_t need_cash = will_return_cash - sub_cash;

                // 计算并卖出部分股票以获取剩下需要返还的资金
                Stock stock = sys->getStock();
                KRecord k = sys->getTO().getKRecord(date);
                PositionRecord position = tm->getPosition(stock);
                if (position.number > 0) {
                    TradeRecord tr;
                    double need_sell_num = need_cash / k.closePrice;
                    need_sell_num =
                      size_t(need_sell_num / stock.minTradeNumber()) * stock.minTradeNumber();
                    if (position.number <= need_sell_num) {
                        // 如果当前持仓数小于等于需要卖出的数量，则全部卖出
                        tr = sys->_sellForce(k, position.number, PART_ALLOCATEFUNDS);
                    } else {
                        if (position.number - need_sell_num >= stock.minTradeNumber()) {
                            // 如果按需要卖出数量卖出后，可能剩余的数量大于等于最小交易数则按需要卖出的数量卖出
                            tr = sys->_sellForce(k, need_sell_num, PART_ALLOCATEFUNDS);
                        } else {
                            // 如果按需要卖出的数量卖出后，剩余的持仓数小于最小交易数量则全部卖出
                            tr = sys->_sellForce(k, position.number, PART_ALLOCATEFUNDS);
                        }
                    }
                    if (!tr.isNull()) {
                        m_tm->addTradeRecord(tr);
                    }
                }

                // 卖出后，尝试将资金取出转移至总账户
                sub_cash = tm->currentCash();
                if (tm->checkout(date, sub_cash)) {
                    m_shadow_tm->checkin(date, sub_cash);
                    can_allocate_cash = roundDown(can_allocate_cash + sub_cash, precision);
                }
            }
        }
    }

    // 部分调整仓位的股票被卖出后，再次将资金分配至等待资金的子系统
    can_allocate_cash = m_shadow_tm->currentCash();
    for (auto iter = wait_list.begin(); iter != wait_list.end(); ++iter) {
        // 如果可分配的现金不足或选中系统的分配权重已经小于等于0，则退出
        if (can_allocate_cash <= 0) {
            break;
        }

        // 获取系统账户的当前资产市值
        SYSPtr sys = iter->first;
        TMPtr tm = sys->getTM();

        price_t need_cash = iter->second;
        if (m_shadow_tm->checkout(date, need_cash)) {
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

    //如果运行中的系统数已大于等于允许的最大系统数，直接返回
    int max_num = getParam<int>("max_sys_num");
    if (running_list.size() >= max_num) {
        return;
    }

    // 计算选中系统中当前正在运行中的子系统
    std::set<SYSPtr> hold_sets;
    for (auto iter = running_list.begin(); iter != running_list.end(); ++iter) {
        hold_sets.insert(*iter);
    }

    // 计算不包含运行中系统的子系统列表
    SystemList pure_se_list;
    for (auto iter = se_list.begin(); iter != se_list.end(); ++iter) {
        if (hold_sets.find(*iter) == hold_sets.end()) {
            pure_se_list.push_back(*iter);
        }
    }

    //获取计划分配的资产权重
    SystemWeightList sw_list = _allocateWeight(date, pure_se_list);
    if (sw_list.size() == 0) {
        return;
    }

    //按权重升序排序（注意：无法保证等权重的相对顺序，即使用stable_sort也一样，后面要倒序遍历）
    std::sort(
      sw_list.begin(), sw_list.end(),
      std::bind(std::less<double>(), std::bind(&SystemWeight::m_weight, std::placeholders::_1),
                std::bind(&SystemWeight::m_weight, std::placeholders::_2)));

    //倒序遍历，在遇到权重为0或等于运行的最大运行时系统数时结束遍历
    size_t count = 0;
    auto sw_iter = sw_list.rbegin();
    for (; sw_iter != sw_list.rend(); ++sw_iter) {
        if (sw_iter->getWeight() <= 0.0 || count >= max_num)
            break;
        count++;
    }

    auto end_iter = sw_iter;  // 记录结束位置

    // 总账号资金精度
    int precision = m_shadow_tm->getParam<int>("precision");

    // 获取当前总资产市值
    price_t total_funds = _getTotalFunds(running_list);

    // 计算需保留的资产
    price_t reserve_funds = total_funds * m_reserve_percent;

    // 如果当前现金小于等于需保留的资产，则直接返回
    if (m_shadow_tm->currentCash() <= reserve_funds) {
        return;
    }

    // 计算可用于分配的现金
    price_t can_allocate_cash = roundDown(m_shadow_tm->currentCash() - reserve_funds, precision);

    // 再次遍历选中子系统列表，并将剩余现金按权重比例转入子账户
    double weight_unit = getParam<double>("weight_unit");
    price_t per_cash = total_funds * weight_unit;  // 每单位权重资金
    sw_iter = sw_list.rbegin();
    for (; sw_iter != end_iter; ++sw_iter) {
        // 该系统期望分配的资金
        price_t will_cash = roundDown(per_cash * (sw_iter->getWeight() / weight_unit), precision);
        if (will_cash <= std::abs(roundDown(0.0, precision))) {
            break;
        }

        // 计算实际可分配的资金
        price_t need_cash = will_cash <= can_allocate_cash ? will_cash : can_allocate_cash;

        // 尝试从总账户中取出资金存入子账户
        TMPtr sub_tm = sw_iter->getSYS()->getTM();
        if (m_shadow_tm->checkout(date, need_cash)) {
            sub_tm->checkin(date, need_cash);
            can_allocate_cash = roundDown(can_allocate_cash - need_cash, precision);
        }
    }
}

} /* namespace hku */
