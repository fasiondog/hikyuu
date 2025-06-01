.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

滚动交易系统
=============

滚动交易系统是使用多个交易系统进行交易，并使用滚动评估系统性能，并使用滚动评估系统性能所选择的系统进行交易。

公共参数(除 SYS 继承的公共参数外，自身其他公共参数):

    * **market='SH'** *(str)* : 指定市场, 用于交易日期对齐
    * **train_len=100** *(int)* : 滚动评估系统绩效时使用的数据长度
    * **test_len=20** *(int)* : 使用在 train_len 中选出的最优系统执行的数据长度
    * **clean_hold_when_select_changed=True** *(bool)* : 当前选中的系统和上次的系统不一致时，在开盘时清空已有持仓
    * **parallel=False** *(bool)* : 并行计算, 如果评估函数为纯python函数时, 可能不能使用并行计算，否则会因 GIL 引起崩溃
    * **se_trace=False** *(bool)* : 跟踪打印 SE 的日志信息
    

创建滚动交易系统
------------------

.. py:function:: SYS_WalkForward(sys_list, [tm=None, train_len=100, test_len=20, se=None, train_tm=None])

    创建滚动寻优系统，当输入的后续系统列表中仅有一个候选系统时，即为滚动系统

    :param sequence sys_list: 后续系统列表
    :param TradeManager tm: 交易账户
    :param int train_len: 滚动评估系统绩效时使用的数据长度
    :param int test_len: 使用在 train_len 中选出的最优系统执行的数据长度
    :param SelectorBase se: 寻优选择器, 默认为按“帐户平均年收益率%”最大选择
    :param TradeManager train_tm: 滚动评估时使用的交易账户, 为None时, 使用 tm 的拷贝进行评估


内建交易系统寻优选择器
-------------------------

系统寻优选择器在 Hikyuu 中虽然也使用 SE 前缀，但和 PF（投资组合） 中的 SE 并不相同，两者不能互换。系统寻优选择器必须和 SYS_WalkForward 配合使用。

系统寻优选择器是在候选交易系统中选出“最优”的一个系统，在接下来的时间使用这个“最优”系统进行交易。

PF（投资组合）中的 SE，则是在调仓日对所有候选系统进行排序评分，并由 PF 根据评分结果进行资金分配，每个候选系统都为实际的交易系统。


.. py:function:: SE_MaxFundsOptimal()

    账户资产最大寻优选择器


.. py:function:: SE_PerformanceOptimal(key="帐户平均年收益率%", mode=0)

    使用 Performance 统计结果进行寻优的选择器

    :param string key: Performance 统计项
    :param int mode:  0 取统计结果最大的值系统 | 1 取统计结果为最小值的系统

.. py:function:: SE_EvaluateOptimal(evalulate_func)

    使用自定义函数进行寻优的选择器

    :param func: 一个可调用对象，接收参数为 (sys, lastdate)，返回一个 float 数值
