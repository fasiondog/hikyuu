#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2022-02-20
#    Author: fasiondog

import unittest
from test_init import *


class SystemWeightTest(unittest.TestCase):
    def test_weight(self):
        x = SystemWeight()
        self.assertEqual(x.weight, 1.0)

        try:
            x.weight = 1.2
        except:
            self.assertEqual(x.weight, 1.0)

        #print(x)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(SystemWeightTest)
