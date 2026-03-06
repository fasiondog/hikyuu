.. currentmodule:: hikyuu
.. highlight:: python

实盘交易
=======================

程序化交易也就是自动化交易，也就是大家常见的各种量化框架，本质就是任务的定时调度 + 通知回调。

Hikyuu 主要聚焦于快速策略分析，本身不提供实盘交易，Strategy 运行时仅供大家学习参考如何和实盘进行对接，造成盈亏请自行负责。

具体可参见安装目录下的 strategy 子目录下的相关 demo。


公共参数：

    * **spot_worker_num=1** *(int)* : 接收行情数据时内部的线程数
    * **quotation_server=""** *(string)* : 指定行情服务地址，为空表示使用本机默认配置（hikyuu.ini）


.. py:class:: Strategy

    策略运行时
    
    创建策略运行时有以下几种方式：
    
    .. code-block:: python
    
        # 方式 1：使用默认参数创建
        stg = Strategy()
        
        # 方式 2：指定证券代码列表和 K 线类型
        stg = Strategy(
            code_list=["sz000001", "sz000002"],
            ktype_list=["day", "min"],
            preload_num={"day_max": 1000, "min_max": 2000},
            name="MyStrategy",
            config=""  # 配置文件路径，为空时使用默认的 hikyuu 配置文件
        )
        
        # 方式 3：使用上下文创建
        context = StrategyContext(stock_list=["sz000001"], ktype_list=["day"])
        stg = Strategy(context, name="MyStrategy", config="")

    .. py:attribute:: name
        
        策略名称，可读可写
        
    .. py:attribute:: context
        
        策略上下文，只读属性，包含证券代码列表、K 线类型等信息
        
    .. py:attribute:: tm
        
        关联的交易管理实例，可读可写，用于管理账户资产和订单
        
    .. py:attribute:: sp
        
        移滑价差算法，可读可写，仅在回测状态下使用
        
    .. py:attribute:: running
        
        获取当前运行状态，只读属性，返回 True 或 False
        
    .. py:attribute:: is_backtesting
        
        获取回测状态，只读属性

    .. py:method:: start(self, auto_recieve_spot=True)

        启动策略执行，请在完成相关回调设置后执行。
        
        注意：在 Python 交互模式下无法启动策略。

        :param bool auto_recieve_spot: 是否自动接收行情数据，默认为 True

    .. py:method:: on_change(self, func)

        设置证券数据更新回调通知
        
        当行情数据发生变化时触发该回调，通常用于调试。只要收到行情采集消息就会触发，不受开、闭市时间限制。

        :param func: 可调用对象，需接收三个参数：func(stg: Strategy, stock: Stock, spot: SpotRecord)

    .. py:method:: on_received_spot(self, func)

        设置证券数据更新通知回调
        
        在一批行情数据接受完毕后通知，通常仅用于调试打印。该批行情数据中不一定含有上下文中包含的 stock，
        且只要收到行情采集消息就会触发，不受开、闭市时间限制。

        :param func: 可调用对象，需接收两个参数：func(stg: Strategy, revTime: Datetime)

    .. py:method:: run_daily(self, func, time, market="SH", ignore_market=False)
        
        设置日内循环执行回调。如果忽略市场开闭市，则自启动时刻开始按间隔时间循环，
        否则第一次执行时将开盘时间对齐时间间隔，且在非开市时间停止执行。

        :param func: 可调用对象，需接收一个参数：func(stg: Strategy)
        :param TimeDelta time: 间隔时间，如间隔 3 秒：TimeDelta(0, 0, 0, 3) 或 Seconds(3)
        :param str market: 使用哪个市场的开闭市时间，默认为"SH"
        :param ignore_market: 忽略市场开闭市时间，默认为 False

    .. py:method:: run_daily_at(self, func, time, ignore_holiday=True)

        设置每日定点执行回调

        :param func: 可调用对象，需接收一个参数：func(stg: Strategy)
        :param TimeDelta time: 执行时刻，如每日 15 点：TimeDelta(0, 15)，必须小于 1 天
        :param ignore_holiday: 节假日不执行，默认为 True
       
        .. note:: 同一时刻只能注册一个任务，重复注册将抛出异常

    .. py:method:: today(self)

        获取当前交易日日期（使用该方法而不是 Datetime.today(), 以便回测和实盘一致）

        :return: 当前交易日日期
        :rtype: Datetime

    .. py:method:: now(self)   

        获取当前时间（使用该方法而不是 Datetime.now(), 以便回测和实盘一致）

        :return: 当前时间
        :rtype: Datetime

    .. py:method:: next_datetime(self)

        下一交易时间点（回测使用）
        
        :return: 下一交易时间点，实盘时返回 Null<Datetime>()
        :rtype: Datetime

    .. py:method:: get_current_price(self, stk, ktype)
    
        获取当前价格
        
        :param Stock stk: 指定的证券
        :param KQuery.KType ktype: K 线类型
        :return: 当前价格，无效时返回 constant.null_price
        :rtype: price_t

    .. py:method:: get_last_kdata(self, stk, start_date, ktype, recover_type)

        方法1：获取指定证券从指定日期开始到当前时间的对应 K 线数据 (为保证实盘和回测一致，请使用本方法获取 K 线数据)
        get_last_kdata(self, stk, start_date, ktype, recover_type)

        方法2：获取指定证券当前能获取到的最后 last_num 条 K 线数据 (为保证实盘和回测一致，请使用本方法获取 K 线数据)
        get_last_kdata(self, stk, lastnum, ktype, recover_type)

        :param Stock stk: 指定的证券
        :param int lastnum: 最后 N 条数据
        :param Datetime start_date: 开始日期
        :param KQuery.KType ktype: K 线类型
        :param KQuery.RecoverType recover_type: 恢复方式，默认为 KQuery.NO_RECOVER
        :return: K 线数据
        :rtype: KData


    .. py:method:: get_kdata(self, stk, start_date, end_date, ktype, recover_type)

        获取指定证券指定日期范围内的 K 线数据 (为保证实盘和回测一致，请使用本方法获取 K 线数据)

        :param Stock stk: 指定的证券
        :param Datetime start_date: 开始日期
        :param Datetime end_date: 结束日期，如为 Null 或大于当前时间，则自动使用 nextDatetime()
        :param KQuery.KType ktype: K 线类型
        :param KQuery.RecoverType recover_type: 恢复方式，默认为 KQuery.NO_RECOVER
        :return: K 线数据
        :rtype: KData

    .. py:method:: order(self, stock, num, remark='')

        按数量下单（正数为买入，负数为卖出）
        
        实际交易数量会受到证券的最小/最大交易数量限制：
        
        - 买入时，如果下单数量超过最大交易数量，则按最大交易数量成交
        - 卖出时，如果下单数量超过最大交易数量且不等于 MAX_DOUBLE，则按最大交易数量成交
        - 卖出时，如果下单数量小于最小交易数量，则全部卖出

        :param Stock stock: 指定的证券
        :param float num: 下单数量
        :param str remark: 下单备注
        :return: 交易记录
        :rtype: TradeRecord

    .. py:method:: order_value(self, stock, value, remark='')

        按预期的证劵市值下单，即希望买入多少钱的证券（正数为买入，负数为卖出）
        
        该方法会根据当前价格计算需要买入的数量，并考虑手续费等因素，确保不会超出可用资金。
        如果资金不足，会自动减少买入数量，如果连最小交易数量都无法买入，则不会下单。

        :param Stock stock: 指定的证券
        :param float value: 投入买入资金
        :param str remark: 下单备注
        :return: 交易记录
        :rtype: TradeRecord
        
    .. py:method:: buy(self, stock, price, num, stoploss=0.0, goal_price=0.0, part=SystemPart.PART_SIGNAL, remark='')
    
        买入操作
        
        :param Stock stock: 指定的证券
        :param price_t price: 买入价格，0 表示使用当前市场价格
        :param float num: 买入数量
        :param float stoploss: 止损价，默认为 0
        :param float goal_price: 目标价，默认为 0
        :param SystemPart part: 系统部分，默认为 PART_SIGNAL
        :param str remark: 备注信息
        :return: 交易记录
        :rtype: TradeRecord
        
    .. py:method:: sell(self, stock, price, num, stoploss=0.0, goal_price=0.0, part=SystemPart.PART_SIGNAL, remark='')
    
        卖出操作
        
        :param Stock stock: 指定的证券
        :param price_t price: 卖出价格，0 表示使用当前市场价格
        :param float num: 卖出数量
        :param price_t stoploss: 止损价，默认为 0
        :param price_t goal_price: 目标价，默认为 0
        :param SystemPart part: 系统部分，默认为 PART_SIGNAL
        :param str remark: 备注信息
        :return: 交易记录
        :rtype: TradeRecord


