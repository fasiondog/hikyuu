#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# 作者：fasiondog
# 历史：1）20130311, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *
from hikyuu import *


class ConditionPython(ConditionBase):
    def __init__(self):
        super(ConditionPython, self).__init__("ConditionPython")
        self.set_param("n", 10)
        self._m_flag = False

    def is_valid(self, datetime):
        return self._m_flag

    def _reset(self):
        if self._m_flag:
            self._m_flag = False
        else:
            self._m_flag = True

    def _clone(self):
        p = ConditionPython()
        p._m_flag = self._m_flag
        return p


class ConditionTest(unittest.TestCase):
    def test_ConditionBase(self):
        p = ConditionPython()
        self.assertEqual(p.name, "ConditionPython")
        self.assertEqual(p.get_param("n"), 10)
        p.set_param("n", 20)
        self.assertEqual(p.get_param("n"), 20)
        self.assertEqual(p.is_valid(Datetime(200101010000)), False)
        p.reset()
        self.assertEqual(p.is_valid(Datetime(200101010000)), True)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "ConditionPython")
        self.assertEqual(p_clone.get_param("n"), 20)
        self.assertEqual(p_clone.is_valid(Datetime(200101010000)), True)

        p.set_param("n", 1)
        p_clone.set_param("n", 3)
        self.assertEqual(p.get_param("n"), 1)
        self.assertEqual(p_clone.get_param("n"), 3)


def testCondition(self):
    self._add_valid(Datetime(201108120000))
    self._add_valid(Datetime(201108250000))


class TestCrtCN(unittest.TestCase):
    def test_crtCN(self):
        p = crtCN(testCondition, params={'n': 10}, name='ConditionPython')
        self.assertEqual(p.get_param("n"), 10)
        p.set_param("n", 20)
        self.assertEqual(p.get_param("n"), 20)

        k = sm['sh000001'].get_kdata(Query(-100))
        self.assertEqual(k.empty(), False)
        p.sg = SignalBase()  # cn设置交易对象时，必须已经指定了信号指示器
        p.to = k  # cn在设置交易对象时才会调用_caculate函数
        self.assertEqual(p.is_valid(Datetime(201108120000)), True)
        self.assertEqual(p.is_valid(Datetime(201108250000)), True)
        self.assertEqual(p.is_valid(Datetime(200101030000)), False)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "ConditionPython")
        self.assertEqual(p_clone.get_param("n"), 20)
        self.assertEqual(p_clone.is_valid(Datetime(201108120000)), True)

        p.set_param("n", 1)
        p_clone.set_param("n", 3)
        self.assertEqual(p.get_param("n"), 1)
        self.assertEqual(p_clone.get_param("n"), 3)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ConditionTest)


def suiteTestCrtCN():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtCN)
