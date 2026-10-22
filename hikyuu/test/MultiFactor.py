#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression test for get_scores filter dispatch: the filter arity is
#          determined statically via check_pyfunction_arg_num; the filter must run
#          exactly once per candidate and exceptions raised inside it propagate.
# ===============================================================================

import math
import threading
import unittest

from test_init import *


def _finite(x):
    return not (math.isnan(x) or math.isinf(x))


def _mean_corr(xs, ys):
    """Return (corr, mean_x, count) over the pairs whose two sides are both finite."""
    pairs = [(x, y) for x, y in zip(xs, ys) if _finite(x) and _finite(y)]
    n = len(pairs)
    if n == 0:
        return float('nan'), float('nan'), 0
    mx = sum(p[0] for p in pairs) / n
    my = sum(p[1] for p in pairs) / n
    cov = sum((p[0] - mx) * (p[1] - my) for p in pairs)
    vx = sum((p[0] - mx) ** 2 for p in pairs)
    vy = sum((p[1] - my) ** 2 for p in pairs)
    return cov / math.sqrt(vx * vy), mx, n


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

        # the _reset hook itself is bound, so it can also be invoked directly
        py_mf._reset()
        self.assertEqual(py_mf.resets, 2)

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

    def test_python_special_normalizer_getters_no_deadlock(self):
        # A Python special normalizer triggers the same GIL hazard as the global one: every getter
        # that normalizes internally must run serially on the caller thread instead of blocking on
        # pool workers.
        stks = [sm['sh000001'], sm['sz000001']]
        query = Query(Datetime(20110104), Datetime(20111206), Query.DAY, Query.FORWARD)
        mf = MF_EqualWeight([MA(CLOSE(), n=5)], stks, query, stks[0])
        mf.set_param('save_all_factors', True)

        class PyNorm(NormalizeBase):
            def __init__(self):
                super().__init__('PyNorm')

            def _clone(self):
                return PyNorm()

            def normalize(self, data):
                return data

        mf.add_special_normalize('MA', PyNorm())

        holder = {}

        def run():
            try:
                holder['src'] = mf.get_all_src_factors()
                holder['factor'] = mf.get_factor(stks[0])
                holder['all'] = mf.get_all_factors()
                holder['ic'] = mf.get_ic(3)
                holder['icir'] = mf.get_icir(3)
            except Exception as e:  # pragma: no cover
                holder['error'] = e

        t = threading.Thread(target=run)
        t.start()
        t.join(timeout=20)
        self.assertFalse(t.is_alive(), "getters deadlocked with a Python special normalizer")
        self.assertIn('icir', holder, getattr(holder.get('error'), 'args', None))

    def test_industry_neutralize_removes_block_mean(self):
        # Industry neutralization removes the block mean of every cross-section, so the residuals
        # of the stocks sharing one industry block must sum to zero (and are not all zero, i.e. the
        # group mean is really removed instead of the values being nulled).
        query = Query(Datetime(20110104), Datetime(20111206), Query.DAY, Query.FORWARD)
        banks = [sm['sh600000'], sm['sz000001']]  # both belong to the same bank block
        mf = MF_EqualWeight([MA(CLOSE(), n=5)], banks, query, sm['sh000001'])
        mf.add_special_normalize('MA', category='行业板块')

        dates = mf.get_datetime_list()
        self.assertGreater(len(dates), 0)
        src = mf.get_all_src_factors()

        checked = 0
        for di in range(len(dates)):
            vals = [src[i][0][di] for i in range(len(banks))]
            if not all(_finite(v) for v in vals):
                continue
            self.assertAlmostEqual(sum(vals), 0.0, delta=1e-9)
            self.assertGreater(abs(vals[0]), 1e-6)
            checked += 1
        self.assertGreater(checked, 0)

    def test_style_neutralize_residual_is_orthogonal(self):
        # Style neutralization regresses the factor on the given style factors with an intercept, so
        # the residual must be orthogonal to the style factor (zero correlation) and, because of the
        # intercept, its mean over the valid stocks must be zero.
        query = Query(Datetime(20110104), Datetime(20111206), Query.DAY, Query.FORWARD)
        stks = [sm['sh600000'], sm['sz000001'], sm['sh600004'], sm['sz000004'],
                sm['sz000005'], sm['sz000002'], sm['sh600005']]
        style = LOG(CLOSE() * LIUTONGPAN())
        mf = MF_EqualWeight([MA(CLOSE(), n=5)], stks, query, sm['sh000001'])
        mf.add_special_normalize('MA', style_inds=[style])

        dates = mf.get_datetime_list()
        self.assertGreater(len(dates), 0)
        src = mf.get_all_src_factors()
        aligned = [ALIGN(style, dates)(s.get_kdata(query)) for s in stks]

        checked = 0
        for di in range(len(dates)):
            vals = [src[i][0][di] for i in range(len(stks))]
            style_vals = [aligned[i][di] for i in range(len(stks))]
            corr, mean, n = _mean_corr(vals, style_vals)
            if n <= 2:  # not enough valid data points for a regression
                continue
            self.assertAlmostEqual(mean, 0.0, delta=1e-9)
            self.assertAlmostEqual(corr, 0.0, delta=1e-8)
            checked += 1
        self.assertGreater(checked, 0)

    def test_get_icir_resolves_effective_ic_n(self):
        # getICIR must record the effective IC window in the result's "n" param (a non-positive
        # ic_n argument falls back to the configured ic_n, but the recorded "n" must be that
        # resolved window, not the raw 0 default), and rolling_n must match ir_n.
        query = Query(Datetime(20110104), Datetime(20111206), Query.DAY, Query.FORWARD)
        mf = MF_EqualWeight([MA(CLOSE(), n=5)], [sm['sh600000'], sm['sz000001']], query,
                            sm['sh000001'])
        mf.set_param('save_all_factors', True)
        mf.set_param('ic_n', 20)

        icir = mf.get_icir(10)  # ic_n defaults to 0 -> should resolve to the configured 20
        self.assertEqual(icir.get_param('n'), 20)
        self.assertEqual(icir.get_param('rolling_n'), 10)
        self.assertGreater(len(icir), 0)

        icir2 = mf.get_icir(10, 30)  # explicit ic_n must be honored
        self.assertEqual(icir2.get_param('n'), 30)
        self.assertEqual(icir2.get_param('rolling_n'), 10)

    def test_get_icir_rejects_invalid_ir_n(self):
        # ir_n < 1 (and an unresolved ic_n < 1) must be rejected instead of producing a degenerate
        # rolling statistic; the getter returns an empty indicator with a warning.
        query = Query(Datetime(20110104), Datetime(20111206), Query.DAY, Query.FORWARD)
        mf = MF_EqualWeight([MA(CLOSE(), n=5)], [sm['sh600000'], sm['sz000001']], query,
                            sm['sh000001'])
        mf.set_param('save_all_factors', True)

        self.assertEqual(len(mf.get_icir(0)), 0)
        self.assertEqual(len(mf.get_icir(-1)), 0)

    def test_set_normalize_none_disables(self):
        # set_normalize(None) and set_normalize(NORM_NOTHING()) must deterministically disable
        # global normalization (no TypeError on a None argument) and yield the un-normalized baseline.
        mf, ref_stk, query = self._build_mf()
        date = Datetime(201106010000)
        baseline = [sc.value for sc in mf.get_scores(date)]

        mf.set_normalize(None)
        self.assertEqual([sc.value for sc in mf.get_scores(date)], baseline)

        mf.set_normalize(NORM_NOTHING())
        self.assertEqual([sc.value for sc in mf.get_scores(date)], baseline)

    def test_get_scores_non_ref_date_returns_empty(self):
        # a date outside the reference dates (e.g. a weekend) must not stay silent: it returns
        # an empty cross-section with a warning logged, for both get_scores and get_scores_np
        mf, ref_stk, query = self._build_mf()
        weekend = Datetime(201106040000)  # 2011-06-04 is a Saturday, not a trading day
        self.assertNotIn(weekend, list(mf.get_datetime_list()))
        self.assertEqual(len(mf.get_scores(weekend)), 0)
        self.assertEqual(len(mf.get_scores_np(weekend)), 0)

    def test_scores_filter_in_get_scores(self):
        # get_scores accepts a ScoresFilterBase instance (a builtin one or a Python subclass)
        # as the filter, in addition to None and callables
        mf, ref_stk, query = self._build_mf()
        date = Datetime(201106010000)
        all_scores = mf.get_scores(date)
        self.assertGreater(len(all_scores), 0)

        top1 = mf.get_scores(date, filter=SCFilter_TopN(1))
        self.assertEqual(len(top1), 1)
        self.assertEqual(top1[0].stock.market_code, all_scores[0].stock.market_code)
        self.assertEqual(top1[0].value, all_scores[0].value)

        class KeepFirstN(ScoresFilterBase):
            def __init__(self, n):
                super(KeepFirstN, self).__init__("KeepFirstN")
                self.n = n

            def _filter(self, scores, date, query):
                return scores[: self.n]

        kept1 = mf.get_scores(date, filter=KeepFirstN(1))
        self.assertEqual(len(kept1), 1)
        self.assertEqual(kept1[0].stock.market_code, all_scores[0].stock.market_code)
        self.assertEqual(kept1[0].value, all_scores[0].value)

        kept_all = mf.get_scores(date, filter=KeepFirstN(len(all_scores)))
        self.assertEqual([sc.stock.market_code for sc in kept_all],
                         [sc.stock.market_code for sc in all_scores])
        self.assertEqual([sc.value for sc in kept_all], [sc.value for sc in all_scores])

    def test_crtMF_construct_with_name(self):
        # crtMF builds the subclass instance via super().__init__(name): MultiFactorBase must
        # expose the name constructor, otherwise the construction raises TypeError
        from hikyuu.trade_sys.trade_sys import crtMF

        mf = crtMF(lambda all_stk_inds: [inds[0] for inds in all_stk_inds])
        self.assertEqual(mf.name, "crtMF")


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MultiFactorTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
