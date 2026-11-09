# -*- coding: utf-8 -*-
# cp936
"""Multi-market end-to-end assertion script

Coverage: market/type registration, kdata config, trading calendar, dimension
normalization, futures long/short backtest, tz truth source and day look-back, stock list
loading, portfolio calendar, runtime variety-parameter override, SQLite same-code
isolation, dimension ratios; the remaining assertions are covered by C++ doctest (a fully
green unit-test proves it).
Follows the zero-diagnosis convention: only failures and funds snapshots are printed; on
full pass only ACCEPTANCE PASS is printed (over-quota short open, lot size and expiry
rejection each leave one expected warning).

Usage (repo root): python hikyuu/test/tools/multimarket_acceptance.py
"""

import configparser
import os
import pickle
import subprocess
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", ".."))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

FAIL = []
SNAPSHOT = []


def check(item, ok, msg=""):
    if not ok:
        FAIL.append("%s: %s" % (item, msg))


def run_acceptance():
    from hikyuu import (Datetime, MA, Query, SYS_Simple, SG_Cross, MM_FixedCount, TC_Zero,
                        MG_Fixed, crtTM, crtAF, MultiSystem, BUSINESS)
    from test_init import sm

    ini = configparser.ConfigParser()
    ini.read("test_data/hikyuu_win.ini", encoding="utf-8")

    # registration takes effect
    mi = sm.get_market_info("SHF")
    check("register", mi.market == "SHF", "market info of SHF not found")
    stk = sm["SHFRB0000"]
    check("register", not stk.is_null(), "SHFRB0000 not loaded")
    ti = sm.get_stock_type_info(12)
    check("register", ti.type == 12, "stock type 12 not registered")

    # kdata config takes effect (time key maps TIMELINE; trans key maps TRANS)
    for key in ini.options("kdata"):
        if key == "type" or "_" not in key:
            continue
        market, suffix = key.split("_", 1)
        ktype = {"time": "TIMELINE"}.get(suffix, suffix.upper())
        idx = sm.get_market_stock(market)
        if idx.is_null():
            FAIL.append("kdata_config: %s has no market stock" % market)
            continue
        k = idx.get_kdata(Query(0, idx.get_count(ktype), ktype))
        if len(k) == 0:
            FAIL.append("kdata_config: %s %s empty" % (market, key))

    # trading calendar (markets with kdata configured; window from the middle of each data)
    for market in ["SH", "SZ", "SHF", "CZC", "CFE", "DCE", "INE", "GFE", "HK", "NASDAQ", "NYSE"]:
        idx = sm.get_market_stock(market)
        total = idx.get_count("DAY")
        mid = idx.get_kdata(Query(max(0, total - 120), total - 20, "DAY"))
        if len(mid) == 0:
            FAIL.append("calendar: %s no day data" % market)
            continue
        cal = sm.get_trading_calendar(Query(mid[0].datetime, mid[-1].datetime, "DAY"), market)
        if len(cal) == 0:
            FAIL.append("calendar: %s calendar empty" % market)
            continue
        dates = set(x.datetime.number for x in mid)
        for d in cal:
            if d.number not in dates:
                FAIL.append("calendar: %s calendar date %s not in index kdata" % (market, d))
                break

    # dimension normalization (SZ399001 minute details and day bar built by caliber)
    sz = sm["SZ399001"]
    day = sz.get_kdata(Query(Datetime(201112050000), Datetime(201112060000), "DAY"))
    m1 = sz.get_kdata(Query(Datetime(201112050000), Datetime(201112060000), "MIN"))
    m5 = sz.get_kdata(Query(Datetime(201112050000), Datetime(201112060000), "MIN5"))
    has_fixture = len(day) == 1 and len(m1) > 0 and len(m5) > 0
    check("caliber_fixture", has_fixture, "fixture kdata missing")
    if has_fixture:
        sum1a = sum(x.amount for x in m1)
        sum5a = sum(x.amount for x in m5)
        sum1c = sum(x.volume for x in m1)
        sum5c = sum(x.volume for x in m5)
        check("caliber_fixture", abs(sum1a / day[0].amount - 1.0) < 1e-9, "1min amount ratio != 1")
        check("caliber_fixture", abs(sum5a / day[0].amount - 1.0) < 1e-9, "5min amount ratio != 1")
        check("caliber_fixture", abs(sum1c / day[0].volume - 100.0) < 1e-9, "1min count ratio != 100")
        check("caliber_fixture", abs(sum5c / day[0].volume - 100.0) < 1e-9, "5min count ratio != 100")

    # futures long backtest: SG_Cross(MA5, MA10) + MM_FixedCount, snapshot and manual replay
    rb = sm["SHFRB0000"]
    unit = rb.unit
    q = Query(0, rb.get_count("DAY"), "DAY")
    rb_day = rb.get_kdata(q)
    tm = crtTM(Datetime(200001010000), 100000.0, TC_Zero())
    tm.set_param("support_borrow_cash", False)
    sys_ = SYS_Simple(tm=tm, sg=SG_Cross(MA(rb_day.close, 5), MA(rb_day.close, 10)),
                      mm=MM_FixedCount(100))
    sys_.run(rb, q)
    funds = tm.get_funds()
    # parameterless get_funds() values positions at last_datetime; long case net_assets = cash + market_value
    val_price = rb.get_market_value(tm.last_datetime, "DAY")
    replay_cash = 100000.0
    for tr in tm.get_trade_list():
        delta = tr.real_price * abs(tr.number) * unit
        if tr.business in (BUSINESS.BUY, BUSINESS.BUY_SHORT):
            replay_cash -= delta + tr.cost.total
        elif tr.business in (BUSINESS.SELL, BUSINESS.SELL_SHORT):
            replay_cash += delta - tr.cost.total
    pos_nominal = sum(pos.number * unit * val_price for pos in tm.get_position_list())
    manual = replay_cash + pos_nominal
    check("long_backtest", len(tm.get_trade_list()) > 0, "no trade in the long backtest")
    check("long_backtest", abs(funds.cash - replay_cash) < 1e-6,
          "cash %r != replay %r" % (funds.cash, replay_cash))
    check("long_backtest", abs(funds.net_assets - manual) < 1e-6,
          "net_assets %r != manual %r" % (funds.net_assets, manual))
    SNAPSHOT.append("long backtest: trades=%d net_assets=%.6f cash=%.6f "
                    "position_nominal=%.6f val_price=%.4f manual=%.6f" % (
                        len(tm.get_trade_list()), funds.net_assets, funds.cash,
                        pos_nominal, val_price, manual))

    # futures short: direct TM calls (no borrow on short open, over-margin second short rejected, cover releases)
    bar1 = rb.get_kdata(Query(100, 101, "DAY"))[0]
    bar2 = rb.get_kdata(Query(150, 151, "DAY"))[0]
    rate = 0.6
    tm2 = crtTM(bar1.datetime, 100000.0, TC_Zero())
    tm2.margin = MG_Fixed(rate, rate)
    tr1 = tm2.sell_short(bar1.datetime, rb, bar1.close, 10.0)
    check("short_direct", tr1.business == BUSINESS.SELL_SHORT, "short open rejected")
    check("short_direct", tm2.have_short(rb) and tm2.get_short_position(rb).number == 10.0,
          "short position not booked")
    check("short_direct", tm2.get_occupied_margin(bar2.datetime) > 0.0, "short occupies no margin")
    funds2 = tm2.get_funds()
    check("short_direct", funds2.borrow_asset == 0.0 and not tm2.get_borrow_stock_list(),
          "futures short created a borrow record")
    manual2 = 100000.0  # unrealized gain not booked: positions valued at last_datetime (= bar1 close)
    check("short_direct", abs(funds2.net_assets - manual2) < 1e-6,
          "short net_assets %r != manual %r" % (funds2.net_assets, manual2))
    avail = tm2.get_available_margin(bar2.datetime)
    n2 = max(1.0, int(avail / (rate * bar2.close * unit))) + 1.0
    tr2 = tm2.sell_short(bar2.datetime, rb, bar2.close, n2)
    check("short_direct", tr2.business == BUSINESS.INVALID, "over-quota second short accepted")
    cover = tm2.buy_short(bar2.datetime, rb, bar2.close, 10.0)
    check("short_direct", cover.business == BUSINESS.BUY_SHORT, "cover rejected")
    funds3 = tm2.get_funds()
    manual3 = 100000.0 + (bar1.close - bar2.close) * 10.0 * unit
    check("short_direct", not tm2.have_short(rb) and not tm2.get_short_position_list()
          and tm2.get_occupied_margin(bar2.datetime) == 0.0, "short not released")
    check("short_direct", abs(funds3.net_assets - manual3) < 1e-6,
          "covered net_assets %r != manual %r" % (funds3.net_assets, manual3))
    SNAPSHOT.append("short direct: open@%s p=%.4f net=%.6f | over-quota rejected | "
                    "cover@%s p=%.4f net=%.6f occupied=%.6f borrow_asset=%.1f" % (
                        bar1.datetime, bar1.close, funds2.net_assets, bar2.datetime, bar2.close,
                        funds3.net_assets, tm2.get_occupied_margin(bar2.datetime),
                        funds3.borrow_asset))

    # tz truth source + pickle roundtrip
    nasdaq = sm.get_market_info("NASDAQ")
    shf = sm.get_market_info("SHF")
    check("tz_source", nasdaq.tz_offset_min == -300 and nasdaq.dst_rule == 1 and nasdaq.data_lag == 1,
          "NASDAQ tz columns wrong")
    check("tz_source", shf.tz_offset_min == 480 and shf.dst_rule == 0 and shf.data_lag == 0,
          "SHF tz columns wrong")
    fx = sm.get_market_info("FX")
    check("tz_source", fx.tz_offset_min == 0 and fx.dst_rule == 0 and fx.data_lag == 1,
          "FX tz columns wrong")
    restored = pickle.loads(pickle.dumps(nasdaq))
    check("tz_source", restored.tz_offset_min == -300 and restored.dst_rule == 1
          and restored.market == "NASDAQ", "pickle roundtrip lost the tz columns")

    # Day look-back: data_lag is consumed by the MultiSystem portfolio layer (the member drives
    # at the last bar at or before the axis day shifted back by the lag trading days); the raw
    # KData of a single Stock still holds the axis-day bar (the look-ahead boundary for direct
    # single-System users)
    ndx = sm["NASDAQNDX"]
    ndx_bars = ndx.get_kdata(Query(0, ndx.get_count("DAY"), "DAY"))
    lag = sm.get_market_info("NASDAQ").data_lag
    check("day_lag", lag == 1 and len(ndx_bars) >= 4, "NASDAQ lag fixture insufficient")
    if lag == 1 and len(ndx_bars) >= 4:
        for i in (len(ndx_bars) - 1, len(ndx_bars) - 2, len(ndx_bars) - 3):
            axis = ndx_bars[i].datetime
            capped = ndx.get_kdata(Query(Datetime(199001010000), axis, "DAY"))
            check("day_lag", len(capped) == i and capped[-1].datetime == ndx_bars[i - lag].datetime,
                  "look-back at %s gave %s, expect the previous trading day"
                  % (axis, capped[-1].datetime if capped else "nothing"))
            raw = ndx.get_kdata(Query(Datetime(199001010000), axis.next_day(), "DAY"))
            check("day_lag", len(raw) == i + 1 and raw[-1].datetime == axis,
                  "axis-day bar unexpectedly hidden at %s" % axis)
        SNAPSHOT.append("day lag look-back: last axis=%s visible=%s (raw keeps the axis bar)"
                        % (ndx_bars[-1].datetime, ndx_bars[-2].datetime))

    # load scope: subprocess + HKU_STOCK_LIST (codes intentionally lowercase, pair intentionally with slash)
    env = dict(os.environ)
    env["HKU_STOCK_LIST"] = "sh000001;shfrb0000;binance/btcusdt"
    r = subprocess.run([sys.executable, __file__, "--phase", "stock_list"], env=env,
                       capture_output=True, encoding="utf-8", errors="replace")
    check("stock_list", r.returncode == 0, "stock_list phase failed: %s" % (r.stdout + r.stderr)[-400:])

    # portfolio calendar: run(query) anchors SH by default; cross-market use must inject the target calendar
    q = Query(Datetime(20050104), Datetime(20111206), Query.DAY)
    sh_cal = set(sm.get_trading_calendar(q))
    nyse_cal = set(sm.get_trading_calendar(q, "NYSE"))
    check("portfolio_calendar", sh_cal and nyse_cal and (nyse_cal - sh_cal),
          "trading calendars of SH and NYSE do not differ")
    seen = []
    dji = sm["NYSEDJI"].get_kdata(q)

    def capture_check_risk(self, date, tm, suggestions, query, mode):
        seen.append(date)

    def equal_weight(self, date, tm, contexts, query, mode):
        return {ctx.sys: 1.0 for ctx in contexts}

    def run_axis(use_calendar):
        seen.clear()
        ms = MultiSystem()
        ms.tm = crtTM(Datetime(200501040000), 1000000)
        ms.set_mode("A")
        ms.af = crtAF(equal_weight, name="calendar_af", check_risk_func=capture_check_risk)
        sub = SYS_Simple()
        sub.tm = crtTM(Datetime(200501040000), 1000000)
        sub.sg = SG_Cross(MA(dji.close, 5), MA(dji.close, 10))
        sub.mm = MM_FixedCount(10)
        sub.to = dji
        ms.add(sub)
        if use_calendar:
            ms.set_axis_mode("calendar")
            ms.set_date_axis(sm.get_trading_calendar(q, "NYSE"))
        ms.run(q)
        return list(seen), [tr.datetime for tr in ms.tm.get_trade_list()]

    default_days, _ = run_axis(False)
    check("portfolio_calendar", default_days and all(d in sh_cal for d in default_days),
          "default multi-system axis is not the SH calendar")
    calendar_days, calendar_trades = run_axis(True)
    check("portfolio_calendar", calendar_days and all(d in nyse_cal for d in calendar_days)
          and any(d not in sh_cal for d in calendar_days),
          "set_date_axis did not switch the axis to the NYSE calendar")
    check("portfolio_calendar", calendar_trades and all(d in nyse_cal for d in calendar_trades),
          "trades fell outside the injected date axis")


