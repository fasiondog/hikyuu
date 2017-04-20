.. currentmodule:: hikyuu.trade_manage
.. highlightlang:: python

交易管理类
==========

交易管理可理解为一个模拟账户进行模拟交易。一般使用 crtTM 创建交易管理实例。

.. py:function:: crtTM([datetime = Datetime(199001010000), initcash = 100000, costfunc = TC_Zero(), name = "SYS"])

    创建交易管理实例，也可以理解为创建一个模拟账户管理交易

    :param Datetime datetime:  账户建立日期
    :param float initcash:    初始资金
    :param TradeCost costfunc: 交易成本算法
    :param string name:        账户名称
    :rtype: TradeManager
    
    
.. py:class:: TradeManager(datetime, initcash, costfunc, name)

    .. py:attribute:: name
        
        名称
        
    .. py:attribute:: costFunc 
        
        交易成本算法
        
    .. py:attribute:: initCash
        
        （只读）初始资金
        
    .. py:attribute:: initDatetime
        
        （只读）账户建立日期
        
    .. py:attribute:: firstDatetime 
        
        （只读）第一笔买入交易发生日期，如未发生交易返回Null<Datetime>()
        
    .. py:attribute:: lastDatetime
        
        （只读）最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期
        
    .. py:attribute:: reinvest 
        
        （只读）红利/股息/送股再投资标志
        
    .. py:attribute:: precision 
        
        （只读）
        
    .. py:method:: checkin(datetime, cash)
    
        向账户内存入现金
    
        :param Datetime datetime: 交易时间
        :param float cash: 存入的现金量
        :rtype: TradeRecord
        
    .. py:method:: checkout(datetime, cash)
    
        从账户内取出现金
        
        :param Datetime datetime: 交易时间
        :param float cash: 取出的资金量
        :rtype: TradeRecord
        
    .. py:method:: buy(datetime, stock, realPrice, number[, stoploss = 0.0, goalPrice = 0.0, planPrice = 0.0, part = System.INVALID])
    
        买入操作
        
        :param Datetime datetime: 买入时间
        :param Stock stock:       买入的证券
        :param float realPrice:  实际买入价格
        :param int num:           买入数量
        :param float stoploss:   止损价
        :param float goalPrice:  目标价格
        :param float planPrice:  计划买入价格
        :param SystemPart part:   交易指示来源
        :rtype: TradeRecord
        
    .. py:method:: sell(datetime, stock, realPrice[, number = Constant.null_size, stoploss = 0.0, goalPrice = 0.0, planPrice = 0.0, part = System.INVALID])
    
        卖出操作
        
        :param Datetime datetime: 卖出时间
        :param Stock stock:       卖出的证券
        :param float realPrice:  实际卖出价格
        :param int num:           卖出数量，如果等于Constant.null_size，表示全部卖出
        :param float stoploss:   新的止损价
        :param float goalPrice:  新的目标价格
        :param float planPrice:  原计划卖出价格
        :param SystemPart part:   交易指示来源
        :rtype: TradeRecord
        
    .. py:method:: have(stock)
    
        当前是否持有指定的证券
        
        :param Stock stock: 指定证券
        :rtype: bool
        
    .. py:method:: getStockNumber()
    
        当前持有的证券种类数量，即当前持有几只股票（非各个股票的持仓数）
        
        :rtype: int
        
    .. py:method:: getHoldNumber(datetime, stock)

        获取指定时刻指定证券的持有数量
        
        :param Datetime datetime: 指定时刻
        :param Stock stock: 指定的证券
        :rtype: int

    .. py:method:: getPosition(stock)

        获取指定证券的当前持仓记录，如当前未持有该票，返回PositionRecord()
        
        :param Stock stock: 指定的证券
        :rtype: PositionRecord
        
    .. py:method:: getTradeList()
    
        获取全部交易记录
        
        :rtype: TradeRecordList
        
    .. py:method:: getPositionList()
    
        获取当前全部持仓记录
        
        :rtype: PositionRecordList
        
    .. py:method:: getHistoryPositionList()
    
        获取全部历史持仓记录，即已平仓记录
        
        :rtype: PositionRecordList

    .. py:method:: getBuyCost(datetime, stock, price, num)
    
        计算买入成本
        
        :param Datetime datetime: 交易时间
        :param Stock stock:       交易的证券
        :param float price:      买入价格
        :param int num:           买入数量
        :rtype: CostRecord
        
    .. py:method:: getSellCost(datetime, stock, price, num)
    
        计算卖出成本

        :param Datetime datetime: 交易时间
        :param Stock stock:       交易的证券
        :param float price:      卖出价格
        :param int num:           卖出数量
        :rtype: CostRecord        
     
    .. py:method:: getFunds([ktype = KQuery.DAY])
    
        获取账户当前时刻的资产详情
        
        :param KQuery.KType ktype: K线类型
        :rtype: FundsRecord
        
    .. py:method:: getFunds(datetime, [ktype = KQuery.DAY])
    
        获取指定时刻的资产市值详情
        
        :param Datetime datetime:  指定时刻
        :param KQuery.KType ktype: K线类型
        :rtype: FundsRecord
        
    .. py:method:: getFundsCurve(dates[, ktype = KQuery.DAY])
    
        获取资产净值曲线
        
        :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的资产净值曲线
        :param KQuery.KType ktype: K线类型，必须与日期列表匹配
        :return: 资产净值列表
        :rtype: PriceList
        
    .. py:method:: getProfitCurve(dates[, ktype = KQuery.DAY])
    
        获取收益曲线，即扣除历次存入资金后的资产净值曲线
        
        :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
        :param KQuery.KType ktype: K线类型，必须与日期列表匹配
        :return: 收益曲线
        :rtype: PriceList
        
    .. py:method:: tocsv(path)
    
        以csv格式输出交易记录、未平仓记录、已平仓记录、资产净值曲线
        
        :param string path: 输出文件所在目录