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
    setParam<bool>("se_trace", false);

    // When the currently selected system differs from the previous one, clear the existing position
    // at the open
    setParam<bool>("clean_hold_when_select_changed", true);
}

void WalkForwardSystem::_checkParam(const string& name) const {
    if ("train_len" == name) {
        HKU_ASSERT(getParam<int>("train_len") > 0);
    } else if ("test_len" == name) {
        HKU_ASSERT(getParam<int>("test_len") > 0);
    } else if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = StockManager::instance().getMarketInfo(market);
        HKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    }
}

void WalkForwardSystem::_reset() {
    m_cur_window = Null<size_t>();
    m_cur_sys.reset();
    m_se->reset();
    if (m_train_tm) {
        m_train_tm->reset();
    }

    m_train_kdata_list.clear();
    m_run_ranges.clear();
    m_trade_list.clear();
    m_buyRequestList.clear();
    m_sellRequestList.clear();
    m_sellShortRequestList.clear();
    m_buyShortRequestList.clear();
}

void WalkForwardSystem::_forceResetAll() {
    this->_reset();
}

SystemPtr WalkForwardSystem::_clone() {
    auto p = make_shared<WalkForwardSystem>();
    p->m_se = m_se->clone();
    p->m_se->reset();
    if (m_train_tm) {
        p->m_train_tm = m_train_tm->clone();
    }
    return p;
}

void WalkForwardSystem::syncDataFromSystem(const SYSPtr& sys, bool isMoment) {
    if (!isMoment) {
        const auto& sys_trade_list = sys->getTradeRecordList();
        for (const auto& tr : sys_trade_list) {
            m_trade_list.emplace_back(tr);
        }
    }

    m_buyRequestList = sys->getBuyTradeRequestList();
    m_sellRequestList = sys->getSellTradeRequestList();
    m_sellShortRequestList = sys->getSellShortTradeRequestList();
    m_buyShortRequestList = sys->getBuyShortTradeRequestList();

    // m_pre_ev_valid = sys->m_pre_ev_valid;
    // m_pre_cn_valid = sys->m_pre_cn_valid;

    m_lastTakeProfit = sys->m_lastTakeProfit;
    m_lastShortTakeProfit = sys->m_lastShortTakeProfit;
}

