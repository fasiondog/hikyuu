.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

选择器算法组件|SE
==================

实现标的、系统策略的评估和选取算法。

公共参数：

    * **get_n** *(int | 0)* : 仅取选中的前 get_n 个系统，小于等于0时为取全部
    * **depend_on_proto_sys** *(bool |False)* : 需要原型系统可独立运行

        ::

            通常原型系统不参与计算, 但某些特殊的场景, 需要依赖于伴生系统策略,
            此时可以认为实际执行的系统行为跟随伴生系统的买卖交易, 如依赖于SG进行选择
            (不过由于仅依赖SG的场景不严谨, 因为原型和实际系统的SG是一样的)
            此时, 需要在自身计算之前执行原型系统, 然后SE自行时可以使用.
            而对于实际系统和被跟随的系统完全不一样的情况, 可以自行设计特殊的SE.

基于多因子评分的选择器, 推荐使用 SE_MultiFactor2, 可以自定义添加对 MF 返回的截面评分记录进行组合过滤。参见：:doc:`scfilter`

::

    # 如为 SE_MultiFactor2 实例，设置过滤：分值不为Nan|分成10组取第0组|价格大于等于10元|成交金额不在当日排名末尾20%之内|取前10
    se.set_scores_filter(SCFilter_IgnoreNan()|SCFilter_Group(10, 0)SCFilter_Price(
            10.) | SCFilter_AmountLimit(0.2) | SCFilter_TopN(10))



内建选择器
-----------

.. py:function:: SE_Fixed([stk_list, sys])

    固定选择器，即始终选择初始划定的标的及其系统策略原型
    
    :param list stk_list: 初始划定的标的
    :param System sys: 系统策略原型
    :return: SE选择器实例

.. py:function:: SE_Signal([stk_list, sys])

    信号选择器，仅依靠系统买入信号进行选中
    
    :param list stk_list: 初始划定的标的
    :param System sys: 系统策略原型
    :return: SE选择器实例

.. py:function:: SE_MultiFactor(input[, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, spearman=True, mode="MF_ICIRWeight"])

    创建基于多因子评分的选择器，支持多种创建方式:

    - 直接指定 MF: ``SE_MultiFactor(mf, topn=10)``
    - 使用 FactorSet: ``SE_MultiFactor(factor_set, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight")``
    - 使用 Indicator 序列: ``SE_MultiFactor(indicators, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight")``
      
    :param input: 因子输入，可以是 MultiFactorBase 对象、FactorSet 对象或 Indicator 序列
    :param int topn: 只选取时间截面中前 topn 个系统, 小于等于0时代表不限制
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :param Stock ref_stk: 参考证券 (未指定时，默认为 sh000300 沪深300)
    :param bool spearman: 默认使用 spearman 计算相关系数，否则使用 pearson
    :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" 因子合成算法名称
    :return: SE选择器实例

    .. code-block:: python
    
        # 使用Indicator列表（原有方式）
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor(indicators, topn=10)
        
        # 使用FactorSet（新增方式）
        factor_set = FactorSet("my_factors")
        factor_set.add(MA(CLOSE(), 5))
        factor_set.add(MA(CLOSE(), 10))
        selector2 = SE_MultiFactor(factor_set, topn=10)
        
        # 直接使用预创建的MultiFactor对象
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor(mf, topn=10)

.. py:function:: SE_MultiFactor2(input[, ic_n=5, ic_rolling_n=120, ref_stk=None, spearman=True, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan()])

    创建基于多因子评分的选择器，支持多种创建方式。:doc:`scfilter`

    - 直接指定 MF: ``SE_MultiFactor2(mf, filter)``
    - 使用 FactorSet: ``SE_MultiFactor2(factor_set, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan())``
    - 使用 Indicator 序列: ``SE_MultiFactor2(indicators, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan())``

    SE_MultiFactor2 有一个单独参数 "mf_recover_type", 使用 set_param 设置。指定 mf 使用固定的复权类型进行计算，比如使用固定前复权

    :param input: 因子输入，可以是 MultiFactorBase 对象、FactorSet 对象或 Indicator 序列
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :param Stock ref_stk: 参考证券 (未指定时，默认为 sh000300 沪深300)
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" 因子合成算法名称
    :param ScoresFilter filter: 评分过滤器
    :return: SE选择器实例

    .. code-block:: python
    
        # 使用Indicator列表
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor2(indicators)
        
        # 使用FactorSet
        factor_set = FactorSet("my_factors")
        factor_set.add(MA(CLOSE(), 5))
        factor_set.add(MA(CLOSE(), 10))
        selector2 = SE_MultiFactor2(factor_set)
        
        # 直接使用预创建的MultiFactor对象
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor2(mf)
        
自定义选择器策略
--------------------

自定义选择器策略接口：

* :py:meth:`SelectorBase.is_match_af` - 【必须】判断是否和AF匹配
* :py:meth:`SelectorBase.get_selected` - 【必须】获取指定时刻的系统实例列表
* :py:meth:`SelectorBase._calculate` - 【必须】计算接口
* :py:meth:`SelectorBase._reset` - 【可选】重置私有属性
* :py:meth:`SelectorBase._clone` - 【必须】克隆接口

选择器策略基类
----------------

.. py:class:: SelectorBase

    选择器策略基类，实现标的、系统策略的评估和选取算法
    
    .. py:attribute:: name 名称

    .. py:attribute:: proto_sys_list 原型系统列表

    .. py:attribute:: real_sys_list 运行时的实际系统列表
    
    .. py:method:: __init__(self[, name="SelectorBase])
    
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

    .. py:method:: reset(self)
    
        复位操作
    
    .. py:method:: clone(self)
    
        克隆操作        
        
    .. py:method:: add_stock(self, stock, sys)

        加入初始标的及其对应的系统策略原型
        
        :param Stock stock: 加入的初始标的
        :param System sys: 系统策略原型

    .. py:method:: add_stock_list(self, stk_list, sys)
    
        加入初始标的列表及其系统策略原型
        
        :param StockList stk_list: 加入的初始标的列表
        :param System sys: 系统策略原型
    
    .. py:method:: remove_all(self)
    
        清除所有已加入的原型系统

    .. py:method:: set_scores_filter(self, scfilter)

        设置 ScoresFilter, 将替换现有的过滤器, 仅适用于 SE_MultiFactor
    
        :param ScoresFilter filter: ScoresFilter

    .. py:method:: add_scores_filter(self, scfilter)

        在已有过滤基础上新增过滤, 仅适用于 SE_MultiFactor

        :param ScoresFilter filter: 新的过滤器    

    .. py:method:: is_match_af(self)

        【重载接口】判断是否和 AF 匹配

        :param AllocateFundsBase af: 资产分配算法


    .. py:method:: get_selected(self, datetime)
    
        【重载接口】获取指定时刻选取的系统实例
        
        :param Datetime datetime: 指定时刻
        :return: 选取的系统实例列表
        :rtype: SystemList


     .. py:method:: _calculate(self)

        【重载接口】子类计算接口

     .. py:method:: _reset(self)
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口  
    
    
        