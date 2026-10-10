#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression test for get_scores filter dispatch: the filter arity is
#          determined statically via check_pyfunction_arg_num; the filter must run
#          exactly once per candidate and exceptions raised inside it propagate.
# ===============================================================================

import unittest

from test_init import *


class MultiFactorTest(unittest.TestCase):
    def _build_mf(self):
        stks = [sm['sh000001'], sm['sz000001']]
        ref_stk = stks[0]
        query = Query(Datetime(20110104), Datetime(20111206), Query.DAY, Query.FORWARD)
        indicators = [MA(CLOSE(), n=5), MA(CLOSE(), n=10)]
        return MF_EqualWeight(indicators, stks, query, ref_stk), ref_stk, query

    def test_get_scores_filter_arity(self):
        mf, ref_stk, query = self._build_mf()
        date = Datetime(201106010000)
        all_scores = mf.get_scores(date)
        self.assertGreater(len(all_scores), 0)

        # 1-argument filter: (ScoreRecord) -> bool
        ret1 = mf.get_scores(date, filter=lambda sc: sc.value > 0)
        self.assertTrue(all(sc.value > 0 for sc in ret1))

        # 2-argument filter: (Datetime, ScoreRecord) -> bool
        ret2 = mf.get_scores(date, filter=lambda d, sc: sc.value <= 0)
        self.assertTrue(all(sc.value <= 0 for sc in ret2))

    def test_get_scores_filter_exception_propagates_and_runs_once(self):
        mf, ref_stk, query = self._build_mf()
        date = Datetime(201106010000)
        calls = []

        def bad_filter(sc):
            calls.append(1)
            raise RuntimeError("boom inside filter")

        with self.assertRaises(RuntimeError):
            mf.get_scores(date, filter=bad_filter)
        # The old catch(...) probe executed the user's callback a second time;
        # the filter must run exactly once per invocation.
        self.assertEqual(len(calls), 1)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MultiFactorTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
