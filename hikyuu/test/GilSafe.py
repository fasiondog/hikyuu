#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression test for interpreter-finalization-safe py::object lifetime.
#          A Py_AtExit flag set at shutdown start lets background-thread deleters
#          skip decref during finalization, so interpreter exit stays clean.
# ===============================================================================

import os
import subprocess
import sys
import unittest

# Registering a Strategy callback wraps it via make_gil_safe, which registers the
# Py_AtExit finalization flag. Tearing the strategy down during interpreter exit
# must not crash: the finalizing path skips decref instead of touching the GIL.
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
CONFIG = "test_data/hikyuu_win.ini" if sys.platform == "win32" else "test_data/hikyuu_linux.ini"
EXIT_PROBE = (
    "import sys; sys.path.insert(0, r'{root}')\n"
    "from hikyuu import *\n"
    "stg = Strategy()\n"
    "stg.run_daily(lambda stg: None, Seconds(1))\n"
).format(root=REPO_ROOT)

# The C++ system holds the Python custom parts only through pybind11 shared_ptr holders
# (the former explicit reference leaks were removed), so dropping every Python-side
# reference and forcing GC must not invalidate the parts still used by the system.
KEEP_ALIVE_PROBE = (
    "import sys; sys.path.insert(0, r'{root}')\n"
    "import os; os.chdir(r'{root}')\n"
    "import gc\n"
    "from hikyuu import *\n"
    "set_log_level(LOG_LEVEL.OFF)\n"
    "hikyuu_init('{config}')\n"
    "class MySG(SignalBase):\n"
    "    def __init__(self):\n"
    "        super().__init__('MySG')\n"
    "    def _clone(self):\n"
    "        return MySG()\n"
    "    def _calculate(self, kdata):\n"
    "        assert len(kdata) > 0, 'empty kdata: the signal part was not invoked'\n"
    "        self._add_buy_signal(Datetime(201111100000))\n"
    "        self._add_sell_signal(Datetime(201111250000))\n"
    "s = SYS_Simple()\n"
    "s.tm = crtTM()\n"
    "s.sg = MySG()\n"
    "s.mm = MM_FixedCount(100)\n"
    "gc.collect()  # the only remaining reference lives in the C++ system\n"
    "k = StockManager.instance()['sh000001'].get_kdata(Query(-100))\n"
    "assert len(k) > 0, 'no test data'\n"
    "s.to = k\n"
    "s.run(k, False, False)\n"
    "# The TM holds an initial check-in record, so a real trade means more than one\n"
    "assert len(s.tm.get_trade_list()) > 1, 'no trades: the Python signal part was destroyed'\n"
    "# Repeated reassignment must release the previous part instead of leaking it\n"
    "s.sg = MySG()\n"
    "gc.collect()\n"
    "s.run(k, True, False)\n"
    "assert len(s.tm.get_trade_list()) > 1\n"
    "# A clone of the system keeps its Python parts alive as well\n"
    "s2 = s.clone()\n"
    "s2.run(k, True, False)\n"
    "assert len(s2.tm.get_trade_list()) > 1\n"
    "print('OK')\n"
).format(root=REPO_ROOT, config=CONFIG)

# The analyze interfaces release the GIL before running the systems. Python-subclass
# components (SG/MM/ST, ...) called from the released context rely on pybind11 >= 3.0
# acquiring the GIL inside the PYBIND11_OVERRIDE macros; a downgrade or a hand-written
# trampoline without it would crash the interpreter here.
ANALYZE_PY_PARTS_PROBE = (
    "import sys; sys.path.insert(0, r'{root}')\n"
    "import os; os.chdir(r'{root}')\n"
    "import faulthandler; faulthandler.enable()\n"
    "from hikyuu import *\n"
    "from hikyuu.analysis import analysis_sys_list_multi\n"
    "set_log_level(LOG_LEVEL.OFF)\n"
    "hikyuu_init('{config}')\n"
    "class MySG(SignalBase):\n"
    "    def __init__(self):\n"
    "        super().__init__('MySG')\n"
    "    def _clone(self):\n"
    "        return MySG()\n"
    "    def _calculate(self, kdata):\n"
    "        assert len(kdata) > 0, 'empty kdata: the signal part was not invoked'\n"
    "        self._add_buy_signal(Datetime(201111100000))\n"
    "        self._add_sell_signal(Datetime(201111250000))\n"
    "class MyMM(MoneyManagerBase):\n"
    "    def __init__(self):\n"
    "        super().__init__('MyMM')\n"
    "    def _clone(self):\n"
    "        return MyMM()\n"
    "    def _get_buy_num(self, datetime, stopprice, price, risk, part_from):\n"
    "        return 100\n"
    "    def _get_sell_num(self, datetime, stock, price, risk, part_from):\n"
    "        return 0\n"
    "class MyST(StoplossBase):\n"
    "    def __init__(self):\n"
    "        super().__init__('MyST')\n"
    "    def _clone(self):\n"
    "        return MyST()\n"
    "    def _get_price(self, datetime, price):\n"
    "        return price * 0.95\n"
    "stk = StockManager.instance()['sh000001']\n"
    "q = Query(Datetime(20111101), Datetime(20111206), Query.DAY, Query.FORWARD)\n"
    "proto = SYS_Simple()\n"
    "proto.tm = crtTM(); proto.sg = MySG(); proto.mm = MyMM(); proto.st = MyST()\n"
    "analysis_sys_list_multi([stk], q, proto)\n"
    "analysis_sys_list_multi([stk, stk], q, proto)\n"
    "print('OK')\n"
).format(root=REPO_ROOT, config=CONFIG)

