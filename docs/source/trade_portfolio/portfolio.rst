.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

投资组合
=============

目前仅实现了多标的、相同策略的投资组合，还需完善，未来接口可能变化（包括选择器策略）！

.. py:function:: PF_Simple([tm, af, se])

    创建一个多标的、单系统策略的投资组合
    
    :param TradeManager tm: 交易管理
    :param SelectorBase se: 选择器
    :param AllocateFundsBase af: 资金分配算法
    
    
投资组合类定义
----------------

.. py:class:: Portfolio

    实现多标的、多策略的投资组合
    
    .. py:attribute:: name  
    
        名称
    
    .. py:attribute:: tm  
    
        关联的交易管理实例
        
    .. py:attribute:: se
    
        选择器策略
        
    .. py:method:: run(self, query)
    
        运行投资组合策略
        
        :param Query query: 查询条件
        
        