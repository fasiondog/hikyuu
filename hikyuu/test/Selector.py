#!/usr/bin/python
# -*- coding: utf-8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression test for TS-003/TS-004 - PyOptimalSelector::evaluate and
#          the SE_EvaluateOptimal lambda must hold the GIL when calling Python,
#          since OptimalSelectorBase::_calculate_parallel runs the evaluate on a
#          thread-pool worker without the GIL. Calling Python without the GIL
#          crashes the interpreter. The selector clone must also keep the Python
#          evaluate, otherwise the parallel path silently degrades.
# ===============================================================================

import os
import re
import subprocess
import sys
import unittest

from test_init import *

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
CONFIG = "test_data/hikyuu_win.ini" if sys.platform == "win32" else "test_data/hikyuu_linux.ini"

# Drives the parallel Python evaluate through the product GIL-releasing entry
# (analysis_sys_list_multi -> inner_analysis_sys_list, which pairs
# py::gil_scoped_release with OStreamToPython off). The probe prints EVAL_COUNT
# so the test can tell "evaluate fired" from "evaluate never ran".
PARALLEL_EVAL_PROBE = (
    "import sys; sys.path.insert(0, r'{root}')\n"
    "import os; os.chdir(r'{root}')\n"
    "import faulthandler; faulthandler.enable()\n"
    "import os\n"
    "from hikyuu import *\n"
    "from hikyuu.analysis import analysis_sys_list_multi\n"
    "set_log_level(LOG_LEVEL.OFF)\n"
    "hikyuu_init('{config}')\n"
    "sm = StockManager.instance()\n"
    "stk = sm['sh000001']\n"
    "sg = SG_Cross(MA(CLOSE(), n=5), MA(CLOSE(), n=10))\n"
    "mm = MM_FixedCount(100)\n"
    "q = Query(Datetime(20000101), Datetime(20200101), Query.DAY, Query.FORWARD)\n"
    "k = stk.get_kdata(q)\n"
    "flag = os.path.join(r'{root}', 'test_data', 'tmp', 'ts003_eval_flag.txt')\n"
    "try:\n"
    "    os.remove(flag)\n"
    "except OSError:\n"
    "    pass\n"
    "def eval_func(s, lastdate):\n"
    "    with open(flag, 'a') as f:\n"
    "        f.write('EVAL\\n')\n"
    "    return 1.0\n"
    "# OptimalSelectorBase::_calculate_parallel runs evaluate on a thread-pool worker\n"
    "# without the GIL; the caller releases the GIL via the analyze interface.\n"
    "se = crtSEOptimal(eval_func)\n"
    "se.set_param('train_len', 500)\n"
    "se.set_param('test_len', 500)\n"
    "proto = SYS_Simple(); proto.tm = crtTM(); proto.sg = sg; proto.mm = mm\n"
    "se.add_stock(stk, proto)\n"
    "se.add_stock(stk, proto)\n"
    "ms = MultiSystem()\n"
    "ms.tm = crtTM(); ms.sg = sg; ms.mm = mm; ms.to = k\n"
    "for i in range(2):\n"
    "    sub = SYS_Simple(); sub.tm = crtTM(); sub.sg = sg; sub.mm = mm; sub.to = k\n"
    "    ms.add(sub)\n"
    "ms.set_se(se)\n"
    "analysis_sys_list_multi([stk], q, ms)\n"
    "count = sum(1 for _ in open(flag)) if os.path.exists(flag) else 0\n"
    "print('EVAL_COUNT', count)\n"
    "print('OK')\n"
).format(root=REPO_ROOT, config=CONFIG)


