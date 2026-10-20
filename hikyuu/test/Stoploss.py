#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: 1)20130321, Added by fasiondog
# ===============================================================================

import unittest
import threading

from test_init import *


class StoplossPython(StoplossBase):
    def __init__(self):
        super(StoplossPython, self).__init__()
        self._x = 0

    def name(self):
        return "StoplossPython"

    def getPrice(self, datetime, price):
        if self._x < 10:
            return 0.0
        return 1.0

    def _reset(self):
        self._x = 0

    def _clone(self):
        p = StoplossPython()
        p._x = self._x
        return p

    def _calculate(self):
        """ do nothin """


class StoplossTest(unittest.TestCase):
    def test_StoplossBase(self):
        p = StoplossPython()
        self.assertEqual(p.name(), "StoplossPython")
        self.assertEqual(p.getPrice(Datetime(200101010000), 1.0), 0.0)

        self.assertEqual(p._x, 0)
        p._x = 10
        self.assertEqual(p._x, 10)
        self.assertEqual(p.getPrice(Datetime(200101010000), 1.0), 1.0)
        p.reset()
        self.assertEqual(p._x, 0)

        p._x = 10
        p_clone = p.clone()
        self.assertEqual(p_clone._x, 10)
        p.reset()
        self.assertEqual(p._x, 0)
        self.assertEqual(p_clone._x, 10)


def test_crtST_func(self):
    pass


def test_getPrice_func(self, datetime, price):
    return 10.0 if datetime == Datetime(200101010000) else 0.0


class TestCrtST(unittest.TestCase):
    def test_crtST(self):
        p = crtST(test_crtST_func, params={'n': 10}, name="StoplossPython")
        p.getPrice = test_getPrice_func
        self.assertEqual(p.name, "StoplossPython")
        self.assertEqual(p.getPrice(p, Datetime(200101010000), 1.0), 10.0)
        self.assertEqual(p.getPrice(p, Datetime(200101020000), 1.0), 0.0)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "StoplossPython")


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(StoplossTest)


def suiteTestCrtST():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtST)


def suiteTestConcurrent():
    return unittest.TestLoader().loadTestsFromTestCase(TestConcurrentSetTO)


class TestConcurrentSetTO(unittest.TestCase):
    def test_concurrent_setTO_clone_reset(self):
        """Concurrent setTO/clone/reset from Python threads must not crash or deadlock"""
        kdata = sm["sh000001"].get_kdata(Query(-100))
        st_cpp = ST_FixedPercent(0.03)
        st_py = StoplossPython()

        errors = []

        def work(st):
            try:
                for _ in range(50):
                    st.to = kdata
                    st.clone()
                    st.reset()
            except Exception as e:  # noqa: BLE001
                errors.append(e)

        # A pure C++ ST releases the GIL during setTO/clone/reset, and a Python ST holds it;
        # both must complete without deadlock
        threads = [
            threading.Thread(target=work, args=(st,)) for st in (st_cpp, st_cpp, st_py, st_py)
        ]
        for t in threads:
            t.start()
        for t in threads:
            t.join(timeout=60)
            self.assertFalse(t.is_alive(), "deadlock detected in setTO/clone/reset")
        self.assertEqual(errors, [])
