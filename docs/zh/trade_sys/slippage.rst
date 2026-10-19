.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

移滑价差算法|SP
===============

内建移滑价差算法
----------------

.. py:function:: SP_FixedPercent([p=0.001, short_p=None])

    固定百分比移滑价差算法，买入实际价格 = 计划买入价格 * (1 + p)，卖出实际价格 = 计划卖出价格 * (1 - p)。p 须为 [0, 1) 之间的值

    :param float p: 偏移的固定百分比
    :param float short_p: 开空与平空使用的固定百分比，须为 [0, 1) 之间的值；None（默认）表示空头侧沿用 p
    :return: 移滑价差算法实例


.. py:function:: SP_FixedValue([value=0.01, short_value=None])

    固定价格移滑价差算法，买入实际价格 = 计划买入价格 + 偏移价格，卖出实际价格 = 计划卖出价格 - 偏移价格

    :param float value: 偏移价格，须 >= 0
    :param float short_value: 开空与平空使用的偏移价格，须 >= 0；None（默认）表示空头侧沿用 value
    :return: 移滑价差算法实例

.. py:function:: SP_Uniform([min_value=-0.05, max_value=0.05, adverse_only=True, short_min_value=None, short_max_value=None])

    均匀分布随机价格移滑价差算法。``adverse_only=True``（默认）时，采样值取绝对值后只往不利方向（买入加、卖出减）叠加，即 [min_value, max_value] 为偏移“幅度区间”。``adverse_only=False`` 时，直接使用带符号的采样值作为零均值执行噪声（不保证对称；区间以 0 为中心时期望为 0）

    :param float min_value: 偏移区间下界
    :param float max_value: 偏移区间上界，须 >= min_value
    :param bool adverse_only: True（默认）折为不利方向；False 直接使用带符号的偏移值
    :param float short_min_value: 空头侧偏移区间下界；None 表示沿用 min_value
    :param float short_max_value: 空头侧偏移区间上界；None 表示沿用 max_value
    :return: 移滑价差算法实例

.. py:function:: SP_Normal([mean=0.0, stddev=0.05, adverse_only=True, short_mean=None, short_stddev=None])

    正态分布随机价格移滑价差算法。``adverse_only=True``（默认）时，采样值取绝对值后只往不利方向叠加，实际期望滑点是折叠分布的期望 ``E|X|``，不是 ``mean``：

        E|X| = stddev * sqrt(2/pi) * exp(-mean^2 / (2 * stddev^2)) + mean * erf(mean / (stddev * sqrt(2)))

    ``mean`` = 0 时 ``E|X| = stddev * sqrt(2/pi)``，即约为 ``0.7979 * stddev``。``adverse_only=False`` 时，直接使用带符号的采样值作为零均值执行噪声（不保证对称；期望为 ``mean``）

    :param float mean: 正态分布的均值
    :param float stddev: 正态分布的标准差，须 >= 0
    :param bool adverse_only: True（默认）折为不利方向；False 直接使用带符号的偏移值
    :param float short_mean: 空头侧正态分布均值；None 表示沿用 mean
    :param float short_stddev: 空头侧正态分布标准差，须 >= 0；None 表示沿用 stddev
    :return: 移滑价差算法实例

.. py:function:: SP_LogNormal([mean=0.0, stddev=0.05, adverse_only=True, short_mean=None, short_stddev=None])

    对数正态分布随机价格移滑价差算法。``adverse_only=True``（默认）时，采样值取绝对值后只往不利方向叠加。``adverse_only=False`` 时，直接使用中心化后的采样值（``X - E[X]``）作为零均值执行噪声：期望严格为 0，但分布右偏、不保证对称，``mean`` 只放大离散程度、不产生漂移（与 ``SP_Normal`` 中 ``mean`` 即漂移不同）

    :param float mean: 对数正态分布的均值
    :param float stddev: 对数正态分布的标准差，须 >= 0
    :param bool adverse_only: True（默认）折为不利方向；False 直接使用带符号的偏移值
    :param float short_mean: 空头侧对数正态分布均值；None 表示沿用 mean
    :param float short_stddev: 空头侧对数正态分布标准差，须 >= 0；None 表示沿用 stddev
    :return: 移滑价差算法实例