def run_md2_checks():
    """Type registration and Type-level unit, runtime variety-parameter override"""
    from hikyuu import BUSINESS, Datetime, MG_Fixed, Query, TC_Zero, crtTM
    from test_init import sm

    # type registration: rows for crypto/futures/FX/HK/US present; Type-level unit = tickValue / tick
    for type_id in (10, 12, 13, 14, 15):
        check("type_register", sm.get_stock_type_info(type_id).type == type_id,
              "stock type %s not registered" % type_id)
    for code in ["SHFRB0000", "FXEURUSD", "HKHSI", "NYSEDJI", "BINANCEBTCUSDT"]:
        stk = sm[code]
        check("type_register", not stk.is_null(), "%s not loaded" % code)
        if stk.is_null():
            continue
        check("type_register", abs(stk.unit - stk.tick_value / stk.tick) < 1e-9,
              "%s unit != tick_value / tick" % code)

    # runtime variety override: contract multiplier, min lot and expiry adopted by the trade entries
    rb = sm["SHFRB0000"]
    tick, tick_value = rb.tick, rb.tick_value
    min_number, last = rb.min_trade_number, rb.last_datetime
    bar = rb.get_kdata(Query(100, 101, "DAY"))[0]
    try:
        rb.tick_value = tick_value * 10.0  # rebar contract multiplier: 10 tons per lot
        check("variety_override", rb.unit == 10.0, "runtime tick_value override not reflected in unit")
        tm = crtTM(bar.datetime, 1000000.0, TC_Zero(), "SYS", MG_Fixed(0.5))
        check("variety_override", tm.buy(bar.datetime, rb, bar.close, 10.0).business == BUSINESS.BUY,
              "legal number rejected after the multiplier override")

        # the margin money flows are absolute amounts, so a dropped multiplier shows up as a 10x gap
        mtm = crtTM(bar.datetime, 1000000.0, TC_Zero(), "SYS", MG_Fixed(0.1, 0.1))
        mtm.set_param("margin_accounting", True)
        margin = round(0.1 * round(bar.close * 10.0 * 10.0, 2), 2)
        check("variety_override", mtm.buy(bar.datetime, rb, bar.close, 10.0).business == BUSINESS.BUY,
              "margin buy rejected after the multiplier override")
        check("variety_override", abs(mtm.current_cash - (1000000.0 - margin)) < 0.01,
              "margin buy did not lock the multiplied margin (%s)" % margin)
        check("variety_override",
              abs(mtm.get_position(bar.datetime, rb).margin - margin) < 0.01,
              "the booked position margin is not the multiplied amount")
        check("variety_override", abs(mtm.get_occupied_margin(bar.datetime) - margin) < 0.01,
              "the occupied margin view is not scaled by the multiplier")

        rb.min_trade_number = 100.0
        tm2 = crtTM(bar.datetime, 1000000.0, TC_Zero(), "SYS", MG_Fixed(0.5))
        check("variety_override", tm2.buy(bar.datetime, rb, bar.close, 10.0).business == BUSINESS.INVALID,
              "buy below the runtime min_trade_number was accepted")
        rb.min_trade_number = min_number

        rb.last_datetime = Datetime(200501010000)
        tm3 = crtTM(bar.datetime, 1000000.0, TC_Zero(), "SYS", MG_Fixed(0.5))
        check("variety_override", tm3.buy(bar.datetime, rb, bar.close, 1.0).business == BUSINESS.INVALID,
              "buy after the runtime last_datetime was accepted")
    finally:
        rb.tick, rb.tick_value = tick, tick_value
        rb.min_trade_number, rb.last_datetime = min_number, last
    check("variety_override", abs(rb.unit - tick_value / tick) < 1e-9 and rb.min_trade_number == min_number,
          "runtime overrides were not restored")

    # SQLite same-code isolation: per-market per-ktype db files, table name is the bare code
    r = subprocess.run([sys.executable, __file__, "--phase", "sqlite_isolation"],
                       capture_output=True, encoding="utf-8", errors="replace")
    check("sqlite_isolation", r.returncode == 0,
          "sqlite isolation phase failed: %s" % (r.stdout + r.stderr)[-400:])

    # dimension ratios: correct-caliber markets must pass; legacy fixtures must match the known deviation
    def caliber_ratios(stk_, max_days=5):
        nd = stk_.get_count("DAY")
        for offset in range(1, max_days + 1):
            bars = stk_.get_kdata(Query(nd - offset, nd - offset + 1, "DAY"))
            if not bars:
                continue
            bar = bars[0]
            day_start = bar.datetime.start_of_day()
            m1 = stk_.get_kdata(Query(day_start, day_start.next_day(), "MIN"))
            m5 = stk_.get_kdata(Query(day_start, day_start.next_day(), "MIN5"))
            if not m1 or not m5 or bar.amount <= 0.0 or bar.volume <= 0.0:
                continue
            a1 = sum(rec.amount for rec in m1)
            a5 = sum(rec.amount for rec in m5)
            if a1 <= 0.0 or a5 <= 0.0:
                continue
            c1 = sum(rec.volume for rec in m1)
            return day_start, bar.amount / a1, bar.amount / a5, c1 / bar.volume
        return None

    def report_caliber(code, expect=None):
        ratios = caliber_ratios(sm[code])
        if ratios is None:
            SNAPSHOT.append("caliber %s N/A (no coherent minute fixture)" % code)
            return
        date, ra1, ra5, rc = ratios
        SNAPSHOT.append("caliber %s %s day/min=%.6f day/min5=%.6f count_min/day=%.6f"
                        % (code, date, ra1, ra5, rc))
        if expect is not None:
            check("caliber", abs(ra1 - expect[0]) < 1e-4 and abs(ra5 - expect[0]) < 1e-4
                  and abs(rc - expect[1]) < 1e-3,
                  "%s caliber off: day/min=%.6f day/min5=%.6f count=%.6f" % (code, ra1, ra5, rc))
        else:
            # known legacy fixture signature (old 1min data): amount 100x true value, volume in lots
            check("caliber", abs(ra1 - 0.01) < 1e-4 and abs(rc - 1.0) < 1e-3,
                  "%s deviates from the known legacy fixture: day/min=%.6f count=%.6f"
                  % (code, ra1, rc))

    report_caliber("SZ399001", expect=(1.0, 100.0))  # rebuilt day+minute fixture, correct caliber
    for legacy_code in ["SH600004", "SH000001", "SZ000001"]:
        report_caliber(legacy_code)
    for day_only in ["SHFRB0000", "DCEI0000", "GFESI0000", "INESC0000", "HKHSI", "FXEURUSD",
                     "NYSEDJI", "NASDAQNDX", "BINANCEBTCUSDT"]:
        report_caliber(day_only)


