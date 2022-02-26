#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20130321, Added by fasiondog
#===============================================================================

import unittest

from test_init import *


class ProfitGoalPython(ProfitGoalBase):
    def __init__(self):
        super(ProfitGoalPython, self).__init__("ProfitGoalPython")
        self._x = 0

    def getGoal(self, datetime, price):
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
        self.assertEqual(p.getGoal(Datetime(200101010000), 1.0), 0.0)

        self.assertEqual(p._x, 0)
        p._x = 10
        self.assertEqual(p._x, 10)
        self.assertEqual(p.getGoal(Datetime(200101010000), 1.0), 1.0)
        p.reset()
        self.assertEqual(p._x, 0)

        p._x = 10
        p_clone = p.clone()
        self.assertEqual(p_clone._x, 10)
        p.reset()
        self.assertEqual(p._x, 0)
        self.assertEqual(p_clone._x, 10)


def testCrtPG(self):
    pass


def testGetGoal(self, datetime, price):
    return 10.0 if datetime == Datetime(200101010000) else 0.0


class TestCrtPG(unittest.TestCase):
    def test_crt_pg(self):
        p = crtPG(testCrtPG, params={'n': 10}, name="ProfitGoalPython")
        p.getGoal = testGetGoal
        self.assertEqual(p.name, "ProfitGoalPython")
        self.assertEqual(p.getGoal(p, Datetime(200101010000), 1.0), 10.0)
        self.assertEqual(p.getGoal(p, Datetime(200101020000), 1.0), 0.0)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "ProfitGoalPython")


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ProfitGoalTest)


def suiteTestCrtPG():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtPG)