#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2022-02-20
#    Author: fasiondog

import math
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
        # The empty list yields a typed empty array, consistent with to_df
        arr = SystemWeightList().to_np()
        self.assertEqual(len(arr), 0)
        self.assertEqual(len(arr.dtype.names), 4)

        swl = SystemWeightList()
        swl.append(SystemWeight(SYS_Simple(), 0.3))
        swl.append(SystemWeight())
        arr = swl.to_np()
        self.assertEqual(len(arr), 2)
        self.assertEqual(arr.dtype.itemsize, 208)
        self.assertEqual(len(arr.dtype.names), 4)
        weight = arr[arr.dtype.names[3]]
        self.assertAlmostEqual(weight[0], 0.3)
        # A null sys is written as the null weight (NaN), aligned with to_df
        self.assertTrue(math.isnan(weight[1]))

    def test_systemweights_np_df(self):
        # to_np and to_df must agree on the weight semantics (including the null sys)
        swl = SystemWeightList()
        swl.append(SystemWeight(SYS_Simple(), 0.5))
        swl.append(SystemWeight())
        arr = swl.to_np()
        np_weights = list(arr[arr.dtype.names[3]])
        df_weights = list(swl.to_df()["weight"])
        self.assertEqual(len(np_weights), len(df_weights))
        for n, d in zip(np_weights, df_weights):
            if math.isnan(n):
                self.assertTrue(math.isnan(d))
            else:
                self.assertAlmostEqual(n, d)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(SystemWeightTest)
