#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression test for get_scores filter dispatch: the filter arity is
#          determined statically via check_pyfunction_arg_num; the filter must run
#          exactly once per candidate and exceptions raised inside it propagate.
# ===============================================================================

import threading
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

    def test_python_mf_subclass_synthesizes_factors(self):
        # A Python subclass must be able to synthesize factors through the trampoline; the
        # hook runs when get_scores() triggers the calculation.
        mf, ref_stk, query = self._build_mf()

        class MyMF(MultiFactorBase):
            def __init__(self):
                super().__init__()

            def _clone(self):
                return MyMF()

            def _calculate(self, all_stk_inds):
                return [inds[0] for inds in all_stk_inds]

        py_mf = MyMF()
        py_mf.set_stock_list(mf.get_stock_list())
        py_mf.query = query
        py_mf.set_ref_stock(ref_stk)
        py_mf.set_ref_factorset([MA(CLOSE(), n=5), MA(CLOSE(), n=10)], Query.DAY)

        scores = py_mf.get_scores(Datetime(201106010000))
        self.assertEqual(len(scores), len(mf.get_stock_list()))

    def test_python_mf_subclass_reset_hook_is_forwarded(self):
        # reset() must forward to the subclass _reset hook through the trampoline.
        class MyMF(MultiFactorBase):
            def __init__(self):
                super().__init__()
                self.resets = 0

            def _clone(self):
                return MyMF()

            def _calculate(self, all_stk_inds):
                return [inds[0] for inds in all_stk_inds]

            def _reset(self):
                self.resets += 1

        py_mf = MyMF()
        py_mf.reset()
        self.assertEqual(py_mf.resets, 1)

    def test_get_scores_np_and_iter_scores(self):
        mf, ref_stk, query = self._build_mf()
        date = Datetime(201106010000)

        # get_scores_np must match get_scores record by record
        records = mf.get_scores(date)
        arr = mf.get_scores_np(date)
        self.assertEqual(len(arr), len(records))
        for i, rec in enumerate(records):
            self.assertEqual(arr[i]["market_code"], rec.stock.market_code)
            self.assertAlmostEqual(float(arr[i]["score"]), float(rec.value))

        # start / end slicing
        part = mf.get_scores_np(date, 0, 1)
        self.assertEqual(len(part), 1)
        self.assertEqual(part[0]["market_code"], records[0].stock.market_code)

        # iter_scores must cover every date, in the reference date order, with the same content
        dates = mf.get_datetime_list()
        all_scores = mf.get_all_scores()
        self.assertEqual(len(dates), len(all_scores))
        n = 0
        for one in mf.iter_scores():
            self.assertEqual(len(one), len(all_scores[n]))
            for i in range(len(one)):
                self.assertEqual(one[i]["market_code"], all_scores[n][i].stock.market_code)
            n += 1
        self.assertEqual(n, len(dates))

        # range slicing of iter_scores
        self.assertEqual(len(list(mf.iter_scores(1, 4))), 3)

        # the iterator keeps the model alive even after the python handle is dropped
        it = mf.iter_scores()
        next(it)
        del mf
        self.assertEqual(sum(1 for _ in it), len(dates) - 1)

    def test_python_normalizer_no_deadlock(self):
        # A Python custom normalizer must not deadlock: its clone()/normalize() need the GIL,
        # so the parallel normalization must run serially on the (GIL-holding) caller thread
        # instead of on pool workers that would block forever.
        mf, ref_stk, query = self._build_mf()

        class PyNorm(NormalizeBase):
            def __init__(self):
                super().__init__('PyNorm')

            def _clone(self):
                return PyNorm()

            def normalize(self, data):
                return data

        mf.set_normalize(PyNorm())

        holder = {}

        def run():
            try:
                holder['scores'] = mf.get_scores(Datetime(201106010000))
            except Exception as e:  # pragma: no cover
                holder['error'] = e

        t = threading.Thread(target=run)
        t.start()
        t.join(timeout=15)
        self.assertFalse(t.is_alive(), "get_scores deadlocked with a Python custom normalizer")
        self.assertIn('scores', holder, getattr(holder.get('error'), 'args', None))
        self.assertGreater(len(holder['scores']), 0)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MultiFactorTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
