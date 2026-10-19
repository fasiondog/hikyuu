.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

系统有效条件|CN
====================

内建系统有效条件
--------------------

.. py:function:: CN_OPLine(ind)

    固定使用股票最小交易量进行交易，计算权益曲线的op值，当权益曲线高于op时，系统有效，否则无效。

    :param Indicator ind: Indicator实例
    :return: 系统有效条件实例
    :rtype: ConditionBase

    注意：CN_OPLine 会为每个标的执行一次嵌入完整回测，成本约等于一次单标的回测；大规模组合（多个子系统）下会使整体回测成本接近翻倍，大组合建议改用 CN_Bool。

    注意：其有效区间依赖 SYS_Simple/System 的默认参数（buy_delay、sell_delay、can_trade_when_high_eq_low、成本模型 TC_Zero、无滑点）；这些默认值未被显式固定，若发生变化将静默改变有效区间。

.. py:function:: CN_Bool(ind)

    布尔信号指标系统有效条件, 指标中相应位置>0则代表系统有效，否则无效

    注意：非有限值（NaN/±Inf）视为无效。

    :param Indicator ind: bool型指标，输入为 KData
    :return: 系统有效条件实例
    :rtype: ConditionBase


组合运算
----------------------

系统有效条件支持组合运算，把多个有效条件合成一个新的有效条件（组合结果本身仍是 ``ConditionBase``，可继续参与组合）。各条件的取值与交易对象逐 bar 等长对齐。

有效性判定规则：某 bar 的值为有限值且 ``> 0`` 时该日有效、否则无效（含 NaN 与 ±inf），``is_valid`` 即按此判定。下表"全 0（无效）"指该运算把这些 bar 置为 ``0.0``。

.. list-table::
   :header-rows: 1

   * - 运算
     - 两侧都有
     - 仅 cond1（右侧为空）
     - 仅 cond2（左侧为空）
     - 两侧都空
   * - ``cn1 + cn2``（Add）
     - 两侧之和；任一非有限则 NaN
     - 透传 cond1
     - 透传 cond2
     - 全 0（无效）
   * - ``cn1 - cn2``（Sub）
     - 两侧之差；任一非有限则 NaN
     - 透传 cond1
     - 取 ``-cond2``
     - 全 0（无效）
   * - ``cn1 | cn2``（Or）
     - 任一侧 >0 则 1.0，否则 0
     - 归一化为 0/1
     - 归一化为 0/1
     - 全 0（无效）
   * - ``cn1 & cn2``（And）
     - 两侧均 >0 则 1.0，否则 0
     - 全 0（无效）
     - 全 0（无效）
     - 全 0（无效）
   * - ``cn1 * cn2``（Multi）
     - 两侧之积；任一非有限则 NaN
     - 全 0（无效）
     - 全 0（无效）
     - 全 0（无效）
   * - ``cn1 / cn2``（Div）
     - 两侧之商；``cn2 == 0`` 或任一非有限则 NaN
     - 全 NaN（无效）
     - 全 0（无效）
     - 全 0（无效）

注意：当运算的任一侧为空指针时，工厂函数会打印一条告警日志（Add/Sub/Or 提示"按单边模式计算"，And/Multi/Div 提示"结果恒为无效"），随后仍按上表语义构造。


自定义系统有效条件
------------------

快速创建不带私有属性的自定义系统有效条件

.. py:function:: crtCN(func, params=None, name='crtCN')

    快速创建自定义不带私有属性的系统有效条件
    
    :param func: 系统有效条件函数
    :param dict params: 参数字典
    :param str name: 自定义名称
    :return: 自定义系统有效条件实例

自定义系统有效条件接口：

* :py:meth:`ConditionBase._calculate` - 【必须】子类计算接口
* :py:meth:`ConditionBase._clone` - 【必须】克隆接口
* :py:meth:`ConditionBase._is_valid` - 【可选】子类有效性判定规则，默认"有限且 >0"
* :py:meth:`ConditionBase._reset` - 【可选】重载私有变量


系统有效条件基类
----------------

.. py:class:: ConditionBase

    系统有效条件基类
    
    .. py:attribute:: name 名称
    .. py:attribute:: to 设置或获取交易对象
    .. py:attribute:: tm 设置或获取交易管理账户
    .. py:attribute:: sg 设置或获取交易信号指示器
    
    .. py:method:: __init__(self[, name="ConditionBase"])
    
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

    .. py:method:: get_datetime_list(self)

        获取系统有效的日期。注意仅返回系统有效的日期列表，和交易对象不等长

    .. py:method:: get_values(self)

        以指标形式获取实际值，与交易对象逐 bar 等长。有效性判定为值有限且 ``> 0``（否则无效）。布尔/与/或等条件有效位为 ``1.0``；算术条件（Add/Sub/Multi/Div）为两侧的运算结果，可为任意实数，缺失或非有限时记为 NaN。

    .. py:method:: _add_valid(self, datetime[, value=1.0])
    
        加入有效时间，在 _calculate 中调用；同一日期多次调用会累加（+=）
        
        :param Datetime datetime: 有效时间
        :param float value: 累加值，默认 1.0
      
    .. py:method:: reset(self)
    
        复位操作
    
    .. py:method:: clone(self)
    
        克隆操作        
        
    .. py:method:: _calculate(self)
    
        【重载接口】子类计算接口
    
    .. py:method:: _is_valid(self, datetime, value)
    
        【重载接口】子类有效性判定规则，由 is_valid 对每个通过 _add_valid 登记的日期调用；默认值有限且 >0
        
        :param Datetime datetime: 查询日期
        :param float value: 该日期存储的值
        :return: True 有效 | False 无效
    
    .. py:method:: _reset(self)
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口
