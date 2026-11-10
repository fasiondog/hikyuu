.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

基础数据结构
============

交易业务类型
------------

.. py:function:: get_business_name(business)

    :param BUSINESS business: 交易业务类型
    :return: 交易业务类型名称("INIT"|"BUY"|"SELL"|"BUY_SHORT"|"SELL_SHORT"|"GIFT"|"BONUS"|"CHECKIN"|"CHECKOUT"|"CHECKIN_STOCK"|"CHECKOUT_STOCK"|"BORROW_CASH"|"RETURN_CASH"|"BORROW_STOCK"|"RETURN_STOCK"|"SUOGU"|"BORROW_ADJUST"|"SHORT_ADJUST"|"DIVIDEND_COMPENSATION"|"SETTLEMENT"|"INTEREST"|"UNKNOWN")
    :rtype: string

.. py:class:: BUSINESS    
    
    - BUSINESS.INIT           - 建立初始账户
    - BUSINESS.BUY            - 买入
    - BUSINESS.SELL           - 卖出
    - BUSINESS.BUY_SHORT      - 买空（归还借入股票）
    - BUSINESS.SELL_SHORT     - 卖空（借入股票卖出）
    - BUSINESS.GIFT           - 送股
    - BUSINESS.BONUS          - 分红
    - BUSINESS.CHECKIN        - 存入现金
    - BUSINESS.CHECKOUT       - 取出现金
    - BUSINESS.CHECKIN_STOCK  - 存入股票
    - BUSINESS.CHECKOUT_STOCK - 取出股票
    - BUSINESS.BORROW_CASH    - 借入资金
    - BUSINESS.RETURN_CASH    - 归还借入资金
    - BUSINESS.BORROW_STOCK   - 借入股票
    - BUSINESS.RETURN_STOCK   - 归还借入股票
    - BUSINESS.SUOGU          - 缩股
    - BUSINESS.BORROW_ADJUST  - 借入股票数量调整（除权送股/缩股）
    - BUSINESS.SHORT_ADJUST   - 卖空持仓数量调整（除权送股/缩股）
    - BUSINESS.DIVIDEND_COMPENSATION - 红利补偿（卖空向出借方支付的现金）
    - BUSINESS.SETTLEMENT     - 保证金持仓逐日结算（结算价与盯市基准之差计入盈亏），见 :doc:`margin`
    - BUSINESS.INTEREST       - 负债计提利息
    - BUSINESS.INVALID        - 无效类型




交易成本记录
------------

交易成本计算的返回结果

.. py:class:: CostRecord

    交易成本记录

    .. py:attribute:: commission  佣金(float)
    .. py:attribute:: stamptax    印花税(float)
    .. py:attribute:: transferfee 过户费(float)
    .. py:attribute:: others      其它费用(float)
    .. py:attribute:: total       总成本(float)，= 佣金 + 印花税 + 过户费 + 其它费用
        
        
交易记录
--------

.. py:class:: TradeRecordList

    交易记录列表，C++ std::vector<TradeRecord>包装
    
    .. py:method:: to_numpy()
    
        仅在安装了numpy模块时生效，转换为numpy.array
    
    .. py:method:: to_pandas()
    
        仅在安装了pandas模块时生效，转换为pandas.DataFrame

    .. py:method:: to_pyarrow()

        转换为 pyarrow.Table