def phase_sqlite_isolation():
    """Subprocess: two markets each with their own SQLite day db, both with table 000001;
    verify no cross-market mixing"""
    import configparser
    import shutil
    import sqlite3

    dates = [20111201, 20111202, 20111205, 20111206]
    sh_closes = [2001.0, 2002.0, 2005.0, 2006.0]
    sz_closes = [11.0, 12.0, 15.0, 16.0]

    base = os.path.join("test_data", "tmp", "m23_sqlite")
    shutil.rmtree(base, ignore_errors=True)
    os.makedirs(os.path.join(base, "tmp"))

    def make_db(path, closes_by_code):
        con = sqlite3.connect(path)
        for code, closes in closes_by_code.items():
            con.execute("create table '%s' (date integer primary key, open real not null, "
                        "high real not null, low real not null, close real not null, "
                        "amount real not null, count real not null)" % code)
            for date, close in zip(dates, closes):
                con.execute("insert into '%s' values (?, ?, ?, ?, ?, 0, 0)" % code,
                            (date, close, close, close, close))
        con.commit()
        con.close()

    sh_db = os.path.join(base, "m23_sh_day.db")
    sz_db = os.path.join(base, "m23_sz_day.db")
    # SH additionally creates 000300 (default must-load), otherwise startup logs a "no such table" error
    make_db(sh_db, {"000001": sh_closes, "000300": sh_closes})
    make_db(sz_db, {"000001": sz_closes})

    ini = configparser.ConfigParser()
    ini.read("test_data/hikyuu_win.ini", encoding="utf-8")
    ini.set("hikyuu", "tmpdir", os.path.join(base, "tmp"))
    ini.remove_section("kdata")
    ini.add_section("kdata")
    ini.set("kdata", "type", "sqlite3")
    ini.set("kdata", "sh_day", sh_db)
    ini.set("kdata", "sz_day", sz_db)
    config = os.path.join(base, "m23.ini")
    with open(config, "w", encoding="utf-8") as f:
        ini.write(f)

    import hikyuu

    try:
        hikyuu.load_hikyuu(config_file=config, stock_list=["SH000001", "SZ000001"])
        sm = hikyuu.StockManager.instance()
        q = hikyuu.Query(0, 100, "DAY")
        got = {}
        for market in ("SH", "SZ"):
            stk = sm.get_stock(market + "000001")
            if stk.is_null():
                print("FAIL: sqlite isolation %s000001 not loaded" % market)
                sys.exit(1)
            got[market] = [bar.close for bar in stk.get_kdata(q)]
        if got["SH"] != sh_closes or got["SZ"] != sz_closes:
            print("FAIL: sqlite isolation cross-market contamination: SH=%s SZ=%s" % (got["SH"], got["SZ"]))
            sys.exit(1)
        print("sqlite isolation: SH/SZ read table 000001 from their own file")
    finally:
        shutil.rmtree(base, ignore_errors=True)