class SelectorTest(unittest.TestCase):
    def test_parallel_python_evaluate_holds_gil(self):
        # crtSEOptimal invokes the Python evaluation function on a thread-pool worker
        # that does not hold the GIL. Without re-acquiring the GIL inside evaluate,
        # the Python call crashes the interpreter (access violation). Run in a
        # subprocess so a regression fails loudly instead of killing this process.
        proc = subprocess.run(
            [sys.executable, "-c", PARALLEL_EVAL_PROBE],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=120,
        )
        if proc.returncode != 0:
            self.fail("parallel Python evaluate crashed:\n%s" % proc.stderr[-2000:])
        self.assertIn("OK", proc.stdout)
        matched = re.search(r"EVAL_COUNT\s+(\d+)", proc.stdout)
        self.assertIsNotNone(matched, proc.stdout[-2000:])
        # The probe prints OK even when evaluate never fired, so check the count
        self.assertGreater(int(matched.group(1)), 0, proc.stdout[-2000:])


class SelectorScoresFilterTest(unittest.TestCase):
    # The pybind default is evaluated once at module load, so each call must build its own filter.
    def test_default_scfilter_not_shared(self):
        se1 = SE_MultiFactor2([MA(CLOSE(), n=5), MA(CLOSE(), n=10)])
        se2 = SE_MultiFactor2([MA(CLOSE(), n=5), MA(CLOSE(), n=10)])
        self.assertIsNotNone(se1.scfilter)
        self.assertIsNotNone(se2.scfilter)
        self.assertIsNot(se1.scfilter, se2.scfilter)

    def test_default_scfilter_not_shared_mf_overload(self):
        stks = [sm['sh000001'], sm['sz000001']]
        query = Query(Datetime(20110104), Datetime(20111206), Query.DAY, Query.FORWARD)
        mf = MF_EqualWeight([MA(CLOSE(), n=5), MA(CLOSE(), n=10)], stks, query, stks[0])
        se1 = SE_MultiFactor2(mf)
        se2 = SE_MultiFactor2(mf)
        self.assertIsNot(se1.scfilter, se2.scfilter)

    def test_set_scores_filter_none_clears(self):
        se = SE_MultiFactor2([MA(CLOSE(), n=5), MA(CLOSE(), n=10)], filter=SCFilter_TopN(3))
        self.assertEqual(se.scfilter.name, "SCFilter_TopN")
        se.set_scores_filter(None)
        self.assertIsNone(se.scfilter)

    def test_add_scores_filter_isolated_between_selectors(self):
        se1 = SE_MultiFactor2([MA(CLOSE(), n=5), MA(CLOSE(), n=10)])
        se2 = SE_MultiFactor2([MA(CLOSE(), n=5), MA(CLOSE(), n=10)])
        # Assert the isolation by behavior, not by the pybind11 wrapper identity.
        scores = ScoreRecordList()
        for stk, value in [(sm['sh000001'], 0.5), (sm['sz000001'], 1.5),
                           (sm['sh600004'], 2.5), (sm['sh600005'], 3.5),
                           (sm['sz000002'], 4.5)]:
            scores.append(ScoreRecord(stk, value))
        date = Datetime(20200101)
        query = Query(-5)
        self.assertEqual(len(se2.scfilter.filter(scores, date, query)), 5)
        se1.add_scores_filter(SCFilter_TopN(2))
        # se1's chain gains the TopN (truncates to 2), se2's chain is untouched (keeps all 5)
        self.assertEqual(len(se1.scfilter.filter(scores, date, query)), 2)
        self.assertEqual(len(se2.scfilter.filter(scores, date, query)), 5)
        self.assertEqual(se2.scfilter.name, "SCFilter_IgnoreNan")

    def test_add_stock_list(self):
        # The python list must be converted before the GIL is released.
        proto = SYS_Simple()
        proto.sg = SG_Cross(MA(CLOSE(), n=5), MA(CLOSE(), n=10))
        proto.mm = MM_FixedCount(100)
        se = SE_Fixed()
        se.add_stock_list([sm['sh000001'], sm['sz000001']], proto)
        self.assertEqual(len(se.proto_sys_list), 2)


def suite():
    suite = unittest.TestSuite()
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(SelectorTest))
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(SelectorScoresFilterTest))
    return suite


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
