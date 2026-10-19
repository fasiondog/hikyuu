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


自定义市场环境判定策略
----------------------

自定义市场环境判定策略接口：

* :py:meth:`EnvironmentBase._calculate` - 【必须】子类计算接口
* :py:meth:`EnvironmentBase._clone` - 【必须】克隆接口
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
