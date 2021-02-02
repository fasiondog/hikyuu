.. py:currentmodule:: hikyuu.trade_sys
.. highlightlang:: python

选择器策略
=============

实现标的、系统策略的评估和选取算法。

公共参数：

    * **freq** *(int | 1)* ：变化频度，选股的变化周期，以Bar为单位

内建选择器
-----------

.. py:function:: SE_Fixed([stocklist, sys])

    固定选择器，即始终选择初始划定的标的及其系统策略原型
    
    :param StockList stocklist: 初始划定的标的
    :param System sys: 系统策略原型
    :return: SE选择器实例

自定义选择器策略
--------------------

自定义选择器策略接口：

* :py:meth:`SelectorBase.get_selected_system_list` - 【必须】获取指定时刻选择的系统实例列表
* :py:meth:`SelectorBase._reset` - 【可选】重置私有属性
* :py:meth:`SelectorBase._clone` - 【必须】克隆接口

选择器策略基类
----------------

.. py:class:: SelectorBase

    选择器策略基类，实现标的、系统策略的评估和选取算法
    
    .. py:attribute:: name 名称
    
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

    .. py:method:: add_stock(self, stock, sys)

        加入初始标的及其对应的系统策略原型
        
        :param Stock stock: 加入的初始标的
        :param System sys: 系统策略原型

    .. py:method:: add_stock_list(self, stk_list, sys)
    
        加入初始标的列表及其系统策略原型
        
        :param StockList stk_list: 加入的初始标的列表
        :param System sys: 系统策略原型
    
    .. py:method:: clear(self)
    
        清除已加入的系统策略实例
    
    .. py:method:: get_selected_system_list(self, datetime)
    
        【重载接口】获取指定时刻选取的系统实例
        
        :param Datetime datetime: 指定时刻
        :return: 选取的系统实例列表
        :rtype: SystemList
        
     .. py:method:: _reset(self)
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口       
    
    
        