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

    .. py:method:: run(self, query[, adjust_cycle=1, force=false, adjust_mode="query", delay_to_trading_day=True])
    
        运行投资组合策略。在查询条件及各组件没有变化时，PF在第二次执行时，默认不会实际进行计算。
        但由于各个组件的参数可能改变，此种情况无法自动判断是否需要重计算，可以手工指定进行强制计算。

        调仓模式 adjust_mode 说明：

        - "query" 模式，跟随输入参数 query 中的 ktype，此时 adjust_cycle 为以 query 中的 ktype 决定周期间隔；
        - "day" 模式，adjust_cycle 为调仓间隔天数
        - "week" | "month" | "quarter" | "year" 模式时，adjust_cycle 为对应的每周第N日、每月第n日、每季度第n日、每年第n日，在 delay_to_trading_day 为 false 时, 如果当日不是交易日将会被跳过调仓；当 delay_to_trading_day 为 true时，如果当日不是交易日将会顺延至当前周期内的第一个交易日，如指定每月第1日调仓，但当月1日不是交易日，则将顺延至当月的第一个交易日。
            
        :param Query query: 查询条件
        :param int adjust_cycle: 调仓周期
        :param bool force: 强制重新计算
        :param str adjust_mode: 调仓模式，默认为"query"
        :param bool delay_to_trading_day: 是否顺延至当前周期内第一个交易日进行调仓
        
        