.. py:function:: SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1, adverse_only=True, short_mean=None, short_stddev=None, short_min_value=None, short_max_value=None])

    截断正态分布随机价格移滑价差算法。``adverse_only=True``（默认）时，采样值取绝对值后只往不利方向叠加。``adverse_only=False`` 时，直接使用带符号的采样值作为零均值执行噪声（不保证对称；仅当截断区间以 ``mean`` 为中心时期望等于 ``mean``）。样本从 [min_value, max_value] 上的截断正态分布中解析采样（逆 CDF）

    :param float mean: 截断正态分布的均值
    :param float stddev: 截断正态分布的标准差，须 >= 0
    :param float min_value: 最小截断值
    :param float max_value: 最大截断值，须 >= min_value
    :param bool adverse_only: True（默认）折为不利方向；False 直接使用带符号的偏移值
    :param float short_mean: 空头侧截断正态分布均值；None 表示沿用 mean
    :param float short_stddev: 空头侧截断正态分布标准差，须 >= 0；None 表示沿用 stddev
    :param float short_min_value: 空头侧最小截断值；None 表示沿用 min_value
    :param float short_max_value: 空头侧最大截断值；None 表示沿用 max_value
    :return: 移滑价差算法实例

.. note::

    内建移滑价差同时接收多头侧与空头侧参数。空头侧涵盖两个动作：开空（融券卖出，走 get_real_sell_short_price）与平空（买回归券，走 get_real_buy_short_price）。short_* 参数留空（None）时，实例在构造时逐个取对应的多头侧参数值，因此新建实例时不传任何 short_* 参数，空头侧与改造前完全一致；构造后再改多头侧参数不会影响空头侧。``adverse_only`` 不分侧，两侧共用。

.. warning::

    由**旧版本序列化的档案**（XML / pickle，含 System 档案）不含 ``short_*`` 键，反序列化后空头侧取的是类的默认值，而不是对应多头侧参数值（例如旧档案中的 ``SP_FixedPercent(0.05)``，升级后其空头侧滑点为 0.001）。需要与旧行为一致时，请在反序列化后显式补齐：``sp.set_param("short_p", sp.get_param("p"))``（其他模型同理）。

.. note::

    随机移滑价差（SP_Uniform / SP_Normal / SP_LogNormal / SP_TruncNormal）内部持有随机数引擎，并非线程安全。并发回测（批量回测、WalkForward 等）必须各自克隆实例，不要将 ``shared_sp`` 设为 ``True`` 在多个 System 间共享同一实例。

.. note::

    四个随机移滑价差均带有 ``seed`` 参数（int64，默认 0 表示随机种子）。设为非 0 值后，同一实例在 ``reset()`` 后的序列可复现；``seed`` 只在 ``reset()`` 中被读取，因此 ``set_param("seed", ...)`` 后须显式调用 ``reset()`` 才生效（框架内 ``System`` 会在每次 ``run()`` 前调用 ``reset()``）。

.. note::

    随机移滑价差的可复现性有边界：``seed`` 仅在**同一平台、同一标准库实现**下保证相同序列。SP_Normal / SP_LogNormal / SP_Uniform 依赖标准库分布（``std::normal_distribution`` / ``std::lognormal_distribution`` / ``std::uniform_real_distribution``），其抽取算法由标准库实现自行决定，同一 ``seed`` 在 libstdc++ / libc++ / MSVC 上并不相同；SP_TruncNormal 使用自带的逆 CDF 采样，不受该项影响。此外，同一实例的序列还取决于钩子的调用次数与顺序。跨平台、跨标准库的复现不在承诺范围内。

.. note::

    四个随机移滑价差对"过大"的参数给出一致告警（仅告警、不改变计算）：标准差 > 1.0（SP_Normal / SP_LogNormal）或幅度 > 1.0（SP_Uniform / SP_TruncNormal，即区间上下界绝对值的较大者）时输出一条告警日志；多头侧与 short_ 侧各自判定；参数本身不设上界。