# EnvironmentBase.clone() must invoke the subclass _clone hook without holding the base mutex:
# a hook that re-enters the exclusive lock (here by setting the query on itself) would deadlock
# while the clone still held the shared lock, which is the lock-order hazard the fix removes.
CLONE_LOCK_PROBE = (
    "import sys; sys.path.insert(0, r'{root}')\n"
    "import os; os.chdir(r'{root}')\n"
    "from hikyuu import *\n"
    "set_log_level(LOG_LEVEL.OFF)\n"
    "hikyuu_init('{config}')\n"
    "class MyEV(EnvironmentBase):\n"
    "    def __init__(self):\n"
    "        super().__init__('MyEV')\n"
    "    def _calculate(self):\n"
    "        self._add_valid(Datetime(201111100000))\n"
    "    def _clone(self):\n"
    "        self.query = Query(-100)  # would deadlock while clone() holds the shared lock\n"
    "        return MyEV()\n"
    "ev = MyEV()\n"
    "ev.query = Query(-100)\n"
    "cloned = ev.clone()\n"
    "assert cloned.is_valid(Datetime(201111100000)), 'the clone lost the valid dates'\n"
    "print('OK')\n"
).format(root=REPO_ROOT, config=CONFIG)

# The pure-C++ MultiFactor compute (EqualWeight over all the test securities) must release the GIL,
# so a background Python thread keeps running while the long calculation is in flight.
# sys.setswitchinterval(1.0) disables the interpreter's periodic GIL drop during the <1s compute,
# so a held GIL would block the background thread entirely (count == 0) and only an explicit
# release lets it progress.
MF_GIL_RELEASE_PROBE = (
    "import sys; sys.path.insert(0, r'{root}')\n"
    "import os; os.chdir(r'{root}')\n"
    "import time, threading\n"
    "from hikyuu import *\n"
    "set_log_level(LOG_LEVEL.OFF)\n"
    "hikyuu_init('{config}')\n"
    "sm = StockManager.instance()\n"
    "blks = sm.get_block_list()\n"
    "seen = set(); stks = []\n"
    "for b in blks:\n"
    "    for s in b.get_stock_list():\n"
    "        if not s.is_null() and s.market_code not in seen:\n"
    "            seen.add(s.market_code); stks.append(s)\n"
    "query = Query(Datetime(20100101), Datetime(20231231), Query.DAY, Query.FORWARD)\n"
    "ref = sm['sh000001']\n"
    "inds = [MA(CLOSE(), n) for n in range(3, 153)]\n"
    "mf = MF_EqualWeight(inds, stks, query, ref)\n"
    "dates = mf.get_datetime_list()\n"
    "mid = dates[len(dates) // 2]\n"
    "_ = mf.get_scores(mid)\n"
    "counter = [0]\n"
    "def compute():\n"
    "    mf.reset(); _ = mf.get_scores(mid)\n"
    "sys.setswitchinterval(1.0)\n"
    "ta = threading.Thread(target=compute, daemon=True)\n"
    "ta.start()\n"
    "while ta.is_alive():\n"
    "    counter[0] += 1\n"
    "    if (counter[0] & 0x3FF) == 0:\n"
    "        time.sleep(0)\n"
    "ta.join()\n"
    "assert counter[0] > 100, 'GIL not released during pure-C++ MF compute: B_count=%d' % counter[0]\n"
    "print('OK')\n"
).format(root=REPO_ROOT, config=CONFIG)

