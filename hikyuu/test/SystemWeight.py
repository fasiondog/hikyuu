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

        # print(x)

    def test_to_np(self):
        # The empty list yields an empty array
        self.assertEqual(len(SystemWeightList().to_np()), 0)

        swl = SystemWeightList()
        swl.append(SystemWeight(SYS_Simple(), 0.3))
        swl.append(SystemWeight())
        arr = swl.to_np()
        self.assertEqual(len(arr), 2)
        self.assertEqual(arr.dtype.itemsize, 208)
        self.assertEqual(len(arr.dtype.names), 4)
        weight = arr[arr.dtype.names[3]]
        self.assertAlmostEqual(weight[0], 0.3)
        self.assertAlmostEqual(weight[1], 1.0)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(SystemWeightTest)
