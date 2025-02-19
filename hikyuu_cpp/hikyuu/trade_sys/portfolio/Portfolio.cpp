/*
 * Portfolio.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include "hikyuu/global/sysinfo.h"
#include "hikyuu/trade_manage/crt/crtTM.h"
#include "hikyuu/trade_sys/selector/imp/optimal/OptimalSelectorBase.h"

#include "Portfolio.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const Portfolio& pf) {
    string strip(",\n");
    string space("  ");
    os << pf.str();
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const PortfolioPtr& pf) {
    if (pf) {
        os << pf->str();
    } else {
        os << "Portfolio(NULL)";
    }

    return os;
}

string Portfolio::str() const {
    std::stringstream os;
    string strip(",\n");
    string space("  ");
    os << "Portfolio{\n"
       << space << name() << strip << space << getParameter() << strip << space << getQuery()
       << strip << space << getAF() << strip << space << getSE() << strip << space
       << (getTM() ? getTM()->str() : "TradeManager(NULL)") << strip << "}";
    return os.str();
}

Portfolio::Portfolio() : m_name("Portfolio"), m_query(Null<KQuery>()), m_need_calculate(true) {
    initParam();
}

Portfolio::Portfolio(const string& name) : m_name(name) {
    initParam();
}

Portfolio::Portfolio(const string& name, const TradeManagerPtr& tm, const SelectorPtr& se,
                     const AFPtr& af)
: m_name(name), m_tm(tm), m_se(se), m_af(af), m_query(Null<KQuery>()), m_need_calculate(true) {
    initParam();
}

Portfolio::~Portfolio() {}

void Portfolio::initParam() {
    setParam<int>("adjust_cycle", 1);          // 调仓周期
    setParam<string>("adjust_mode", "query");  // 调仓模式

    // 延迟至交易日，当调仓日为非交易日时，自动延迟至下一个交易日作为调仓日
    setParam<bool>("delay_to_trading_day", true);

    setParam<bool>("trace", false);      // 打印跟踪
    setParam<int>("trace_max_num", 10);  // 打印跟踪时，显示当前持仓证券最大数量
}

void Portfolio::baseCheckParam(const string& name) const {
    if ("adjust_mode" == name || "adjust_cycle" == name) {
        if (!haveParam("adjust_mode") || !haveParam("adjust_cycle")) {
            // 同时判断两个参数时，可能一个参数还未设定
            return;
        }
        string adjust_mode = getParam<string>("adjust_mode");
        to_lower(adjust_mode);
        int adjust_cycle = getParam<int>("adjust_cycle");
        if ("query" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1);
        } else if ("day" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1);
        } else if ("week" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 5);
        } else if ("month" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 31);
        } else if ("quarter" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 92);
        } else if ("year" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 366);
        } else {
            HKU_THROW("Invalid adjust_mode: {}!", adjust_mode);
        }

    } else if ("trace" == name) {
        if (getParam<bool>("trace") && pythonInJupyter()) {
            HKU_THROW("You can't trace in jupyter!");
        }
    }
}

void Portfolio::paramChanged() {
    m_need_calculate = true;
}

void Portfolio::reset() {
    if (m_tm)
        m_tm->reset();
    if (m_cash_tm)
        m_cash_tm->reset();
    if (m_se)
        m_se->reset();
    if (m_af)
        m_af->reset();
    m_need_calculate = true;
    m_real_sys_list.clear();
    m_running_sys_set.clear();
    _reset();
}

PortfolioPtr Portfolio::clone() const {
    PortfolioPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_query = m_query;
    p->m_need_calculate = true;
    if (m_se)
        p->m_se = m_se->clone();
    if (m_af)
        p->m_af = m_af->clone();
    if (m_tm)
        p->m_tm = m_tm->clone();
    if (m_cash_tm)
        p->m_cash_tm = m_cash_tm->clone();
    return p;
}

void Portfolio::readyForRun() {
    HKU_CHECK(m_se, "m_se is null!");
    HKU_CHECK(m_tm, "m_tm is null!");
    reset();
    _readyForRun();
}

void Portfolio::runMoment(const Datetime& date, const Datetime& nextCycle, bool adjust) {
    // 当前日期小于账户建立日期，直接忽略
    HKU_IF_RETURN(date < m_tm->initDatetime(), void());

    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "{} ===========================================================", date);
    if (trace && adjust) {
        HKU_INFO("****************************************************");
        HKU_INFO("**                                                **");
        HKU_INFO("**  [PF] Position adjustment will be made today.  **");
        HKU_INFO("**                                                **");
        HKU_INFO("****************************************************");
    }
    HKU_INFO_IF(trace, "[PF] current running system size: {}", m_running_sys_set.size());

    // 开盘前，调整账户权息
    m_tm->updateWithWeight(date);

    _runMoment(date, nextCycle, adjust);

    //----------------------------------------------------------------------
    // 跟踪打印各运行中子系统持仓情况
    //----------------------------------------------------------------------
    traceMomentTM(date);

    // 跟踪打印当前账户资产
    if (trace) {
        FundsRecord funds = m_tm->getFunds(date, m_query.kType());
        HKU_INFO("[PF] total asset: {:.2f}, current cash: {:<.2f}, market value: {:<.2f}",
                 funds.total_assets(), funds.cash, funds.market_value);
    }
}

void Portfolio::run(const KQuery& query, bool force) {
    SPEND_TIME(Portfolio_run);

    int adjust_cycle = getParam<int>("adjust_cycle");
    string mode = getParam<string>("adjust_mode");
    bool delay_to_trading_day = getParam<bool>("delay_to_trading_day");
    to_lower(mode);
    if (mode != "query") {
        HKU_CHECK(query.kType() == KQuery::DAY,
                  R"(The kType of query({}) must be DAY when adjust_mode is not "query"!)",
                  query.kType());
    }

    setQuery(query);

    if (force) {
        m_need_calculate = true;
    }
    HKU_IF_RETURN(!m_need_calculate, void());

    _readyForRun();

    if (m_real_sys_list.empty()) {
        HKU_WARN("There is no system in portfolio!");
        m_need_calculate = true;
        return;
    }

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    HKU_IF_RETURN(datelist.empty(), void());

    if ("query" == mode || "day" == mode) {
        size_t cur_adjust_ix = 0;
        Datetime cur_cycle_end;
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            bool adjust = false;
            if (i == cur_adjust_ix) {
                adjust = true;
                cur_adjust_ix += adjust_cycle;
                cur_cycle_end =
                  cur_adjust_ix < total ? datelist[cur_adjust_ix] : datelist.back() + Seconds(1);
            }

            const auto& date = datelist[i];
            runMoment(date, cur_cycle_end, adjust);
        }

    } else if (delay_to_trading_day) {
        _runOnModeDelayToTradingDay(datelist, adjust_cycle, mode);
    } else {
        _runOnMode(datelist, adjust_cycle, mode);
    }

    m_need_calculate = false;
}

void Portfolio::_runOnMode(const DatetimeList& datelist, int adjust_cycle, const string& mode) {
    if ("week" == mode) {
        Datetime cur_cycle_end = datelist.front().nextWeek();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            bool adjust = (date.dayOfWeek() == adjust_cycle);
            if (adjust) {
                cur_cycle_end = date.nextWeek();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }
            runMoment(date, cur_cycle_end, adjust);
        }
    } else if ("month" == mode) {
        Datetime cur_cycle_end = datelist.front().nextMonth();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            bool adjust = (date.day() == adjust_cycle);
            if (adjust) {
                cur_cycle_end = date.nextMonth();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }
            runMoment(date, cur_cycle_end, adjust);
        }
    } else if ("quarter" == mode) {
        Datetime cur_cycle_end = datelist.front().nextQuarter();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            bool adjust = (date.day() == adjust_cycle);
            if (adjust) {
                cur_cycle_end = date.nextQuarter();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }
            runMoment(date, cur_cycle_end, adjust);
        }
    } else if ("year" == mode) {
        Datetime cur_cycle_end = datelist.front().nextYear();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            bool adjust = (date.dayOfYear() == adjust_cycle);
            if (adjust) {
                cur_cycle_end = date.nextYear();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }
            runMoment(date, cur_cycle_end, adjust);
        }
    }
}

void Portfolio::_runOnModeDelayToTradingDay(const DatetimeList& datelist, int adjust_cycle,
                                            const string& mode) {
    std::set<Datetime> adjust_date_set;
    if ("week" == mode) {
        Datetime cur_cycle_end = datelist.front().nextWeek();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            Datetime adjust_date = date.startOfWeek() + Days(adjust_cycle - 1);
            bool adjust = false;
            if (date == adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                       date > adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            }

            if (adjust) {
                cur_cycle_end = date.nextWeek();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }

            runMoment(date, cur_cycle_end, adjust);
        }

    } else if ("month" == mode) {
        Datetime cur_cycle_end = datelist.front().nextMonth();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            Datetime adjust_date = date.startOfMonth() + Days(adjust_cycle - 1);
            bool adjust = false;
            if (date == adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                       date > adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            }

            if (adjust) {
                cur_cycle_end = date.nextMonth();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }

            runMoment(date, cur_cycle_end, adjust);
        }

    } else if ("quarter" == mode) {
        Datetime cur_cycle_end = datelist.front().nextQuarter();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            Datetime adjust_date = date.startOfQuarter() + Days(adjust_cycle - 1);
            bool adjust = false;
            if (date == adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                       date > adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            }

            if (adjust) {
                cur_cycle_end = date.nextQuarter();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }

            runMoment(date, cur_cycle_end, adjust);
        }

    } else if ("year" == mode) {
        Datetime cur_cycle_end = datelist.front().nextYear();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            Datetime adjust_date = date.startOfYear() + Days(adjust_cycle - 1);
            bool adjust = false;
            if (date == adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                       date > adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            }

            if (adjust) {
                cur_cycle_end = date.nextYear();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }

            runMoment(date, cur_cycle_end, adjust);
        }
    }
}

void Portfolio::traceMomentTM(const Datetime& date) {
    HKU_IF_RETURN(!getParam<bool>("trace") || m_running_sys_set.empty(), void());

    //----------------------------------------------------------------------
    // 跟踪打印持仓情况
    //----------------------------------------------------------------------
    // clang-format off
    HKU_INFO("+------------+------------+------------+--------------+--------------+-------------+-------------+");
    HKU_INFO("| code       | name       | position   | market value | remain cash  | open price  | close price  |");
    HKU_INFO("+------------+------------+------------+--------------+--------------+-------------+-------------+");
    // clang-format on

    size_t count = 0;
    for (const auto& sys : m_running_sys_set) {
        Stock stk = sys->getStock();
        auto funds = sys->getTM()->getFunds(date, m_query.kType());
        size_t position = sys->getTM()->getHoldNumber(date, stk);
        KRecord krecord = stk.getKRecord(date, m_query.kType());
        auto stk_name = stk.name();
        HKU_INFO("| {:<11}| {:<11}| {:<11}| {:<13.2f}| {:<13.2f}| {:<12.2f}| {:<12.2f}|",
                 stk.market_code(), stk_name, position, funds.market_value, funds.cash,
                 krecord.openPrice, krecord.closePrice);
        // clang-format off
        HKU_INFO("+------------+------------+------------+--------------+--------------+-------------+-------------+");
        count++;
        int trace_max_num = getParam<int>("trace_max_num");
        if (count >= trace_max_num) {
            if (m_running_sys_set.size() > trace_max_num) {
                HKU_INFO("+ ... ... more                                                                                   +");
                HKU_INFO("+------------+------------+------------+--------------+--------------+-------------+-------------+");
            }
            break;
        }
        // clang-format on
    }
}

} /* namespace hku */
