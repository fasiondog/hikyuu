/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "hikyuu/trade_sys/selector/crt/SE_Optimal.h"
#include "hikyuu/trade_sys/selector/imp/optimal/OptimalSelectorBase.h"
#include "hikyuu/trade_manage/Performance.h"
#include "WalkForwardSystem.h"
#include "WalkForwardTradeManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::WalkForwardSystem)
#endif

namespace hku {

string WalkForwardSystem::str() const {
    std::ostringstream os;
    string strip(",\n");
    string space("  ");
    TMPtr train_tm = m_train_tm ? m_train_tm : m_tm;
    string train_tm_str("Train TradeManager(NULL)");
    if (train_tm) {
        train_tm_str = fmt::format("Train TradeManager(init date: {}, init cash: {})",
                                   train_tm->initDatetime(), train_tm->initCash());
    }
    os << "System{\n"
       << space << name() << strip << space << getTO().getQuery() << strip << space << getStock()
       << strip << space << getParameter() << strip << space << m_se << strip << space
       << train_tm_str << strip << space << getEV() << strip << space << getCN() << strip << space
       << getMM() << strip << space << getSG() << strip << space << getST() << strip << space
       << getTP() << strip << space << getPG() << strip << space << getSP() << strip << space
       << (getTM() ? getTM()->str() : "TradeManager(NULL)") << strip << "}";
    return os.str();
}

WalkForwardSystem::WalkForwardSystem() : System("SYS_WalkForward"), m_se(SE_MaxFundsOptimal()) {
    initParam();
}

WalkForwardSystem::WalkForwardSystem(const SystemList& candidate_sys_list, const SelectorPtr& se,
                                     const TradeManagerPtr& train_tm)
: System("SYS_WalkForward"), m_train_tm(train_tm) {
    CLS_ASSERT(!candidate_sys_list.empty());
    CLS_ASSERT(se);

    OptimalSelectorBase const* se_ptr = dynamic_cast<OptimalSelectorBase*>(se.get());
    CLS_CHECK(se_ptr != nullptr, "Only the OptimalSelectorBase type is accepted!");

    initParam();
    m_se = se;
    m_se->setParam<int>("train_len", getParam<int>("train_len"));
    m_se->setParam<int>("test_len", getParam<int>("test_len"));
    m_se->addSystemList(candidate_sys_list);
}

void WalkForwardSystem::initParam() {
    setParam<string>("market", "SH");
    setParam<int>("train_len", 100);
    setParam<int>("test_len", 20);
    setParam<bool>("parallel", false);
    setParam<bool>("se_trace", false);
}

void WalkForwardSystem::_checkParam(const string& name) const {
    if ("train_len" == name) {
        HKU_ASSERT(getParam<int>("train_len") > 0);
    } else if ("test_len" == name) {
        HKU_ASSERT(getParam<int>("test_len") > 0);
    } else if ("depend_on_proto_sys" == name) {
        HKU_ASSERT(getParam<bool>("depend_on_proto_sys"));
    } else if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = StockManager::instance().getMarketInfo(market);
        HKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    }
}

void WalkForwardSystem::_reset() {
    m_cur_kdata = 0;
    m_cur_sys.reset();
    m_se->reset();
    if (m_train_tm) {
        m_train_tm->reset();
    }

    m_trade_list.clear();
    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();
}

void WalkForwardSystem::_forceResetAll() {
    this->_reset();
}

SystemPtr WalkForwardSystem::_clone() {
    WalkForwardSystem* p = new WalkForwardSystem();
    p->m_se = m_se->clone();
    p->m_se->reset();
    if (m_train_tm) {
        p->m_train_tm = m_train_tm->clone();
    }
    return SystemPtr(p);
}

