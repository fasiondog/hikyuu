.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _portfolio:

投资组合
=============

在 Hikyuu 中 Portfolio 投资组合是以系统交易策略为单位的，Hikyuu 即可以使用不同目标同一交易策略的组合，未来也可以使用不同目标不同交易逻辑的系统策略进行组合（需要 tm 和各组件支持），和常见程序化交易中的多目标但策略逻辑相同的投资组合并不完全相同。

PF部件说明:

.. raw:: html

    <table border="1">
        <thead>
            <tr>
                <th>部件命名规范</th>
                <th>部件说明</th>
                <th>部件用途</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td>MF_Xxx</td>
                <td>多因子合成（时间截面评分板）</td>
                <td>多因子本质是在时间截面上对候选标的进行评分，所以实际需要配合 Selector (策略选择算法) 使用。</td>
            </tr>
            <tr>
                <td>SE_Xxx</td>
                <td>系统选择算法</td>
                <td>实现标的、系统策略的评估和选取算法。<br>注意：滚动寻优系统中的寻优选择器同样以 SE 为前缀命名，但不能再这里。</td>
            </tr>
            <tr>
                <td>AF_Xxx</td>
                <td>资产分配算法</td>
                <td>用于对时间截面上选中的系统进行资产分配。</td>
            </tr>            
        </tbody>
    </table>
    <p></p>


.. py:function:: PF_Simple([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True])

    创建一个多标的、单系统策略的投资组合

    调仓模式 adjust_mode 说明：
    - "query" 模式，跟随输入参数 query 中的 ktype，此时 adjust_cycle 为以 query 中的 ktype决定周期间隔；
    - "day" 模式，adjust_cycle 为调仓间隔天数
    - "week" | "month" | "quarter" | "year" 模式时，adjust_cycle

      为对应的每周第N日、每月第n日、每季度第n日、每年第n日，在 delay_to_trading_day 为 false 时
      如果当日不是交易日将会被跳过调仓；当 delay_to_trading_day 为 true时，如果当日不是交易日
      将会顺延至当前周期内的第一个交易日，如指定每月第1日调仓，但当月1日不是交易日，则将顺延至当月
      的第一个交易日。    

    :param TradeManager tm: 交易管理
    :param SelectorBase se: 交易对象选择算法
    :param AllocateFundsBase af: 资金分配算法
    :param int adjust_cycle: 调仓周期
    :param str adjust_mode: 调仓模式
    :param bool delay_to_trading_day: 如果当日不是交易日将会被顺延至当前周期内的第一个交易日


.. py:function:: PF_WithoutAF([tm, se, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sys_use_self_tm=False,sell_at_not_selected=False])
    
    创建无资金分配算法的投资组合，所有单系统策略使用共同的 tm 管理账户

    调仓模式 adjust_mode 说明：
    - "query" 模式，跟随输入参数 query 中的 ktype，此时 adjust_cycle 为以 query 中的 ktype 决定周期间隔；
    - "day" 模式，adjust_cycle 为调仓间隔天数
    - "week" | "month" | "quarter" | "year" 模式时，adjust_cycle
    
      为对应的每周第N日、每月第n日、每季度第n日、每年第n日，在 delay_to_trading_day 为 false 时
      如果当日不是交易日将会被跳过调仓；当 delay_to_trading_day 为 true时，如果当日不是交易日
      将会顺延至当前周期内的第一个交易日，如指定每月第1日调仓，但当月1日不是交易日，则将顺延至当月
      的第一个交易日。    

    :param TradeManager tm: 交易管理
    :param SelectorBase se: 交易对象选择算法
    :param int adjust_cycle: 调仓周期
    :param str adjust_mode: 调仓模式
    :param bool delay_to_trading_day: 如果当日不是交易日将会被顺延至当前周期内的第一个交易日
    :param bool trade_on_close: 交易是否在收盘时进行
    :param bool sys_use_self_tm: 原型系统使用自身附带的tm进行计算
    :param bool sell_at_not_selected: 调仓日未选中的股票是否强制卖出
    
    
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
        
        