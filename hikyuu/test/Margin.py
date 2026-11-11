#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Create on: 2026-11-03
#    Author: fasiondog

import unittest
from decimal import Decimal, ROUND_HALF_UP

from test_init import *


def round_ex(value, ndigits=2):
    """The rounding of the C++ core: half away from zero at the given decimal place"""
    quantum = Decimal(1).scaleb(-ndigits)
    return float(Decimal(repr(value)).quantize(quantum, rounding=ROUND_HALF_UP))


class PythonMargin(MarginBase):
    def __init__(self):
        super(PythonMargin, self).__init__("PythonMargin")

    def is_margin_trading(self, stock):
        return True

    def need_borrow_asset(self, stock):
        return False

    def get_open_margin(self, date, stock, price, num, side=MarginSide.LONG):
        return 5000.0

    def _clone(self):
        return PythonMargin()


class PythonMaintainMargin(PythonMargin):
    """A margin model whose maintenance requirement is above the equity of the test account"""

    def __init__(self, maintain=20000.0):
        super(PythonMaintainMargin, self).__init__()
        self._maintain = maintain

    def get_maintain_margin(self, date, stock, price, num, side=MarginSide.LONG):
        return self._maintain

    def _clone(self):
        return PythonMaintainMargin(self._maintain)


class PythonCollateralMargin(PythonMargin):
    """A margin model answering its own collateral conversion rate on the long side"""

    def __init__(self):
        super(PythonCollateralMargin, self).__init__()
        self.set_param("collateral_rate", 1.0)

    def get_collateral_rate(self, date, stock, side=MarginSide.LONG):
        return self.get_param("collateral_rate") if side == MarginSide.LONG else 1.0

    def _clone(self):
        copied = PythonCollateralMargin()
        copied.set_param("collateral_rate", self.get_param("collateral_rate"))
        return copied


class PythonInterestCost(TradeCostBase):
    """A cost model answering a fixed annualized rate for each kind of liability

    Only the two rate hooks are declared: the accounts of this suite borrow and settle but never
    trade through this model, and the buy and sell costs have no C++ default to fall back to.
    """

    def __init__(self, cash_rate=0.0, stock_rate=0.0):
        super(PythonInterestCost, self).__init__("PythonInterestCost")
        self._cash_rate = cash_rate
        self._stock_rate = stock_rate

    def get_borrow_cash_rate(self, date):
        return self._cash_rate

    def get_borrow_stock_rate(self, date, stock):
        return self._stock_rate

    def _clone(self):
        return PythonInterestCost(self._cash_rate, self._stock_rate)


def crtInterestTM(dt, init_cash, cash_rate, stock_rate, annual_days=365):
    """A settling margin account with the given annualized liability rates"""
    tm = crtTM(dt, init_cash, PythonInterestCost(cash_rate, stock_rate), "SYS", PythonMargin())
    tm.set_param("margin_accounting", True)
    tm.set_param("annual_days", annual_days)
    return tm


def count_interest(tm, date=None):
    """The number of the INTEREST records of the account, at one moment when the date is given"""
    return len([tr for tr in tm.get_trade_list()
                if tr.business == BUSINESS.INTEREST and (date is None or tr.datetime == date)])