.. py:class:: TradeRecord([stock, datetime, business, planPrice, realPrice, goalPrice, number, cost, stoploss, cash, part])

    交易记录
    
    .. py:attribute:: stock     股票（Stock）
    .. py:attribute:: datetime  交易时间（Datetime）
    .. py:attribute:: business  交易类型
    .. py:attribute:: plan_price 计划交易价格（float）
    .. py:attribute:: real_price 实际交易价格（float）
    .. py:attribute:: goal_price 目标价格（float），如果为0表示未限定目标
    .. py:attribute:: number    成交数量（float）
    .. py:attribute:: cost      交易成本

        类型：:py:class:`CostRecord`
        
    .. py:attribute:: stoploss 止损价（float）
    .. py:attribute:: cash     现金余额（float）
    .. py:attribute:: margin   本次开仓锁定的保证金（float）；仅在保证金记账的买入/卖空开仓时非 0，其余业务为 0，用于回放时还原当时的账面；逐日结算记录（``SETTLEMENT``）中为结算后重算的锁定额
    .. py:attribute:: part     
    
        交易指示来源，区别是交易系统哪个部件发出的指示，参见： :py:class:`System.Part`

    .. note::

        逐日结算记录（``BUSINESS.SETTLEMENT``）不是成交：``real_price`` 为结算价，``number`` 带方向
        （多头为正、空头为负，用于区分同一标的的多空持仓），``cash`` 为过账后的现金余额，``cost`` 为 0，
        ``part`` 为无效部件（``System.Part.INVALID``）。

    

持仓记录
--------

.. py:class:: PositionRecordList

    持仓记录列表，C++ std::vector<PositionRecord>包装
    
    .. py:method:: to_numpy()
    
        仅在安装了numpy模块时生效，转换为numpy.array
    
    .. py:method:: to_pandas()
    
        仅在安装了pandas模块时生效，转换为pandas.DataFrame

    .. py:method:: to_pyarrow()

        转换为 pyarrow.Table
        

.. py:class:: PositionRecord([stock, take_datetime, clean_datetime, number, stoploss, goal_price, total_number, buy_money, total_cost, total_risk, sell_money])

    持仓记录
    
    .. py:attribute:: stock          交易对象（Stock）
    .. py:attribute:: take_datetime  初次建仓时刻（Datetime）
    .. py:attribute:: clean_datetime 平仓日期，当前持仓记录中为 constant.null_datetime
    .. py:attribute:: number       当前持仓数量（float）
    .. py:attribute:: stoploss     当前止损价（float）
    .. py:attribute:: goal_price   当前的目标价格（float）
    .. py:attribute:: total_number 累计持仓数量（float）
    .. py:attribute:: buy_money    累计买入资金（float）
    .. py:attribute:: total_cost   累计交易总成本（float）
    .. py:attribute:: total_risk   累计交易风险（float） = 各次 （买入价格-止损)*买入数量, 不包含交易成本
    .. py:attribute:: sell_money   累计卖出资金（float）
    .. py:attribute:: margin       当前锁定的保证金（float），0 表示现金记账（见 :doc:`margin`）
    .. py:attribute:: open_basis   仍持仓批次的开仓名义金额（float）= 开仓价 × 数量 × unit，平仓时按数量比例释放，仅作成本参考
    .. py:attribute:: settle_basis 仍持仓批次的盯市基准（float）；逐日结算时滚动到结算价，首次结算前等于 open_basis，是已实现盈亏与保证金持仓权益的计算基准

    
资产情况记录
------------

由TradeManager::getFunds返回

.. py:class:: FundsRecord([cash, market_value, short_market_value, base_cash, base_asset, borrow_cash, borrow_asset])

    当前资产情况记录，由 :py:meth:`TradeManager.getFunds` 返回
    
    .. py:attribute:: cash               当前现金（float）
    .. py:attribute:: market_value       当前多头市值（float）
    .. py:attribute:: short_market_value 当前空头仓位市值（float）
    .. py:attribute:: base_cash          当前投入本金（float）
    .. py:attribute:: base_asset         当前投入的资产价值（float）
    .. py:attribute:: borrow_cash        当前借入的资金（float），即负债
    .. py:attribute:: borrow_asset       当前借入证券资产价值（float）
    .. py:attribute:: occupied_margin    保证金持仓锁定的保证金合计（float），其权益贡献已计入 market_value

    只读属性，自动根据上面的属性计算得到的结果:

    .. py:attribute:: total_assets  总资产
    .. py:attribute:: net_assets  净资产
    .. py:attribute:: total_borrow  总负债
    .. py:attribute:: total_base  投入本值资产（本钱）
    .. py:attribute:: profit  收益

