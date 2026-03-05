.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

交易管理
==========

交易管理可理解为一个模拟账户进行模拟交易。一般使用 crtTM 创建交易管理实例。

公共参数：

    * **precision=2** *(int)* : 价格计算精度
    * **support_borrow_cash=False** *(bool)* : 是否自动融资
    * **support_borrow_stock=False** *(bool)* : 是否自动融券
    * **save_action=True** *(bool)* : 是否保存 Python 命令序列


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
        
        （只读）价格精度，同公共参数"precision"
        
    .. py:attribute:: broker_last_datetime
    
        实际开始订单代理操作的时刻。
        
        默认情况下，TradeManager 会在执行买入/卖出操作时，调用订单代理执行代理的买入/卖出动作，但这样在实盘操作时会存在问题。因为系统在计算信号指示时，需要回溯历史数据才能得到最新的信号，这样 TradeManager 会在历史时刻就执行买入/卖出操作，此时如果订单代理本身没有对发出买入/卖出指令的时刻进行控制，会导致代理发送错误的指令。此时，需要指定在某一个时刻之后，才允许指定订单代理的买入/卖出操作。属性 brokeLastDatetime 即用于指定该时刻。
        

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
        :type value: int | bool | float | string | Query | KData | Stock | DatetimeList
        :raises logic_error: Unsupported type! 不支持的参数类型
        
    .. py:method:: have_param(self, name)
    
        检查是否存在指定参数
        
        :param str name: 参数名称
        :rtype: bool
        
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
        
    .. py:method:: checkin_stock(self, datetime, stock, price, number)
    
        存入股票资产
    
        :param Datetime datetime: 交易时间
        :param Stock stock: 待存入的股票
        :param float price: 存入股票的每股价格
        :param float number: 存入股票的数量
        :rtype: TradeRecord
        
    .. py:method:: checkout_stock(self, datetime, stock, price, number)
    
        取出股票资产
        
        :param Datetime datetime: 交易时间
        :param Stock stock: 待取出的股票
        :param float price: 取出的每股价格
        :param float number: 取出的数量
        :rtype: TradeRecord
        
    .. py:method:: borrow_cash(self, datetime, cash)
    
        借入资金（融资）
    
        :param Datetime datetime: 交易时间
        :param float cash: 借入的现金额
        :rtype: TradeRecord
        
    .. py:method:: return_cash(self, datetime, cash)
    
        归还借入的资金
        
        :param Datetime datetime: 交易时间
        :param float cash: 归还的现金额
        :rtype: TradeRecord
        
    .. py:method:: borrow_stock(self, datetime, stock, price, number)
    
        借入股票（融券）
    
        :param Datetime datetime: 交易时间
        :param Stock stock: 借入的股票
        :param float price: 借入时的每股价格
        :param float number: 借入的数量
        :rtype: TradeRecord
        
    .. py:method:: return_stock(self, datetime, stock, price, number)
    
        归还借入的股票
        
        :param Datetime datetime: 交易时间
        :param Stock stock: 归还的股票
        :param float price: 归还时的每股价格
        :param float number: 归还的数量
        :rtype: TradeRecord
        
    .. py:method:: buy(self, datetime, stock, real_price, number[, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
        买入操作
        
        :param Datetime datetime: 买入时间
        :param Stock stock:       买入的证券
        :param float real_price:  实际买入价格
        :param float number:      买入数量
        :param float stoploss:    止损价
        :param float goal_price:  目标价格
        :param float plan_price:  计划买入价格
        :param SystemPart part:   交易指示来源
        :param str remark:        备注信息
        :rtype: TradeRecord
        
    .. py:method:: sell(self, datetime, stock, real_price[, number=constant.max_double, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
        卖出操作
        
        :param Datetime datetime: 卖出时间
        :param Stock stock:       卖出的证券
        :param float real_price:  实际卖出价格
        :param float number:      卖出数量，如果等于 constant.max_double, 表示全部卖出
        :param float stoploss:    新的止损价
        :param float goal_price:  新的目标价格
        :param float plan_price:  原计划卖出价格
        :param SystemPart part:   交易指示来源
        :param str remark:        备注信息
        :rtype: TradeRecord
        
    .. py:method:: buy_short(self, datetime, stock, real_price, number[, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
        卖空操作（先卖后买）
        
        :param Datetime datetime: 卖空时间
        :param Stock stock:       卖空的证券
        :param float real_price:  实际卖空价格
        :param float number:      卖空数量
        :param float stoploss:    止损价
        :param float goal_price:  目标价格
        :param float plan_price:  计划卖空价格
        :param SystemPart part:   交易指示来源
        :param str remark:        备注信息
        :rtype: TradeRecord
        
    .. py:method:: sell_short(self, datetime, stock, real_price[, number=constant.max_double, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
        卖空回补操作（买回平仓）
        
        :param Datetime datetime: 回补时间
        :param Stock stock:       回补的证券
        :param float real_price:  实际回补价格
        :param float number:      回补数量，如果等于 constant.max_double, 表示全部回补
        :param float stoploss:    止损价
        :param float goal_price:  目标价格
        :param float plan_price:  计划回补价格
        :param SystemPart part:   交易指示来源
        :param str remark:        备注信息
        :rtype: TradeRecord
        
    .. py:method:: have(self, stock)
    
        当前是否持有指定的证券（多头仓位）
        
        :param Stock stock: 指定证券
        :rtype: bool
        
    .. py:method:: have_short(self, stock)
    
        当前空头仓位是否持有指定的证券
        
        :param Stock stock: 指定证券
        :rtype: bool
        
    .. py:method:: cash(self, datetime[, ktype=Query.KType.DAY])
    
        获取指定日期的现金。（注：如果不带日期参数，无法根据权息信息调整持仓。）
        
        :param Datetime datetime: 指定时刻
        :param ktype: K 线类型
        :rtype: float
        
    .. py:method:: get_stock_num(self)
    
        当前持有的证券种类数量，即当前持有几只股票（非各个股票的持仓数）
        
        :rtype: int
        
    .. py:method:: get_short_stock_num(self)
    
        当前空头持有的证券种类数量
        
        :rtype: int
        
    .. py:method:: get_hold_num(self, datetime, stock)

        获取指定时刻指定证券的多头持有数量
        
        :param Datetime datetime: 指定时刻
        :param Stock stock: 指定的证券
        :rtype: float

    .. py:method:: get_short_hold_num(self, datetime, stock)

        获取指定时刻指定证券的空头持有数量
        
        :param Datetime datetime: 指定时刻
        :param Stock stock: 指定的证券
        :rtype: float
        
    .. py:method:: get_debt_number(self, datetime, stock)
    
        获取指定时刻已借入的股票数量
        
        :param Datetime datetime: 指定时刻
        :param Stock stock: 指定的证券
        :rtype: float
        
    .. py:method:: get_debt_cash(self, datetime)
    
        获取指定时刻已借入的现金额
        
        :param Datetime datetime: 指定时刻
        :rtype: float
        
    .. py:method:: get_position(self, date, stock)

        获取指定时间证券持仓记录，如当前未持有该票，返回 PositionRecord()
        
        :param Datetime date: 指定时间
        :param Stock stock: 指定的证券
        :rtype: PositionRecord
        
    .. py:method:: get_short_position(self, stock)
    
        获取指定证券的当前空头仓位持仓记录，如当前未持有该票，返回 PositionRecord()
        
        :param Stock stock: 指定的证券
        :rtype: PositionRecord
        
    .. py:method:: get_position_list(self)
    
        获取当前全部持仓记录（多头）
        
        :rtype: PositionRecordList
        
    .. py:method:: get_positions(self)
    
        以字典方式获取当前全部持仓记录，stock 为 key，PositionRecord 为 value
        
        :rtype: dict
        
    .. py:method:: get_history_position_list(self)
    
        获取全部历史持仓记录，即已平仓记录（多头）
        
        :rtype: PositionRecordList

    .. py:method:: get_short_position_list(self)
    
        获取当前全部空头仓位记录
        
        :rtype: PositionRecordList
        
    .. py:method:: get_short_history_position_list(self)
    
        获取全部空头历史持仓记录
        
        :rtype: PositionRecordList
        
    .. py:method:: get_borrow_stock_list(self)
    
        获取当前借入的股票列表
        
        :rtype: BorrowRecordList
        
    .. py:method:: get_trade_list(self[, start, end])
    
        获取交易记录，未指定参数时，获取全部交易记录
        
        :param Datetime start: 起始日期
        :param Datetime end: 结束日期
        :rtype: TradeRecordList
        
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
        
    .. py:method:: get_borrow_cash_cost(self, datetime, cash)
    
        计算借入资金的成本
        
        :param Datetime datetime: 交易时间
        :param float cash: 借入的现金额
        :rtype: CostRecord
        
    .. py:method:: get_return_cash_cost(self, datetime, cash)
    
        计算归还借入资金的成本
        
        :param Datetime datetime: 交易时间
        :param float cash: 归还的现金额
        :rtype: CostRecord
        
    .. py:method:: get_borrow_stock_cost(self, datetime, stock, price, num)
    
        计算借入股票的成本
        
        :param Datetime datetime: 交易时间
        :param Stock stock: 借入的股票
        :param float price: 借入时的每股价格
        :param float num: 借入的数量
        :rtype: CostRecord
        
    .. py:method:: get_return_stock_cost(self, datetime, stock, price, num)
    
        计算归还借入股票的成本
        
        :param Datetime datetime: 交易时间
        :param Stock stock: 归还的股票
        :param float price: 归还时的每股价格
        :param float num: 归还的数量
        :rtype: CostRecord
     
    .. py:method:: get_funds(self[, ktype = Query.DAY])
    
        获取当前时刻的资产市值详情
        
        :param Query.KType ktype: K 线类型
        :rtype: FundsRecord

    .. py:method:: get_funds(self, datetime[, ktype = Query.DAY])
    
        获取指定时刻的资产市值详情
        
        :param Datetime datetime:  指定时刻
        :param Query.KType ktype: K 线类型
        :rtype: FundsRecord

    .. py:method:: get_funds_list(self, dates[, ktype = Query.DAY])
    
        获取指定日期列表的每日资产记录
        
        :param DatetimeList dates: 日期列表
        :param Query.KType ktype: K 线类型
        :rtype: FundsList

    .. py:method:: get_funds_curve(self, dates[, ktype = Query.DAY])
    
        获取资产净值曲线
        
        :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的资产净值曲线
        :param Query.KType ktype: K 线类型，必须与日期列表匹配
        :return: 资产净值列表
        :rtype: PriceList
        
    .. py:method:: get_profit_curve(self, dates[, ktype = Query.DAY])
    
        获取收益曲线，即扣除历次存入资金后的资产净值曲线
        
        :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
        :param Query.KType ktype: K 线类型，必须与日期列表匹配
        :return: 收益曲线
        :rtype: PriceList
        
    .. py:method:: get_profit_cum_change_curve(self, dates[, ktype = Query.DAY])
    
        获取累积收益率曲线
        
        :param DatetimeList dates: 日期列表
        :param Query.KType ktype: K 线类型，必须与日期列表匹配
        :rtype: PriceList
        
    .. py:method:: get_base_assets_curve(self, dates[, ktype = Query.DAY])
    
        获取投入本值资产曲线（投入本钱）
        
        :param DatetimeList dates: 日期列表
        :param Query.KType ktype: K 线类型，必须与日期列表匹配
        :rtype: PriceList
        
    .. py:method:: add_trade_record(self, tr)

        直接加入交易记录，如果加入初始化账户记录，将清除全部已有交易及持仓记录。

        :param TradeRecord tr: 交易记录
        :return: True（成功） | False（失败）
        :rtype: bool
        
    .. py:method:: add_position(self, position)
    
        建立初始账户后，直接加入持仓记录，仅用于构建初始有持仓的账户
        
        :param PositionRecord position: 持仓记录
        :return: True | False
        :rtype: bool
        
    .. py:method:: tocsv(self, path)
    
        以 csv 格式输出交易记录、未平仓记录、已平仓记录、资产净值曲线
        
        :param str path: 输出文件所在目录
        
    .. py:method:: reg_broker(self, broker)
    
        注册订单代理。可执行多次该命令注册多个订单代理。
        
        :param OrderBrokerBase broker: 订单代理实例
        
    .. py:method:: clear_broker(self)

        清空所有已注册订单代理

    .. py:method:: get_margin_rate(self, datetime, stock)
    
        获取指定对象的保证金比率
        
        :param Datetime datetime: 日期
        :param Stock stock: 指定对象
        :rtype: float
        
    .. py:method:: update_with_weight(self, date)
    
        根据权息信息更新当前持仓及交易记录，必须按时间顺序被调用
        
        :param Datetime date: 当前时刻
        
    .. py:method:: fetch_asset_info_from_broker(self, broker[, date=Datetime.now()])
    
        从 Broker 同步当前时刻的资产信息，必须按时间顺序被调用
        
        :param OrderBrokerBase broker: 订单代理实例
        :param Datetime date: 同步时，通常为当前时间（Null)，也可以强制为指定的时间点
        
    .. py:method:: get_performance(self[, datetime=Datetime.now(), ktype=Query.DAY, ext=False]) -> Performance
        
        获取账户指定时刻的账户表现

        :param Datetime datetime: 指定时刻
        :param Query.KType ktype: K 线类型
        :param bool ext: 是否获取扩展统计项 (需 VIP 权限)，否则仅为基础统计项
        :return: 账户表现
        :rtype: Performance

    .. py:method:: get_max_pull_back(self, date, ktype=Query.DAY) -> float
    
        获取指定时刻时账户的最大回撤百分比（负数）

        :param Datetime date: 指定日期（包含该时刻）
        :param Query.KType ktype: k 线类型
        :return: 最大回撤百分比

    .. py:method:: get_position_ext_info_list(self, current_time, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo]
          
        获取账户最后交易时刻之后指定时间的持仓详情（未平常记录）
    
        :param Datetime current_time: 当前时刻（需大于等于最后交易时刻）
        :param Query.KType ktype: k 线类型
        :param int trade_mode: 交易模式，影响部分统计项：0-收盘时交易，1-下一开盘时交易
        :return: 持仓扩展详情列表

    .. py:method:: get_position_ext_info(self, current_time, ktype=Query.DAY, trade_mode=0) -> dict
    
        获取账户最后交易时刻之后指定时间的持仓详情，以字典返回，stock 为 key, PositionExtInfo 为 value
 
        :param Datetime current_time: 当前时刻（需大于等于最后交易时刻）
        :param Query.KType ktype: k 线类型
        :param int trade_mode: 交易模式，影响部分统计项：0-收盘时交易，1-下一开盘时交易
        :return: 持仓扩展详情字典

    .. py:method:: get_history_position_ext_info_list(self, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo]
          
        获取账户历史持仓扩展详情（已平仓记录）
    
        :param Query.KType ktype: k 线类型
        :param int trade_mode: 交易模式，影响部分统计项：0-收盘时交易，1-下一开盘时交易
        :return: 持仓扩展详情列表

    .. py:method:: get_profit_percent_monthly(self[, datetime=Datetime.now()]) -> list[tuple[Datetime, double]]

        获取账户指定截止时刻的账户收益百分比（月度）

        :param Datetime datetime: 指定截止时刻
        :return: 账户收益百分比（月度），元素为 (日期，收益率) 的元组列表

    .. py:method:: get_profit_percent_yearly(self[, datetime=Datetime.now()]) -> list[tuple[Datetime, double]]

        获取账户指定截止时刻的账户收益百分比（年度）

        :param Datetime datetime: 指定截止时刻
        :return: 账户收益百分比（年度），元素为 (日期，收益率) 的元组列表
