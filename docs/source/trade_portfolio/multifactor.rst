.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

多因子合成
============

多因子本质是在时间截面上对候选标的进行评分，所以实际需要配合 Selector (策略选择算法) 使用。


内建对因子合成算法
--------------------------------

.. py:function:: MF_Weight(inds, weights, stks, query, ref_stk[, ic_n=5, spearman=True])

    按指定权重合成因子 = ind1 * weight1 + ind2 * weight2 + ... + indn * weightn

    :param sequense(Indicator) inds: 原始因子列表
    :param sequense(float) weights: 权重列表(需和 inds 等长)
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券 (未指定时，默认为 sh000300 沪深300)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :rtype: MultiFactorBase


.. py:function:: MF_EqualWeight(inds, stks, query, ref_stk[, ic_n=5, spearman=True])

    等权重合成因子

    :param sequense(Indicator) inds: 原始因子列表
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :rtype: MultiFactorBase


.. py:function:: MF_ICWeight(inds, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120, spearman=True])

    滚动IC权重合成因子

    :param sequense(Indicator) inds: 原始因子列表
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :rtype: MultiFactorBase


.. py:function:: MF_ICIRWeight(inds, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120, spearman=True])

    滚动ICIR权重合成因子

    :param sequense(Indicator) inds: 原始因子列表
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :rtype: MultiFactorBase


自定义多因子合成算法基类
--------------------------------------

自定义多因子合成算法接口：

* :py:meth:`MultiFactorBase._calculate` - 【必须】计算合成因子


多因子合成算法基类
---------------------------------------

.. py:class:: MultiFactorBase

    多因子合成基类
    
    .. py:attribute:: name 名称
    .. py:attribute:: query 查询条件

    .. py:method:: __init__(self)
    
        初始化构造函数
        
        :param str name: 名称
        
    .. py:method:: get_param(self, name)

        获取指定的参数
    
        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数
        
    .. py:method:: set_param(self, name, value)
    
        设置参数
        
        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型        

    .. py:method:: clone(self)
    
        克隆操作 

    .. py:method:: get_ref_stock(self)

        获取参考证券

    .. py:method:: set_ref_stock(self, ref_stk)

        重新设置参考证券

        :param Stock ref_stk: 新指定的参考证券

    .. py:method:: get_stock_list(self)

        获取创建时指定的证券列表

    .. py:method:: set_stock_list(self, stks)

        重新指定证券列表

        :param list stks: 指定的证券列表

    .. py:method:: get_stock_list_num(self)

        获取创建时指定的证券列表中证券数量

    .. py:method:: get_datetime_list(self)

        获取参考日期列表（由参考证券通过查询条件获得）

    .. py:method:: get_ref_indicators(self)

        获取创建时输入的原始因子列表

    .. py::method:: set_ref_indicators(self, inds)

        重新设置原始因子列表

        :param list Indicator: 原始因子列表

    .. py:method:: get_factor(self, stock)

        获取指定证券合成后的新因子

        :param Stock stock: 指定证券

    .. py:method:: get_all_factors(self)

        获取所有证券合成后的因子列表

        :return: [factor1, factor2, ...] 顺序与参考证券顺序相同

    .. py:method:: get_ic(self[, ndays=0])

        获取合成因子的IC, 长度与参考日期同

        ndays 对于使用 IC/ICIR 加权的新因子，最好保持好 ic_n 一致，
        但对于等权计算的新因子，不一定非要使用 ic_n 计算。
        所以，ndays 增加了一个特殊值 0, 表示直接使用 ic_n 参数计算 IC
     
        :param int ndays: ic 的 ndays 日收益率
        :rtype: Indicator

    .. py:method:: get_icir(self, ir_n[, ic_n=0])

        获取合成因子的 ICIR

        :param int ir_n: 计算 IR 的 n 窗口
        :param int ic_n: 计算 IC 的 n 窗口 (同 get_ic 中的 ndays)

    .. py:method:: get_score(self, date[, start=0, end=Null])

        获取指定日期截面的所有因子值，已经降序排列，相当于各证券日期截面评分。

        :param Datetime date: 指定日期
        :param int start: 取当日排名开始
        :param int end: 取当日排名结束(不包含本身)
        :rtype: ScoreRecordList

    .. py:method:: get_all_scores(self)

        获取所有日期的所有评分，长度与参考日期相同

        :return: 每日 ScoreRecordList 结果的 list

    .. py:method:: _calculate(self, stks_inds)

        计算每日证券合成因子，输入参数由上层函数计算后传入，如：

        待计算的证券列表 - stk1, stk2
        原始因子列表 - ind1, ind2
        则传入的 stks_inds 为：[IndicatorList(stk1)[ind1, ind2], IndicatorList(stk2)[ind1, ind2]]

        :param list stks_inds: 与证券列表顺序相同已经计算好的所有证券的原始因子列表
        :return: 按证券列表顺序存放的所有新的因子



    