.. py:function:: start_spot_agent(print=False, worker_num=1, addr="")
    
    启动行情数据接收代理
    
    如果之前已经处于运行状态，将抛出异常。

    :param bool print: 是否打印日志，默认为 False
    :param int worker_num: 工作线程数，默认为 1
    :param str addr: 行情采集服务地址，为空表示使用 hikyuu 配置文件中的行情服务器地址

.. py:function:: stop_spot_agent()

    停止行情数据接收代理

.. py:function:: spot_agent_is_running()

    判断行情数据接收代理是否在运行

    :return: True 或 False

.. py:function:: spot_agent_is_connected()

    判断行情数据接收代理是否已连接

    :return: True 或 False


.. py:function:: crtBrokerTM(broker, cost_func=TC_Zero(), name="SYS", other_brokers=[])

    创建券商交易管理器
    
    :param broker: 券商实例
    :param TradeCost cost_func: 交易成本函数，默认为 TC_Zero()
    :param str name: 名称，默认为"SYS"
    :param list other_brokers: 其他订单代理列表，默认为空
    :return: 交易管理器实例
    :rtype: TradeManagerPtr


.. py:function:: run_in_strategy(sys, stock, query, broker, costfunc, other_brokers=[])
          
    方式1：在策略运行时执行系统交易 SYS

    run_in_strategy(sys, stock, query, broker, costfunc, other_brokers=[])   
    目前仅支持 buy_delay|sell_delay 均为 false 的系统，即 close 时执行交易
     
    :param sys: 交易系统
    :param stock: 交易对象
    :param query: 查询条件
    :param broker: 订单代理（专用与和账户资产同步的订单代理）
    :param costfunc: 成本函数
    :param list other_brokers: 其他的订单代理，默认为空列表

    方式2: 在策略运行时执行组合策略 PF
    
    目前仅支持 buy_delay|sell_delay 均为 false 的系统，即 close 时执行交易

    :param Portfolio pf: 资产组合
    :param Query query: 查询条件
    :param broker: 订单代理（专用与和账户资产同步的订单代理）
    :param costfunc: 成本函数
    :param list other_brokers: 其他的订单代理，默认为空列表


.. py:function:: crt_sys_strategy(sys, stk_market_code, query, broker, cost_func, other_brokers=[], name="SYSStrategy", config="")

    创建系统策略
    
    :param sys: 交易系统
    :param str stk_market_code: 证券市场代码
    :param query: 查询条件
    :param broker: 订单代理
    :param cost_func: 成本函数
    :param list other_brokers: 其他订单代理，默认为空列表
    :param str name: 策略名称，默认为"SYSStrategy"
    :param str config: 配置文件路径，默认为空


.. py:function:: crt_pf_strategy(pf, query, broker, cost_func, other_brokers=[], name="PFStrategy", config="")

    创建组合策略
    
    :param pf: 资产组合
    :param query: 查询条件
    :param broker: 订单代理
    :param cost_func: 成本函数
    :param list other_brokers: 其他订单代理，默认为空列表
    :param str name: 策略名称，默认为"PFStrategy"
    :param str config: 配置文件路径，默认为空
