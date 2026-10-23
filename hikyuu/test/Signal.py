#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: 1)20130321, Added by fasiondog
# ===============================================================================

import pickle as pl
import threading
import time
import unittest

from test_init import *


class SignalPython(SignalBase):
    def __init__(self):
        super(SignalPython, self).__init__("SignalPython")
        self._x = 0
        self.set_param("test", 30)

    def _reset(self):
        self._x = 0

    def _clone(self):
        p = SignalPython()
        p._x = self._x
        return p

    def _calculate(self, kdata):
        self._add_buy_signal(Datetime(201201210000))
        self._add_sell_signal(Datetime(201201300000))


class CountingSG(SignalBase):
    """Counts the _calculate callbacks of every instance, clones included"""

    instances = []

    def __init__(self):
        super(CountingSG, self).__init__("CountingSG")
        self.called = 0
        CountingSG.instances.append(self)

    def _clone(self):
        p = CountingSG()
        p.called = self.called
        return p

    def _calculate(self, kdata):
        self.called += 1
        if len(kdata) > 0:
            self._add_buy_signal(kdata[0].datetime)


class SignalTest(unittest.TestCase):
    def test_SignalBase(self):
        p = SignalPython()
        self.assertEqual(p.name, "SignalPython")
        p.name = "SignalPythonTest"
        self.assertEqual(p.name, "SignalPythonTest")

        self.assertEqual(p.should_buy(Datetime(201201210000)), False)
        self.assertEqual(p.should_sell(Datetime(201201300000)), False)
        k = sm['sh000001'].get_kdata(Query(-100))
        self.assertEqual(k.empty(), False)
        p.to = k
        self.assertEqual(p.should_buy(Datetime(201201210000)), True)
        self.assertEqual(p.should_sell(Datetime(201201300000)), True)

        self.assertEqual(p.should_buy(Datetime(200101010000)), False)
        p._add_buy_signal(Datetime(200101010000))
        self.assertEqual(p.should_buy(Datetime(200101010000)), True)

        self.assertEqual(p.should_sell(Datetime(200101030000)), False)
        p._add_sell_signal(Datetime(200101030000))
        self.assertEqual(p.should_sell(Datetime(200101030000)), True)

        d = p.get_buy_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201210000), Datetime(200101010000)])

        d = p.get_sell_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201300000), Datetime(200101030000)])

        p_clone = p.clone()
        d = p_clone.get_buy_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201210000), Datetime(200101010000)])

        d = p_clone.get_sell_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201300000), Datetime(200101030000)])

        self.assertEqual(p._x, 0)
        p._x = 10
        self.assertEqual(p._x, 10)
        p.reset()
        self.assertEqual(p._x, 0)

        p._x = 20
        p_clone = p.clone()
        self.assertEqual(p_clone._x, 20)
        p.reset()
        self.assertEqual(p._x, 0)
        self.assertEqual(p_clone._x, 20)

        self.assertEqual(p.get_param("test"), 30)
        self.assertEqual(p_clone.get_param("test"), 30)
        p.set_param("test", 10)
        self.assertEqual(p.get_param("test"), 10)
        self.assertEqual(p_clone.get_param("test"), 30)


def testSignal(self, kdata):
    self._add_buy_signal(Datetime(201201210000))
    self._add_sell_signal(Datetime(201201300000))


