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


class TestEVManual(unittest.TestCase):
    def test_ev_manual_get_value_and_get_values(self):
        ev = EV_Manual()
        self.assertFalse(ev.is_valid(Datetime(202001010000)))
        self.assertEqual(ev.get_value(Datetime(202001010000)), 0.0)

        ev._add_valid(Datetime(202001010000), 2.0)
        ev._add_valid(Datetime(202001030000), 1.0)
        self.assertTrue(ev.is_valid(Datetime(202001010000)))
        self.assertTrue(ev.is_valid(Datetime(202001030000)))
        self.assertEqual(ev.get_value(Datetime(202001010000)), 2.0)
        self.assertEqual(ev.get_value(Datetime(202001020000)), 0.0)

        # get_values: one record per registered date, ordered by date
        ind = ev.get_values()
        self.assertEqual(len(ind), 2)
        self.assertAlmostEqual(ind[0], 2.0)
        self.assertAlmostEqual(ind[1], 1.0)

    def test_ev_manual_query_reset(self):
        # The dates survive an identical setQuery and are cleared by a different one
        ev = EV_Manual()
        ev.query = Query(-10)
        ev._add_valid(Datetime(202001010000))
        ev.query = Query(-10)
        self.assertTrue(ev.is_valid(Datetime(202001010000)))
        ev.query = Query(-20)
        self.assertFalse(ev.is_valid(Datetime(202001010000)))


class TestEVComboGetValues(unittest.TestCase):
    def test_combo_get_values_consistency(self):
        # (a*b).get_values() must match the C++ operator semantics; the combination is
        # query-driven, so both sides register their dates from _calculate
        def calc_a(self):
            self._add_valid(Datetime(20010101), 2.0)
            self._add_valid(Datetime(20010103), 1.0)

        def calc_b(self):
            self._add_valid(Datetime(20010101), 3.0)
            self._add_valid(Datetime(20010102), 1.0)

        a = crtEV(calc_a, name='EVA')
        b = crtEV(calc_b, name='EVB')
        q = Query(Datetime(20010101), Datetime(20010105))
        a.query = q
        b.query = q

        # Multi: both sides valid on 0101 -> positive product; no output elsewhere
        m = a * b
        m.query = q
        self.assertTrue(m.is_valid(Datetime(20010101)))
        self.assertFalse(m.is_valid(Datetime(20010102)))
        self.assertFalse(m.is_valid(Datetime(20010103)))
        self.assertAlmostEqual(m.get_value(Datetime(20010101)), 6.0)
        ind = m.get_values()
        self.assertEqual(len(ind), 1)
        self.assertAlmostEqual(ind[0], 6.0)

        # Add: the union of both sides; a date present on only one side takes that side's value
        s = a + b
        s.query = q
        self.assertAlmostEqual(s.get_value(Datetime(20010101)), 5.0)
        self.assertAlmostEqual(s.get_value(Datetime(20010102)), 1.0)
        self.assertAlmostEqual(s.get_value(Datetime(20010103)), 1.0)
        ind_s = s.get_values()
        self.assertEqual(len(ind_s), 3)
        self.assertAlmostEqual(ind_s[0], 5.0)
        self.assertAlmostEqual(ind_s[1], 1.0)
        self.assertAlmostEqual(ind_s[2], 1.0)

        # And: only 0101 is valid on both sides, with the value fixed to 1.0
        both = a & b
        both.query = q
        self.assertTrue(both.is_valid(Datetime(20010101)))
        self.assertFalse(both.is_valid(Datetime(20010102)))
        self.assertAlmostEqual(both.get_value(Datetime(20010101)), 1.0)


class EnvironmentCustomRule(EnvironmentBase):
    def __init__(self):
        super(EnvironmentCustomRule, self).__init__("EnvironmentCustomRule")

    def _calculate(self):
        self._add_valid(Datetime(202001010000), -1.0)
        self._add_valid(Datetime(202001020000), 1.0)

    def _is_valid(self, datetime, value):
        # A custom rule: a negative value is valid, everything else is invalid
        return value < 0.0

    def _clone(self):
        return EnvironmentCustomRule()


class TestEVCustomRule(unittest.TestCase):
    def test_custom_is_valid_rule(self):
        p = EnvironmentCustomRule()
        p.query = Query(-10)
        self.assertTrue(p.is_valid(Datetime(202001010000)))
        self.assertFalse(p.is_valid(Datetime(202001020000)))

        # The clone keeps the custom rule
        p_clone = p.clone()
        self.assertTrue(p_clone.is_valid(Datetime(202001010000)))
        self.assertFalse(p_clone.is_valid(Datetime(202001020000)))


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(EnvironmentTest)


def suiteTestCrtEV():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtEV)


def suiteTestEVManual():
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(TestEVManual))
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(TestEVComboGetValues))
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(TestEVCustomRule))
    return tests
