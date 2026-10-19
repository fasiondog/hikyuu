.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

市场环境判定策略|EV
===========================

内建市场环境判定策略
---------------------------

.. py:function:: EV_TwoLine(fast, slow[, market = 'SH'])

    快慢线判断策略，市场指数的快线大于慢线时，市场有效，否则无效。

    :param Indicator fast: 快线指标
    :param Indicator slow: 慢线指标
    :param string market: 市场名称

.. py:function:: EV_Bool(ind[, market = 'SH'])

    布尔信号指标市场环境

    :param Indicator ind: bool类型的指标, 指标中相应位置大于0则代表市场有效, 否则无效
    :param str market: 指定的市场，用于获取相应的交易日历

.. py:function:: EV_Manual()

    手工市场环境，有效日期只能通过 :py:meth:`EnvironmentBase._add_valid` 登记，用于测试等场景。

    已登记的日期在 ``query`` 变更前一直有效，因此应先设置 ``query`` 再登记日期。注意组合运算会经
    ``setQuery`` 重跑各侧的 ``_calculate``（手工侧会被清空），如需手工指定组合结果的有效日期，可直接
    在组合结果上调用 :py:meth:`EnvironmentBase._add_valid`。


组合运算
----------------------

市场环境判定策略支持组合运算，把多个环境判定合成一个新的环境判定（组合结果本身仍是 ``EnvironmentBase``，可继续参与组合）。

有效性判定规则：日期的值为有限值且 ``> 0`` 时有效、否则无效（含 NaN 与 ±inf），``is_valid`` 即按此判定；下表中的"不输出"表示该日期不出现在结果里（视为缺失，``is_valid`` 为 False）。

.. list-table::
   :header-rows: 1

   * - 运算
     - 输出日期
     - 输出值
     - 任一侧为空时
   * - ``ev1 & ev2``（And）
     - 两侧均有效的日期
     - 恒为 1.0
     - 结果恒为无效
   * - ``ev1 | ev2``（Or）
     - 任一侧有效的日期
     - 恒为 1.0
     - 按单边模式计算
   * - ``ev1 + ev2``（Add）
     - 两侧日期的并集（含无效日）
     - 两侧值之和；仅一侧存在时取该侧值
     - 按单边模式计算
   * - ``ev1 - ev2``（Sub）
     - 两侧日期的并集（含无效日）
     - 两侧值之差；仅右侧存在时取 ``-ev2``
     - 按单边模式计算
   * - ``ev1 * ev2``（Multi）
     - 两侧一致的日期（同为有效或同为无效）
     - 见下表
     - 结果恒为无效
   * - ``ev1 / ev2``（Div）
     - 两侧一致的日期（同为有效或同为无效）
     - 见下表
     - 结果恒为无效

``Multi`` / ``Div`` 的输出值由两侧当日值的符号共同决定：

.. list-table::
   :header-rows: 1

   * - 左侧值
     - 右侧值
     - ``ev1 * ev2``
     - ``ev1 / ev2``
   * - ``> 0``
     - ``> 0``
     - ``v1*v2``（正，有效）
     - ``v1/v2``（正，有效）
   * - ``> 0``
     - ``< 0``
     - 不输出（分歧，无共识强度）
     - 不输出（分歧，无共识强度）
   * - ``< 0``
     - ``> 0``
     - 不输出（分歧，无共识强度）
     - 不输出（分歧，无共识强度）
   * - ``< 0``
     - ``< 0``
     - ``-(abs(v1)*abs(v2))``（负，无效）
     - ``-(abs(v1)/abs(v2))``（负，无效）
   * - ``= 0``（任一侧）
     - 任意值
     - 不输出（该侧无值）
     - 不输出（该侧无值）


共享与并发
----------------------

市场环境与具体交易对象无关，默认被多个系统共享（``System`` 的 ``shared_ev=True``）：多个系统绑定同一
``EnvironmentBase`` 实例，其结果只与查询条件有关，仅需计算一次。

内部并发语义：

* 读取类接口（如 ``is_valid``）每次调用取**共享锁**（系统回测中每根 bar 调用一次）；
* 设置 ``query`` 与 ``reset`` 取**排他锁**，查询条件与当前不同时完整重算，相同时立即返回；
* ``_add_valid`` 自身不加锁，只能在 ``_calculate`` 中调用。

