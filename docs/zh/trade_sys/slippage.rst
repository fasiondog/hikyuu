.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

移滑价差算法|SP
===============

内建移滑价差算法
----------------

.. py:function:: SP_FixedPercent([p=0.001])

    固定百分比移滑价差算法，买入实际价格 = 计划买入价格 * (1 + p)，卖出实际价格 = 计划卖出价格 * (1 - p)。p 须为 [0, 1) 之间的值

    :param float p: 偏移的固定百分比
    :return: 移滑价差算法实例


.. py:function:: SP_FixedValue([value=0.01])

    固定价格移滑价差算法，买入实际价格 = 计划买入价格 + 偏移价格，卖出实际价格 = 计划卖出价格 - 偏移价格

    :param float value: 偏移价格
    :return: 移滑价差算法实例

.. py:function:: SP_Uniform([min_value=-0.05, max_value=0.05])

    均匀分布随机价格移滑价差算法。[min_value, max_value] 是偏移“幅度区间”：采样值取绝对值后只往不利方向（买入加、卖出减）叠加，因此并非对称的 ± 偏移。买入实际价格 = 计划买入价格 + 幅度，卖出实际价格 = 计划卖出价格 - 幅度

    :param float min_value: 偏移幅度区间下界
    :param float max_value: 偏移幅度区间上界
    :return: 移滑价差算法实例

.. py:function:: SP_Normal([mean=0.0, stddev=0.05])

    正态分布随机价格移滑价差算法。采样值取绝对值后只往不利方向（买入加、卖出减）叠加，因此实际期望滑点是折叠分布的期望 ``E|X|``，不是 ``mean``：

        E|X| = stddev * sqrt(2/pi) * exp(-mean^2 / (2 * stddev^2)) + mean * erf(mean / (stddev * sqrt(2)))

    ``mean`` = 0 时 ``E|X| = stddev * sqrt(2/pi)``，即约为 ``0.7979 * stddev``：``SP_Normal(0.0, 0.05)`` 的期望滑点约 4%。``mean`` 只有与 ``stddev`` 同量级时才明显（``SP_Normal(0.05, 0.05)`` 约 5.8%）；``mean`` 远大于 ``stddev`` 时 ``E|X|`` 趋近 ``mean`` —— ``mean`` 只决定折叠前分布的中心，不直接等于期望滑点

    :param float mean: 正态分布的均值（折叠前分布的中心，不是实际期望滑点）
    :param float stddev: 正态分布的标准差
    :return: 移滑价差算法实例

.. py:function:: SP_LogNormal([mean=0.0, stddev=0.05])

    对数正态分布随机价格移滑价差算法。采样值取绝对值后只往不利方向（买入加、卖出减）叠加

    :param float mean: 对数正态分布的均值
    :param float stddev: 对数正态分布的标准差
    :return: 移滑价差算法实例

.. py:function:: SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1])

    截断正态分布随机价格移滑价差算法。采样值取绝对值后只往不利方向（买入加、卖出减）叠加，样本从 [min_value, max_value] 上的截断正态分布中解析采样（逆 CDF）

    :param float mean: 截断正态分布的均值
    :param float stddev: 截断正态分布的标准差
    :param float min_value: 最小截断值
    :param float max_value: 最大截断值
    :return: 移滑价差算法实例

.. note::

    随机移滑价差（SP_Uniform / SP_Normal / SP_LogNormal / SP_TruncNormal）内部持有随机数引擎，并非线程安全。并发回测（批量回测、WalkForward 等）必须各自克隆实例，不要将 ``shared_sp`` 设为 ``True`` 在多个 System 间共享同一实例。

.. note::

    随机移滑价差的可复现性有边界：``seed`` 仅在**同一平台、同一标准库实现**下保证相同序列。SP_Normal / SP_LogNormal / SP_Uniform 依赖标准库分布（``std::normal_distribution`` / ``std::lognormal_distribution`` / ``std::uniform_real_distribution``），其抽取算法由标准库实现自行决定，同一 ``seed`` 在 libstdc++ / libc++ / MSVC 上并不相同；SP_TruncNormal 使用自带的逆 CDF 采样，不受该项影响。此外，同一实例的序列还取决于钩子的调用次数与顺序。跨平台、跨标准库的复现不在承诺范围内。


自定义移滑价差算法
------------------

自定义移滑价差接口：

* :py:meth:`SlippageBase._getRealBuyPrice` - 【必须】计算原始买入价格（由 System 层校验、按最小变动单位取整并钳制到当日 [最低价, 最高价] 区间）
* :py:meth:`SlippageBase._getRealSellPrice` - 【必须】计算原始卖出价格（由 System 层校验、按最小变动单位取整并钳制到当日 [最低价, 最高价] 区间）
* :py:meth:`SlippageBase._calculate` - 【必须】子类计算接口
* :py:meth:`SlippageBase._clone` - 【必须】克隆接口
* :py:meth:`SlippageBase._reset` - 【可选】重载私有变量


移滑价差算法基类
----------------

.. py:class:: SlippageBase

    移滑价差算法基类

    .. py:attribute:: name 名称

    .. py:method:: __init__(self[, name="SlippageBase"])

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

    .. py:method:: get_real_buy_price(self, datetime, price)

        公共方法，返回滑点模型的原始买入价格：内部调用子类钩子 _get_real_buy_price。框架内唯一的调用方 System 会校验该返回值（非法时回退计划价）、按最小变动单位取整并钳制到当日 [最低价, 最高价] 区间后成交。Python 子类应重写 _get_real_buy_price，不要重写此方法。

        :param Datetime datetime: 买入时间
        :param float price: 计划买入价格
        :return: 滑点模型的原始买入价格
        :rtype: float

    .. py:method:: get_real_sell_price(self, datetime, price)

        公共方法，返回滑点模型的原始卖出价格：内部调用子类钩子 _get_real_sell_price。框架内唯一的调用方 System 会校验该返回值（非法时回退计划价）、按最小变动单位取整并钳制到当日 [最低价, 最高价] 区间后成交。Python 子类应重写 _get_real_sell_price，不要重写此方法。

        :param Datetime datetime: 卖出时间
        :param float price: 计划卖出价格
        :return: 滑点模型的原始卖出价格
        :rtype: float

    .. py:method:: _get_real_buy_price(self, datetime, price)

        【重载接口】计算原始买入价格；滑点算法只负责给出模型价，校验与当日区间钳制由 System 层负责

        :param Datetime datetime: 买入时间
        :param float price: 计划买入价格
        :return: 滑点模型的原始买入价格
        :rtype: float

    .. py:method:: _get_real_sell_price(self, datetime, price)

        【重载接口】计算原始卖出价格；滑点算法只负责给出模型价，校验与当日区间钳制由 System 层负责

        :param Datetime datetime: 卖出时间
        :param float price: 计划卖出价格
        :return: 滑点模型的原始卖出价格
        :rtype: float

    .. py:method:: reset(self)

        复位操作

    .. py:method:: clone(self)

        克隆操作

    .. py:method:: _calculate(self)

        【重载接口】子类计算接口

    .. py:method:: _reset(self)

        【重载接口】子类复位接口，复位内部私有变量

    .. py:method:: _clone(self)

        【重载接口】子类克隆接口
