#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Create on: 2026-11-03
#    Author: fasiondog

import unittest

from test_init import *


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
        """Phase B: a margin security locks the margin only and reports it"""
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
        self.assertAlmostEqual(tm.get_position(dt, stock).margin, 5000.0)
        self.assertAlmostEqual(tm.get_position(dt, stock).open_basis, price * 100.0 * stock.unit)
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
        self.assertAlmostEqual(tm.get_funds().occupied_margin, 0.0)
        self.assertAlmostEqual(tm.get_position(dt, stock).open_basis, 0.0)

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


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MarginTest)


if __name__ == '__main__':
    unittest.main()
