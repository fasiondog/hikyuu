.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

多因子合成|MF
==============

多因子本质是在时间截面上对候选标的进行评分，所以实际需要配合 Selector (策略选择算法) 使用。

因子计算时的标准化与中性化，参见: :doc:`normalize`

公共参数:

    * **fill_null** *(bool|True)*: 数据缺失时是否填充 nan 值, 否则以最近值填充
    * **ic_n** *(int|5)*: 用于计算合成后因子 IC 时的 ic_n 日收益率
    * **spearman** *(bool|True)*: 使用 spearman 获取相关系数，否则为 pearson
    * **mode** *(int|0)*: 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    * **save_all_factors** *(bool|False)*: 是否保存所有因子值,影响 get_actor/get_all_factors 方法


内建对因子合成算法
--------------------------------

.. py:function:: MF_Weight(input, stks, weights, query, ref_stk[, ic_n=5, spearman=True, mode=0, save_all_factors=False])

    按指定权重合成因子 = ind1 * weight1 + ind2 * weight2 + ... + indn * weightn，支持多种输入类型

    :param input: 因子输入，可以是FactorSet对象或Indicator序列
    :param sequence(stock) stks: 计算证券列表
    :param sequence(float) weights: 权重列表(需和因子数量等长)
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券用于日期对齐 (未指定时，默认为 sh000001)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :param int mode: 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    :param bool save_all_factors: 是否保存所有因子值,影响 get_actor/get_all_factors 方法
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # 使用Indicator列表
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        weights = [0.6, 0.4]
        mf1 = MF_Weight(indicators, stocks, weights, query)
        
        # 使用FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_Weight(factor_set, stocks, weights, query)


