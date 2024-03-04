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
    setParam<bool>("adjust_running_sys", false);
    setParam<int>("max_sys_num", 1000000);             // 允许运行的最大系统实例数
    setParam<double>("weight_unit", 0.0001);           // 最小权重单位
    setParam<double>("default_reserve_percent", 0.0);  // 默认保留不参与重分配的资产比例
}

AllocateFundsBase::AllocateFundsBase(const string& name)
: m_name("AllocateMoneyBase"), m_reserve_percent(0) {
    setParam<bool>("adjust_running_sys", false);
    setParam<int>("max_sys_num", 100000);              // 最大系统实例数
    setParam<double>("weight_unit", 0.0001);           // 最小权重单位
    setParam<double>("default_reserve_percent", 0.0);  // 默认保留不参与重分配的资产比例
}

AllocateFundsBase::~AllocateFundsBase() {}

void AllocateFundsBase::reset() {
    m_reserve_percent = getParam<double>("default_reserve_percent");
    _reset();

    // 参数检查
    HKU_ERROR_IF(getParam<int>("max_sys_num") <= 0, R"(AF param["max_sys_num"]({}) need > 0!)",
                 getParam<int>("max_sys_num"));
    HKU_ERROR_IF(
      getParam<double>("default_reserve_percent") >= 1.0,
      R"(AF param(default_reserve_percent)({}) >= 1.0, No asset adjustments will be made!)");
    HKU_CHECK(getParam<double>("default_reserve_percent") >= 0.0,
              R"(Invalid AF param["default_reserve_percent"] ({}))",
              getParam<double>("default_reserve_percent"));
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

void AllocateFundsBase::adjustFunds(const Datetime& date, const SystemList& se_list,
                                    const std::list<SYSPtr>& running_list,
                                    const SystemList& ignore_list) {
    if (getParam<bool>("adjust_running_sys")) {
        _adjust_with_running(date, se_list, running_list, ignore_list);
    } else {
        _adjust_without_running(date, se_list, running_list);
    }
}

price_t AllocateFundsBase::_getTotalFunds(const Datetime& date,
                                          const std::list<SYSPtr>& running_list) {
    price_t total_value = 0;

    // 计算运行中的子系统总资产净值
    for (auto& sub_sys : running_list) {
        TMPtr sub_tm = sub_sys->getTM();
        KQuery sub_query = sub_sys->getTO().getQuery();
        FundsRecord funds = sub_tm->getFunds(date, sub_query.kType());
        total_value +=
          funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;
    }

    // 加上当前总账户现金余额
    m_shadow_tm->updateWithWeight(date);
    total_value = roundDown(total_value + m_shadow_tm->cash(date, m_query.kType()),
                            m_shadow_tm->getParam<int>("precision"));
    return total_value;
}

bool AllocateFundsBase::_returnAssets(const SYSPtr& sys, const Datetime& date) {
    KData kdata = sys->getTO();
    size_t pos = kdata.getPos(date);
    HKU_IF_RETURN(pos == Null<size_t>(), false);

    KRecord record = kdata.getKRecord(pos);
    Stock stock = kdata.getStock();
    KRecord srcRecord = stock.getKRecord(kdata.startPos() + pos);

    // 如果存在持仓则卖出
    TMPtr tm = sys->getTM();
    if (tm->have(stock)) {
        TradeRecord tr = sys->sell(record, srcRecord, PART_ALLOCATEFUNDS);
        if (!tr.isNull()) {
            m_tm->addTradeRecord(tr);
        }
    }

    // 回收当前子账号资金至总账户
    price_t cash = tm->cash(date, kdata.getQuery().kType());
    if (cash > 0.0 && tm->checkout(date, cash)) {
        m_shadow_tm->checkin(date, cash);
    }

    return true;
}

// 调整运行中子系统持仓
void AllocateFundsBase::_adjust_with_running(const Datetime& date, const SystemList& se_list,
                                             const std::list<SYSPtr>& running_list,
                                             const SystemList& ignore_list) {
    // 计算当前选中系统列表的权重
    SystemWeightList sw_list = _allocateWeight(date, se_list);
    HKU_IF_RETURN(sw_list.size() == 0, void());

    // 构建实际分配权重大于零的的系统集合，权重小于等于0的将被忽略
    std::unordered_set<System*> selected_sets;
    for (auto& sw : sw_list) {
        if (sw.getWeight() > 0.0) {
            selected_sets.insert(sw.getSYS().get());
        }
    }

    std::unordered_set<System*> ignore_sets;
    for (auto& sys : ignore_list) {
        ignore_sets.insert(sys.get());
    }

    std::unordered_set<System*> selected_running_sets;

    // 如果当前运行的系统不在实际的选中系统集合且不在忽略列表中，则强制清仓卖出，并回收资产
    for (auto& sys : running_list) {
        // 不在忽略列表中
        if (ignore_sets.find(sys.get()) == ignore_sets.end()) {
            // 当前持仓的系统仍旧被选中，记入仍被选中的运行系统列表
            if (selected_sets.find(sys.get()) != selected_sets.end()) {
                selected_running_sets.insert(sys.get());
            } else {
                _returnAssets(sys, date);
            }
        }
    }

    // 按权重升序排序（注意：无法保证等权重的相对顺序，即使用stable_sort也一样，后面要倒序遍历）
    std::sort(
      sw_list.begin(), sw_list.end(),
      std::bind(std::less<double>(), std::bind(&SystemWeight::m_weight, std::placeholders::_1),
                std::bind(&SystemWeight::m_weight, std::placeholders::_2)));

    // 过滤掉超出允许范围的系统
    // 按权重从大到小遍历，构建不超过最大允许的运行子系统数的新权重列表（此时按从大到小顺序存放）
    // 同时，将超出最大允许的运行子系统数范围外的运行中子系统清仓回收资金
    int max_num = getParam<int>("max_sys_num");
    std::list<SystemWeight> new_sw_list;  // 存放新的权重列表
    size_t count = 0;
    for (auto iter = sw_list.rbegin(); iter != sw_list.rend(); ++iter) {
        // 忽略小于等于零的权重
        if (iter->getWeight() <= 0.0) {
            break;
        }

        // 小于最大允许运行数，直接保存至新的权重列表
        if (count < max_num) {
            new_sw_list.emplace_back(std::move(*iter));
            count++;
            continue;
        }

        // 处理超出允许的最大系统数范围外的系统，尝试强制清仓，但不在放入权重列表（即后续不参与资金分配）
        if (selected_running_sets.find(iter->getSYS().get()) != selected_running_sets.end()) {
            _returnAssets(iter->getSYS(), date);
        }
    }

    // 账户资金精度
    int precision = m_shadow_tm->getParam<int>("precision");

    // 获取当前总账户资产净值，并计算每单位权重代表的资金
    price_t total_funds = _getTotalFunds(date, running_list);

    // 计算需保留的资产
    HKU_ERROR_IF(m_reserve_percent < 0.0,
                 "Invalid reserve_percent({}) in AF, Calculations that will cause errors!",
                 m_reserve_percent);
    price_t reserve_funds = roundUp(total_funds * m_reserve_percent, precision);

    // 计算每单位权重资产
    double weight_unit = getParam<double>("weight_unit");
    HKU_CHECK(weight_unit > 0, "Invalid weight_unit! {}", weight_unit);
    price_t per_weight_funds = total_funds * weight_unit;

    // 计算可分配现金
    price_t can_allocate_cash =
      roundDown(m_shadow_tm->cash(date, m_query.kType()) - reserve_funds, precision);
    if (can_allocate_cash < 0.0) {
        can_allocate_cash = 0.0;
    }

    // 缓存需要进一步处理的系统及其待补充资金的列表
    std::list<std::pair<SYSPtr, price_t>> wait_list;

    // 按权重从大到小遍历
    // 1.如果子系统当前资产已经等于期望被分配的资产则不做处理
    // 2.如果子系统当前资产小于期望被分配的资产，则尝试补充资金，否则放入等待列表
    // 3.如果当前资产大于期望分配的资产，则子账户是否有现金可以取出抵扣，否则卖掉部分股票
    for (auto iter = new_sw_list.begin(); iter != new_sw_list.end(); ++iter) {
        // 选中系统的分配权重已经小于等于0，则退出
        if (iter->getWeight() <= 0.0) {
            break;
        }

        auto& sys = iter->getSYS();

        // 如果在忽略列表中，则跳过
        if (ignore_sets.find(sys.get()) != ignore_sets.end()) {
            continue;
        }

        // 获取系统账户的当前资产市值
        TMPtr tm = sys->getTM();

        // 更新子账号权息数据
        tm->updateWithWeight(date);

        FundsRecord funds = tm->getFunds(date, m_query.kType());
        price_t funds_value =
          funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value;

        price_t will_funds_value = (iter->getWeight() / weight_unit) * per_weight_funds;
        if (funds_value == will_funds_value) {
            // 如果当前资产已经等于期望分配的资产，则忽略
            continue;

        } else if (funds_value < will_funds_value) {
            // 如果当前资产小于期望分配的资产，则补充现金
            price_t will_cash = roundUp(will_funds_value - funds_value, precision);

            // 如果当前可用于分配的资金大于期望的资金，则尝试从总账户中将现金补充进子账户中
            if (will_cash > 0.0 && will_cash <= can_allocate_cash) {
                if (m_shadow_tm->checkout(date, will_cash)) {
                    tm->checkin(date, will_cash);
                    can_allocate_cash = roundDown(can_allocate_cash - will_cash, precision);
                }

            } else {
                // 如果当前可用于分配的资金已经不足，则先全部转入子账户，并调整该系统实例权重。
                // 同时，将该系统实例放入带重分配列表中，等有需要腾出仓位的系统卖出后，再重新分配补充现金
                if (can_allocate_cash > 0.0 && m_shadow_tm->checkout(date, can_allocate_cash)) {
                    tm->checkin(date, can_allocate_cash);
                    can_allocate_cash = 0.0;
                }

                // 缓存至等待列表，以便后续处理
                price_t reserve_cash = roundUp(will_cash - can_allocate_cash, precision);
                if (reserve_cash > 0.0) {
                    wait_list.push_back(std::make_pair(sys, reserve_cash));
                }
            }

        } else {
            // 如果当前资产大于期望分配的资产，则子账户是否有现金可以取出抵扣，否则卖掉部分股票
            price_t will_return_cash = roundDown(funds_value - will_funds_value, precision);
            if (will_return_cash <= 0.0) {
                continue;
            }
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
                PositionRecord position = tm->getPosition(date, stock);
                if (position.number > 0) {
                    KData kdata = sys->getTO();
                    size_t pos = kdata.getPos(date);
                    if (pos == Null<size_t>()) {
                        continue;
                    }
                    KRecord k = kdata.getKRecord(pos);
                    KRecord srcK = stock.getKRecord(kdata.startPos() + pos);
                    TradeRecord tr;
                    double need_sell_num = sys->getParam<bool>("sell_delay")
                                             ? need_cash / k.closePrice
                                             : need_cash / k.openPrice;
                    if (position.number <= need_sell_num) {
                        // 如果当前持仓数小于等于需要卖出的数量，则全部卖出
                        tr = sys->sellForce(k, srcK, position.number, PART_ALLOCATEFUNDS);
                    } else {
                        if (position.number - need_sell_num >= stock.minTradeNumber()) {
                            // 如果按需要卖出数量卖出后，可能剩余的数量大于等于最小交易数则按需要卖出的数量卖出
                            tr = sys->sellForce(k, srcK, need_sell_num, PART_ALLOCATEFUNDS);
                        } else {
                            // 如果按需要卖出的数量卖出后，剩余的持仓数小于最小交易数量则全部卖出
                            tr = sys->sellForce(k, srcK, position.number, PART_ALLOCATEFUNDS);
                        }
                    }
                    if (!tr.isNull()) {
                        m_tm->addTradeRecord(tr);
                    }

                    // 卖出后，尝试将资金取出转移至总账户
                    sub_cash = tm->currentCash();
                    if (sub_cash > 0 && tm->checkout(date, sub_cash)) {
                        m_shadow_tm->checkin(date, sub_cash);
                        can_allocate_cash = roundDown(can_allocate_cash + sub_cash, precision);
                    }
                }
            }
        }
    }

    // 部分调整仓位的股票被卖出后，再次将资金分配至等待资金的子系统
    can_allocate_cash = m_shadow_tm->currentCash();
    for (auto iter = wait_list.begin(); iter != wait_list.end(); ++iter) {
        // 如果可分配的现金不足或选中系统的分配权重已经小于等于0，则退出
        if (can_allocate_cash <= 0.0) {
            break;
        }

        // 获取系统账户的当前资产市值
        SYSPtr sys = iter->first;
        TMPtr tm = sys->getTM();

        price_t need_cash = iter->second;
        if (need_cash <= can_allocate_cash) {
            if (m_shadow_tm->checkout(date, need_cash)) {
                tm->checkin(date, need_cash);
                can_allocate_cash = roundDown(can_allocate_cash - need_cash, precision);
            }
        } else {
            if (m_shadow_tm->checkout(date, can_allocate_cash)) {
                tm->checkin(date, can_allocate_cash);
                can_allocate_cash = 0.0;
            }
        }
    }
}