class MarginTest(unittest.TestCase):
    def test_MG_None(self):
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        price = stock.get_market_value(dt, "DAY")
        mg = MG_None()
        self.assertEqual(mg.name, "MG_None")
        self.assertFalse(mg.is_margin_trading(stock))
        self.assertAlmostEqual(mg.get_open_margin(dt, stock, price, 100),
                               price * 100 * stock.unit)
        self.assertEqual(mg.get_margin_rate(dt, stock), 1.0)

    def test_MG_Fixed(self):
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        price = stock.get_market_value(dt, "DAY")
        mg = MG_Fixed(0.2, 0.3, 0.15)
        self.assertEqual(mg.name, "MG_Fixed")
        self.assertAlmostEqual(mg.get_open_margin(dt, stock, price, 100),
                               0.2 * price * 100 * stock.unit)
        self.assertAlmostEqual(mg.get_open_margin(dt, stock, price, 100, MarginSide.SHORT),
                               0.3 * price * 100 * stock.unit)
        self.assertAlmostEqual(mg.get_maintain_margin(dt, stock, price, 100),
                               0.15 * price * 100 * stock.unit)
        lots = stock.min_trade_number
        per_lot = 0.2 * price * lots * stock.unit
        available = price * 100 * stock.unit
        self.assertEqual(mg.get_max_open_number(dt, stock, price, available),
                         float(int(available / per_lot) * lots))

    def test_MG_Fixed_per_lot(self):
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        mg = MG_Fixed(margin_per_lot=5000.0)
        lots = stock.min_trade_number
        self.assertAlmostEqual(mg.get_open_margin(dt, stock, 10.0, 3 * lots), 15000.0)
        # the per-lot mode gives no ratio view
        self.assertEqual(mg.get_margin_rate(dt, stock), 1.0)

    def test_MG_ByType(self):
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        mg = MG_ByType("2:0.20", 0.0, 1.0)
        self.assertEqual(mg.get_margin_rate(dt, stock), 0.20)
        self.assertTrue(mg.is_margin_trading(stock))

        mg = MG_ByType("SH:0.10", 0.0, 1.0)
        self.assertEqual(mg.get_margin_rate(dt, stock), 0.10)

        mg = MG_ByType("12:0.10", 0.0, 1.0)
        self.assertEqual(mg.get_margin_rate(dt, stock), 1.0)
        self.assertFalse(mg.is_margin_trading(stock))

    def test_MG_bad_params(self):
        for rate in (0.0, 1.1):
            self.assertRaises(Exception, MG_Fixed, rate)
        # maintain_rate must stay at or below the opening rate (a higher one would call at open)
        self.assertRaises(Exception, MG_Fixed, 0.1, 0.0, 0.15)
        MG_Fixed(0.1, 0.0, 0.05)
        self.assertRaises(Exception, MG_Fixed, 0.1, 0.0, 0.0, -1.0)
        for rates in ("SH:abc", "SH:0", "SH:1.2", "SH"):
            self.assertRaises(Exception, MG_ByType, rates)

    def test_PythonMargin(self):
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        mg = PythonMargin()
        self.assertEqual(mg.get_open_margin(dt, stock, 10.0, 100), 5000.0)
        self.assertFalse(mg.need_borrow_asset(stock))
        self.assertEqual(mg.clone().get_open_margin(dt, stock, 10.0, 100), 5000.0)

        tm = crtTM(dt, 20000.0, TC_Zero(), "SYS", mg)
        self.assertEqual(tm.margin.name, "PythonMargin")
        self.assertEqual(tm.get_max_open_number(dt, stock, 10.0, 20000.0), 4.0)

    def test_MG_ByType_clone(self):
        """The clone answers from its own table, not from the fallback ratio"""
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        mg = MG_ByType("2:0.20", 0.0, 0.9)
        self.assertEqual(mg.get_margin_rate(dt, stock), 0.20)

        cloned = mg.clone()
        self.assertEqual(cloned.name, "MG_ByType")
        self.assertEqual(cloned.get_param("rates"), "2:0.20")
        self.assertEqual(cloned.get_margin_rate(dt, stock), 0.20)

    def test_pickle(self):
        """The name and the parameters survive a pickle round-trip"""
        if not constant.pickle_support:
            return

        import pickle as pl
        stock = sm['sh000001']
        dt = Datetime(200001040000)

        mg = MG_ByType("2:0.20", 0.0, 0.9)
        b = pl.loads(pl.dumps(mg))
        self.assertEqual(b.name, "MG_ByType")
        self.assertEqual(b.get_param("rates"), "2:0.20")
        self.assertEqual(b.get_margin_rate(dt, stock), 0.20)

        fixed = MG_Fixed(0.2, 0.3, 0.15)
        b = pl.loads(pl.dumps(fixed))
        self.assertEqual(b.name, "MG_Fixed")
        self.assertEqual(b.get_param("rate"), 0.2)
        self.assertEqual(b.get_param("rate_short"), 0.3)
        self.assertEqual(b.get_param("maintain_rate"), 0.15)

        b = pl.loads(pl.dumps(MG_None()))
        self.assertEqual(b.name, "MG_None")
        self.assertFalse(b.is_margin_trading(stock))

    def test_TradeManager_margin(self):
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        price = stock.get_market_value(dt, "DAY")

        tm = crtTM(dt, 100000.0, TC_Zero())
        self.assertEqual(tm.margin.name, "MG_Fixed")
        self.assertEqual(tm.get_margin_rate(dt, stock), 0.6)
        self.assertEqual(tm.get_occupied_margin(dt), 0.0)
        self.assertEqual(tm.get_available_margin(dt), 100000.0)

        # a plain cash account is not constrained by the margin view
        tm.margin = MG_None()
        self.assertEqual(tm.margin.name, "MG_None")
        self.assertEqual(tm.get_margin_rate(dt, stock), 1.0)
        self.assertEqual(tm.get_max_open_number(dt, stock, price, 100000.0),
                         int(100000.0 / (price * stock.unit) / stock.min_trade_number)
                         * stock.min_trade_number)

        # assigning None falls back to the plain cash model
        tm.margin = None
        self.assertEqual(tm.margin.name, "MG_None")

    def test_TradeManager_margin_accounting(self):
        """A margin security locks the margin only and reports it"""
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        price = stock.get_market_value(dt, "DAY")

        mg = PythonMargin()
        tm = crtTM(dt, 100000.0, TC_Zero(), "SYS", mg)
        self.assertFalse(tm.is_margin_accounted(stock))
        tm.set_param("margin_accounting", True)
        self.assertTrue(tm.is_margin_accounted(stock))

        tr = tm.buy(dt, stock, price, 100.0)
        self.assertEqual(tr.business, BUSINESS.BUY)
        self.assertAlmostEqual(tr.cash, 95000.0)
        self.assertAlmostEqual(tr.margin, 5000.0)
        self.assertAlmostEqual(tm.get_position(dt, stock).margin, 5000.0)
        self.assertAlmostEqual(tm.get_position(dt, stock).open_basis, price * 100.0 * stock.unit)
        # before any settlement the profit basis is still the opening notional
        self.assertAlmostEqual(tm.get_position(dt, stock).settle_basis,
                               price * 100.0 * stock.unit)
        self.assertAlmostEqual(tm.current_cash, 95000.0)

        funds = tm.get_funds()
        self.assertAlmostEqual(funds.occupied_margin, 5000.0)
        self.assertAlmostEqual(funds.market_value, 5000.0)
        self.assertAlmostEqual(funds.net_assets, 100000.0)

        # the requirement is recomputed, the locked amount is what the openings booked
        self.assertAlmostEqual(tm.get_occupied_margin(dt), 5000.0)
        self.assertAlmostEqual(tm.get_available_margin(dt),
                               funds.net_assets - tm.get_occupied_margin(dt))

        # a temporary model must survive inside the account, both as a constructor argument
        # and through the setter: its Python half decides the margin
        tm2 = crtTM(dt, 100000.0, TC_Zero(), "SYS", PythonMargin())
        tm2.set_param("margin_accounting", True)
        self.assertTrue(tm2.is_margin_accounted(stock))
        self.assertAlmostEqual(tm2.buy(dt, stock, price, 100.0).cash, 95000.0)

        tm3 = crtTM(dt, 100000.0, TC_Zero())
        tm3.margin = PythonMargin()
        tm3.set_param("margin_accounting", True)
        self.assertTrue(tm3.is_margin_accounted(stock))

        # closing at the same price returns the whole margin
        tr = tm.sell(dt, stock, price, 100.0)
        self.assertEqual(tr.business, BUSINESS.SELL)
        self.assertAlmostEqual(tr.cash, 100000.0)
        self.assertAlmostEqual(tr.margin, 0.0)
        self.assertAlmostEqual(tm.get_funds().occupied_margin, 0.0)
        self.assertAlmostEqual(tm.get_position(dt, stock).open_basis, 0.0)
        self.assertAlmostEqual(tm.get_position(dt, stock).settle_basis, 0.0)

    def test_TradeManager_margin_accounting_futures_multiplier(self):
        """Every money flow of a futures trade scales with the contract multiplier

        The fixture data keeps the neutral type-level unit of 1, so the multiplier is overridden at
        runtime (the variety parameter) and the expected amounts are written as absolute numbers
        """
        rb = sm['SHFRB0000']
        dt = Datetime(200903270000)
        close = rb.get_market_value(dt, "DAY")
        tick_value = rb.tick_value
        self.assertEqual(rb.unit, 1.0)

        multiplier = 10.0       # rebar: 10 tons per lot
        number = 10.0           # lots
        nominal = round(close * number * multiplier, 2)
        margin = round(0.1 * nominal, 2)

        try:
            rb.tick_value = tick_value * multiplier
            self.assertEqual(rb.unit, multiplier)

            tm = crtTM(dt, 1000000.0, TC_Zero(), "SYS", MG_Fixed(0.1, 0.1))
            tm.set_param("margin_accounting", True)
            self.assertTrue(tm.is_margin_accounted(rb))

            # the openable quantity is measured on the margin of one lot
            self.assertAlmostEqual(tm.get_max_open_number(dt, rb, close, margin), number)

            tr = tm.buy(dt, rb, close, number)
            self.assertEqual(tr.business, BUSINESS.BUY)
            self.assertAlmostEqual(tr.cash, 1000000.0 - margin)
            position = tm.get_position(dt, rb)
            self.assertAlmostEqual(position.margin, margin)
            self.assertAlmostEqual(position.open_basis, nominal)
            self.assertAlmostEqual(position.settle_basis, nominal)
            self.assertAlmostEqual(tm.get_occupied_margin(dt), margin)
            self.assertAlmostEqual(tm.get_available_margin(dt), 1000000.0 - margin)
            funds = tm.get_funds(dt)
            self.assertAlmostEqual(funds.occupied_margin, margin)
            self.assertAlmostEqual(funds.net_assets, 1000000.0)

            # closing higher realizes the multiplied difference, the margin comes back
            up = close + 10.0
            realized = round((up - close) * number * multiplier, 2)
            tr = tm.sell(dt, rb, up, number)
            self.assertEqual(tr.business, BUSINESS.SELL)
            self.assertAlmostEqual(tm.current_cash, 1000000.0 + realized)
            self.assertAlmostEqual(tm.get_funds(dt).occupied_margin, 0.0)

            # the short side keeps the same money basis
            tm2 = crtTM(dt, 1000000.0, TC_Zero(), "SYS", MG_Fixed(0.1, 0.1))
            tm2.set_param("margin_accounting", True)
            tr = tm2.sell_short(dt, rb, close, number)
            self.assertEqual(tr.business, BUSINESS.SELL_SHORT)
            self.assertAlmostEqual(tm2.current_cash, 1000000.0 - margin)
            self.assertAlmostEqual(tm2.get_short_position(rb).margin, margin)
            tr = tm2.buy_short(dt, rb, up, number)
            self.assertEqual(tr.business, BUSINESS.BUY_SHORT)
            self.assertAlmostEqual(tm2.current_cash, 1000000.0 - realized)
        finally:
            rb.tick_value = tick_value

        self.assertAlmostEqual(rb.unit, 1.0)

    def test_TradeManager_short_position(self):
        """The short-side position bookkeeping is visible through the accessors"""
        rb = sm['SHFRB0000']
        bar = rb.get_kdata(Query(100, 101, "DAY"))[0]

        tm = crtTM(bar.datetime, 100000.0, TC_Zero())
        tm.margin = MG_Fixed(0.6, 0.6)
        self.assertFalse(tm.have_short(rb))
        self.assertEqual(len(tm.get_short_position_list()), 0)
        self.assertEqual(len(tm.get_borrow_stock_list()), 0)
        self.assertEqual(tm.get_debt_number(bar.datetime, rb), 0.0)
        self.assertEqual(tm.get_debt_cash(bar.datetime), 0.0)

        tr = tm.sell_short(bar.datetime, rb, bar.close, 10.0)
        self.assertEqual(tr.business, BUSINESS.SELL_SHORT)
        self.assertTrue(tm.have_short(rb))
        self.assertAlmostEqual(tm.get_short_position(rb).number, 10.0)
        self.assertEqual(len(tm.get_short_position_list()), 1)

        tr = tm.buy_short(bar.datetime, rb, bar.close, 10.0)
        self.assertEqual(tr.business, BUSINESS.BUY_SHORT)
        self.assertFalse(tm.have_short(rb))
        self.assertEqual(len(tm.get_short_position_list()), 0)
        self.assertEqual(len(tm.get_short_history_position_list()), 1)

    def test_TradeManager_settlement(self):
        """The daily settlement marks the holding to the close and relocks it"""
        rb = sm['SHFRB0000']
        bars = rb.get_kdata(Query(100, 104, "DAY"))
        self.assertGreaterEqual(len(bars), 3)
        dt1, close1 = bars[0].datetime, bars[0].close
        dt2, close2 = bars[1].datetime, bars[1].close
        self.assertNotEqual(close1, close2)

        unit = rb.unit
        number = 10.0
        nominal1 = round(close1 * number * unit, 2)
        lock1 = round(0.1 * nominal1, 2)
        nominal2 = round(close2 * number * unit, 2)
        lock2 = round(0.1 * nominal2, 2)

        tm = crtTM(dt1, 1000000.0, TC_Zero(), "SYS", MG_Fixed(0.1, 0.0, 0.1))
        tm.set_param("margin_accounting", True)
        self.assertEqual(tm.buy(dt1, rb, close1, number).business, BUSINESS.BUY)

        # nothing is settled before the call, and the settlement is idempotent
        self.assertFalse(tm.is_settled(dt1))
        self.assertFalse(tm.is_settled(dt2))
        self.assertTrue(tm.settlement(dt2))
        self.assertTrue(tm.is_settled(dt2))
        self.assertFalse(tm.is_settled(dt1))

        # the basis moves to the settlement price and the lock is recomputed on it
        position = tm.get_position(dt2, rb)
        self.assertAlmostEqual(position.settle_basis, nominal2)
        self.assertAlmostEqual(position.margin, lock2)

        # the cash carries the profit net of the margin difference
        expect_cash = round(1000000.0 - lock1, 2)
        expect_cash = round(expect_cash + (nominal2 - nominal1), 2)
        expect_cash = round(expect_cash - round(lock2 - lock1, 2), 2)
        self.assertAlmostEqual(tm.current_cash, expect_cash)

        # the record carries the settlement price, the signed quantity and the new lock
        record = tm.get_trade_list()[-1]
        self.assertEqual(record.business, BUSINESS.SETTLEMENT)
        self.assertEqual(record.datetime, dt2)
        self.assertAlmostEqual(record.real_price, close2)
        self.assertAlmostEqual(record.number, number)
        self.assertAlmostEqual(record.margin, lock2)
        self.assertAlmostEqual(record.cash, tm.current_cash)

        count = len(tm.get_trade_list())
        self.assertTrue(tm.settlement(dt2))
        self.assertEqual(len(tm.get_trade_list()), count)
        self.assertAlmostEqual(tm.current_cash, expect_cash)

        # the short side settles with the sign of the quantity
        tm2 = crtTM(dt1, 1000000.0, TC_Zero(), "SYS", MG_Fixed(0.1, 0.0, 0.1))
        tm2.set_param("margin_accounting", True)
        self.assertEqual(tm2.sell_short(dt1, rb, close1, number).business, BUSINESS.SELL_SHORT)
        self.assertTrue(tm2.settlement(dt2))
        self.assertAlmostEqual(tm2.get_short_position(rb).settle_basis, nominal2)
        self.assertEqual(tm2.get_trade_list()[-1].number, -number)

        # the plain cash books never settle
        plain = crtTM(dt1, 1000000.0, TC_Zero())
        self.assertEqual(plain.buy(dt1, rb, close1, number).business, BUSINESS.BUY)
        self.assertFalse(plain.settlement(dt1))
        self.assertFalse(plain.is_settled(dt1))

    def test_TradeManager_settlement_margin_call(self):
        """A maintenance shortfall queues the forced liquidation only with force_liquidate on"""
        rb = sm['SHFRB0000']
        bars = rb.get_kdata(Query(100, 104, "DAY"))
        self.assertGreaterEqual(len(bars), 3)
        dt1, close1 = bars[0].datetime, bars[0].close
        dt2 = bars[1].datetime

        tm = crtTM(dt1, 10000.0, TC_Zero(), "SYS", PythonMaintainMargin(20000.0))
        tm.set_param("margin_accounting", True)
        self.assertEqual(tm.buy(dt1, rb, close1, 10.0).business, BUSINESS.BUY)
        self.assertTrue(tm.settlement(dt2))
        self.assertFalse(tm.is_forced_liquidation_queued(dt2))

        queued = crtTM(dt1, 10000.0, TC_Zero(), "SYS", PythonMaintainMargin(20000.0))
        queued.set_param("margin_accounting", True)
        queued.set_param("force_liquidate", True)
        self.assertEqual(queued.buy(dt1, rb, close1, 10.0).business, BUSINESS.BUY)
        self.assertFalse(queued.is_forced_liquidation_queued(dt1))
        self.assertTrue(queued.settlement(dt2))
        self.assertTrue(queued.is_forced_liquidation_queued(dt2))
        self.assertFalse(queued.is_forced_liquidation_queued(dt1))

        # a flat account has nothing left to liquidate
        queued.sell(dt2, rb, bars[1].close, 10.0)
        self.assertFalse(queued.is_forced_liquidation_queued(dt2))

    def test_business_records_for_settlement(self):
        """The settlement and the interest record types are exposed by name"""
        self.assertEqual(get_business_name(BUSINESS.SETTLEMENT), "SETTLEMENT")
        self.assertEqual(get_business_name(BUSINESS.INTEREST), "INTEREST")

    def test_MG_ByType_collateral(self):
        """The collateral conversion table of MG_ByType"""
        stock = sm['sh000001']
        dt = Datetime(200001040000)

        # an unconfigured table counts every holding in full, on either side
        plain = MG_ByType("2:0.20")
        self.assertEqual(plain.get_collateral_rate(dt, stock), 1.0)
        self.assertEqual(plain.get_collateral_rate(dt, stock, MarginSide.SHORT), 1.0)
        self.assertEqual(plain.get_param("default_collateral_rate"), 1.0)

        # the type key hits before the market one, with the same syntax as the margin table
        mg = MG_ByType("2:0.20", 0.0, 0.6, "2:0.75, SH:0.5", 0.9)
        self.assertAlmostEqual(mg.get_collateral_rate(dt, stock), 0.75)
        # one table serves both sides, and it does not feed the margin ratios
        self.assertAlmostEqual(mg.get_collateral_rate(dt, stock, MarginSide.SHORT), 0.75)
        self.assertAlmostEqual(mg.get_margin_rate(dt, stock), 0.20)

        # a security matched by no collateral key falls back to default_collateral_rate
        fallback = MG_ByType("2:0.20", 0.0, 0.6, "SZ:0.5", 0.9)
        self.assertAlmostEqual(fallback.get_collateral_rate(dt, stock), 0.9)

        # the margin table stays untouched by the collateral one, and the other way around
        only_collateral = MG_ByType(collateral_rates="SH:0.4")
        self.assertAlmostEqual(only_collateral.get_collateral_rate(dt, stock), 0.4)
        self.assertAlmostEqual(only_collateral.get_margin_rate(dt, stock), 0.6)

        # the models without a table count every holding in full
        self.assertEqual(MG_Fixed(0.6).get_collateral_rate(dt, stock), 1.0)
        self.assertEqual(MG_None().get_collateral_rate(dt, stock), 1.0)

        # an invalid collateral expression or fallback throws instead of being ignored
        for bad in ("SH:abc", "SH:0", "SH:1.2", "SH"):
            self.assertRaises(Exception, MG_ByType, "", 0.0, 0.6, bad)
        for rate in (0.0, 1.1):
            self.assertRaises(Exception, MG_ByType, "", 0.0, 0.6, "", rate)

        # a valid expression replaces the table and clone carries it over
        mg.set_param("collateral_rates", "2:0.55, SH:0.5")
        self.assertAlmostEqual(mg.get_collateral_rate(dt, stock), 0.55)
        copied = mg.clone()
        self.assertEqual(copied.get_param("collateral_rates"), "2:0.55, SH:0.5")
        self.assertAlmostEqual(copied.get_collateral_rate(dt, stock), 0.55)

    def test_TradeManager_collateral_rate(self):
        """The conversion rate discounts the long holdings of the account"""
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        price = stock.get_market_value(dt, "DAY")
        number = 100.0
        margin = 5000.0

        mg = PythonCollateralMargin()
        tm = crtTM(dt, 1000000.0, TC_Zero(), "SYS", mg)
        tm.set_param("margin_accounting", True)

        # an unconfigured rate discounts nothing
        self.assertEqual(tm.get_collateral_rate(dt, stock), 1.0)
        self.assertEqual(tm.buy(dt, stock, price, number).business, BUSINESS.BUY)
        available = tm.get_available_margin(dt)
        self.assertAlmostEqual(tm.get_collateral_haircut(dt), 0.0)
        self.assertAlmostEqual(available, tm.get_funds(dt).net_assets - margin)

        # discounting the holding takes the haircut off what it contributes to the equity (the
        # locked margin before any settlement), that amount being rounded before it is discounted
        mg.set_param("collateral_rate", 0.7)
        haircut = round_ex(margin * 0.3)
        self.assertAlmostEqual(tm.get_collateral_rate(dt, stock), 0.7)
        self.assertAlmostEqual(tm.get_collateral_haircut(dt), haircut, delta=1e-6)
        self.assertAlmostEqual(tm.get_available_margin(dt), available - haircut, delta=1e-6)

        # the discounted base is the very amount get_funds counts of the holding
        self.assertAlmostEqual(tm.get_collateral_haircut(dt),
                               round_ex(tm.get_funds(dt).market_value * 0.3), delta=1e-6)

        # an asset side rule, leaving the occupation and the equity as they were
        self.assertAlmostEqual(tm.get_funds(dt).occupied_margin, margin)
        self.assertAlmostEqual(tm.get_funds(dt).net_assets, 1000000.0)
        self.assertEqual(tm.get_collateral_rate(dt, stock, MarginSide.SHORT), 1.0)

        # back to 1.0 the quota is exactly what it was before the table
        mg.set_param("collateral_rate", 1.0)
        self.assertAlmostEqual(tm.get_collateral_haircut(dt), 0.0)
        self.assertAlmostEqual(tm.get_available_margin(dt), available)

        # a rate outside (0, 1] broke the contract of the interface: the holding is left
        # undiscounted with a warning, instead of a haircut growing over the equity it discounts
        for bad in (-0.5, 0.0, float("nan")):
            mg.set_param("collateral_rate", bad)
            self.assertAlmostEqual(tm.get_collateral_haircut(dt), 0.0)

        # the discount runs on the current holdings, so a past moment is refused while an unset
        # one still values at the last trade moment
        with self.assertRaises(Exception):
            tm.get_collateral_haircut(Datetime(200001010000))
        self.assertAlmostEqual(tm.get_collateral_haircut(Datetime()), 0.0)

    def test_TradeManager_settlement_interest_cash(self):
        """The financing interest is billed into the debt itself, not paid in cash"""
        # a book made only of a cash debt needs no quote, so it can be dated freely
        dt = Datetime(200912310000)
        borrow = 10000.0
        tm = crtInterestTM(dt, 1000000.0, 0.365, 0.0)
        self.assertTrue(tm.borrow_cash(dt, borrow))
        cash = tm.current_cash

        # a flat book with an outstanding debt does settle, because the bill was posted
        self.assertFalse(tm.is_settled(dt))
        self.assertTrue(tm.settlement(dt))
        self.assertTrue(tm.is_settled(dt))

        # 0.365 over an annual base of 365 is exactly one thousandth of the debt per billed day
        self.assertAlmostEqual(tm.get_debt_cash(dt), borrow + 10.0)
        self.assertAlmostEqual(tm.current_cash, cash)

        # one record for the cash liability, with the amount and no quantity
        records = [tr for tr in tm.get_trade_list() if tr.business == BUSINESS.INTEREST]
        self.assertEqual(len(records), 1)
        record = records[0]
        self.assertEqual(record.datetime, dt)
        self.assertTrue(record.stock.is_null())
        self.assertAlmostEqual(record.real_price, 10.0)
        self.assertAlmostEqual(record.number, 0.0)
        self.assertAlmostEqual(record.margin, 0.0)

        # the debt is a liability of the account, so the net assets shrink by the bill
        self.assertAlmostEqual(tm.get_funds(dt).net_assets, 1000000.0 - 10.0)

        # a second call of the same moment bills nothing again
        self.assertTrue(tm.settlement(dt))
        self.assertEqual(count_interest(tm, dt), 1)
        self.assertAlmostEqual(tm.get_debt_cash(dt), borrow + 10.0)

        # the billed interest is repayable: return_cash still covers the whole debt
        self.assertTrue(tm.return_cash(dt, borrow + 10.0))
        self.assertAlmostEqual(tm.get_debt_cash(dt), 0.0)

    def test_TradeManager_settlement_interest_stock(self):
        """The securities lending is billed in cash on the market value of the borrowed shares"""
        rb = sm['SHFRB0000']
        bars = rb.get_kdata(Query(100, 104, "DAY"))
        self.assertGreaterEqual(len(bars), 3)
        dt1, close1 = bars[0].datetime, bars[0].close
        dt2, close2 = bars[1].datetime, bars[1].close

        number = 10.0
        tm = crtInterestTM(dt1, 1000000.0, 0.0, 0.365)
        self.assertTrue(tm.borrow_stock(dt1, rb, close1, number))
        cash = tm.current_cash
        self.assertTrue(tm.settlement(dt2))

        # the bill lands on the cash debt, not on the borrowed shares
        records = [tr for tr in tm.get_trade_list() if tr.business == BUSINESS.INTEREST]
        self.assertEqual(len(records), 1)
        interest = records[0].real_price
        self.assertAlmostEqual(tm.get_debt_cash(dt2), interest)
        self.assertAlmostEqual(tm.get_debt_number(dt2, rb), number)
        self.assertAlmostEqual(tm.current_cash, cash)

        # it is prorated on the settlement price of the borrowed quantity
        debt_base = round_ex(close2 * number * rb.unit)
        self.assertAlmostEqual(interest, round_ex(debt_base * 0.365 / 365.0), delta=1e-6)
        self.assertEqual(records[0].stock, rb)
        self.assertAlmostEqual(records[0].number, number)

        # the borrowed shares are not an asset, so only the cash debt reduces the net assets
        self.assertAlmostEqual(tm.get_funds(dt2).net_assets, 1000000.0 - interest)

    def test_TradeManager_settlement_interest_year_boundary(self):
        """The proration base stays the same across a calendar boundary, the debt compounds"""
        dt = Datetime(200912310000)
        dt2 = Datetime(201001010000)
        dt3 = Datetime(201001050000)
        borrow = 10000.0
        tm = crtInterestTM(dt, 1000000.0, 0.365, 0.0)
        self.assertTrue(tm.borrow_cash(dt, borrow))
        self.assertTrue(tm.settlement(dt))

        # the first bill covers the single day the debt was taken on
        self.assertAlmostEqual(tm.get_debt_cash(dt), borrow + 10.0)

        # the next calendar day bills on the grown debt, still over 365
        self.assertTrue(tm.settlement(dt2))
        self.assertAlmostEqual(tm.get_debt_cash(dt2), borrow + 10.0 + 10.01)
        self.assertEqual(count_interest(tm), 2)

        # after four untouched days the same rate bills all four of them at once
        self.assertTrue(tm.settlement(dt3))
        self.assertEqual(count_interest(tm, dt3), 1)
        self.assertAlmostEqual(tm.get_debt_cash(dt3), borrow + 10.0 + 10.01 + 40.08)

        # the earlier days are still measured without the later bills
        self.assertAlmostEqual(tm.get_debt_cash(dt), borrow + 10.0)
        self.assertAlmostEqual(tm.get_funds(dt).borrow_cash, borrow + 10.0)
        self.assertAlmostEqual(tm.get_debt_cash(dt2), borrow + 10.0 + 10.01)

    def test_TradeManager_settlement_interest_default_zero(self):
        """A cost model that knows nothing about the rates bills nothing at all"""
        dt = Datetime(200912310000)
        borrow = 10000.0
        tm = crtTM(dt, 1000000.0, TC_Zero(), "SYS", PythonMargin())
        tm.set_param("margin_accounting", True)
        self.assertTrue(tm.borrow_cash(dt, borrow))

        # the base cost model answers a zero rate for both kinds of liability
        self.assertEqual(tm.cost_func.get_borrow_cash_rate(dt), 0.0)
        self.assertEqual(tm.cost_func.get_borrow_stock_rate(dt, sm['sh000001']), 0.0)

        # nothing is billed, so a book with no holding is not settled by the interest
        self.assertFalse(tm.settlement(dt))
        self.assertFalse(tm.is_settled(dt))
        self.assertEqual(count_interest(tm), 0)
        self.assertAlmostEqual(tm.get_debt_cash(dt), borrow)
        self.assertAlmostEqual(tm.get_funds(dt).net_assets, 1000000.0)

    def test_TradeManager_settlement_annual_days(self):
        """The proration base is validated before anything is posted, and it is the divisor"""
        dt = Datetime(200912310000)
        borrow = 10000.0
        tm = crtInterestTM(dt, 1000000.0, 0.365, 0.0, 0)
        self.assertTrue(tm.borrow_cash(dt, borrow))

        # a base that is not positive fails the whole settlement and posts nothing
        self.assertRaises(Exception, tm.settlement, dt)
        self.assertFalse(tm.is_settled(dt))
        self.assertEqual(count_interest(tm), 0)
        self.assertAlmostEqual(tm.get_debt_cash(dt), borrow)

        # a base of 100 prorates the same annual rate over 100 days
        tm.set_param("annual_days", 100)
        self.assertTrue(tm.settlement(dt))
        self.assertAlmostEqual(tm.get_debt_cash(dt), borrow + 36.50)

    def test_TC_MarginInterest(self):
        """The interest part answers the rates and leaves the fees to the inner cost model"""
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        price = stock.get_market_value(dt, "DAY")

        # with nothing configured it answers a zero rate for both kinds of liability
        plain = TC_MarginInterest()
        self.assertEqual(plain.name, "TC_MarginInterest")
        self.assertEqual(plain.get_borrow_cash_rate(dt), 0.0)
        self.assertEqual(plain.get_borrow_stock_rate(dt, stock), 0.0)

        # the fees come from the inner model, its own rates stay unused
        inner = TC_FixedA()
        cost = TC_MarginInterest(inner, 0.06, "2:0.10", 0.20)
        self.assertGreater(inner.get_buy_cost(dt, stock, price, 100).commission, 0.0)
        self.assertAlmostEqual(cost.get_buy_cost(dt, stock, price, 100).commission,
                               inner.get_buy_cost(dt, stock, price, 100).commission)
        self.assertAlmostEqual(cost.get_borrow_cash_rate(dt), 0.06)
        self.assertAlmostEqual(cost.get_borrow_stock_rate(dt, stock), 0.10)

        # the lending table matches the type before the market, then falls back; in this data
        # sh000001 is type 2 of SH, sz000001 is type 1 of SZ
        by_market = TC_MarginInterest(TC_Zero(), 0.0, "SZ:0.12", 0.20)
        self.assertAlmostEqual(by_market.get_borrow_stock_rate(dt, sm['sz000001']), 0.12)
        self.assertAlmostEqual(by_market.get_borrow_stock_rate(dt, stock), 0.20)
        both = TC_MarginInterest(TC_Zero(), 0.0, "1:0.10, SZ:0.12", 0.20)
        self.assertAlmostEqual(both.get_borrow_stock_rate(dt, sm['sz000001']), 0.10)

        # a replacement of the table takes effect at once, the clone carries it over
        cost.set_param("borrow_stock_rates", "SH:0.08")
        self.assertAlmostEqual(cost.get_borrow_stock_rate(dt, stock), 0.08)
        copied = cost.clone()
        self.assertEqual(copied.get_param("borrow_stock_rates"), "SH:0.08")
        self.assertAlmostEqual(copied.get_borrow_cash_rate(dt), 0.06)
        self.assertAlmostEqual(copied.get_borrow_stock_rate(dt, stock), 0.08)

        # the rates are annualized ratios of the [0, 1) interval: unlike the margin ratios a zero
        # rate is valid and bills nothing, while 1.0 is out of range
        self.assertEqual(TC_MarginInterest(TC_Zero(), 0.0, "SH:0").get_borrow_stock_rate(dt, stock), 0.0)
        for bad in ("SH:abc", "SH", "SH:1.0", "SH:-0.1"):
            self.assertRaises(Exception, TC_MarginInterest, TC_Zero(), 0.0, bad)
        for rate in (1.0, 1.2, -0.1):
            self.assertRaises(Exception, TC_MarginInterest, TC_Zero(), rate)
            self.assertRaises(Exception, TC_MarginInterest, TC_Zero(), 0.0, "", rate)

    def test_TradeManager_settlement_interest_part(self):
        """An account billed through the TC_MarginInterest part carries both liabilities"""
        rb = sm['SHFRB0000']
        bars = rb.get_kdata(Query(100, 104, "DAY"))
        self.assertGreaterEqual(len(bars), 2)
        dt1, close1 = bars[0].datetime, bars[0].close
        dt2, close2 = bars[1].datetime, bars[1].close

        number = 10.0
        tm = crtTM(dt1, 1000000.0, TC_MarginInterest(TC_FixedA(), 0.365, "12:0.365"), "SYS",
                   PythonMargin())
        tm.set_param("margin_accounting", True)
        self.assertTrue(tm.borrow_cash(dt1, 10000.0))
        self.assertTrue(tm.borrow_stock(dt1, rb, close1, number))
        self.assertTrue(tm.settlement(dt2))

        # one record per liability, and the same table answers both queries
        records = [tr for tr in tm.get_trade_list() if tr.business == BUSINESS.INTEREST]
        self.assertEqual(len(records), 2)
        cash_bill = [tr for tr in records if tr.stock.is_null()][0]
        stock_bill = [tr for tr in records if not tr.stock.is_null()][0]
        self.assertAlmostEqual(cash_bill.real_price, 10.0)
        self.assertEqual(stock_bill.stock, rb)
        self.assertAlmostEqual(stock_bill.number, number)
        self.assertAlmostEqual(stock_bill.real_price,
                               round_ex(round_ex(close2 * number * rb.unit) * 0.365 / 365.0))
        self.assertAlmostEqual(tm.get_debt_cash(dt2),
                               10000.0 + cash_bill.real_price + stock_bill.real_price)

        # the trading fees still come from the inner model, the commission of a buy being the one
        # the A-share cost model answers for the same trade
        buy_price = rb.get_market_value(dt2, "DAY")
        trade = tm.buy(dt2, rb, buy_price, 10.0)
        self.assertEqual(trade.business, BUSINESS.BUY)
        self.assertAlmostEqual(trade.cost.commission,
                               TC_FixedA().get_buy_cost(dt2, rb, buy_price, 10.0).commission)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MarginTest)


if __name__ == '__main__':
    unittest.main()
