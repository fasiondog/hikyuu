.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Walk-forward Trading System
===========================

The walk-forward trading system trades with multiple trading systems, evaluates the system performance in a rolling way, and trades with the system selected by the rolling performance evaluation.

Common parameters (in addition to the common parameters inherited from SYS, its own other common parameters):

    * **market='SH'** *(str)* : the specified market, used for the trading date alignment
    * **train_len=100** *(int)* : the data length used when evaluating the system performance in a rolling way
    * **test_len=20** *(int)* : the data length executed with the optimal system selected in train_len
    * **clean_hold_when_select_changed=True** *(bool)* : when the currently selected system differs from the previous one, clear the existing position at the open
    * **parallel=False** *(bool)* : parallel calculation; if the evaluation function is a pure python function, parallel calculation may not be usable, otherwise it will crash due to the GIL
    * **se_trace=False** *(bool)* : trace and print the SE log information
    

Create a Walk-forward Trading System
------------------------------------

.. py:function:: SYS_WalkForward(sys_list, [tm=None, train_len=100, test_len=20, se=None, train_tm=None])

    Create a walk-forward optimization system; when there is only one candidate system in the input subsequent system list, it is a rolling system

    :param sequence sys_list: the list of the subsequent systems
    :param TradeManager tm: the trading account
    :param int train_len: the data length used when evaluating the system performance in a rolling way
    :param int test_len: the data length executed with the optimal system selected in train_len
    :param SelectorBase se: the optimization selector, defaults to selecting the one with the maximum "Account Avg Annual Return %"
    :param TradeManager train_tm: the trading account used in the rolling evaluation; when None, a copy of tm is used for the evaluation


Built-in Trading System Optimization Selectors
---------------------------------------------

Although the system optimization selector also uses the SE prefix in Hikyuu, it is not the same as the SE in the PF (portfolio), and the two cannot be interchanged. The system optimization selector must be used together with SYS_WalkForward.

The system optimization selector selects the "optimal" system among the candidate trading systems, and uses this "optimal" system for trading in the following time.

The SE in the PF (portfolio), on the other hand, sorts and scores all the candidate systems on the position adjustment date, and the PF allocates the funds according to the scoring result; every candidate system is an actual trading system.


.. py:function:: SE_MaxFundsOptimal()

    The maximum account assets optimization selector


.. py:function:: SE_PerformanceOptimal(key="Account Avg Annual Return %", mode=0)

    The selector that performs the optimization with the Performance statistics results

    :param string key: the Performance statistics item
    :param int mode:  0 take the system with the maximum statistics result | 1 take the system with the minimum statistics result

.. py:function:: SE_EvaluateOptimal(evalulate_func)

    The selector that performs the optimization with a custom function

    :param func: a callable object, whose parameters are (sys, lastdate), and which returns a float value
