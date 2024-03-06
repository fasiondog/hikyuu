.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

系统有效条件
============

内建系统有效条件
----------------

.. py:function:: CN_OPLine(ind)

    固定使用股票最小交易量进行交易，计算权益曲线的op值，当权益曲线高于op时，系统有效，否则无效。

    :param Indicator ind: Indicator实例
    :return: 系统有效条件实例
    :rtype: ConditionBase

.. py:function:: CN_Bool(ind)

    布尔信号指标系统有效条件, 指标中相应位置>0则代表系统有效，否则无效

    :param Indicator ind: bool型指标，输入为 KData
    :return: 系统有效条件实例
    :rtype: ConditionBase


自定义系统有效条件
------------------

快速创建不带私有属性的自定义系统有效条件

.. py:function:: crtCN(func, params={}, name='crtSG')

    快速创建自定义不带私有属性的系统有效条件
    
    :param func: 系统有效条件函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义系统有效条件实例

自定义系统有效条件接口：

* :py:meth:`ConditionBase._calculate` - 【必须】子类计算接口
* :py:meth:`ConditionBase._clone` - 【必须】克隆接口
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

        以指标的形式获取实际值，与交易对象等长，0表示无效，1表示系统有效

    .. py:method:: _add_valid(self, datetime)
    
        加入有效时间，在_calculate中调用
        
        :param Datetime datetime: 有效时间
      
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