class TestCrtSG(unittest.TestCase):
    def test_crtSG(self):
        p = crtSG(testSignal, params={'test': 30}, name='SG_TEST')
        self.assertEqual(p.name, "SG_TEST")
        p.name = "SignalPythonTest"
        self.assertEqual(p.name, "SignalPythonTest")

        self.assertEqual(p.should_buy(Datetime(201201210000)), False)
        self.assertEqual(p.should_sell(Datetime(201201300000)), False)
        k = sm['sh000001'].get_kdata(Query(-100))
        self.assertEqual(k.empty(), False)
        p.to = k
        self.assertEqual(p.should_buy(Datetime(201201210000)), True)
        self.assertEqual(p.should_sell(Datetime(201201300000)), True)

        self.assertEqual(p.should_buy(Datetime(200101010000)), False)
        p._add_buy_signal(Datetime(200101010000))
        self.assertEqual(p.should_buy(Datetime(200101010000)), True)

        self.assertEqual(p.should_sell(Datetime(200101030000)), False)
        p._add_sell_signal(Datetime(200101030000))
        self.assertEqual(p.should_sell(Datetime(200101030000)), True)

        d = p.get_buy_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201210000), Datetime(200101010000)])

        d = p.get_sell_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201300000), Datetime(200101030000)])

        p_clone = p.clone()
        self.assertFalse(p_clone is p)
        d = p_clone.get_buy_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201210000), Datetime(200101010000)])

        d = p_clone.get_sell_signal()
        for i in range(len(d)):
            self.assertIn(d[i], [Datetime(201201300000), Datetime(200101030000)])


class TestCrtSGWithClone(unittest.TestCase):
    def test_crtSG(self):
        p = crtSG(testSignal, params={'test': 30}, name='SG_TEST')
        p.x = 10
        p2 = p.clone()
        self.assertFalse(p2 is p)
        self.assertEqual(p2.name, "SG_TEST")
        self.assertEqual(p2.x, p.x)


class TestSGTradingObject(unittest.TestCase):
    """Setting the trading object recalculates with the GIL released"""

    def test_set_to_releases_gil(self):
        kdata = sm['sh000001'].get_kdata(Query(0))
        if len(kdata) < 3000:
            self.skipTest("not enough bars to observe the released GIL")

        # different queries, so that no assignment is short-circuited by the unchanged object
        objects = [sm['sh000001'].get_kdata(Query(i, i + 3000)) for i in range(40)]
        sg = SG_Cross(MA(CLOSE(), 250), AMA(CLOSE(), 20))

        ticks = [0]
        stopped = threading.Event()

        def ticker():
            while not stopped.is_set():
                ticks[0] += 1

        thread = threading.Thread(target=ticker, daemon=True)
        thread.start()
        time.sleep(0.05)
        ticks[0] = 0
        try:
            for one in objects:
                sg.to = one
        finally:
            stopped.set()
            thread.join()

        # a call holding the GIL would leave the other thread at a handful of ticks
        self.assertGreater(ticks[0], 1000)
        self.assertGreater(len(sg.get_buy_signal()), 0)
        self.assertGreater(len(sg.get_sell_signal()), 0)

    def test_python_part_still_driven(self):
        kdata = sm['sh000001'].get_kdata(Query(-100))
        self.assertFalse(kdata.empty())

        sg = CountingSG()
        sg.to = kdata
        self.assertEqual(sg.called, 1)
        self.assertEqual(len(sg.get_buy_signal()), 1)

        # a composition drives its own clone of the Python signal through the trampoline
        composed = sg + SG_AllwaysBuy()
        nested = CountingSG.instances[-1]
        self.assertIsNot(nested, sg)
        self.assertEqual(nested.called, sg.called)
        composed.to = kdata
        self.assertEqual(nested.called, sg.called + 1)
        self.assertEqual(len(composed.get_buy_signal()), 1)
        self.assertEqual(composed.should_buy(kdata[0].datetime), True)


class FixedValueSG(SignalBase):
    """Emits a fixed 2.0 buy on the first bar and a fixed -1.0 sell on the last bar"""

    def __init__(self):
        super(FixedValueSG, self).__init__("FixedValueSG")

    def _clone(self):
        return FixedValueSG()

    def _calculate(self, kdata):
        self._add_buy_signal(kdata[0].datetime, 2.0)
        self._add_sell_signal(kdata[-1].datetime, -1.0)