void WalkForwardSystem::syncDataToSystem(const SYSPtr& sys) {
    sys->m_buyRequestList = m_buyRequestList;
    sys->m_sellRequestList = m_sellRequestList;
    sys->m_sellShortRequestList = m_sellShortRequestList;
    sys->m_buyShortRequestList = m_buyShortRequestList;

    // sys->m_pre_ev_valid = m_pre_ev_valid;
    // sys->m_pre_cn_valid = m_pre_cn_valid;

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

void WalkForwardSystem::ensureWindows() {
    HKU_CHECK(m_se, "WalkForwardSystem needs an SE! [{}]", name());
    OptimalSelectorBase* se_ptr = dynamic_cast<OptimalSelectorBase*>(m_se.get());
    HKU_CHECK(se_ptr, "WalkForwardSystem only supports OptimalSelectorBase! [{}]", name());

    const auto& ranges = se_ptr->getRunRanges();
    if (ranges.size() == m_run_ranges.size() && m_train_kdata_list.size() == ranges.size()) {
        bool same = true;
        for (size_t i = 0; i < ranges.size(); ++i) {
            if (ranges[i].start != m_run_ranges[i].start ||
                ranges[i].run_start != m_run_ranges[i].run_start ||
                ranges[i].end != m_run_ranges[i].end) {
                same = false;
                break;
            }
        }
        if (same) {
            return;
        }
    }
    m_run_ranges = ranges;
    m_train_kdata_list.assign(ranges.size(), KData());
    m_cur_window = Null<size_t>();
    // m_cur_sys is kept: it is the clean-hold baseline, so a rebuilt table with the same
    // restored candidate is not misjudged as a switch
}

KData WalkForwardSystem::windowKDataAt(size_t index) {
    HKU_ASSERT(index < m_train_kdata_list.size());
    if (m_train_kdata_list[index].empty()) {
        const KQuery& query = m_kdata.getQuery();
        m_train_kdata_list[index] = m_stock.getKData(KQueryByDate(
          m_run_ranges[index].start, m_run_ranges[index].end, query.kType(), query.recoverType()));
    }
    return m_train_kdata_list[index];
}

size_t WalkForwardSystem::windowIndexAt(const Datetime& datetime) const {
    // The run windows are contiguous and [run_start, end) is end-exclusive; a datetime in a
    // training period (before the first run_start) or after the last end belongs to no window
    size_t lo = 0, hi = m_run_ranges.size();
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (m_run_ranges[mid].run_start <= datetime) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    if (lo == 0) {
        return Null<size_t>();
    }
    size_t index = lo - 1;
    return datetime >= m_run_ranges[index].end ? Null<size_t>() : index;
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

    m_run_ranges.clear();  // a full run always re-reads the windows and window data
    ensureWindows();
    size_t run_ranges_len = m_run_ranges.size();
    HKU_IF_RETURN(run_ranges_len == 0, void());

    const Stock& stock = kdata.getStock();
    bool trace = getParam<bool>("trace");
    SYSPtr pre_selected_sys;
    for (size_t i = 0, total = run_ranges_len; i < total; i++) {
        KData run_kdata = windowKDataAt(i);
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
              "\n|  sys score: {}"
              "\n|"
              "\n+=======================================================================\n",
              name(), i + 1, total, m_run_ranges[i].run_start,
              run_kdata[run_kdata.size() - 1].datetime, sys->name(), sw_list.front().weight);

            if (getParam<bool>("clean_hold_when_select_changed")) {
                if (sys != pre_selected_sys && m_tm->have(m_stock)) {
                    const KRecord* kr = run_kdata.data();
                    const KRecord* kr_end = kr + run_kdata.size();
                    KRecord temp;
                    temp.datetime = m_run_ranges[i].run_start;
                    const auto* it = std::lower_bound(
                      kr, kr_end, temp,
                      [](const KRecord& a, const KRecord& b) { return a.datetime < b.datetime; });
                    if (it != kr_end) {
                        KRecord src_kr = stock.getKRecord(it->datetime);
                        if (src_kr.openPrice > 0.0) {
                            auto tr =
                              m_tm->sell(src_kr.datetime, m_stock, src_kr.openPrice, MAX_DOUBLE);
                            if (!tr.isNull()) {
                                CLS_INFO_IF(trace, "clean_hold_when_select_changed on open: {}",
                                            tr);
                                m_trade_list.push_back(tr);
                            }
                        } else {
                            CLS_INFO_IF(
                              trace, "clean_hold_when_select_changed skipped: no open price at {}.",
                              src_kr.datetime);
                        }
                    }
                }
            }

            sys->setParam<bool>("shared_tm", true);
            sys->setParam<bool>("trace", trace);
            sys->setTM(crtWalkForwardTM(getTM(), m_run_ranges[i].run_start));
            syncDataToSystem(sys);
            sys->run(run_kdata, false, false);
            syncDataFromSystem(sys, false);
            pre_selected_sys = sys;

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

            if (getParam<bool>("clean_hold_when_select_changed")) {
                if (m_tm->have(m_stock)) {
                    const KRecord* kr = run_kdata.data();
                    const KRecord* kr_end = kr + run_kdata.size();
                    KRecord temp;
                    temp.datetime = m_run_ranges[i].run_start;
                    const auto* it = std::lower_bound(
                      kr, kr_end, temp,
                      [](const KRecord& a, const KRecord& b) { return a.datetime < b.datetime; });
                    if (it != kr_end) {
                        KRecord src_kr = stock.getKRecord(it->datetime);
                        if (src_kr.openPrice > 0.0) {
                            auto tr =
                              m_tm->sell(src_kr.datetime, m_stock, src_kr.openPrice, MAX_DOUBLE);
                            if (!tr.isNull()) {
                                CLS_INFO_IF(trace, "clean_hold_when_select_changed on open: {}",
                                            tr);
                                m_trade_list.push_back(tr);
                            }
                        } else {
                            CLS_INFO_IF(
                              trace, "clean_hold_when_select_changed skipped: no open price at {}.",
                              src_kr.datetime);
                        }
                    }
                }
            }
        }
    }

    m_calculated = true;
}