void WalkForwardSystem::syncDataFromSystem(const SYSPtr& sys, bool isMoment) {
    if (!isMoment) {
        const auto& sys_trade_list = sys->getTradeRecordList();
        for (const auto& tr : sys_trade_list) {
            m_trade_list.emplace_back(tr);
        }
    }

    m_buyRequest = sys->getBuyTradeRequest();
    m_sellRequest = sys->getSellTradeRequest();
    m_sellShortRequest = sys->getSellShortTradeRequest();
    m_buyShortRequest = sys->getBuyShortTradeRequest();

    // m_pre_ev_valid = sys->m_pre_ev_valid;
    // m_pre_cn_valid = sys->m_pre_cn_valid;

    m_buy_days = sys->m_buy_days;
    m_sell_short_days = sys->m_sell_short_days;
    m_lastTakeProfit = sys->m_lastTakeProfit;
    m_lastShortTakeProfit = sys->m_lastShortTakeProfit;
}

void WalkForwardSystem::syncDataToSystem(const SYSPtr& sys) {
    sys->m_buyRequest = m_buyRequest;
    sys->m_sellRequest = m_sellRequest;
    sys->m_sellShortRequest = m_sellShortRequest;
    sys->m_buyShortRequest = m_buyShortRequest;

    // sys->m_pre_ev_valid = m_pre_ev_valid;
    // sys->m_pre_cn_valid = m_pre_cn_valid;

    sys->m_buy_days = m_buy_days;
    sys->m_sell_short_days = m_sell_short_days;
    sys->m_lastTakeProfit = m_lastTakeProfit;
    sys->m_lastShortTakeProfit = m_lastShortTakeProfit;
}

void WalkForwardSystem::readyForRun() {
    CLS_CHECK(m_tm, "Not TradeManager was specified! {}", name());
    if (!m_train_tm) {
        m_train_tm = m_tm->clone();
    }

    m_se->setParam<string>("market", getParam<string>("market"));
    m_se->setParam<int>("train_len", getParam<int>("train_len"));
    m_se->setParam<int>("test_len", getParam<int>("test_len"));
    m_se->setParam<bool>("parallel", getParam<bool>("parallel"));
    m_se->setParam<bool>("trace", getParam<bool>("se_trace"));

    m_se->reset();
    const auto& candidate_sys_list = m_se->getProtoSystemList();
    CLS_CHECK(!candidate_sys_list.empty(), "Candidate sys list is empty!");

    for (auto& sys : candidate_sys_list) {
        sys->setTM(m_train_tm->clone());
        sys->setStock(m_stock);
    }
    m_se->calculate(SystemList(), m_kdata.getQuery());
}

void WalkForwardSystem::run(const KData& kdata, bool reset, bool resetAll) {
    SPEND_TIME(WalkForwardSystem_run);
    HKU_IF_RETURN(kdata.empty(), void());
    if (resetAll) {
        this->forceResetAll();
    } else if (reset) {
        this->reset();
    }

    CLS_DEBUG_IF_RETURN(m_calculated && m_kdata == kdata, void(), "Not need calculate.");

    m_kdata = kdata;
    m_stock = m_kdata.getStock();
    readyForRun();

    OptimalSelectorBase* se_ptr = dynamic_cast<OptimalSelectorBase*>(m_se.get());
    m_run_ranges = se_ptr->getRunRanges();
    size_t run_ranges_len = m_run_ranges.size();
    HKU_IF_RETURN(run_ranges_len == 0, void());

    const KQuery& query = kdata.getQuery();
    const Stock& stock = kdata.getStock();

    for (size_t i = 0; i < run_ranges_len; i++) {
        m_train_kdata_list.emplace_back(stock.getKData(KQueryByDate(
          m_run_ranges[i].start, m_run_ranges[i].end, query.kType(), query.recoverType())));
    }

    bool trace = getParam<bool>("trace");
    for (size_t i = 0, total = m_train_kdata_list.size(); i < total; i++) {
        const auto& run_kdata = m_train_kdata_list[i];
        if (run_kdata.empty()) {
            CLS_INFO_IF(
              trace,
              "\n+======================================================================="
              "\n|"
              "\n|  name: {} "
              "\n|  iteration {}|{}"
              "\n|  ignore, this iteration kdata is empty."
              "\n|"
              "\n+=======================================================================\n",
              name(), i + 1, total);
            continue;
        }

        auto sw_list = m_se->getSelected(m_run_ranges[i].run_start);
        if (!sw_list.empty()) {
            auto& sys = sw_list.front().sys;
            CLS_INFO_IF(
              trace,
              "\n+======================================================================="
              "\n|"
              "\n|  name: {} "
              "\n|  iteration {}|{}: {}, {}"
              "\n|  use sys: {}"
              "\n|"
              "\n+=======================================================================\n",
              name(), i + 1, total, m_run_ranges[i].run_start,
              run_kdata[run_kdata.size() - 1].datetime, sys->name());

            sys->setParam<bool>("shared_tm", true);
            sys->setParam<bool>("trace", trace);
            sys->setTM(crtWalkForwardTM(getTM(), m_run_ranges[i].run_start));
            syncDataToSystem(sys);
            sys->run(run_kdata, false, false);
            syncDataFromSystem(sys, false);

        } else {
            CLS_INFO_IF(trace,
                        "\n+======================================================================="
                        "\n|"
                        "\n|  name: {} "
                        "\n|  iteration {}|{}"
                        "\n|  ignore no selected sys."
                        "\n|"
                        "\n+======================================================================="
                        "\n",
                        name(), i + 1, total);
        }
    }

    m_calculated = true;
}

