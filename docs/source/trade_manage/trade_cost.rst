.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

交易成本算法
============

内建交易成本算法
----------------

零交易成本算法
^^^^^^^^^^^^^^

.. py:function:: TC_Zero()

    :return: :py:class:`TradeCostBase` 子类实例


沪深A股交易成本算法
^^^^^^^^^^^^^^^^^^^

* 2015年8月1日之前，上证过户费为交易数量的千分之一，不足1元，按1元计。
* 2015年8月1日之后，上证过户费为成交金额的千分之0.02

计算规则如下:: python

    1）上证交易所
        买入：佣金＋过户费
        卖出：佣金＋过户费＋印花税
    2）深证交易所：
        买入：佣金
        卖出：佣金＋印花税

    其中，佣金最低5元

.. py:function:: TC_FixedA2015([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.00002])

    2015年8月1日及之后的A股交易成本算法，上证过户费改为成交金额的千分之0.02

    :param float commission: 佣金比例
    :param float lowestCommission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :return: :py:class:`TradeCostBase` 子类实例
    
.. py:function:: TC_FixedA([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.001, lowestTransferfee=1.0])

    2015年8月1日之前的A股交易成本算法

    :param float commission: 佣金比例
    :param float lowestCommission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :param float lowestTransferfee: 最低过户费
    :return: :py:class:`TradeCostBase` 子类实例


    
自定义交易成本算法
------------------

自定义交易成本算法接口：

* :py:meth:`TradeCostBase.getBuyCost` - 【必须】获取买入成本
* :py:meth:`TradeCostBase.getSellCost` - 【必须】获取卖出成本
* :py:meth:`TradeCostBase._clone` - 【必须】子类克隆接口



交易算法成本基类
----------------

.. py:class:: TradeCostBase(name)

    交易成本算法基类
    
    .. py:attribute:: name 名称
        
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

    .. py:method:: clone(self)
    
        克隆操作

    .. py:method:: get_buy_cost(self, datetime, stock, price, num)
    
        【重载接口】获取买入成本
        
        :param Datetime datetime: 买入时刻
        :param Stock stock: 买入对象
        :param float price: 买入价格
        :param int num: 买入数量
        :return: 交易成本记录
        :rtype: CostRecord
    
    .. py:method:: get_sell_cost(self, datetime, stock, price, num)
    
        【重载接口】获取卖出成本
        
        :param Datetime datetime: 卖出时刻
        :param Stock stock: 卖出对象
        :param float price: 卖出价格
        :param int num: 卖出数量
        :return: 交易成本记录
        :rtype: CostRecord
        
    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口        