使用约束：

* 共享同一 ev 的各系统必须使用**相同的查询条件**（同一 K 线类型、复权方式与起止范围），否则每次不同的
  ``setQuery`` 都会在排他锁下完整重算，并发回测退化为串行重算；
* 多线程并发共享同一 ev 时，共享锁存在缓存行争用；极端高频调用场景可为每个系统克隆独立的 ev
  （即 ``System`` 的 ``shared_ev=False``）；
* 对 Python 子类，设置 ``query`` 与 ``reset`` 会在持锁期间回调（需要 GIL），不可与同一对象的其它调用
  并发执行（见下文基类说明）。


自定义市场环境判定策略
----------------------

自定义市场环境判定策略接口：

* :py:meth:`EnvironmentBase._calculate` - 【必须】子类计算接口
* :py:meth:`EnvironmentBase._clone` - 【必须】克隆接口
* :py:meth:`EnvironmentBase._is_valid` - 【可选】自定义有效性判定规则（默认为有限值且 > 0）
* :py:meth:`EnvironmentBase._reset` - 【可选】重载私有变量

市场环境判定策略基类
--------------------

.. py:class:: EnvironmentBase

    市场环境判定策略基类

    .. py:attribute:: name 名称
    .. py:attribute:: query 设置或获取查询条件

    .. py:method:: __init__(self[, name='EnvironmentBase'])

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

    .. py:method:: is_valid(self, datetime)

        指定时间系统是否有效

        :param Datetime datetime: 指定时间
        :return: True 有效 | False 无效

        判定规则为"该日期的值为有限值且 > 0"；从未通过 ``_add_valid`` 登记过的日期为无效。

        判定规则由 :py:meth:`_is_valid` 提供，需要按具体值判定时可重载之。

    .. py:method:: _is_valid(self, datetime, value)

        【重载接口】自定义有效性判定规则，由 :py:meth:`is_valid` 对每个经 ``_add_valid`` 登记的日期调用

        :param Datetime datetime: 查询日期
        :param float value: 该日期的存储值
        :return: True 有效 | False 无效

        基类实现为"有限值且 > 0"，子类通常无需修改；需要根据具体 value 值判定时可重载。System 在回测中
        每根 bar 仅通过 :py:meth:`is_valid` 判定市场有效与否，重载即直接改变交易行为；注意组合运算的
        各侧仍使用内建的 "> 0" 规则。

    .. py:method:: get_value(self, datetime)

        获取指定日期的存储值

        :param Datetime datetime: 指定时间
        :return: 该日期的存储值；从未登记过的日期返回 0.0

        0.0 同时表示"无效"与"未登记"，需用 :py:meth:`is_valid` 判定有效性。

    .. py:method:: get_values(self)

        以时间序列指标的形式获取各日期的值：每个经 ``_add_valid`` 登记的日期一条记录，按日期排序

        :return: Indicator

        日期集合与值的含义取决于产生该对象所用的运算（如 ``Add``/``Sub`` 为两侧并集（含无效日）、
        ``And``/``Or`` 仅保留有效日）；仅有限值且大于 0 表示有效。

    .. py:method:: _add_valid(self, datetime[, value=1.0])

        加入有效时间，在_calculate中调用

        :param Datetime datetime: 有效时间
        :param float value: 默认 1.0；大于 0 表示有效，小于等于 0 表示无效

        非有限值（NaN/inf）会被丢弃；同一日期重复加入时按累加处理（而非覆盖）。

        它自身不加锁，只能在 ``_calculate`` 中调用（该方法在对象加锁期间运行）；在其它位置调用会与 ``is_valid`` 竞争。

    .. py:method:: reset(self)

        复位操作

    .. py:method:: clone(self)

        克隆操作

    .. py:method:: _calculate(self)

        【重载接口】子类计算接口

    .. py:method:: _reset(self)

        【重载接口】子类复位接口，用于复位内部私有变量

    .. py:method:: _clone(self)

        【重载接口】子类克隆接口

    .. note::

        对 Python 子类，设置 ``query`` 会在持锁期间回调 ``_calculate``、``reset`` 会在持锁期间回调
        ``_reset``，两者都需要 GIL，因此不可与同一对象的其它调用并发执行；``clone`` 的 ``_clone``
        回调已在锁外调用，无此限制。
