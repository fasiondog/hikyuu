#!/usr/bin/python
# -*- coding: utf-8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression tests for the batch helpers parallel_run_sys, get_funds_list
#          and get_performance_list: results stay correct, and the GIL is released
#          while running pure C++ inputs so other Python threads keep running.
#          The same holds for the system entries (System/MultiSystem run, runMoment):
#          the GIL is released there as well, Python parts included.
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


def _make_ms(stock, kdata, count=1):
    ms = MultiSystem("MS")
    ms.tm = crtTM()
    for _ in range(count):
        ms.add(_make_sys(stock, kdata))
    return ms


_TARGET_WINDOW = 0.5


def _spin_fraction(work):
    """Run work() once with a spinning background Python thread and return the fraction
    of its free-running speed the spinner kept.

    The absolute iteration count is machine speed and load dependent (a busy test suite
    may starve the spinner), so the rate is normalized by the spinner's own free rate
    measured just before the work: ~1% while the GIL is held, tens of percent when it
    is released, shrinking under CPU saturation."""
    counter = [0]
    stop = threading.Event()

    def spin():
        while not stop.is_set():
            counter[0] += 1
            time.sleep(0)

    thread = threading.Thread(target=spin)
    thread.start()
    time.sleep(0.05)
    # the free-running rate under the current machine speed and load
    before = counter[0]
    start = time.perf_counter()
    time.sleep(0.1)
    free_rate = (counter[0] - before) / (time.perf_counter() - start)
    # the rate while work() runs
    before = counter[0]
    start = time.perf_counter()
    work()
    elapsed = time.perf_counter() - start
    progress = counter[0] - before
    stop.set()
    thread.join()
    if free_rate <= 0 or elapsed <= 0:
        return 0.0
    return progress / (free_rate * elapsed)


def _measure_progress(work, attempts=3):
    """The best of a few attempts, so a transient scheduling hiccup does not fail the
    check."""
    return max(_spin_fraction(work) for _ in range(attempts))


def _calibrated_work(build_call, probe):
    """Repeat the probe call until the measured window is long enough, so the
    progress assertion stays stable across machines of different speed.

    The window targets ~0.5s of work; fast operations (e.g. a single
    runMoment step) need many repetitions to fill it, while slow operations are
    capped by a maximum total window so the suite does not hang."""
    start = time.perf_counter()
    probe()
    elapsed = time.perf_counter() - start
    max_window = 5.0
    rounds = max(1, int(_TARGET_WINDOW / max(elapsed, 1e-9)))
    rounds = min(rounds, max(1, int(max_window / max(elapsed, 1e-9))))

    def work():
        for _ in range(rounds):
            probe()

    work.rounds = rounds
    return work


def _gil_holding_control(rounds):
    """A pure-Python workload performing the same number of GIL-holding calls as the
    work under check, so both measurements give the spinner the same boundary steals
    while the GIL is held; it serves as the reference level of _assert_releases_gil."""
    start = time.perf_counter()
    total = 0
    for i in range(10000):
        total += i
    unit = time.perf_counter() - start  # seconds per 10000 busy iterations
    body = max(1, int(10000 * (_TARGET_WINDOW / max(rounds, 1)) / max(unit, 1e-9)))

    def work():
        for _ in range(rounds):
            total = 0
            for i in range(body):
                total += i

    return work


