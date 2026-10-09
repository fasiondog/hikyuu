#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression tests for the batch helpers parallel_run_sys, get_funds_list
#          and get_performance_list: results stay correct, and the GIL is released
#          while running pure C++ inputs so other Python threads keep running.
# ===============================================================================

import threading
import time
import unittest

from test_init import *


def _make_sys(stock, kdata):
    sys = SYS_Simple()
    sys.tm = crtTM()
    sys.mm = MM_Nothing()
    sys.sg = SG_Cross(MA(CLOSE(), n=5), MA(CLOSE(), n=20))
    sys.to = kdata
    return sys


def _measure_progress(work):
    """Run work() and return how many loop iterations a background Python thread
    managed meanwhile; a large count proves the GIL was released."""
    counter = [0]
    stop = threading.Event()

    def spin():
        while not stop.is_set():
            counter[0] += 1
            time.sleep(0)

    thread = threading.Thread(target=spin)
    thread.start()
    time.sleep(0.05)
    before = counter[0]
    work()
    progress = counter[0] - before
    stop.set()
    thread.join()
    return progress


def _calibrated_work(build_call, probe):
    """Repeat the probe call until the measured window is long enough, so the
    progress assertion stays stable on fast machines."""
    start = time.perf_counter()
    probe()
    elapsed = time.perf_counter() - start
    rounds = max(1, min(50, int(0.5 / max(elapsed, 0.001))))

    def work():
        for _ in range(rounds):
            probe()

    return work


class MiscTest(unittest.TestCase):
    def setUp(self):
        self.stock = sm['sh000001']
        self.kdata = self.stock.get_kdata(Query(0, 10000))
        self.ref_dates = self.kdata.get_datetime_list()

    def test_parallel_run_sys_result(self):
        sys_list = [_make_sys(self.stock, self.kdata) for _ in range(4)]
        result = parallel_run_sys(sys_list, Query(0, 10000), reset=True)
        self.assertEqual(len(result), 4)
        for funds in result:
            self.assertGreater(len(funds), 0)

    def test_parallel_run_sys_releases_gil(self):
        sys_list = [_make_sys(self.stock, self.kdata) for _ in range(20)]
        def probe(): return parallel_run_sys(sys_list, Query(0, 10000), reset=True)
        work = _calibrated_work(None, probe)
        progress = _measure_progress(work)
        self.assertGreater(progress, 100)

    def test_get_funds_list_result(self):
        tm_list = [crtTM() for _ in range(4)]
        result = get_funds_list(tm_list, self.ref_dates)
        self.assertEqual(len(result), 4)
        for funds in result:
            self.assertEqual(len(funds), len(self.ref_dates))

    def test_get_funds_list_releases_gil(self):
        tm_list = [crtTM() for _ in range(50)]
        def probe(): return get_funds_list(tm_list, self.ref_dates)
        work = _calibrated_work(None, probe)
        progress = _measure_progress(work)
        self.assertGreater(progress, 100)

    def test_get_performance_list_result(self):
        tm_list = [crtTM() for _ in range(4)]
        result = get_performance_list(tm_list, Datetime(2024, 12, 31))
        self.assertEqual(len(result), 4)
        for perf in result:
            self.assertIsInstance(perf, Performance)

    def test_get_performance_list_releases_gil(self):
        tm_list = [crtTM() for _ in range(50)]
        def probe(): return get_performance_list(tm_list, Datetime(2024, 12, 31))
        work = _calibrated_work(None, probe)
        progress = _measure_progress(work)
        self.assertGreater(progress, 100)

    def test_now_default_args_not_frozen_at_import(self):
        # The "current time" defaults used to be evaluated once at import time and frozen
        # for the whole process life; they must stay None in the signatures and resolve to
        # the real current time on each call.
        tm_cls = type(crtTM())
        self.assertIn("datetime=None", tm_cls.get_performance.__doc__)
        self.assertIn("date=None", tm_cls.get_max_pull_back.__doc__)
        self.assertIn("datetime=None", tm_cls.get_profit_percent_monthly.__doc__)
        self.assertIn("datetime=None", tm_cls.get_profit_percent_yearly.__doc__)
        self.assertIn("datetime: Datetime = None", get_performance_list.__doc__)
        # Calling without the datetime argument must work (resolving to now internally)
        tm = crtTM()
        self.assertIsInstance(tm.get_performance(), Performance)
        self.assertIsInstance(tm.get_max_pull_back(), float)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MiscTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