class TestSGSignalValues(unittest.TestCase):
    """The signal values are readable and scaled by the value operators"""

    def test_get_value(self):
        kdata = sm['sh000001'].get_kdata(Query(-100))
        self.assertFalse(kdata.empty())
        d_buy = kdata[0].datetime
        d_sell = kdata[-1].datetime

        sg = FixedValueSG()
        sg.to = kdata
        self.assertEqual(sg.get_buy_value(d_buy), 2.0)
        self.assertEqual(sg.get_sell_value(d_buy), 0.0)
        self.assertEqual(sg.get_value(d_buy), 2.0)
        self.assertEqual(sg.get_buy_value(d_sell), 0.0)
        self.assertEqual(sg.get_sell_value(d_sell), -1.0)
        self.assertEqual(sg.get_value(d_sell), -1.0)
        self.assertEqual(sg.get_value(Datetime(200101010000)), 0.0)

        doubled = sg * 2.0
        doubled.to = kdata
        self.assertEqual(doubled.get_value(d_buy), 2.0 * sg.get_value(d_buy))
        self.assertEqual(doubled.get_value(d_sell), 2.0 * sg.get_value(d_sell))


class TestSGCycleDates(unittest.TestCase):
    """The Python side declares the rebalancing days consumed by a cycle signal"""

    def test_set_cycle_dates(self):
        kdata = sm['sh000001'].get_kdata(Query(-200))
        self.assertFalse(kdata.empty())
        d50 = kdata[50].datetime
        d100 = kdata[100].datetime
        d150 = kdata[150].datetime

        sg = SG_Cycle()
        sg.set_cycle_dates([d150, d50, d50])
        self.assertEqual(len(sg.get_cycle_dates()), 2)

        sg.to = kdata
        self.assertEqual(len(sg.get_buy_signal()), 2)
        self.assertTrue(sg.should_buy(d50))
        self.assertTrue(sg.should_buy(d150))

        # a declaration after the trading object is set recalculates at once
        sg.set_cycle_dates([d100])
        self.assertEqual(len(sg.get_buy_signal()), 1)
        self.assertTrue(sg.should_buy(d100))
        self.assertEqual(sg.get_value(d100), 1.0)

    def test_set_cycle_dates_none(self):
        kdata = sm['sh000001'].get_kdata(Query(-200))
        sg = SG_Cycle()
        sg.set_cycle_dates([kdata[50].datetime])
        sg.to = kdata
        self.assertEqual(len(sg.get_buy_signal()), 1)

        # None clears the declared days and recalculates at once
        sg.set_cycle_dates(None)
        self.assertEqual(len(sg.get_cycle_dates()), 0)
        self.assertEqual(len(sg.get_buy_signal()), 0)


class TestSGManual(unittest.TestCase):
    """The manual signal generator accepts pre-seeded signals"""

    def test_manual_signals(self):
        d_buy = Datetime(201201210000)
        d_sell = Datetime(201201300000)

        sg = SG_Manual()
        self.assertEqual(sg.name, "SG_Manual")
        sg._add_buy_signal(d_buy)
        sg._add_sell_signal(d_sell)
        self.assertTrue(sg.should_buy(d_buy))
        self.assertTrue(sg.should_sell(d_sell))
        self.assertFalse(sg.should_sell(d_buy))
        self.assertEqual(sg.get_value(d_buy), 1.0)

    def test_manual_compose(self):
        kdata = sm['sh000001'].get_kdata(Query(-100))
        self.assertFalse(kdata.empty())
        d_buy = kdata[10].datetime

        sg = SG_Manual()
        sg._add_buy_signal(d_buy)
        composed = SG_Add(sg, SG_AllwaysBuy(), False)
        composed.to = kdata
        # the operator keeps the preset signals of its manual sub-signal: 1.0 + 1.0
        self.assertTrue(composed.should_buy(d_buy))
        self.assertEqual(composed.get_buy_value(d_buy), 2.0)

    def test_manual_direct_drive(self):
        kdata = sm['sh000001'].get_kdata(Query(-100))
        self.assertFalse(kdata.empty())
        d_buy = kdata[10].datetime

        sg = SG_Manual()
        sg._add_buy_signal(d_buy)
        # The preset signals are the input of a manual signal: assigning the trading object and
        # resetting keep them
        sg.to = kdata
        self.assertTrue(sg.should_buy(d_buy))
        sg.reset()
        self.assertTrue(sg.should_buy(d_buy))


