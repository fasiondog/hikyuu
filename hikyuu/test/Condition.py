#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: 1)20130311, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *
from hikyuu import *


class ConditionPython(ConditionBase):
    def __init__(self):
        super(ConditionPython, self).__init__("ConditionPython")
        self.set_param("n", 10)
        self._m_calculated = False

    def _calculate(self):
        # _calculate takes no argument; the KData must be read from self.to
        self._m_calculated = True
        k = self.to
        for i in range(len(k)):
            if k[i].close > k[i].open:
                self._add_valid(k[i].datetime)

    def _reset(self):
        self._m_calculated = False

    def _clone(self):
        return ConditionPython()


class ConditionTest(unittest.TestCase):
    def test_ConditionBase(self):
        p = ConditionPython()
        self.assertEqual(p.name, "ConditionPython")
        self.assertEqual(p.get_param("n"), 10)
        p.set_param("n", 20)
        self.assertEqual(p.get_param("n"), 20)

        k = sm['sh000001'].get_kdata(Query(-100))
        self.assertEqual(k.empty(), False)
        expect = [k[i].datetime for i in range(len(k)) if k[i].close > k[i].open]
        self.assertTrue(len(expect) > 0)

        # setting the trading object must dispatch _calculate; is_valid is no longer overridden
        p.sg = SignalBase()
        p.to = k
        self.assertEqual(p._m_calculated, True)
        self.assertEqual(len(p.get_datetime_list()), len(expect))
        for d in expect:
            self.assertTrue(p.is_valid(d))

        invalid = [k[i].datetime for i in range(len(k)) if k[i].close <= k[i].open]
        if invalid:
            self.assertFalse(p.is_valid(invalid[0]))
        self.assertFalse(p.is_valid(Datetime(200101010000)))

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "ConditionPython")
        self.assertEqual(p_clone.get_param("n"), 20)
        self.assertEqual(len(p_clone.get_datetime_list()), len(expect))

        p.set_param("n", 1)
        p_clone.set_param("n", 3)
        self.assertEqual(p.get_param("n"), 1)
        self.assertEqual(p_clone.get_param("n"), 3)

        p.reset()
        self.assertEqual(len(p.get_datetime_list()), 0)


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
        p.sg = SignalBase()  # The signal generator must already be specified when setting the trading object of cn
        p.to = k  # The _calculate function is called only when cn sets the trading object
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
