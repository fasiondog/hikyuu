.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

交易管理
==========

交易管理可理解为一个模拟账户进行模拟交易。一般使用 crtTM 创建交易管理实例。

公共参数：

    * **precision=2** *(int)* : 价格计算精度
    * **support_borrow_cash=False** *(bool)* : 是否自动融资
    * **support_borrow_stock=False** *(bool)* : 是否自动融券
    * **save_action=True** *(bool)* : 是否保存Python命令序列


.. py:function:: crtTM([date = Datetime(199001010000), init_cash = 100000, cost_func = TC_Zero(), name = "SYS"])

    创建交易管理模块，管理帐户的交易记录及资金使用情况
    
    :param Datetime date:  账户建立日期
    :param float init_cash:    初始资金
    :param TradeCost cost_func: 交易成本算法
    :param string name:        账户名称
    :rtype: TradeManager
    
    
.. py:class:: TradeManager

    交易管理类，可理解为一个模拟账户进行模拟交易。一般使用 crtTM 创建交易管理实例。

    .. py:attribute:: name
        
        名称
        
    .. py:attribute:: cost_func 
        
        交易成本算法
        
    .. py:attribute:: init_cash
        
        （只读）初始资金
        
    .. py:attribute:: current_cash
    
        （只读）当前资金
        
    .. py:attribute:: init_datetime
        
        （只读）账户建立日期
        
    .. py:attribute:: first_datetime 
        
        （只读）第一笔买入交易发生日期，如未发生交易返回 Datetime()
        
    .. py:attribute:: last_datetime
        
        （只读）最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期
        
    .. py:attribute:: precision 
        
        （只读）价格精度，同公共参数“precision”
        
    .. py:attribute:: broker_last_datetime
    
        实际开始订单代理操作的时刻。
        
        默认情况下，TradeManager会在执行买入/卖出操作时，调用订单代理执行代理的买入/卖出动作，但这样在实盘操作时会存在问题。因为系统在计算信号指示时，需要回溯历史数据才能得到最新的信号，这样TradeManager会在历史时刻就执行买入/卖出操作，此时如果订单代理本身没有对发出买入/卖出指令的时刻进行控制，会导致代理发送错误的指令。此时，需要指定在某一个时刻之后，才允许指定订单代理的买入/卖出操作。属性 brokeLastDatetime 即用于指定该时刻。
        

    .. py:method:: __init__()
    
        初始化构造函数
        

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
    
        复位，清空交易、持仓记录
        
    .. py:method:: clone(self)

        克隆（深复制）实例
        
        :rtype: TradeManager
        
    .. py:method:: checkin(self, datetime, cash)
    
        向账户内存入现金
    
        :param Datetime datetime: 交易时间
        :param float cash: 存入的现金量
        :rtype: TradeRecord
        
    .. py:method:: checkout(self, datetime, cash)
    
        从账户内取出现金
        
        :param Datetime datetime: 交易时间
        :param float cash: 取出的资金量
        :rtype: TradeRecord
        
    .. py:method:: buy(self, datetime, stock, real_price, number[, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID])
    
        买入操作
        
        :param Datetime datetime: 买入时间
        :param Stock stock:       买入的证券
        :param float real_price:  实际买入价格
        :param float num:         买入数量
        :param float stoploss:    止损价
        :param float goal_price:  目标价格
        :param float plan_price:  计划买入价格
        :param SystemPart part:   交易指示来源
        :rtype: TradeRecord
        
    .. py:method:: sell(self, datetime, stock, real_price[, number=constant.max_double, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID])
    
        卖出操作
        
        :param Datetime datetime: 卖出时间
        :param Stock stock:       卖出的证券
        :param float real_price:  实际卖出价格
        :param float num:         卖出数量，如果等于constant.max_double, 表示全部卖出
        :param float stoploss:    新的止损价
        :param float goal_price:  新的目标价格
        :param float plan_price:  原计划卖出价格
        :param SystemPart part:   交易指示来源
        :rtype: TradeRecord
        
    .. py:method:: have(self, stock)
    
        当前是否持有指定的证券
        
        :param Stock stock: 指定证券
        :rtype: bool
        
    .. py:method:: cash(self, datetime[, ktype=Query.KType.DAY])
    
        获取指定日期的现金。（注：如果不带日期参数，无法根据权息信息调整持仓。）
        
        :param Datetime datetime: 指定时刻
        :param ktype: K线类型
        :rtype: float
        
    .. py:method:: get_stock_num(self)
    
        当前持有的证券种类数量，即当前持有几只股票（非各个股票的持仓数）
        
        :rtype: int
        
    .. py:method:: get_hold_num(self, datetime, stock)

        获取指定时刻指定证券的持有数量
        
        :param Datetime datetime: 指定时刻
        :param Stock stock: 指定的证券
        :rtype: int

    .. py:method:: get_position(self, date, stock)

        获取指定时间证券持仓记录，如当前未持有该票，返回PositionRecord()
        
        :param Datetime date: 指定时间
        :param Stock stock: 指定的证券
        :rtype: PositionRecord
        
    .. py:method:: get_trade_list(self[, start, end])
    
        获取交易记录，未指定参数时，获取全部交易记录
        
        :param Datetime start: 起始日期
        :param Datetime end: 结束日期
        :rtype: TradeRecordList
        
    .. py:method:: get_position_list(self)
    
        获取当前全部持仓记录
        
        :rtype: PositionRecordList
        
    .. py:method:: get_history_position_list(self)
    
        获取全部历史持仓记录，即已平仓记录
        
        :rtype: PositionRecordList

    .. py:method:: get_buy_cost(self, datetime, stock, price, num)
    
        计算买入成本
        
        :param Datetime datetime: 交易时间
        :param Stock stock:       交易的证券
        :param float price:       买入价格
        :param float num:         买入数量
        :rtype: CostRecord
        
    .. py:method:: get_sell_cost(self, datetime, stock, price, num)
    
        计算卖出成本

        :param Datetime datetime: 交易时间
        :param Stock stock:       交易的证券
        :param float price:       卖出价格
        :param float num:         卖出数量
        :rtype: CostRecord        
     
    .. py:method:: get_funds(self, datetime, [ktype = Query.DAY])
    
        获取指定时刻的资产市值详情
        
        :param Datetime datetime:  指定时刻
        :param Query.KType ktype: K线类型
        :rtype: FundsRecord

    .. py:method:: get_funds_list(self, datetime, [ktype = Query.DAY])
    
        获取指定日期列表每日资产记录
        
        :param Datetime datetime:  指定时刻
        :param Query.KType ktype: K线类型
        :rtype: FundsList

    .. py:method:: get_funds_curve(self, dates[, ktype = Query.DAY])
    
        获取资产净值曲线
        
        :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的资产净值曲线
        :param Query.KType ktype: K线类型，必须与日期列表匹配
        :return: 资产净值列表
        :rtype: PriceList
        
    .. py:method:: get_profit_curve(self, dates[, ktype = Query.DAY])
    
        获取收益曲线，即扣除历次存入资金后的资产净值曲线
        
        :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
        :param Query.KType ktype: K线类型，必须与日期列表匹配
        :return: 收益曲线
        :rtype: PriceList
        
    .. py:method:: add_trade_record(self, tr)

        直接加入交易记录，如果加入初始化账户记录，将清除全部已有交易及持仓记录。

        :param TradeRecord tr: 交易记录
        :return: True（成功） | False（失败）
        :rtype: bool
        
    .. py:method:: tocsv(self, path)
    
        以csv格式输出交易记录、未平仓记录、已平仓记录、资产净值曲线
        
        :param str path: 输出文件所在目录
        
    .. py:method:: reg_broker(self, broker)
    
        注册订单代理。可执行多次该命令注册多个订单代理。
        
        :param OrderBrokerBase broker: 订单代理实例
        
    .. py:method:: clear_broker(self)

        清空所有已注册订单代理
        