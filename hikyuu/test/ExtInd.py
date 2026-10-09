#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression test for the plugin AGG_FUNC/GROUP_FUNC callbacks: the
#          returned arrays are forced to a contiguous C-order array of the element
#          type before the flat copy (the user may return sliced/transposed/strided
#          views), and the callback functors hold their Python function through a
#          gil-safe owner.
# ===============================================================================

import unittest

import numpy as np

from test_init import *


class ExtIndTest(unittest.TestCase):
    def setUp(self):
        stk = sm['sh000001']
        self.k = stk.get_kdata(Query(Datetime(20111101), Datetime(20111206), Query.MIN))
        self.assertGreater(len(self.k), 0)

    def _day_groups(self):
        """Yield (group_start, group_last) index pairs, one group per calendar day."""
        dts = self.k.get_datetime_list()
        n = len(dts)
        gs = 0
        for i in range(n):
            nxt = i + 1
            if nxt == n or dts[i].day != dts[nxt].day or dts[i].month != dts[nxt].month:
                yield gs, i
                gs = nxt

    def test_group_func_strided_view_return(self):
        # A reversed view has a negative stride; the flat copy must use the logical
        # (C-order materialized) values, not the raw buffer order.
        ind = GROUP_FUNC(CLOSE(self.k),
                         lambda ds, data: np.arange(len(ds), dtype=float)[::-1],
                         ktype=Query.DAY)
        self.assertEqual(len(ind), len(self.k))
        for gs, gl in self._day_groups():
            expected = [float(gl - i) for i in range(gs, gl + 1)]
            got = [ind[i] for i in range(gs, gl + 1)]
            self.assertEqual(got, expected, "wrong values in the day group %d..%d" % (gs, gl))

    def test_group_func_int_dtype_return(self):
        # A non-float dtype must be cast to the element type by the forcecast conversion
        ind = GROUP_FUNC(CLOSE(self.k),
                         lambda ds, data: (np.arange(len(ds), dtype=np.int64) * 2)[::-1],
                         ktype=Query.DAY)
        self.assertEqual(len(ind), len(self.k))
        for gs, gl in self._day_groups():
            got = [ind[i] for i in range(gs, gl + 1)]
            self.assertEqual(got, [float(2 * (gl - i)) for i in range(gs, gl + 1)])

    def test_agg_func_scalar(self):
        # AGG_FUNC binds an unevaluated indicator and produces a ktype-aligned result
        # (one aggregated value per day bar); the built-in AGG_SUM serves as the oracle.
        day_k = sm['sh000001'].get_kdata(
            Query(Datetime(20111101), Datetime(20111206), Query.DAY))
        self.assertGreater(len(day_k), 0)
        expected = AGG_SUM(CLOSE(), ktype=Query.MIN)(day_k)
        got = AGG_FUNC(CLOSE(), lambda ds, data: float(np.sum(data)), ktype=Query.MIN)(day_k)
        self.assertEqual(len(got), len(day_k))
        for i in range(len(day_k)):
            self.assertAlmostEqual(got[i], expected[i], places=6)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ExtIndTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