def _assert_releases_gil(testcase, work):
    """Check on testcase that work() releases the GIL, adaptive to machine speed and
    load: the spinner's fraction of its free rate while work() runs is compared with a
    GIL-holding pure-Python control of the same call count. Releasing the GIL leaves
    the spinner a multiple of the control level (measured 4x-50x here), holding it does
    not (~1x). The control is measured only when the fraction is not already clearly
    above any control level."""
    released = _measure_progress(work)
    if released > 0.1:
        return
    held = _measure_progress(_gil_holding_control(work.rounds))
    testcase.assertGreater(
        released, held * 2.5, 'the GIL does not appear to be released: fraction '
        '{:.4f}, GIL-holding control {:.4f}'.format(released, held))


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
        _assert_releases_gil(self, _calibrated_work(None, probe))

    def test_sys_run_releases_gil(self):
        sys = _make_sys(self.stock, self.kdata)

        def probe():
            return sys.run(self.kdata, True, True)

        _assert_releases_gil(self, _calibrated_work(None, probe))

    def test_sys_run_query_releases_gil(self):
        sys = _make_sys(self.stock, self.kdata)

        def probe():
            return sys.run(Query(0, 10000), True, True)

        _assert_releases_gil(self, _calibrated_work(None, probe))

    def test_sys_run_with_python_part(self):
        # The GIL is released even with Python parts: the trampolines reacquire it per
        # callback, so a Python part must still be driven and produce trades.
        class MySG(SignalBase):
            def __init__(self):
                super().__init__("MySG")

            def _clone(self):
                return MySG()

            def _calculate(self, kdata):
                self._add_buy_signal(kdata[10].datetime)
                self._add_sell_signal(kdata[len(kdata) // 2].datetime)

        sys = _make_sys(self.stock, self.kdata)
        sys.sg = MySG()
        sys.run(self.kdata, True, True)
        self.assertGreater(len(sys.tm.get_trade_list()), 1)

    def test_multi_system_run_releases_gil(self):
        ms = _make_ms(self.stock, self.kdata, 3)

        def probe():
            return ms.run(self.kdata, True, True)

        _assert_releases_gil(self, _calibrated_work(None, probe))

    def test_multi_system_run_moment_releases_gil(self):
        ms = _make_ms(self.stock, self.kdata)
        ms.run(self.kdata, True, True)
        last = self.kdata[-1].datetime

        def probe():
            return ms.runMoment(last)

        _assert_releases_gil(self, _calibrated_work(None, probe))

    def test_multi_system_run_with_python_part(self):
        class MySG(SignalBase):
            def __init__(self):
                super().__init__("MySG")

            def _clone(self):
                return MySG()

            def _calculate(self, kdata):
                self._add_buy_signal(kdata[10].datetime)
                self._add_sell_signal(kdata[len(kdata) // 2].datetime)

        ms = MultiSystem("MS")
        ms.tm = crtTM()
        sub = _make_sys(self.stock, self.kdata)
        sub.sg = MySG()
        ms.add(sub)
        ms.run(self.kdata, True, True)
        self.assertGreater(len(ms.tm.get_trade_list()), 1)

    def test_multi_system_nested_python_subclass(self):
        # A Python MultiSystem subclass works as a nested child of another aggregate: it keeps its
        # dynamic type on the parent drive and its leaf still trades through the two levels
        class MyMS(MultiSystem):
            def __init__(self, name="PyMS"):
                super().__init__(name)

        inner_leaf = _make_sys(self.stock, self.kdata)
        inner = MyMS("py_inner")
        inner.tm = crtTM()
        inner.add(inner_leaf)

        outer = MultiSystem("outer")
        outer.tm = crtTM()
        outer.add(inner)
        outer.run(self.kdata, True, True)

        self.assertIs(type(outer.get_system_list()[0]), MyMS)
        self.assertGreater(len(outer.tm.get_trade_list()), 0)

    def test_get_funds_list_result(self):
        tm_list = [crtTM() for _ in range(4)]
        result = get_funds_list(tm_list, self.ref_dates)
        self.assertEqual(len(result), 4)
        for funds in result:
            self.assertEqual(len(funds), len(self.ref_dates))

    def test_get_funds_list_releases_gil(self):
        tm_list = [crtTM() for _ in range(50)]
        def probe(): return get_funds_list(tm_list, self.ref_dates)
        _assert_releases_gil(self, _calibrated_work(None, probe))

    def test_get_performance_list_result(self):
        tm_list = [crtTM() for _ in range(4)]
        result = get_performance_list(tm_list, Datetime(2024, 12, 31))
        self.assertEqual(len(result), 4)
        for perf in result:
            self.assertIsInstance(perf, Performance)

    def test_get_performance_list_releases_gil(self):
        tm_list = [crtTM() for _ in range(50)]
        def probe(): return get_performance_list(tm_list, Datetime(2024, 12, 31))
        _assert_releases_gil(self, _calibrated_work(None, probe))

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
