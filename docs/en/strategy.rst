.. currentmodule:: hikyuu
.. highlight:: python

Live Trading
============

Programmatic (automated) trading is what quantitative frameworks are commonly built for: at its core, it boils down to scheduled task execution plus event-driven notification callbacks.

Hikyuu focuses on fast strategy analysis and does not provide live trading itself. The Strategy runtime is offered solely as a reference showing how to connect a strategy to a live trading environment. You are fully responsible for any profits or losses that result from using it.

For complete examples, see the demos in the strategy subdirectory under the installation directory.


Common parameters:

    * **spot_worker_num=1** *(int)* : number of internal worker threads used to receive market data
    * **quotation_server=""** *(string)* : address of the market data service; if empty, the local default configuration (hikyuu.ini) is used


.. py:class:: Strategy

    Strategy runtime.

    A Strategy can be created in one of the following ways:

    .. code-block:: python

        # Option 1: create with default parameters
        stg = Strategy()

        # Option 2: specify the security code list and bar (candlestick) types
        stg = Strategy(
            code_list=["sz000001", "sz000002"],
            ktype_list=["day", "min"],
            preload_num={"day_max": 1000, "min_max": 2000},
            name="MyStrategy",
            config=""  # Path to the configuration file; if empty, the default hikyuu configuration file is used
        )

        # Option 3: create from a context
        context = StrategyContext(stock_list=["sz000001"], ktype_list=["day"])
        stg = Strategy(context, name="MyStrategy", config="")

    .. py:attribute:: name

        Strategy name. Read/write.

    .. py:attribute:: context

        Strategy context. Read-only; holds the security code list, bar types, and other settings.

    .. py:attribute:: tm

        Associated trade manager instance. Read/write; manages account assets and orders.

    .. py:attribute:: sp

        Slippage algorithm. Read/write; used only in backtest mode.

    .. py:attribute:: running

        Current running state. Read-only; returns True or False.

    .. py:attribute:: is_backtesting

        Whether the strategy is running as a backtest. Read-only.

    .. py:method:: start(self, auto_recieve_spot=True)

        Starts the strategy. Call this only after you have finished registering all required callbacks.

        Note: The strategy cannot be started from the Python interactive interpreter.

        :param bool auto_recieve_spot: whether to receive market data automatically. Defaults to True

    .. py:method:: on_change(self, func)

        Registers a callback for security data updates.

        The callback fires whenever market data changes and is typically used for debugging. It is triggered on every incoming market-data collection message, regardless of market open/close times.

        :param func: callable that accepts three arguments: func(stg: Strategy, stock: Stock, spot: SpotRecord)

    .. py:method:: on_received_spot(self, func)

        Registers a callback fired after market data is received.

        The callback is notified once a complete batch of market data has been received and is mostly used for debug logging. The batch does not necessarily contain any of the securities in the context, and it fires on every incoming market-data collection message, regardless of market open/close times.

        :param func: callable that accepts two arguments: func(stg: Strategy, revTime: Datetime)

    .. py:method:: run_daily(self, func, time, market="SH", ignore_market=False)

        Registers a callback that runs repeatedly during the trading day. If market open/close times are ignored, the callback loops at the given interval starting from when the strategy starts; otherwise, the first execution is aligned to the market open time, and execution pauses outside trading hours.

        :param func: callable that accepts one argument: func(stg: Strategy)
        :param TimeDelta time: loop interval, e.g. every 3 seconds: TimeDelta(0, 0, 0, 3) or Seconds(3)
        :param str market: market whose open/close times to use. Defaults to "SH"
        :param ignore_market: whether to ignore market open/close times. Defaults to False

    .. py:method:: run_daily_at(self, func, time, ignore_holiday=True)

        Registers a callback that runs every day at a fixed time.

        :param func: callable that accepts one argument: func(stg: Strategy)
        :param TimeDelta time: time of day to run, e.g. 15:00 every day: TimeDelta(0, 15). Must be less than one day
        :param ignore_holiday: if True, the callback is not executed on holidays. Defaults to True

       .. note:: Only one task can be registered for a given time of day; registering a second task at the same time raises an exception

    .. py:method:: today(self)

        Returns the current trading day. Use this method instead of Datetime.today() so that backtest and live-trading behavior stay consistent.

        :return: current trading day
        :rtype: Datetime

    .. py:method:: now(self)

        Returns the current time. Use this method instead of Datetime.now() so that backtest and live-trading behavior stay consistent.

        :return: current time
        :rtype: Datetime

    .. py:method:: next_datetime(self)

        Returns the next trading timestamp (used during backtesting).

        :return: next trading timestamp; in live trading, returns Null<Datetime>()
        :rtype: Datetime

    .. py:method:: get_current_price(self, stk, ktype)

        Returns the current price.

        :param Stock stk: target security
        :param KQuery.KType ktype: bar type
        :return: current price; returns constant.null_price when the price is invalid
        :rtype: price_t

    .. py:method:: get_last_kdata(self, stk, start_date, ktype, recover_type)

        Form 1: returns the bar data for the specified security from the given start date up to the current time (to keep live trading and backtests consistent, always use this method to obtain bar data)

        get_last_kdata(self, stk, start_date, ktype, recover_type)

        Form 2: returns the most recent lastnum bars currently available for the specified security (to keep live trading and backtests consistent, always use this method to obtain bar data)

        get_last_kdata(self, stk, lastnum, ktype, recover_type)

        :param Stock stk: target security
        :param int lastnum: number of most recent records (Form 2)
        :param Datetime start_date: start date (Form 1)
        :param KQuery.KType ktype: bar type
        :param KQuery.RecoverType recover_type: adjustment type. Defaults to KQuery.NO_RECOVER
        :return: bar data
        :rtype: KData


    .. py:method:: get_kdata(self, stk, start_date, end_date, ktype, recover_type)

        Returns the bar data for the specified security within the given date range (to keep live trading and backtests consistent, always use this method to obtain bar data)

        :param Stock stk: target security
        :param Datetime start_date: start date
        :param Datetime end_date: end date; if it is Null or later than the current time, nextDatetime() is used automatically
        :param KQuery.KType ktype: bar type
        :param KQuery.RecoverType recover_type: adjustment type. Defaults to KQuery.NO_RECOVER
        :return: bar data
        :rtype: KData

    .. py:method:: order(self, stock, num, remark='')

        Submits an order by quantity (a positive quantity buys, a negative quantity sells).

        The quantity actually filled is constrained by the security's minimum and maximum tradable quantities:

        - When buying, if the requested quantity exceeds the maximum tradable quantity, the order is filled at the maximum tradable quantity
        - When selling, if the requested quantity exceeds the maximum tradable quantity and is not equal to MAX_DOUBLE, the order is filled at the maximum tradable quantity
        - When selling, if the requested quantity is below the minimum tradable quantity, the entire holding is sold

        :param Stock stock: target security
        :param float num: order quantity
        :param str remark: order remark
        :return: trade record
        :rtype: TradeRecord

    .. py:method:: order_value(self, stock, value, remark='')

        Submits an order sized to a target market value, i.e. how much money you want to put into the security (a positive value buys, a negative value sells).

        This method derives the quantity from the current price and takes commission and other costs into account, making sure the order does not exceed the available funds. If funds are insufficient, the buy quantity is reduced automatically; if not even the minimum tradable quantity can be bought, no order is placed.

        :param Stock stock: target security
        :param float value: amount of cash to invest
        :param str remark: order remark
        :return: trade record
        :rtype: TradeRecord

    .. py:method:: buy(self, stock, price, num, stoploss=0.0, goal_price=0.0, part=SystemPart.PART_SIGNAL, remark='')

        Submits a buy order.

        :param Stock stock: target security
        :param price_t price: buy price; 0 means use the current market price
        :param float num: quantity to buy
        :param float stoploss: stop-loss price. Defaults to 0
        :param float goal_price: price target (take-profit). Defaults to 0
        :param SystemPart part: system part originating the order. Defaults to PART_SIGNAL
        :param str remark: free-form remark
        :return: trade record
        :rtype: TradeRecord

    .. py:method:: sell(self, stock, price, num, stoploss=0.0, goal_price=0.0, part=SystemPart.PART_SIGNAL, remark='')

        Submits a sell order.

        :param Stock stock: target security
        :param price_t price: sell price; 0 means use the current market price
        :param float num: quantity to sell
        :param price_t stoploss: stop-loss price. Defaults to 0
        :param price_t goal_price: price target (take-profit). Defaults to 0
        :param SystemPart part: system part originating the order. Defaults to PART_SIGNAL
        :param str remark: free-form remark
        :return: trade record
        :rtype: TradeRecord


