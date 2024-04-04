#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# 作者：fasiondog
# 历史：1）20130321, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *


class SignalPython(SignalBase):
    def __init__(self):
        super(SignalPython, self).__init__("SignalPython")
        self._x = 0
        self.set_param("test", 30)

    def _reset(self):
        self._x = 0

    def _clone(self):
        p = SignalPython()
        p._x = self._x
        return p

    def _calculate(self, kdata):
        self._add_buy_signal(Datetime(201201210000))
        self._add_sell_signal(Datetime(201201300000))


class SignalTest(unittest.TestCase):
    def test_SignalBase(self):
        p = SignalPython()
        self.assertEqual(p.name, "SignalPython")
        p.name = "SignalPythonTest"
        self.assertEqual(p.name, "SignalPythonTest")

        self.assertEqual(p.should_buy(Datetime(201201210000)), False)
        self.assertEqual(p.should_sell(Datetime(201201300000)), False)
        k = sm['sh000001'].get_kdata(Query(-100))
        self.assertEqual(k.empty(), False)
        p.to = k
        self.assertEqual(p.should_buy(Datetime(201201210000)), True)
        self.assertEqual(p.should_sell(Datetime(201201300000)), True)

        self.assertEqual(p.should_buy(Datetime(200101010000)), False)
        p._add_buy_signal(Datetime(200101010000))
        self.assertEqual(p.should_buy(Datetime(200101010000)), True)

        self.assertEqual(p.should_sell(Datetime(200101030000)), False)
        p._add_sell_signal(Datetime(200101030000))
        self.assertEqual(p.should_sell(Datetime(200101030000)), True)

        d = p.get_buy_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201210000), Datetime(200101010000)])

        d = p.get_sell_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201300000), Datetime(200101030000)])

        p_clone = p.clone()
        d = p_clone.get_buy_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201210000), Datetime(200101010000)])

        d = p_clone.get_sell_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201300000), Datetime(200101030000)])

        self.assertEqual(p._x, 0)
        p._x = 10
        self.assertEqual(p._x, 10)
        p.reset()
        self.assertEqual(p._x, 0)

        p._x = 20
        p_clone = p.clone()
        self.assertEqual(p_clone._x, 20)
        p.reset()
        self.assertEqual(p._x, 0)
        self.assertEqual(p_clone._x, 20)

        self.assertEqual(p.get_param("test"), 30)
        self.assertEqual(p_clone.get_param("test"), 30)
        p.set_param("test", 10)
        self.assertEqual(p.get_param("test"), 10)
        self.assertEqual(p_clone.get_param("test"), 30)


def testSignal(self, kdata):
    self._add_buy_signal(Datetime(201201210000))
    self._add_sell_signal(Datetime(201201300000))


class TestCrtSG(unittest.TestCase):
    def test_crtSG(self):
        p = crtSG(testSignal, params={'test': 30}, name='SG_TEST')
        self.assertEqual(p.name, "SG_TEST")
        p.name = "SignalPythonTest"
        self.assertEqual(p.name, "SignalPythonTest")

        self.assertEqual(p.should_buy(Datetime(201201210000)), False)
        self.assertEqual(p.should_sell(Datetime(201201300000)), False)
        k = sm['sh000001'].get_kdata(Query(-100))
        self.assertEqual(k.empty(), False)
        p.to = k
        self.assertEqual(p.should_buy(Datetime(201201210000)), True)
        self.assertEqual(p.should_sell(Datetime(201201300000)), True)

        self.assertEqual(p.should_buy(Datetime(200101010000)), False)
        p._add_buy_signal(Datetime(200101010000))
        self.assertEqual(p.should_buy(Datetime(200101010000)), True)

        self.assertEqual(p.should_sell(Datetime(200101030000)), False)
        p._add_sell_signal(Datetime(200101030000))
        self.assertEqual(p.should_sell(Datetime(200101030000)), True)

        d = p.get_buy_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201210000), Datetime(200101010000)])

        d = p.get_sell_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201300000), Datetime(200101030000)])

        p_clone = p.clone()
        self.assertFalse(p_clone is p)
        d = p_clone.get_buy_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201210000), Datetime(200101010000)])

        d = p_clone.get_sell_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201300000), Datetime(200101030000)])


class TestCrtSGWithClone(unittest.TestCase):
    def test_crtSG(self):
        p = crtSG(testSignal, params={'test': 30}, name='SG_TEST')
        p.x = 10
        p2 = p.clone()
        self.assertFalse(p2 is p)
        self.assertEqual(p2.name, "SG_TEST")
        self.assertEqual(p2.x, p.x)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(SignalTest)


def suiteTestCrtSG():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtSG)


def suiteTestCrtSGWithClone():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtSGWithClone)
