#!/usr/bin/python
# -*- coding: utf-8 -*-
# gb18030

#===============================================================================
# Author: fasiondog
# History: 1)20120221, Added by fasiondog
#===============================================================================

import unittest

from test_init import *


class PythonTradeCost(TradeCostBase):
    def __init__(self):
        super(PythonTradeCost, self).__init__("PythonTradeCost")

    def get_buy_cost(self, date, stock, price, num):
        return CostRecord(1.0, 1.0, 1.0, 1.0, 4.0)

    def get_sell_cost(self, date, stock, price, num):
        return CostRecord(2.0, 2.0, 2.0, 2.0, 8.0)

    def _clone(self):
        return PythonTradeCost()


class TradeCostTest(unittest.TestCase):
    def test_PythonTradeCost(self):
        stock = sm['sh000001']
        tc = PythonTradeCost()
        self.assertEqual(tc.name, "PythonTradeCost")

        cost = tc.get_sell_cost(Datetime(201001010000), stock, 10.0, 100)
        self.assertEqual(cost, CostRecord(2, 2, 2, 2, 8))
        #print tc

        clone_tc = tc.clone()
        self.assertEqual(clone_tc.name, "PythonTradeCost")
        cost = clone_tc.get_sell_cost(Datetime(201001010000), stock, 10.0, 100)
        self.assertEqual(cost, CostRecord(2, 2, 2, 2, 8))

    def test_PythonTradeCost_dispatch(self):
        """The overrides must be reachable from the C++ account, not only when called directly

        Looking them up by the C++ names used to find nothing and fall back to the base class, so
        the same trade silently got no cost at all.
        """
        stock = sm['sh000001']
        dt = Datetime(200001040000)
        price = stock.get_market_value(dt, "DAY")

        tm = crtTM(dt, 1000000.0, PythonTradeCost())
        zero = crtTM(dt, 1000000.0, TC_Zero())

        tr = tm.buy(dt, stock, price, 100)
        ztr = zero.buy(dt, stock, price, 100)
        self.assertEqual(tr.cost.total, 4.0)
        self.assertAlmostEqual(ztr.cash - tr.cash, 4.0)

        # the fee comes out of the proceeds, counted apart from the buy cost already in the cash
        held, zheld = tm.current_cash, zero.current_cash
        tr = tm.sell(dt, stock, price, 100)
        ztr = zero.sell(dt, stock, price, 100)
        self.assertEqual(tr.cost.total, 8.0)
        self.assertAlmostEqual((ztr.cash - zheld) - (tr.cash - held), 8.0)

    def test_ZeroTC(self):
        stock = sm['sh000001']
        tc = TC_Zero()
        cost = tc.get_sell_cost(Datetime(201001010000), stock, 10.0, 100)
        self.assertEqual(cost, CostRecord(0, 0, 0, 0, 0))
        cost = tc.get_sell_cost(Datetime(201001010000), stock, 10.0, 100)
        self.assertEqual(cost, CostRecord(0, 0, 0, 0, 0))

        clone_tc = tc.clone()
        cost = clone_tc.get_sell_cost(Datetime(201001010000), stock, 10.0, 100)
        self.assertEqual(cost, CostRecord(0, 0, 0, 0, 0))
        cost = clone_tc.get_sell_cost(Datetime(201001010000), stock, 10.0, 100)
        self.assertEqual(cost, CostRecord(0, 0, 0, 0, 0))

    def test_FixedAPre2015TC(self):
        stock = sm['sh000001']
        tc = TC_FixedAPre2015()
        cost = tc.get_sell_cost(Datetime(200101010000), stock, 10.0, 2100)
        self.assertEqual(cost, CostRecord(37.8, 0, 2.1, 0, 39.9))
        cost = tc.get_sell_cost(Datetime(200101010000), stock, 10.0, 2100)
        self.assertEqual(cost, CostRecord(37.8, 0, 2.1, 0, 39.9))

        clone_tc = tc.clone()
        cost = clone_tc.get_sell_cost(Datetime(200101010000), stock, 10.0, 2100)
        self.assertEqual(cost, CostRecord(37.8, 0, 2.1, 0, 39.9))
        cost = clone_tc.get_sell_cost(Datetime(200101010000), stock, 10.0, 2100)
        self.assertEqual(cost, CostRecord(37.8, 0, 2.1, 0, 39.9))


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(TradeCostTest)