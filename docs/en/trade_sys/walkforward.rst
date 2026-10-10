.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Walk-forward Trading System
===========================

The walk-forward trading system runs multiple candidate trading systems, evaluates their performance on a rolling basis over trailing windows, and trades with the system selected by the most recent rolling evaluation.

Common parameters (in addition to those inherited from SYS):

    * **market='SH'** *(str)* : the market used to align the trading dates
    * **train_len=100** *(int)* : the length of the rolling in-sample window over which system performance is evaluated
    * **test_len=20** *(int)* : the length of the out-of-sample period traded with the best system selected during train_len
    * **clean_hold_when_select_changed=True** *(bool)* : when the newly selected system differs from the previous one, liquidate the existing position at the open
    * **parallel=False** *(bool)* : enable parallel evaluation. Parallel evaluation is safe with a Python evaluation function (the GIL is handled internally), but the evaluate calls are serialized by the GIL; prefer the built-in C++ selectors when the evaluation is expensive
    * **se_trace=False** *(bool)* : trace and print the SE log output


Create a Walk-forward Trading System
------------------------------------

.. py:function:: SYS_WalkForward(sys_list, [tm=None, train_len=100, test_len=20, se=None, train_tm=None])

    Create a walk-forward optimization system. When the input list contains only a single candidate system, it behaves as a plain rolling system.

    :param sequence sys_list: the list of candidate trading systems
    :param TradeManager tm: the trading account
    :param int train_len: the length of the rolling in-sample window over which system performance is evaluated
    :param int test_len: the length of the out-of-sample period traded with the best system selected during train_len
    :param SelectorBase se: the selector that chooses the optimal system; defaults to the one with the maximum "Account Avg Annual Return %"
    :param TradeManager train_tm: the trade manager used during the rolling evaluation; when None, a copy of tm is used


Built-in Trading System Optimization Selectors
----------------------------------------------

Although the system optimization selector also uses the SE prefix in Hikyuu, it is different from the SE used in a PF (portfolio), and the two cannot be interchanged. The system optimization selector must be used together with SYS_WalkForward.

The system optimization selector picks the single "optimal" system among the candidate trading systems, and that system is used for trading over the following period.

The SE in a PF (portfolio), in contrast, ranks and scores all candidate systems on each rebalance date, and the PF allocates funds according to those scores; every candidate system is traded for real.


.. py:function:: SE_MaxFundsOptimal()

    Selector that chooses the system with the largest total account assets.


.. py:function:: SE_PerformanceOptimal(key="Account Avg Annual Return %", mode=0)

    Selector that performs the optimization using the Performance statistics results.

    :param string key: the Performance statistic item
    :param int mode:  0 select the system with the maximum statistic | 1 select the system with the minimum statistic

.. py:function:: SE_EvaluateOptimal(evalulate_func)

    Selector that performs the optimization using a custom function.

    :param func: a callable taking the parameters (sys, lastdate) and returning a float

    .. note::

        The custom evaluation function runs on a thread-pool worker. Each call acquires and releases the GIL automatically, so the evaluate calls are serialized with a small per-call overhead; keep the evaluation function lightweight. When the evaluation is expensive, prefer the built-in C++ selectors (such as :py:func:`SE_PerformanceOptimal` and :py:func:`SE_MaxFundsOptimal`), which run without any GIL involvement and are fully parallel. This limitation disappears when running on a free-threaded CPython build (3.13t and later, PEP 703).
