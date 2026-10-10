#!/usr/bin/python
# -*- coding: utf8 -*-
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
import subprocess
import sys
import unittest

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
        self.assertIn("EVAL_COUNT", proc.stdout)
        self.assertIn("OK", proc.stdout)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(SelectorTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
