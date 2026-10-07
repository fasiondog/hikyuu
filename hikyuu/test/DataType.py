#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ===============================================================================
# Author: fasiondog
# History: 1)20261008, Added by fasiondog
# ===============================================================================

import unittest

import numpy as np

from test_init import *


class DataTypeTest(unittest.TestCase):
    """toPriceList exercises python_list_to_vector<price_t>, which has a numpy fast path (exact
    matching dtype, 1-D and C-contiguous) besides the generic element-wise path. These cases pin
    that both paths return identical results for every accepted input shape/dtype."""

    def test_toPriceList_python_sequence(self):
        self.assertEqual(toPriceList([1.0, 2.5, 3.25]), [1.0, 2.5, 3.25])
        self.assertEqual(toPriceList((1.0, 2.5, 3.25)), [1.0, 2.5, 3.25])
        self.assertEqual(toPriceList([1, 2, 3]), [1.0, 2.0, 3.0])
        self.assertEqual(toPriceList([]), [])

    def test_toPriceList_numpy_1d(self):
        # Same values as a python list, regardless of dtype; float64 is the fast path, the others
        # fall back to the element-wise conversion.
        values = [1.0, 2.0, 3.0, 4.0, 5.0]
        ref = toPriceList(values)
        for dtype in (np.float64, np.float32, np.int64, np.int32, np.int16, np.uint8):
            arr = np.array(values, dtype=dtype)
            self.assertEqual(toPriceList(arr), ref)
            self.assertEqual(toPriceList(arr), values)

    def test_toPriceList_numpy_negative(self):
        values = [-1.5, -2.5, 3.5]
        self.assertEqual(toPriceList(np.array(values, dtype=np.float64)), values)

    def test_toPriceList_numpy_noncontiguous(self):
        base = np.arange(10, dtype=np.float64)
        self.assertEqual(toPriceList(base[::2]), [0.0, 2.0, 4.0, 6.0, 8.0])
        self.assertEqual(toPriceList(base[::-1]), [float(v) for v in range(9, -1, -1)])

    def test_toPriceList_numpy_empty(self):
        self.assertEqual(toPriceList(np.array([], dtype=np.float64)), [])

    def test_toPriceList_multidim_raises(self):
        # Multi-dim arrays keep the original element-wise behavior: obj[i] is a row, not a scalar
        with self.assertRaises(Exception):
            toPriceList(np.array([[1.0, 2.0], [3.0, 4.0]]))


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(DataTypeTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