MomentResult WalkForwardSystem::driveMoment(const Datetime& datetime, bool open, bool close) {
    MomentResult ret;
    ret.datetime = datetime;
    ensureWindows();
    HKU_CHECK(m_tm, "Not TradeManager was specified! {}", name());
    HKU_CHECK(!m_run_ranges.empty(),
              "The WalkForwardSystem has no windows: the SE was never calculated (call run or "
              "drive it through a strategy first)! [{}]",
              name());

    const bool composite = open && close;
    KQuery::KType ktype = m_kdata.getQuery().kType();
    if (composite) {
        ret.funds_before_open = m_tm->getFunds(datetime, ktype);
    }

    size_t idx = windowIndexAt(datetime);
    if (idx == Null<size_t>()) {
        // A training period or a past-the-end moment drives nothing; keep the current candidate
        // as the clean-hold baseline so a restored candidate is not misjudged as a switch
        m_cur_window = idx;
        return ret;
    }

    if (idx != m_cur_window) {
        m_cur_window = idx;
        auto sw_list = m_se->getSelected(m_run_ranges[idx].run_start);
        SYSPtr new_sys = sw_list.empty() ? SYSPtr() : sw_list.front().sys;
        if (new_sys != m_cur_sys && new_sys && getParam<bool>("clean_hold_when_select_changed") &&
            m_tm->have(m_stock)) {
            price_t open_price = srcOpenPriceAt(datetime);
            if (open_price > 0.0) {
                TradeRecord tr = m_tm->sell(datetime, m_stock, open_price, MAX_DOUBLE);
                if (!tr.isNull()) {
                    ret.tradesOnOpen.push_back(tr);
                    m_trade_list.push_back(tr);
                }
            } else {
                HKU_WARN("clean_hold_when_select_changed skipped, {} has no open price at {}!",
                         m_stock.market_code(), datetime);
            }
        }
        m_cur_sys = new_sys;
        if (m_cur_sys) {
            m_cur_sys->setParam<bool>("shared_tm", true);
            m_cur_sys->setParam<bool>("trace", getParam<bool>("trace"));
            m_cur_sys->setTM(crtWalkForwardTM(getTM(), m_run_ranges[idx].run_start));
            m_cur_sys->readyForRun();
            m_cur_sys->setTO(windowKDataAt(idx));
            syncDataToSystem(m_cur_sys);
        }
    }

    if (m_cur_sys) {
        if (open) {
            MomentResult r = m_cur_sys->runMomentOnOpen(datetime);
            for (auto& tr : r.tradesOnOpen) {
                if (!tr.isNull()) {
                    ret.tradesOnOpen.push_back(tr);
                    m_trade_list.push_back(tr);
                }
            }
            syncDataFromSystem(m_cur_sys, true);
        }
        if (close) {
            if (composite) {
                ret.funds_before_close = m_tm->getFunds(datetime, ktype);
            }
            MomentResult r = m_cur_sys->runMomentOnClose(datetime);
            for (auto& tr : r.tradesOnClose) {
                if (!tr.isNull()) {
                    ret.tradesOnClose.push_back(tr);
                    m_trade_list.push_back(tr);
                }
            }
            ret.funds = m_tm->getFunds(datetime, ktype);
            syncDataFromSystem(m_cur_sys, true);
            if (composite) {
                PositionRecord position = m_tm->getPosition(datetime, m_stock);
                if (position.number > 0.0) {
                    ret.positions.push_back(position);
                }
                PositionRecord short_position = m_tm->getShortPosition(m_stock);
                if (short_position.number > 0.0) {
                    ret.positions.push_back(short_position);
                }
                for (const auto& req : m_buyRequestList) {
                    if (req.valid) {
                        ret.delayOnNextOpen.push_back(req);
                    }
                }
                for (const auto& req : m_sellRequestList) {
                    if (req.valid) {
                        ret.delayOnNextOpen.push_back(req);
                    }
                }
                for (const auto& req : m_sellShortRequestList) {
                    if (req.valid) {
                        ret.delayOnNextOpen.push_back(req);
                    }
                }
                for (const auto& req : m_buyShortRequestList) {
                    if (req.valid) {
                        ret.delayOnNextOpen.push_back(req);
                    }
                }
            }
        }
    }
    return ret;
}

price_t WalkForwardSystem::srcOpenPriceAt(const Datetime& datetime) {
    Stock stock = m_stock;
    if (stock.isNull()) {
        stock = m_kdata.getStock();
    }
    KRecord src = stock.getKRecord(datetime.startOfDay());
    return src.openPrice > 0.0 ? src.openPrice : 0.0;
}

MomentResult WalkForwardSystem::runMoment(const Datetime& datetime) {
    return driveMoment(datetime, true, true);
}

MomentResult WalkForwardSystem::runMomentOnOpen(const Datetime& datetime) {
    return driveMoment(datetime, true, false);
}

MomentResult WalkForwardSystem::runMomentOnClose(const Datetime& datetime) {
    return driveMoment(datetime, false, true);
}

bool WalkForwardSystem::hasPythonPart() const {
    return System::hasPythonPart() || (m_se && m_se->hasPythonPart()) ||
           (m_train_tm && m_train_tm->hasPythonPart()) || (m_cur_sys && m_cur_sys->hasPythonPart());
}

TradeRecord WalkForwardSystem::sellForceOnOpen(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    if (m_cur_sys) {
        ret = m_cur_sys->sellForceOnOpen(date, num, from);
        if (!ret.isNull()) {
            m_trade_list.push_back(ret);
        }
        syncDataFromSystem(m_cur_sys, true);
    }
    return ret;
}

TradeRecord WalkForwardSystem::sellForceOnClose(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    if (m_cur_sys) {
        ret = m_cur_sys->sellForceOnClose(date, num, from);
        if (!ret.isNull()) {
            m_trade_list.push_back(ret);
        }
        syncDataFromSystem(m_cur_sys, true);
    }
    return ret;
}

void WalkForwardSystem::clearDelayBuyRequest() {
    if (m_cur_sys) {
        m_cur_sys->clearDelayBuyRequest();
        m_buyRequestList.clear();
    }
}

TradeRecord WalkForwardSystem::pfProcessDelaySellRequest(const Datetime& date) {
    TradeRecord ret;
    if (m_cur_sys) {
        ret = m_cur_sys->pfProcessDelaySellRequest(date);
        if (!ret.isNull()) {
            m_trade_list.push_back(ret);
        }
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