def phase_stock_list():
    import hikyuu
    # HKU_STOCK_LIST is consumed only by the Python-layer load_hikyuu (hikyuu_init binding is fixed to all)
    hikyuu.load_hikyuu(config_file="test_data/hikyuu_win.ini")
    sm = hikyuu.StockManager.instance()
    # list is case-insensitive: recorded lowercase, both spellings must hit
    for code in ["sh000001", "SHFrb0000", "SHFRB0000"]:
        s = sm.get_stock(code)
        if s.is_null() or s.get_count("DAY") == 0:
            print("FAIL: stock_list %s not loaded" % code)
            sys.exit(1)
    # unlisted securities are not registered, hence no kdata loaded and not preloaded
    for code in ["sz000001", "SHFcu0000"]:
        if not sm.get_stock(code).is_null():
            print("FAIL: stock_list %s registered but not listed" % code)
            sys.exit(1)
    # pair with the market_code() slash spelling must also be accepted (no bars; registration only)
    btc = sm.get_stock("BINANCE/BTCUSDT")
    if btc.is_null() or btc.get_count("DAY") != 0:
        print("FAIL: BINANCE/BTCUSDT not registered")
        sys.exit(1)
    print("stock_list phase: len(sm)=%d, listed loaded, unlisted absent" % len(sm))


if __name__ == "__main__":
    if len(sys.argv) >= 3 and sys.argv[1] == "--phase" and sys.argv[2] == "stock_list":
        phase_stock_list()
        sys.exit(0)
    if len(sys.argv) >= 3 and sys.argv[1] == "--phase" and sys.argv[2] == "sqlite_isolation":
        phase_sqlite_isolation()
        sys.exit(0)
    run_acceptance()
    run_md2_checks()
    for line in SNAPSHOT:
        print(line)
    if FAIL:
        for line in FAIL:
            print("FAIL:", line)
        sys.exit(1)
    print("ACCEPTANCE PASS")