.. py:function:: MF_EqualWeight(input, stks, query, ref_stk[, ic_n=5])

    等权重合成因子，支持多种输入类型

    :param input: 因子输入，可以是FactorSet对象或Indicator序列
    :param sequence(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券用于日期对齐 (未指定时，默认为 sh000001)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :param int mode: 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    :param bool save_all_factors: 是否保存所有因子值,影响 get_actor/get_all_factors 方法    
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # 使用Indicator列表
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_EqualWeight(indicators, stocks, query)
        
        # 使用FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_EqualWeight(factor_set, stocks, query)


.. py:function:: MF_ICWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    滚动IC权重合成因子，支持多种输入类型

    :param input: 因子输入，可以是FactorSet对象或Indicator序列
    :param sequence(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 用于日期对齐的参考证券 (未指定时，默认为 sh000001)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :param int mode: 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    :param bool save_all_factors: 是否保存所有因子值,影响 get_actor/get_all_factors 方法    
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # 使用Indicator列表
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICWeight(indicators, stocks, query)
        
        # 使用FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_ICWeight(factor_set, stocks, query)


.. py:function:: MF_ICIRWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    滚动ICIR权重合成因子，支持多种输入类型

    :param input: 因子输入，可以是FactorSet对象或Indicator序列
    :param sequence(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 用于日期对齐的参考证券 (未指定时，默认为 sh000001)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :param int mode: 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    :param bool save_all_factors: 是否保存所有因子值,影响 get_actor/get_all_factors 方法    
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # 使用Indicator列表
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICIRWeight(indicators, stocks, query)
        
        # 使用FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_ICIRWeight(factor_set, stocks, query)


自定义多因子合成算法基类
--------------------------------------

自定义多因子合成算法接口：

* :py:meth:`MultiFactorBase._calculate` - 【必须】计算合成因子
* :py:meth:`MultiFactorBase._clone` - 【必须】返回当前子类的一个新实例
* :py:meth:`MultiFactorBase._reset` - 【可选】重置子类的私有成员变量，由 :py:meth:`MultiFactorBase.reset` 调用


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

    .. py:method:: reset(self)

        复位操作，清除已计算的结果并调用子类的 _reset 钩子

    .. py:method:: get_ref_stock(self)

        获取参考证券

    .. py:method:: set_ref_stock(self, stk)

        重新设置参考证券

        :param Stock stk: 新指定的参考证券

    .. py:method:: get_stock_list(self)

        获取创建时指定的证券列表

    .. py:method:: set_stock_list(self, stks)

        重新指定证券列表

        :param list stks: 指定的证券列表

        .. note::
            不拒绝重复证券：重复项会在每个日期的截面中各计一条（会改变等权分母），
            证券索引中只保留最后一次出现的位置。

    .. py:method:: get_stock_list_num(self)

        获取创建时指定的证券列表中证券数量

    .. py:method:: get_datetime_list(self)

        获取参考日期列表（由参考证券通过查询条件获得）

    .. py:method:: get_ref_indicators(self)

        获取创建时输入的原始因子列表

    .. py:method:: set_ref_indicators(self, inds)

        重新设置原始因子列表

        :param list inds: 新的原始因子列表

    .. py:method:: get_factor(self, stock)

        获取指定证券合成后的新因子

        :param Stock stock: 指定证券
        :raises: 当参数 ``save_all_factors`` 为 False 时抛出异常

    .. py:method:: get_all_factors(self)

        获取所有证券合成后的因子列表

        :return: [factor1, factor2, ...] 顺序与参考证券顺序相同
        :raises: 当参数 ``save_all_factors`` 为 False 时抛出异常

    .. py:method:: get_ic(self[, ndays=0])

        获取合成因子的IC, 长度与参考日期同

        ndays 对于使用 IC/ICIR 加权的新因子，最好保持好 ic_n 一致，
        但对于等权计算的新因子，不一定非要使用 ic_n 计算。
        所以，ndays 增加了一个特殊值 0, 表示直接使用 ic_n 参数计算 IC
     
        :param int ndays: ic 的 ndays 日收益率
        :rtype: Indicator

    .. py:method:: get_icir(self, ir_n[, ic_n=0])

        获取合成因子的 ICIR

        ir_n 小于 1，或 ic_n 解析后小于 1 时，返回空 Indicator 并告警。

        :param int ir_n: 计算 IR 的 n 窗口
        :param int ic_n: 计算 IC 的 n 窗口 (同 get_ic 中的 ndays)，非正数表示使用因子的 ic_n 参数

    .. py:method:: get_score(self, date[, start=0, end=Null])

        获取指定日期截面的所有因子值，排序方式取决于 mode 参数（0-降序, 1-升序, 2-不排序），相当于各证券日期截面评分。

        :param Datetime date: 指定日期；若该日期不在参考日期内（如非交易日），返回空列表并输出警告日志
        :param int start: 取当日排名开始
        :param int end: 取当日排名结束(不包含本身)
        :param function func: (ScoreRecord)->bool 或 (Datetime, ScoreRecord)->bool 为原型的可调用对象
        :rtype: ScoreRecordList

    .. py:method:: get_all_scores(self)

        获取所有日期的所有评分，长度与参考日期相同

        :note: 每条记录都会成为一个 Python 对象，内存随"证券数 x 日期数"增长；结果较大时请改用 get_scores_np(date) 或 iter_scores()

        :return: ScoreRecordList

    .. py:method:: get_scores_np(self, date[, start=0, end=Null])

        获取指定日期截面的 numpy 结构化数组，按 mode 排序（0-降序, 1-升序, 2-不排序）。结果较大时优先于 get_scores 使用：整个截面只构建一个数组，而非每条记录一个 Python 对象。字段名为 market_code、name、score。

        :param Datetime date: 指定日期；若该日期不在参考日期内（如非交易日），返回空数组并输出警告日志
        :param int start: 取当日排名开始
        :param int end: 取当日排名结束(不包含本身)
        :rtype: numpy.ndarray

    .. py:method:: iter_scores(self[, start=0, end=Null])

        按日期迭代所有截面，每次产出一个 numpy 结构化数组，顺序与 get_datetime_list() 一致。某一时刻只有当日截面被物化，内存保持在一个截面而非全部日期。

        :param int start: 迭代的日期范围开始
        :param int end: 迭代的日期范围结束(不包含本身)
        :rtype: iterator

    .. py:method:: get_all_src_factors(self)

        获取所有原始因子列表(如果指定了标准化、行业中性化, 返回为已处理的因子列表)

        :rtype: list
        :return: list IndicatorList stks x inds

    .. py:method:: set_normalize(self, norm)

        设置标准化或归一化方法（影响全部因子）
    
        :param NormalizeBase norm: 标准化或归一化方法实例

    
    .. py:method:: add_special_normalize(self, name[, norm=None, category="", style_inds=[]])
        
        对指定名称的指标应用特定的标准化/归一化、行业中性化、风格因子中性化操作。标准化操作、行业中性化、风格因子中性化彼此无关，可同时指定也可分开指定。

        :param str name: 特殊归一化方法名称
        :param Normalize norm: 特殊归一化方法
        :param str category: 行业中性化时，指定板块类别
        :param list[Indicator] style_inds: 用于中性化的风格指标列表


    .. py:method:: _calculate(self, stks_inds)

        计算每日证券合成因子，输入参数由上层函数计算后传入，如：

        待计算的证券列表 - stk1, stk2
        原始因子列表 - ind1, ind2
        则传入的 stks_inds 为：[IndicatorList(stk1)[ind1, ind2], IndicatorList(stk2)[ind1, ind2]]

        :param list stks_inds: 与证券列表顺序相同已经计算好的所有证券的原始因子列表
        :return: 按证券列表顺序存放的所有新的因子

    .. py:method:: _clone(self)

        返回当前子类的一个新实例，由 clone() 调用。返回值必须是当前子类的实例，而非基类实例。

        :rtype: MultiFactorBase

    .. py:method:: _reset(self)

        重置子类的私有成员变量，由 reset() 调用。若子类无需复位成员变量，可不实现该接口。

        .. note::
            该钩子在基类持有内部锁期间被调用，其实现不可再次调用本实例需要该内部锁的其它方法，否则会死锁。