@unittest.skipUnless(constant.pickle_support, "pickle support disabled")
class TestSGPickle(unittest.TestCase):
    """pickle round-trips keep the signal tables, the cycle days and the composition"""

    def test_pickle_roundtrip(self):
        kdata = sm['sh000001'].get_kdata(Query(-500))
        self.assertFalse(kdata.empty())

        cross = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10))
        cross.to = kdata
        restored = pl.loads(pl.dumps(cross))
        self.assertEqual(restored.get_buy_signal(), cross.get_buy_signal())
        self.assertEqual(restored.get_sell_signal(), cross.get_sell_signal())

        cycle = SG_Cycle()
        cycle.set_cycle_dates([kdata[10].datetime, kdata[20].datetime])
        cycle.to = kdata
        restored = pl.loads(pl.dumps(cycle))
        self.assertEqual(list(restored.get_cycle_dates()), list(cycle.get_cycle_dates()))
        self.assertEqual(restored.get_buy_signal(), cycle.get_buy_signal())

        composed = SG_Add(cross, SG_AllwaysBuy(), False)
        composed.to = kdata
        restored = pl.loads(pl.dumps(composed))
        self.assertEqual(restored.get_buy_signal(), composed.get_buy_signal())

        manual = SG_Manual()
        manual._add_buy_signal(kdata[3].datetime)
        restored = pl.loads(pl.dumps(manual))
        self.assertTrue(restored.should_buy(kdata[3].datetime))
        self.assertEqual(restored.get_value(kdata[3].datetime), 1.0)


class TestSGConcurrentSetTO(unittest.TestCase):
    """Driving C++ and Python SG instances from several threads must not deadlock"""

    def test_concurrent_setTO_clone_reset(self):
        kdata = sm['sh000001'].get_kdata(Query(-100))
        self.assertFalse(kdata.empty())
        py_signals = [CountingSG(), CountingSG()]
        signals = [SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)),
                   SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10))] + py_signals
        errors = []

        def work(sg):
            try:
                for _ in range(50):
                    sg.to = kdata
                    sg.clone()
                    sg.reset()
            except Exception as e:  # noqa: BLE001
                errors.append(e)

        threads = [threading.Thread(target=work, args=(sg,)) for sg in signals]
        for t in threads:
            t.start()
        for t in threads:
            t.join(timeout=60)
            self.assertFalse(t.is_alive(), "deadlock detected in setTO/clone/reset")
        self.assertEqual(errors, [])
        self.assertEqual([sg.called for sg in py_signals], [50, 50])


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(SignalTest)


def suiteTestCrtSG():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtSG)


def suiteTestCrtSGWithClone():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtSGWithClone)


def suiteTestSGTradingObject():
    return unittest.TestLoader().loadTestsFromTestCase(TestSGTradingObject)


def suiteTestSGSignalValues():
    return unittest.TestLoader().loadTestsFromTestCase(TestSGSignalValues)


def suiteTestSGCycleDates():
    return unittest.TestLoader().loadTestsFromTestCase(TestSGCycleDates)


def suiteTestSGManual():
    return unittest.TestLoader().loadTestsFromTestCase(TestSGManual)


def suiteTestSGPickle():
    return unittest.TestLoader().loadTestsFromTestCase(TestSGPickle)


def suiteTestSGConcurrentSetTO():
    return unittest.TestLoader().loadTestsFromTestCase(TestSGConcurrentSetTO)
