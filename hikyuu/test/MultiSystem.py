#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2026-10-03
#    Author: fasiondog

import unittest
from test_init import *


class TestMultiSystem(unittest.TestCase):
    def test_py_subclass_override(self):
        """The Python subclass overriding the MultiSystem virtual function takes effect (trampoline registration)"""

        class MyMS(MultiSystem):
            def __init__(self):
                MultiSystem.__init__(self, "MyMS")
                self.reset_called = False

            def _reset(self):
                self.reset_called = True

        ms = MyMS()
        ms.reset()
        self.assertTrue(ms.reset_called)

    def test_py_subclass_run_moment(self):
        """The Python subclass overriding runMoment takes effect"""

        class MyMS2(MultiSystem):
            def __init__(self):
                MultiSystem.__init__(self, "MyMS2")
                self.moment_called = False

            def runMoment(self, datetime):
                self.moment_called = True
                return MomentResult()

        ms = MyMS2()
        ms.runMoment(Datetime(200001010000))
        self.assertTrue(ms.moment_called)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(TestMultiSystem)
