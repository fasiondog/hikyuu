.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

滚动交易系统
=============

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