# A Python custom normalizer makes the MF a python-part model. The compute still releases the GIL
# (the norm callback re-acquires it per call through the pybind11 trampoline), so the background
# thread must keep running and the call must not deadlock. The waiting loop yields every iteration:
# a spinning thread starves the per-callback GIL handoff (convoy), inflating the few-ms compute to
# minutes. sys.setswitchinterval(1.0) keeps a held GIL from being masked by the periodic drop.
MF_PY_PART_GIL_RELEASE_PROBE = (
    "import sys; sys.path.insert(0, r'{root}')\n"
    "import os; os.chdir(r'{root}')\n"
    "import time, threading\n"
    "from hikyuu import *\n"
    "set_log_level(LOG_LEVEL.OFF)\n"
    "hikyuu_init('{config}')\n"
    "sm = StockManager.instance()\n"
    "stks = []\n"
    "stks_set = set()\n"
    "for b in sm.get_block_list():\n"
    "    for s in b.get_stock_list():\n"
    "        if not s.is_null() and s.market_code not in stks_set:\n"
    "            stks_set.add(s.market_code); stks.append(s)\n"
    "            if len(stks) >= 50:\n"
    "                break\n"
    "    if len(stks) >= 50:\n"
    "        break\n"
    "ref = sm['sh000001']\n"
    "query = Query(Datetime(20110104), Datetime(20111206), Query.DAY, Query.FORWARD)\n"
    "inds = [MA(CLOSE(), n) for n in range(3, 8)]\n"
    "mf = MF_EqualWeight(inds, stks, query, ref)\n"
    "class PyNorm(NormalizeBase):\n"
    "    def __init__(self):\n"
    "        super().__init__('PyNorm')\n"
    "    def _clone(self):\n"
    "        return PyNorm()\n"
    "    def normalize(self, data):\n"
    "        return data\n"
    "mf.set_normalize(PyNorm())\n"
    "dates = mf.get_datetime_list()\n"
    "mid = dates[len(dates) // 2]\n"
    "_ = mf.get_scores(mid)\n"
    "counter = [0]\n"
    "def compute():\n"
    "    mf.reset(); _ = mf.get_scores(mid)\n"
    "sys.setswitchinterval(1.0)\n"
    "ta = threading.Thread(target=compute, daemon=True)\n"
    "ta.start()\n"
    "while ta.is_alive():\n"
    "    counter[0] += 1\n"
    "    time.sleep(0)\n"
    "ta.join()\n"
    "assert counter[0] > 100, 'GIL not released for python-part MF: B_count=%d' % counter[0]\n"
    "print('OK')\n"
).format(root=REPO_ROOT, config=CONFIG)


class GilSafeTest(unittest.TestCase):
    def test_exit_clean_after_gil_safe_use(self):
        proc = subprocess.run(
            [sys.executable, "-c", EXIT_PROBE],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=180,
        )
        self.assertEqual(
            proc.returncode, 0, "interpreter exit crashed:\n%s" % proc.stderr[-2000:]
        )

    def test_system_python_parts_keep_alive(self):
        # Removing the explicit reference leaks in the set_* bindings means the Python
        # custom parts are kept alive only by the pybind11 shared_ptr holders. Dropping
        # every Python-side reference and forcing GC must leave the parts usable, and the
        # holder destructors (possibly on non-Python threads) must not crash.
        proc = subprocess.run(
            [sys.executable, "-c", KEEP_ALIVE_PROBE],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=120,
        )
        self.assertEqual(proc.returncode, 0, "keep-alive probe crashed:\n%s" % proc.stderr[-2000:])
        self.assertIn("OK", proc.stdout)

    def test_analyze_gil_release_with_python_parts(self):
        # The analyze interfaces run the systems with the GIL released. The Python
        # subclass components are called back through the trampolines, which rely on
        # pybind11 >= 3.0 acquiring the GIL inside the PYBIND11_OVERRIDE macros.
        proc = subprocess.run(
            [sys.executable, "-c", ANALYZE_PY_PARTS_PROBE],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=120,
        )
        self.assertEqual(proc.returncode, 0, "analyze probe crashed:\n%s" % proc.stderr[-2000:])
        self.assertIn("OK", proc.stdout)

    def test_clone_hook_runs_without_the_object_lock(self):
        # clone() must invoke the subclass _clone hook with the base mutex free: a hook that
        # re-enters the exclusive lock (or reaches the GIL) would otherwise deadlock.
        proc = subprocess.run(
            [sys.executable, "-c", CLONE_LOCK_PROBE],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=120,
        )
        self.assertEqual(
            proc.returncode, 0, "clone lock probe crashed:\n%s" % proc.stderr[-2000:]
        )
        self.assertIn("OK", proc.stdout)

    def test_multifactor_gil_release_pure_cpp(self):
        # A pure-C++ MultiFactor compute must release the GIL, so a background Python thread keeps
        # running during the long calculation. sys.setswitchinterval(1.0) disables the periodic GIL
        # drop inside the <1s compute, so without the explicit release the background thread would be
        # fully blocked (count == 0) and the probe would fail its assertion.
        proc = subprocess.run(
            [sys.executable, "-c", MF_GIL_RELEASE_PROBE],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=120,
        )
        self.assertEqual(
            proc.returncode, 0, "MF GIL-release probe failed:\n%s" % proc.stderr[-2000:]
        )
        self.assertIn("OK", proc.stdout)

    def test_multifactor_gil_release_with_python_part(self):
        # A Python custom normalizer makes the MF a python-part model; the compute must still release
        # the GIL (the norm callback re-acquires it per call) and must not deadlock.
        proc = subprocess.run(
            [sys.executable, "-c", MF_PY_PART_GIL_RELEASE_PROBE],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=120,
        )
        self.assertEqual(
            proc.returncode, 0, "MF python-part GIL-release probe failed:\n%s" % proc.stderr[-2000:]
        )
        self.assertIn("OK", proc.stdout)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(GilSafeTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
