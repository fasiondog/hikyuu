#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# Author: fasiondog
# History: 1)20130321, Added by fasiondog
#===============================================================================

import unittest

from test_init import *


class ProfitGoalPython(ProfitGoalBase):
    def __init__(self):
        super(ProfitGoalPython, self).__init__("ProfitGoalPython")
        self._x = 0

    def _get_goal(self, datetime, price):
        if self._x < 10:
            return 0.0
        return 1.0

    def _reset(self):
        self._x = 0

    def _clone(self):
        p = ProfitGoalPython()
        p._x = self._x
        return p

    def _calculate(self):
        """ do nothing """


class ProfitGoalTest(unittest.TestCase):
    def test_ProfitGoalBase(self):
        p = ProfitGoalPython()
        self.assertEqual(p.name, "ProfitGoalPython")
        self.assertEqual(p.get_goal(Datetime(200101010000), 1.0), 0.0)

        self.assertEqual(p._x, 0)
        p._x = 10
        self.assertEqual(p._x, 10)
        self.assertEqual(p.get_goal(Datetime(200101010000), 1.0), 1.0)
        p.reset()
        self.assertEqual(p._x, 0)

        p._x = 10
        p_clone = p.clone()
        self.assertEqual(p_clone._x, 10)
        p.reset()
        self.assertEqual(p._x, 0)
        self.assertEqual(p_clone._x, 10)


def testGetGoal(self, datetime, price):
    return 10.0 if datetime == Datetime(200101010000) else 0.0


class TestCrtPG(unittest.TestCase):
    def test_crt_pg(self):
        # crtPG assigns testGetGoal to the type's _get_goal, so the C++ trampoline can find it
        p = crtPG(testGetGoal, params={'n': 10}, name="ProfitGoalCrtPG")
        self.assertEqual(p.name, "ProfitGoalCrtPG")

        # assert via get_goal to go through the C++ wrapper and the pybind trampoline dispatch
        self.assertEqual(p.get_goal(Datetime(200101010000), 1.0), 10.0)
        self.assertEqual(p.get_goal(Datetime(200101020000), 1.0), 0.0)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "ProfitGoalCrtPG")
        self.assertEqual(p_clone.get_goal(Datetime(200101010000), 1.0), 10.0)
        self.assertEqual(p_clone.get_goal(Datetime(200101020000), 1.0), 0.0)

    def test_crt_pg_notify(self):
        invoked = []

        def on_buy(self, tr):
            invoked.append("buy")

        def on_sell(self, tr):
            invoked.append("sell")

        p = crtPG(testGetGoal, buy_notify=on_buy, sell_notify=on_sell, name="ProfitGoalCrtPGNotify")
        # the callbacks must sit under the names the C++ trampoline looks up (no leading underscore)
        self.assertIs(type(p).__dict__.get("buy_notify"), on_buy)
        self.assertIs(type(p).__dict__.get("sell_notify"), on_sell)
        p.buy_notify(None)
        p.sell_notify(None)
        self.assertEqual(invoked, ["buy", "sell"])


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ProfitGoalTest)


def suiteTestCrtPG():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtPG)