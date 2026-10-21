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

    def _get_price(self, datetime, price):
        # gated by _x so the unit test can toggle the 0 / 0.95*price branches
        if self._x < 10:
            return 0.0
        return price * 0.95

    def _get_short_price(self, datetime, price):
        if self._x < 10:
            return 0.0
        return price * 1.05

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
        # _x < 10 -> hook returns 0.0 (no stop-loss)
        self.assertEqual(p.get_price(Datetime(200101010000), 1.0), 0.0)
        self.assertEqual(p.get_short_price(Datetime(200101010000), 1.0), 0.0)

        self.assertEqual(p._x, 0)
        p._x = 10
        self.assertEqual(p._x, 10)
        # _x >= 10 -> hooks return 0.95/1.05 * price, dispatched through the C++ layer
        self.assertAlmostEqual(p.get_price(Datetime(200101010000), 1.0), 0.95)
        self.assertAlmostEqual(p.get_short_price(Datetime(200101010000), 1.0), 1.05)
        p.reset()
        self.assertEqual(p._x, 0)
        self.assertEqual(p.get_price(Datetime(200101010000), 1.0), 0.0)

        p._x = 10
        p_clone = p.clone()
        self.assertEqual(p_clone._x, 10)
        self.assertAlmostEqual(p_clone.get_price(Datetime(200101010000), 1.0), 0.95)
        self.assertAlmostEqual(p_clone.get_short_price(Datetime(200101010000), 1.0), 1.05)
        p.reset()
        self.assertEqual(p._x, 0)
        self.assertEqual(p_clone._x, 10)


class StoplossPythonHook(StoplossBase):
    """Stateless hook used to prove the Python _get_price is dispatched by System.run().

    It must not gate on any mutable flag: System.setTO() calls _reset() on the part,
    which would clear such a flag before the run and make the hook return 0.
    """

    def name(self):
        return "StoplossPythonHook"

    def _get_price(self, datetime, price):
        return price * 0.95

    def _get_short_price(self, datetime, price):
        return price * 1.05

    def _clone(self):
        return StoplossPythonHook()

    def _calculate(self):
        """ do nothin """


class MySG(SignalBase):
    def __init__(self):
        super(MySG, self).__init__()

    def _clone(self):
        return MySG()

    def _calculate(self, kdata):
        # emit a buy 10 bars from the end and a sell on the last bar
        if len(kdata) > 10:
            self._add_buy_signal(kdata[len(kdata) - 10].datetime)
            self._add_sell_signal(kdata[len(kdata) - 1].datetime)


class TestStoplossDispatch(unittest.TestCase):
    def test_python_stoploss_effective_in_run(self):
        """The Python _get_price hook must be invoked by System.run(), not just shadowed."""
        k = sm["sh600000"].get_kdata(Query(-100))
        self.assertGreater(len(k), 10)

        sys = SYS_Simple()
        sys.tm = crtTM(Datetime(200101010000), 1000000)
        sys.sg = MySG()
        sys.mm = MM_FixedCount(100)
        sys.st = StoplossPythonHook()
        sys.to = k
        sys.run(k, False, False)

        trades = sys.tm.get_trade_list()
        buys = [t for t in trades if t.business == BUSINESS.BUY]
        self.assertGreater(len(buys), 0)
        for t in buys:
            # The Python hook returned 0.95 * planPrice; with no price adjustment the
            # recorded stoploss must be ~95% of the actual buy price. A default/zero
            # stoploss (e.g. if the hook were not dispatched) would fail both asserts.
            self.assertNotEqual(t.stoploss, 0.0)
            self.assertAlmostEqual(t.stoploss / t.real_price, 0.95, delta=0.02)


def test_crtST_get_price(self, datetime, price):
    return 10.0 if datetime == Datetime(200101010000) else 0.0


class TestCrtST(unittest.TestCase):
    def test_crtST(self):
        p = crtST(test_crtST_get_price, name="StoplossPython")
        self.assertEqual(p.name, "StoplossPython")
        # get_price delegates to the bound _get_price hook (C++ -> Python dispatch)
        self.assertEqual(p.get_price(Datetime(200101010000), 1.0), 10.0)
        self.assertEqual(p.get_price(Datetime(200101020000), 1.0), 0.0)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "StoplossPython")


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(StoplossTest)


def suiteTestCrtST():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtST)


def suiteTestDispatch():
    return unittest.TestLoader().loadTestsFromTestCase(TestStoplossDispatch)


def suiteTestConcurrent():
    return unittest.TestLoader().loadTestsFromTestCase(TestConcurrentSetTO)


class TestConcurrentSetTO(unittest.TestCase):
    def test_concurrent_setTO_clone_reset(self):
        """Concurrent setTO/clone/reset from Python threads must not crash or deadlock"""
        kdata = sm["sh600000"].get_kdata(Query(-100))
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