void AllocateFundsBase::_adjust_without_running(const Datetime& date, const SystemList& se_list,
                                                const std::list<SYSPtr>& running_list) {
    HKU_IF_RETURN(se_list.size() == 0, void());

    // 如果运行中的系统数已大于等于允许的最大系统数，直接返回
    int max_num = getParam<int>("max_sys_num");
    HKU_IF_RETURN(running_list.size() >= max_num, void());

    // 计算选中系统中当前正在运行中的子系统
    std::unordered_set<System*> hold_sets;
    for (auto& sys : running_list) {
        hold_sets.insert(sys.get());
    }

    // 计算不包含运行中系统的子系统列表
    SystemList pure_se_list;
    for (auto& sys : se_list) {
        if (hold_sets.find(sys.get()) == hold_sets.end()) {
            pure_se_list.push_back(sys);
        }
    }

    // 获取计划分配的资产权重
    SystemWeightList sw_list = _allocateWeight(date, pure_se_list);
    HKU_IF_RETURN(sw_list.size() == 0, void());

    // 按权重升序排序（注意：无法保证等权重的相对顺序，即使用stable_sort也一样，后面要倒序遍历）
    std::sort(
      sw_list.begin(), sw_list.end(),
      std::bind(std::less<double>(), std::bind(&SystemWeight::m_weight, std::placeholders::_1),
                std::bind(&SystemWeight::m_weight, std::placeholders::_2)));

    // 检测是否有信号发生，过滤掉没有发生信号的系统 以及 权重为 0 的系统
    SystemWeightList new_sw_list;
    auto sw_iter = sw_list.rbegin();
    for (; sw_iter != sw_list.rend(); ++sw_iter) {
        if (sw_iter->getWeight() <= 0.0)
            break;
        if (sw_iter->getSYS()->getSG()->shouldBuy(date)) {
            new_sw_list.emplace_back(*sw_iter);
        }
    }

    // 总账号资金精度
    int precision = m_shadow_tm->getParam<int>("precision");

    // 获取当前总资产市值
    price_t total_funds = _getTotalFunds(date, running_list);

    // 计算需保留的资产
    HKU_ERROR_IF(m_reserve_percent < 0.0,
                 "Invalid reserve_percent({}) in AF, Calculations that will cause errors!",
                 m_reserve_percent);
    price_t reserve_funds = total_funds * m_reserve_percent;

    // 如果当前现金小于等于需保留的资产，则直接返回
    HKU_IF_RETURN(m_shadow_tm->cash(date, m_query.kType()) <= reserve_funds, void());

    // 计算可用于分配的现金
    price_t can_allocate_cash =
      roundDown(m_shadow_tm->cash(date, m_query.kType()) - reserve_funds, precision);
    if (can_allocate_cash <= 0.0) {
        return;
    }

    // 再次遍历选中子系统列表，并将剩余现金按权重比例转入子账户
    double weight_unit = getParam<double>("weight_unit");
    price_t per_cash = total_funds * weight_unit;  // 每单位权重资金
    size_t can_run_count = 0;
    for (auto sw_iter = new_sw_list.begin(), end_iter = new_sw_list.end(); sw_iter != end_iter;
         ++sw_iter) {
        // 该系统期望分配的资金
        price_t will_cash = roundUp(per_cash * (sw_iter->getWeight() / weight_unit), precision);
        if (will_cash <= 0.0) {
            continue;
        }

        // 计算实际可分配的资金
        price_t need_cash = will_cash <= can_allocate_cash ? will_cash : can_allocate_cash;

        // 检测是否可以发生交易，不能的话，忽略
        auto krecord = sw_iter->getSYS()->getTO().getKRecord(date);
        if (krecord.closePrice < need_cash) {
            continue;
        }

        // 尝试从总账户中取出资金存入子账户
        TMPtr sub_tm = sw_iter->getSYS()->getTM();
        if (m_shadow_tm->checkout(date, need_cash)) {
            sub_tm->checkin(date, need_cash);
            can_allocate_cash = roundDown(can_allocate_cash - need_cash, precision);
            if (can_allocate_cash <= 0.0) {
                continue;
            }

            // 如果超出允许运行的最大系统数，跳出循环
            can_run_count++;
            if (can_run_count > max_num) {
                break;
            }
        }
    }
}

} /* namespace hku */
