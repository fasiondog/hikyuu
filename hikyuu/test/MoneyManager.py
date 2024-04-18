#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# 作者：fasiondog
# 历史：1）20130316, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *


class MoneyManagerPython(MoneyManagerBase):
    def __init__(self):
        super(MoneyManagerPython, self).__init__("MoneyManagerPython")
        self.set_param("n", 10)
        self._m_flag = False

    def get_buy_num(self, datetime, stock, price, risk):
        if self._m_flag:
            return 10
        else:
            return 20

    def _reset(self):
        if self._m_flag:
            self._m_flag = False
        else:
            self._m_flag = True

    def _clone(self):
        p = MoneyManagerPython()
        p._m_flag = self._m_flag
        return p


class MoneyManagerTest(unittest.TestCase):
    def test_ConditionBase(self):
        stock = sm['sh000001']
        p = MoneyManagerPython()
        self.assertEqual(p.name, "MoneyManagerPython")
        self.assertEqual(p.get_param("n"), 10)
        p.set_param("n", 20)
        self.assertEqual(p.get_param("n"), 20)
        self.assertEqual(p.get_buy_num(Datetime(200101010000), stock, 10.0, 0.0), 20)
        p.reset()
        self.assertEqual(p.get_buy_num(Datetime(200101010000), stock, 10.0, 0.0), 10)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "MoneyManagerPython")
        self.assertEqual(p_clone.get_param("n"), 20)
        self.assertEqual(p_clone.get_buy_num(Datetime(200101010000), stock, 10, 0.0), 10)

        p.set_param("n", 1)
        p_clone.set_param("n", 3)
        self.assertEqual(p.get_param("n"), 1)
        self.assertEqual(p_clone.get_param("n"), 3)


def testCrtMM(self):
    pass


def testget_buy_num(self, datetime, stock, price, risk, part):
    return 10.0 if datetime == Datetime(200101010000) else 0.0


class TestCrtMM(unittest.TestCase):
    def test_crt_mm(self):
        p = crtMM(testget_buy_num, testCrtMM, params={'n': 10}, name="TestMM")
        p.tm = crtTM(Datetime(200101010000))
        self.assertEqual(p.name, "TestMM")
        stock = sm['sh000001']
        self.assertEqual(p.get_buy_num(Datetime(200101010000), stock, 1.0, 1.0, SystemPart.MM), 10.0)
        self.assertEqual(p.get_buy_num(Datetime(200101020000), stock, 1.0, 1.0, SystemPart.MM), 0.0)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "TestMM")


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MoneyManagerTest)


def suiteTestCrtMM():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtMM)