自定义移滑价差算法
------------------

自定义移滑价差接口：

* :py:meth:`SlippageBase._getRealBuyPrice` - 【必须】计算原始买入价格（由 System 层校验、按最小变动单位取整并钳制到当日 [最低价, 最高价] 区间）
* :py:meth:`SlippageBase._getRealSellPrice` - 【必须】计算原始卖出价格（由 System 层校验、按最小变动单位取整并钳制到当日 [最低价, 最高价] 区间）
* :py:meth:`SlippageBase._calculate` - 【必须】子类计算接口
* :py:meth:`SlippageBase._clone` - 【必须】克隆接口
* :py:meth:`SlippageBase._reset` - 【可选】重载私有变量
* :py:meth:`SlippageBase._getRealBuyShortPrice` - 【可选】计算平空（买回归券）的原始价格，不重载时委托给 ``_getRealBuyPrice``
* :py:meth:`SlippageBase._getRealSellShortPrice` - 【可选】计算开空（融券卖出）的原始价格，不重载时委托给 ``_getRealSellPrice``

自定义实现的注意事项：

* ``to`` 属性赋值时，若行情非空且与上次已计算的不同，会调用一次 ``_calculate``（重复赋同一行情不会重复调用）；``reset()`` 会清空 ``to`` 并调用 ``_reset()``；``clone()`` 调用 ``_clone()``，克隆体保留 ``to`` 但标记为未计算，下次 ``to`` 赋值时会重新 ``_calculate``。
* 不要重写 ``get_real_buy_price`` / ``get_real_sell_price``，只实现 ``_get_real_buy_price`` / ``_get_real_sell_price``：基类版本只做转发（计划价非正时直接返回计划价），价格权威在 System 层。
* 钩子返回值必须是有限正数：非有限值（NaN / Inf）或 ``<= 0`` 视为非法，System 层回退为计划价并告警。
* System 层会先按最小变动单位取整，再钳制到当日 [最低价, 最高价]：当滑点把价格推出该区间（如买入价高于当日最高价、卖出价低于当日最低价）时，钳制会削弱甚至抵消滑点效果。
* Python 子类的 ``_calculate`` 不要调用 ``super()._calculate()``，否则会无限递归。
* 只在需要多空不对称滑点时才重载 ``_get_real_buy_short_price`` / ``_get_real_sell_short_price``；不重载时基类分别委托给 ``_get_real_buy_price`` / ``_get_real_sell_price``，旧实现行为不变。


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

    .. py:method:: get_real_buy_short_price(self, datetime, price)

        公共方法，返回平空（买回归券）使用的原始价格：内部调用子类钩子 _get_real_buy_short_price，该钩子未被重写时回落到 _get_real_buy_price。System 层对此返回值的校验与 get_real_buy_price 一致。

        :param Datetime datetime: 买入时间
        :param float price: 计划买入价格
        :return: 平空使用的滑点模型原始价格
        :rtype: float

    .. py:method:: get_real_sell_short_price(self, datetime, price)

        公共方法，返回开空（融券卖出）使用的原始价格：内部调用子类钩子 _get_real_sell_short_price，该钩子未被重写时回落到 _get_real_sell_price。

        :param Datetime datetime: 卖出时间
        :param float price: 计划卖出价格
        :return: 开空使用的滑点模型原始价格
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

    .. py:method:: _get_real_buy_short_price(self, datetime, price)

        【重载接口】计算平空（买回归券）的原始价格；仅在空头侧需要与多头侧不同的滑点时重写，默认委托给 _get_real_buy_price

        :param Datetime datetime: 买入时间
        :param float price: 计划买入价格
        :return: 平空使用的原始价格
        :rtype: float

    .. py:method:: _get_real_sell_short_price(self, datetime, price)

        【重载接口】计算开空（融券卖出）的原始价格；仅在空头侧需要与多头侧不同的滑点时重写，默认委托给 _get_real_sell_price

        :param Datetime datetime: 卖出时间
        :param float price: 计划卖出价格
        :return: 开空使用的原始价格
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