.. py:function:: start_spot_agent(print=False, worker_num=1, addr="")

    Starts the market-data receiving agent.

    Raises an exception if the agent is already running.

    :param bool print: whether to print logs. Defaults to False
    :param int worker_num: number of worker threads. Defaults to 1
    :param str addr: address of the market data collection service; if empty, the market server address from the hikyuu configuration file is used

.. py:function:: stop_spot_agent()

    Stops the market-data receiving agent.

.. py:function:: spot_agent_is_running()

    Checks whether the market-data receiving agent is running.

    :return: True or False

.. py:function:: spot_agent_is_connected()

    Checks whether the market-data receiving agent is connected.

    :return: True or False


.. py:function:: crtBrokerTM(broker, cost_func=TC_Zero(), name="SYS", other_brokers=[])

    Creates a broker-backed trade manager.

    :param broker: broker instance
    :param TradeCost cost_func: transaction cost function. Defaults to TC_Zero()
    :param str name: name. Defaults to "SYS"
    :param list other_brokers: additional order brokers. Defaults to an empty list
    :return: trade manager instance
    :rtype: TradeManagerPtr


.. py:function:: run_in_strategy(sys, stock, query, broker, cost_func, other_brokers=[])

    Form 1: runs the trading system SYS inside the strategy runtime

    run_in_strategy(sys, stock, query, broker, cost_func, other_brokers=[])
    Currently, only systems with both buy_delay and sell_delay set to false are supported, i.e. trading is executed at the close.

    :param sys: trading system
    :param stock: instrument to trade
    :param query: query criteria
    :param broker: order broker (the broker dedicated to synchronizing orders with account assets)
    :param cost_func: cost function
    :param list other_brokers: additional order brokers. Defaults to an empty list

    Form 2: runs the portfolio strategy PF inside the strategy runtime

    Currently, only systems with both buy_delay and sell_delay set to false are supported, i.e. trading is executed at the close.

    :param Portfolio pf: portfolio
    :param Query query: query criteria
    :param broker: order broker (the broker dedicated to synchronizing orders with account assets)
    :param cost_func: cost function
    :param list other_brokers: additional order brokers. Defaults to an empty list


.. py:function:: crt_sys_strategy(sys, stk_market_code, query, broker, cost_func, other_brokers=[], name="SYSStrategy", config="")

    Creates a strategy from a trading system.

    :param sys: trading system
    :param str stk_market_code: security market code
    :param query: query criteria
    :param broker: order broker
    :param cost_func: cost function
    :param list other_brokers: additional order brokers. Defaults to an empty list
    :param str name: strategy name. Defaults to "SYSStrategy"
    :param str config: path to the configuration file. Defaults to empty


.. py:function:: crt_pf_strategy(pf, query, broker, cost_func, other_brokers=[], name="PFStrategy", config="")

    Creates a strategy from a portfolio.

    :param pf: portfolio
    :param query: query criteria
    :param broker: order broker
    :param cost_func: cost function
    :param list other_brokers: additional order brokers. Defaults to an empty list
    :param str name: strategy name. Defaults to "PFStrategy"
    :param str config: path to the configuration file. Defaults to empty
