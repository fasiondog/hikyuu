.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _portfolio:

投资组合
=============

在 Hikyuu 中 Portfolio 投资组合是以系统交易策略为单位的，Hikyuu 即可以使用不同目标同一交易策略的组合，也可以使用不同目标不同交易逻辑的系统策略进行组合，和常见程序化交易中的多目标但策略逻辑相同的投资组合并不完全相同。

.. py:function:: PF_Simple([tm, af, se])

    创建一个多标的、单系统策略的投资组合
    
    :param TradeManager tm: 交易管理
    :param SelectorBase se: 选择器
    :param AllocateFundsBase af: 资金分配算法
    
    
投资组合类定义
----------------

.. py:class:: Portfolio

    实现多标的、多策略的投资组合
    
    .. py:attribute:: name  名称
    
    .. py:attribute:: query 运行条件

    .. py:attribute:: tm 关联的交易管理实例
        
    .. py:attribute:: se 选择器策略
        
    .. py:attribute:: af 资产分配算法

    .. py:attribute:: proto_sys_list 原型系统列表

    .. py:attribute:: real_sys_list 运行时的实际系统列表

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

    .. py:method:: run(self, query)
    
        运行投资组合策略
        
        :param Query query: 查询条件
        
        