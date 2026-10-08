#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ===============================================================================
# Author: fasiondog
# History: 1)20261009, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *


class StrategyCallbackTest(unittest.TestCase):
    """check_pyfunction_arg_num decides whether a callback can be registered: it must accept
    every callable that can be invoked with the expected number of positional arguments, and
    reject those that cannot. The implicit self of a bound method (or of a callable object) is
    not counted; callables without an introspectable signature are accepted."""

    def setUp(self):
        self.stg = Strategy()

    def test_run_daily_expected_arity(self):
        self.stg.run_daily(lambda stg: None, Seconds(1))

    def test_run_daily_wrong_arity_rejected(self):
        with self.assertRaises(Exception):
            self.stg.run_daily(lambda: None, Seconds(1))
        with self.assertRaises(Exception):
            self.stg.run_daily(lambda stg, extra: None, Seconds(1))

    def test_run_daily_optional_and_var_args(self):
        self.stg.run_daily(lambda stg, extra=None: None, Seconds(1))
        self.stg.run_daily(lambda *args: None, Seconds(1))
        self.stg.run_daily(lambda stg, *, mode=1: None, Seconds(1))

    def test_run_daily_required_keyword_only_rejected(self):
        with self.assertRaises(Exception):
            self.stg.run_daily(lambda stg, *, mode: None, Seconds(1))

    def test_run_daily_self_not_counted(self):
        class Handler:
            def on_bar(self, stg):
                pass

        self.stg.run_daily(Handler().on_bar, Seconds(1))

        class CallableHandler:
            def __call__(self, stg):
                pass

        self.stg.run_daily(CallableHandler(), Seconds(1))

    def test_run_daily_without_introspectable_signature(self):
        self.stg.run_daily(type, Seconds(1))

    def test_on_change_expected_arity(self):
        self.stg.on_change(lambda stg, stk, spot: None)

        class Handler:
            def on_change(self, stg, stk, spot):
                pass

        self.stg.on_change(Handler().on_change)

    def test_on_change_wrong_arity_rejected(self):
        with self.assertRaises(Exception):
            self.stg.on_change(lambda stg, stk: None)
        with self.assertRaises(Exception):
            self.stg.on_change(lambda stg, stk, spot, extra: None)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(StrategyCallbackTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
