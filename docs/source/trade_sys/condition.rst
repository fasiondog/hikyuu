.. py:currentmodule:: hikyuu.trade_sys
.. highlightlang:: python

系统有效条件
============

内建系统有效条件
----------------

.. py:function:: CN_OPLine(op)

    固定使用股票最小交易量进行交易，计算权益曲线的op值，当权益曲线高于op时，系统有效，否则无效。

    :param Operand op: Operand实例
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
    
    .. py:method:: __init__(self[, name="ConditionBase"])
    
        初始化构造函数
        
        :param str name: 名称
        
    .. py:method:: getParam(self, name)

        获取指定的参数
    
        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数
        
    .. py:method:: setParam(self, name, value)
    
        设置参数
        
        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型
        
    .. py:method:: setTO(self, k)
    
        :param KData k: 设置交易对象
        
    .. py:method:: getTO(self)
    
        :return: 交易对象
        :rtype: KData    

    .. py:method:: setTM(self, tm)
    
        :param TradeManager tm: 设置交易管理账户
        
    .. py:method:: getTM(self)
    
        获取交易管理账户
        
        :return: 交易管理账户
        :rtype: TradeManager        
        
    .. py:method:: setSG(self, sg)

        设置交易信号指示器
        
        :param SignalBase sg:
        
    .. py:method:: getSG(self)
    
        获取信号指示器
        
        :rtype: SignalBase
    
    .. py:method:: isValid(self, datetime)
    
        指定时间系统是否有效
        
        :param Datetime datetime: 指定时间
        :return: True 有效 | False 无效
    
    .. py:method:: _addValid(self, datetime)
    
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
