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


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MarginTest)


if __name__ == '__main__':
    unittest.main()
