#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: 1)20130311, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *


class EnvironmentPython(EnvironmentBase):
    def __init__(self):
        super(EnvironmentPython, self).__init__("EnvironmentPython")
        self.set_param("n", 10)
        self._m_flag = False

    def _calculate(self):
        # _calculate is dispatched by setQuery; self.query carries the query condition and the
        # valid dates must be registered through _add_valid
        k = sm['sh000001'].get_kdata(self.query)
        if k.empty():
            return
        self._add_valid(k[0].datetime)
        self._add_valid(k[len(k) - 1].datetime)

    def _reset(self):
        self._m_flag = False

    def _clone(self):
        p = EnvironmentPython()
        p._m_flag = self._m_flag
        return p


class EnvironmentTest(unittest.TestCase):
    def test_EnvironmentBase(self):
        p = EnvironmentPython()
        self.assertEqual(p.name, "EnvironmentPython")
        self.assertEqual(p.get_param("n"), 10)
        p.set_param("n", 20)
        self.assertEqual(p.get_param("n"), 20)

        # _calculate is not dispatched until the query is set, so nothing is valid yet
        self.assertEqual(p.is_valid(Datetime(200101010000)), False)

        # Setting the query triggers the real _calculate, after which is_valid goes through the
        # C++ side (m_date_index/m_values) instead of any Python-side override
        p.query = Query(-100)
        k = sm['sh000001'].get_kdata(Query(-100))
        self.assertEqual(p.is_valid(k[0].datetime), True)
        self.assertEqual(p.is_valid(k[len(k) - 1].datetime), True)
        self.assertEqual(p.is_valid(Datetime(200101010000)), False)

        # The clone dispatches the real _clone and keeps the private state
        p._m_flag = True
        p_clone = p.clone()
        self.assertIsNot(p_clone, p)
        self.assertEqual(p_clone.name, "EnvironmentPython")
        self.assertEqual(p_clone.get_param("n"), 20)
        self.assertEqual(p_clone._m_flag, True)
        self.assertEqual(p_clone.is_valid(k[0].datetime), True)

        p.set_param("n", 1)
        p_clone.set_param("n", 3)
        self.assertEqual(p.get_param("n"), 1)
        self.assertEqual(p_clone.get_param("n"), 3)

        # The reset clears the valid dates and dispatches the real _reset
        p.reset()
        self.assertEqual(p._m_flag, False)
        self.assertEqual(p.is_valid(k[0].datetime), False)


def test_crtEV_func(self):
    self._add_valid(Datetime(200101010000))


class TestCrtEV(unittest.TestCase):
    def test_crtEV(self):
        p = crtEV(test_crtEV_func, params={'n': 10}, name='EnvironmentPython')
        self.assertEqual(p.name, "EnvironmentPython")
        self.assertEqual(p.get_param("n"), 10)
        p.set_param("n", 20)
        self.assertEqual(p.get_param("n"), 20)
        p.query = Query(-100)  # The _calculate function is called only when ev sets the query object
        self.assertEqual(p.is_valid(Datetime(200101010000)), True)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "EnvironmentPython")
        self.assertEqual(p_clone.get_param("n"), 20)
        self.assertEqual(p_clone.is_valid(Datetime(200101010000)), True)

        p.set_param("n", 1)
        p_clone.set_param("n", 3)
        self.assertEqual(p.get_param("n"), 1)
        self.assertEqual(p_clone.get_param("n"), 3)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(EnvironmentTest)


def suiteTestCrtEV():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtEV)
