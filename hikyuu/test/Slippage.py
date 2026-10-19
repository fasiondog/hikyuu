#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: 1)20130321, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *


class SlippagePython(SlippageBase):
    def __init__(self):
        super(SlippagePython, self).__init__("SlippagePython")
        self._x = 0

    def _get_real_buy_price(self, datetime, price):
        if self._x < 10:
            return 0.0
        return 1.0

    def _get_real_sell_price(self, datetime, price):
        if self._x < 10:
            return 0.0
        return 1.0

    def _reset(self):
        self._x = 0

    def _clone(self):
        p = SlippagePython()
        p._x = self._x
        return p

    def _calculate(self):
        """ do nothin """


class SlippageAsym(SlippageBase):
    def __init__(self):
        super(SlippageAsym, self).__init__("SlippageAsym")

    def _get_real_buy_price(self, datetime, price):
        return price + 1.0

    def _get_real_sell_price(self, datetime, price):
        return price - 1.0

    def _get_real_buy_short_price(self, datetime, price):
        return price + 5.0

    def _get_real_sell_short_price(self, datetime, price):
        return price - 5.0

    def _calculate(self):
        """ do nothin """

    def _clone(self):
        return SlippageAsym()


class SlippageTest(unittest.TestCase):
    def test_SlippageBase(self):
        p = SlippagePython()
        self.assertEqual(p.name, "SlippagePython")
        self.assertEqual(p.get_real_buy_price(Datetime(200101010000), 1.0), 0.0)
        self.assertEqual(p.get_real_sell_price(Datetime(200101010000), 1.0), 0.0)

        self.assertEqual(p._x, 0)
        p._x = 10
        self.assertEqual(p._x, 10)
        self.assertEqual(p.get_real_buy_price(Datetime(200101010000), 1.0), 1.0)
        self.assertEqual(p.get_real_sell_price(Datetime(200101010000), 1.0), 1.0)
        p.reset()
        self.assertEqual(p._x, 0)

        p._x = 10
        p_clone = p.clone()
        self.assertEqual(p_clone._x, 10)
        p.reset()
        self.assertEqual(p._x, 0)
        self.assertEqual(p_clone._x, 10)

    def test_short_side_default_delegate(self):
        """A custom slippage without short hooks slips both sides the same way"""
        p = SlippagePython()
        p._x = 10
        dt = Datetime(200101010000)
        self.assertEqual(p.get_real_buy_short_price(dt, 1.0), 1.0)
        self.assertEqual(p.get_real_sell_short_price(dt, 1.0), 1.0)

    def test_short_side_override(self):
        """Overriding the short hooks makes the short side slip differently"""
        p = SlippageAsym()
        dt = Datetime(200101010000)
        self.assertEqual(p.get_real_buy_price(dt, 10.0), 11.0)
        self.assertEqual(p.get_real_sell_price(dt, 10.0), 9.0)
        self.assertEqual(p.get_real_buy_short_price(dt, 10.0), 15.0)
        self.assertEqual(p.get_real_sell_short_price(dt, 10.0), 5.0)

    def test_builtin_short_params(self):
        """The built-in short parameters drive the short side alone"""
        dt = Datetime(200101010000)
        sp = SP_FixedPercent(0.01, short_p=0.05)
        self.assertAlmostEqual(sp.get_real_buy_short_price(dt, 10.0), 10.5)
        self.assertAlmostEqual(sp.get_real_sell_short_price(dt, 10.0), 9.5)
        self.assertAlmostEqual(sp.get_real_buy_price(dt, 10.0), 10.1)

        sp2 = SP_FixedPercent(0.01)
        self.assertAlmostEqual(sp2.get_real_buy_short_price(dt, 10.0), 10.1)

    def test_pickle(self):
        """The name and the parameters survive a pickle round-trip"""
        if not constant.pickle_support:
            return

        import pickle as pl
        p = SP_Uniform(-0.05, 0.05, short_min_value=-0.2, short_max_value=0.2)
        p.set_param("seed", 42)

        b = pl.loads(pl.dumps(p))
        self.assertEqual(p.name, b.name)
        self.assertEqual(b.get_param("min_value"), -0.05)
        self.assertEqual(b.get_param("max_value"), 0.05)
        self.assertEqual(b.get_param("short_min_value"), -0.2)
        self.assertEqual(b.get_param("short_max_value"), 0.2)
        self.assertEqual(b.get_param("seed"), 42)


def test_crtSL_func(self):
    pass


def test_get_real_buy_price_func(self, datetime, price):
    return 10.0 if datetime == Datetime(200101010000) else 0.0


def test_get_real_sell_price_func(self, datetime, price):
    return 10.0 if datetime == Datetime(200101010000) else 0.0


def test_get_real_buy_short_price_func(self, datetime, price):
    return price + 3.0


def test_get_real_sell_short_price_func(self, datetime, price):
    return price - 3.0


class TestCrtSL(unittest.TestCase):
    def test_crtSP(self):
        p = crtSP(test_get_real_buy_price_func, test_crtSL_func, params={'n': 10}, name="TestSL")
        self.assertEqual(p.name, "TestSL")
        self.assertEqual(p.get_real_buy_price(Datetime(200101010000), 1.0), 10.0)
        self.assertEqual(p.get_real_buy_price(Datetime(200101020000), 1.0), 0.0)
        p_clone = p.clone()
        self.assertEqual(p_clone.name, "TestSL")

    def test_crtSP_short(self):
        p = crtSP(test_get_real_buy_price_func, test_get_real_sell_price_func,
                  name="TestSPShort",
                  get_real_buy_short_price=test_get_real_buy_short_price_func,
                  get_real_sell_short_price=test_get_real_sell_short_price_func)
        dt = Datetime(200101010000)
        self.assertEqual(p.get_real_buy_price(dt, 10.0), 10.0)
        self.assertEqual(p.get_real_buy_short_price(dt, 10.0), 13.0)
        self.assertEqual(p.get_real_sell_short_price(dt, 10.0), 7.0)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(SlippageTest)


def suiteTestCrtSL():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtSL)
