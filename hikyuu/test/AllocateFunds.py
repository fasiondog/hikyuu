#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2026-09-12
#    Author: fasiondog

import unittest
from test_init import *


def test_allocate(self, date, tm, contexts, query, mode):
    return {SYS_Simple(): 1.0}


def test_to_targets(self, date, tm, suggestions, sys_weight, query, mode):
    self._l2_called = True


def test_to_targets_mutate(self, date, tm, suggestions, sys_weight, query, mode):
    for s in suggestions:
        s.number = 999.0


def test_check_risk(self, date, tm, suggestions, query, mode):
    self._l3_called = True


class TestCrtAF(unittest.TestCase):
    def test_crt_af(self):
        af = crtAF(test_allocate, {'n': 10}, name="TestAF")
        self.assertTrue(isinstance(af, AllocateFundsBase))
        self.assertEqual(af.name, "TestAF")
        self.assertEqual(af.get_param("n"), 10)

        tm = crtTM(Datetime(200101010000))
        # Call through the C++ binding so that the trampoline forwards the trailing mode to the hook
        weights = AllocateFundsBase._allocate(af, Datetime(200101010000), tm,
                                              SubSystemContextList(), Query(-100), "A")
        self.assertEqual(len(weights), 1)
        self.assertEqual(list(weights.values())[0], 1.0)

        # The hook signature must carry the trailing mode, otherwise the call must fail
        with self.assertRaises(TypeError):
            AllocateFundsBase._allocate(af, Datetime(200101010000), tm,
                                        SubSystemContextList(), Query(-100))

        af_clone = af.clone()
        self.assertEqual(af_clone.name, "TestAF")
        self.assertEqual(af_clone.get_param("n"), 10)

    def test_crt_af_l2_l3(self):
        af = crtAF(test_allocate, name="TestAF2",
                   to_targets_func=test_to_targets, check_risk_func=test_check_risk)
        tm = crtTM(Datetime(200101010000))
        AllocateFundsBase._to_targets(af, Datetime(200101010000), tm, TradeSuggestionList(), {},
                                      Query(-100), "A")
        self.assertTrue(af._l2_called)
        AllocateFundsBase._check_risk(af, Datetime(200101010000), tm, TradeSuggestionList(),
                                      Query(-100), "A")
        self.assertTrue(af._l3_called)

    def test_py_af_suggestions_inplace(self):
        """The in-place rewrite of the suggestions by the Python custom AF must take effect (opaque container)"""
        af = crtAF(test_allocate, name="TestAF4", to_targets_func=test_to_targets_mutate)
        lst = TradeSuggestionList()
        s = TradeSuggestion()
        s.number = 1.0
        lst.append(s)
        tm = crtTM(Datetime(200101010000))
        AllocateFundsBase._to_targets(af, Datetime(200101010000), tm, lst, {}, Query(-100), "A")
        self.assertEqual(lst[0].number, 999.0)

    def test_multisystem_drives_hooks(self):
        """The aggregate system must drive L1/L2/L3 with the trailing mode, matching the hook signatures"""
        calls = []

        def allocate(self, date, tm, contexts, query, mode):
            calls.append(("L1", mode))
            return {}

        def to_targets(self, date, tm, suggestions, sys_weight, query, mode):
            calls.append(("L2", mode))

        def check_risk(self, date, tm, suggestions, query, mode):
            calls.append(("L3", mode))

        af = crtAF(allocate, name="TestMSAF", to_targets_func=to_targets,
                   check_risk_func=check_risk)

        query = Query(Datetime(20111101), Datetime(20111230), Query.DAY, Query.NO_RECOVER)
        k = sm['sh600000'].get_kdata(query)
        self.assertTrue(len(k) > 0)

        ms = MultiSystem()
        ms.tm = crtTM(Datetime(201111010000), 100000)
        ms.set_mode("A")
        ms.af = af
        sub = SYS_Simple()
        sub.tm = crtTM(Datetime(201111010000), 100000)
        sub.sg = SG_Cross(MA(CLOSE(), 2), MA(CLOSE(), 3))
        sub.mm = MM_FixedCount(100)
        sub.to = k
        ms.add(sub)
        ms.run(k)

        self.assertIn(("L1", "A"), calls)
        self.assertIn(("L2", "A"), calls)
        self.assertIn(("L3", "A"), calls)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtAF)
