#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: 1)20130316, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *


class MoneyManagerPython(MoneyManagerBase):
    def __init__(self):
        super(MoneyManagerPython, self).__init__("MoneyManagerPython")
        self.set_param("n", 10)
        self._m_flag = False

    def _get_buy_num(self, datetime, stock, price, risk, part):
        return 100.0 if self._m_flag else 200.0

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
    def test_MoneyManagerPython(self):
        stock = sm['sh600000']
        p = MoneyManagerPython()
        p.tm = crtTM(Datetime(200101010000), 100000)
        self.assertEqual(p.name, "MoneyManagerPython")
        self.assertEqual(p.get_param("n"), 10)
        p.set_param("n", 20)
        self.assertEqual(p.get_param("n"), 20)
        # 经 C++ 公共接口派发至 Python 的 _get_buy_num 钩子
        self.assertEqual(p.get_buy_num(Datetime(200101010000), stock, 10.0, 1.0, SystemPart.MM),
                         200.0)
        p.reset()
        # reset 会清空内部 tm，需重新注入
        p.tm = crtTM(Datetime(200101010000), 100000)
        self.assertEqual(p.get_buy_num(Datetime(200101010000), stock, 10.0, 1.0, SystemPart.MM),
                         100.0)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "MoneyManagerPython")
        self.assertEqual(p_clone.get_param("n"), 20)
        self.assertEqual(p_clone.get_buy_num(Datetime(200101010000), stock, 10.0, 1.0,
                                             SystemPart.MM), 100.0)

        p.set_param("n", 1)
        p_clone.set_param("n", 3)
        self.assertEqual(p.get_param("n"), 1)
        self.assertEqual(p_clone.get_param("n"), 3)

    def test_support_mult_buy_sell(self):
        # @arg defaults to not supporting multi-trading
        p = MoneyManagerPython()
        self.assertEqual(p.support_mult_buy_sell, False)

        # @arg a Python custom MM can declare the multi-trading capability
        p.support_mult_buy_sell = True
        self.assertEqual(p.support_mult_buy_sell, True)

        # @arg a C++ built-in MM must not rewrite the capability flag via the property
        mm = MM_FixedCount()
        self.assertEqual(mm.support_mult_buy_sell, False)
        with self.assertRaises(HKUException):
            mm.support_mult_buy_sell = True
        self.assertEqual(mm.support_mult_buy_sell, False)

        # @arg the clone keeps the declared capability flag
        p_clone = p.clone()
        self.assertEqual(p_clone.support_mult_buy_sell, True)


def testget_buy_num(self, datetime, stock, price, risk, part):
    return 10.0 if datetime == Datetime(200101010000) else 0.0


class TestCrtMM(unittest.TestCase):
    def test_crt_mm(self):
        p = crtMM(testget_buy_num, None, params={'n': 10}, name="TestMM")
        p.tm = crtTM(Datetime(200101010000))
        self.assertEqual(p.name, "TestMM")
        stock = sm['sh000001']
        self.assertEqual(p.get_buy_num(Datetime(200101010000), stock, 1.0, 1.0, SystemPart.MM),
                         10.0)
        self.assertEqual(p.get_buy_num(Datetime(200101020000), stock, 1.0, 1.0, SystemPart.MM), 0.0)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "TestMM")

    def test_system_buy_number(self):
        # 端到端派发：System -> MoneyManagerBase::getBuyNumber -> Python _get_buy_num
        # （System 运行前会 reset MM，故使用无状态的自定义 MM）
        stock = sm['sh600000']
        tm = crtTM(Datetime(199901010000), 100000)
        mm = crtMM(lambda self, datetime, stock, price, risk, part: 100.0, name="FixedMM")
        sys = SYS_Simple()
        sys.tm = tm
        sys.sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10))
        sys.mm = mm
        sys.run(stock, Query(Datetime(199911100000), Datetime(200002250000), Query.DAY))
        buys = [tr for tr in tm.get_trade_list() if tr.business == BUSINESS.BUY]
        self.assertGreater(len(buys), 0)
        for tr in buys:
            self.assertEqual(tr.number, 100.0)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MoneyManagerTest)


def suiteTestCrtMM():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtMM)