TradeRecord WalkForwardSystem::runMoment(const Datetime& datetime) {
    TradeRecord ret;
    auto sw_list = m_se->getSelected(datetime);
    HKU_IF_RETURN(sw_list.empty(), ret);

    const auto& sys = sw_list.front().sys;
    if (sys && sys != m_cur_sys) {
        m_cur_kdata++;
        if (m_cur_kdata < m_train_kdata_list.size()) {
            m_cur_sys = sys;
            m_cur_sys->setParam<bool>("shared_tm", true);
            m_cur_sys->setParam<bool>("trace", getParam<bool>("trace"));
            m_cur_sys->setTM(crtWalkForwardTM(getTM(), m_run_ranges[m_cur_kdata].run_start));
            m_cur_sys->readyForRun();
            m_cur_sys->setTO(m_train_kdata_list[m_cur_kdata]);
            syncDataToSystem(m_cur_sys);
        } else {
            m_cur_sys.reset();
        }
    }

    if (m_cur_sys) {
        ret = m_cur_sys->runMoment(datetime);
        m_trade_list.push_back(ret);
        syncDataFromSystem(m_cur_sys, true);
    }
    return ret;
}

TradeRecord WalkForwardSystem::sellForceOnOpen(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    if (m_cur_sys) {
        ret = m_cur_sys->sellForceOnOpen(date, num, from);
        m_trade_list.push_back(ret);
        syncDataFromSystem(m_cur_sys, true);
    }
    return ret;
}

TradeRecord WalkForwardSystem::sellForceOnClose(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    if (m_cur_sys) {
        ret = m_cur_sys->sellForceOnClose(date, num, from);
        m_trade_list.push_back(ret);
        syncDataFromSystem(m_cur_sys, true);
    }
    return ret;
}

void WalkForwardSystem::clearDelayBuyRequest() {
    if (m_cur_sys) {
        m_cur_sys->clearDelayBuyRequest();
        m_buyRequest.clear();
    }
}

bool WalkForwardSystem::haveDelaySellRequest() const {
    return m_sellRequest.valid;
}

TradeRecord WalkForwardSystem::pfProcessDelaySellRequest(const Datetime& date) {
    TradeRecord ret;
    if (m_cur_sys) {
        ret = m_cur_sys->pfProcessDelaySellRequest(date);
        m_trade_list.push_back(ret);
        syncDataFromSystem(m_cur_sys, true);
    }
    return ret;
}

SystemPtr HKU_API SYS_WalkForward(const SystemList& candidate_sys_list, const TradeManagerPtr& tm,
                                  size_t train_len, size_t test_len, const SelectorPtr& se,
                                  const TradeManagerPtr& train_tm) {
    HKU_CHECK(tm, "Input tm is null!");
    TradeManagerPtr new_train_tm = train_tm ? train_tm : tm->clone();
    SystemPtr ret = make_shared<WalkForwardSystem>(candidate_sys_list, se, new_train_tm);
    ret->setTM(tm);
    ret->setParam<int>("train_len", train_len);
    ret->setParam<int>("test_len", test_len);
    return ret;
}

}  // namespace hku