#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2026-09-12
#    Author: fasiondog

import unittest
from test_init import *


def test_allocate(self, date, tm, contexts, query):
    return {SYS_Simple(): 1.0}


def test_to_targets(self, date, tm, suggestions, sys_weight, query):
    self._l2_called = True


def test_check_risk(self, date, tm, suggestions, query):
    self._l3_called = True


class TestCrtAF(unittest.TestCase):
    def test_crt_af(self):
        af = crtAF(test_allocate, {'n': 10}, name="TestAF")
        self.assertTrue(isinstance(af, AllocateFundsBase))
        self.assertEqual(af.name, "TestAF")
        self.assertEqual(af.get_param("n"), 10)

        tm = crtTM(Datetime(200101010000))
        weights = af._allocate(Datetime(200101010000), tm, [], Query(-100))
        self.assertEqual(len(weights), 1)
        self.assertEqual(list(weights.values())[0], 1.0)

        af_clone = af.clone()
        self.assertEqual(af_clone.name, "TestAF")
        self.assertEqual(af_clone.get_param("n"), 10)

    def test_crt_af_l2_l3(self):
        af = crtAF(test_allocate, name="TestAF2",
                   to_targets_func=test_to_targets, check_risk_func=test_check_risk)
        tm = crtTM(Datetime(200101010000))
        af._to_targets(Datetime(200101010000), tm, [], {}, Query(-100))
        self.assertTrue(af._l2_called)
        af._check_risk(Datetime(200101010000), tm, [], Query(-100))
        self.assertTrue(af._l3_called)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtAF)
