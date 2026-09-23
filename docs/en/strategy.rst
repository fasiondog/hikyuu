.. currentmodule:: hikyuu
.. highlight:: python

Live Trading
============

Programmatic (i.e. automated) trading is what the various quantitative frameworks commonly provide; at its core, it boils down to scheduled task execution plus notification callbacks.

Hikyuu focuses primarily on fast strategy analysis and does not itself provide live trading. The Strategy runtime is provided only as a reference for learning how to integrate Hikyuu with a live trading setup; you are solely responsible for any resulting profits or losses.

For details, see the demos in the strategy subdirectory of the installation directory.


Common parameters:

    * **spot_worker_num=1** *(int)* : number of internal threads used when receiving market data
    * **quotation_server=""** *(string)* : the market data service address; when empty, the local default configuration (hikyuu.ini) is used


.. py:class:: Strategy

    The strategy runtime.

    A strategy runtime can be created in the following ways:

    .. code-block:: python

        # Option 1: create with the default parameters
        stg = Strategy()

        # Option 2: specify the security code list and bar types
        stg = Strategy(
            code_list=["sz000001", "sz000002"],
            ktype_list=["day", "min"],
            preload_num={"day_max": 1000, "min_max": 2000},
            name="MyStrategy",
            config=""  # the configuration file path; when empty, the default hikyuu configuration file is used
        )

        # Option 3: create with a context
        context = StrategyContext(stock_list=["sz000001"], ktype_list=["day"])
        stg = Strategy(context, name="MyStrategy", config="")

    .. py:attribute:: name

        The strategy name. Read/write.

    .. py:attribute:: context

        The strategy context, a read-only attribute containing the security code list, bar types, and other settings.

    .. py:attribute:: tm

        The associated trade manager instance. Read/write. Used to manage account assets and orders.

    .. py:attribute:: sp

        The slippage algorithm. Read/write. Used only during backtesting.

    .. py:attribute:: running

        The current running state, a read-only attribute returning True or False.

    .. py:attribute:: is_backtesting

        The backtest state, a read-only attribute.

    .. py:method:: start(self, auto_recieve_spot=True)

        Start the strategy. Call this method only after all relevant callbacks have been set up.

        Note: the strategy cannot be started in the Python interactive interpreter.

        :param bool auto_recieve_spot: whether to receive market data automatically; defaults to True

    .. py:method:: on_change(self, func)

        Register the callback for security data updates.

        This callback fires whenever market data changes and is usually used for debugging. It is triggered on every received market-data collection message, regardless of market open/close hours.

        :param func: a callable that must accept three parameters: func(stg: Strategy, stock: Stock, spot: SpotRecord)

    .. py:method:: on_received_spot(self, func)

        Register the callback for security data updates.

        Fires after a complete batch of market data has been received; usually used only for debug printing. The batch does not necessarily contain the securities in the context,
        and the callback is triggered on every received market-data collection message, regardless of market open/close hours.

        :param func: a callable that must accept two parameters: func(stg: Strategy, revTime: Datetime)

    .. py:method:: run_daily(self, func, time, market="SH", ignore_market=False)

        Register a callback that runs repeatedly throughout the trading day. If market open/close hours are ignored, it loops from the start moment at the given interval;
        otherwise, on the first run the interval is aligned to the market open time, and execution pauses outside trading hours.

        :param func: a callable that must accept one parameter: func(stg: Strategy)
        :param TimeDelta time: the time interval, e.g. an interval of 3 seconds: TimeDelta(0, 0, 0, 3) or Seconds(3)
        :param str market: which market's open/close hours to use; defaults to "SH"
        :param ignore_market: whether to ignore market open/close hours; defaults to False

    .. py:method:: run_daily_at(self, func, time, ignore_holiday=True)

        Register a callback that runs at a fixed time every day.

        :param func: a callable that must accept one parameter: func(stg: Strategy)
        :param TimeDelta time: the execution time, e.g. 15:00 every day: TimeDelta(0, 15); must be less than 1 day
        :param ignore_holiday: do not execute on holidays; defaults to True

        .. note:: only one task can be registered at a given time; registering another one at the same time raises an exception

    .. py:method:: today(self)

        Get the current trading-day date (use this method instead of Datetime.today(), so that backtesting and live trading stay consistent).

        :return: the current trading-day date
        :rtype: Datetime

    .. py:method:: now(self)

        Get the current time (use this method instead of Datetime.now(), so that backtesting and live trading stay consistent).

        :return: the current time
        :rtype: Datetime

    .. py:method:: next_datetime(self)

        The next trading time point (used in backtesting).

        :return: the next trading time point; in live trading, returns Null<Datetime>()
        :rtype: Datetime

    .. py:method:: get_current_price(self, stk, ktype)

        Get the current price.

        :param Stock stk: the specified security
        :param KQuery.KType ktype: the bar type
        :return: the current price; returns constant.null_price when invalid
        :rtype: price_t

    .. py:method:: get_last_kdata(self, stk, start_date, ktype, recover_type)

        Form 1: get the bar data of the specified security from the specified date to the current time (to keep live trading and backtesting consistent, use this method to obtain bar data).
        get_last_kdata(self, stk, start_date, ktype, recover_type)

        Form 2: get the last ``lastnum`` bars currently available for the specified security (to keep live trading and backtesting consistent, use this method to obtain bar data).
        get_last_kdata(self, stk, lastnum, ktype, recover_type)

        :param Stock stk: the specified security
        :param int lastnum: the last N bars
        :param Datetime start_date: the start date
        :param KQuery.KType ktype: the bar type
        :param KQuery.RecoverType recover_type: the price adjustment type; defaults to KQuery.NO_RECOVER
        :return: the bar data
        :rtype: KData


    .. py:method:: get_kdata(self, stk, start_date, end_date, ktype, recover_type)

        Get the bar data of the specified security over the specified date range (to keep live trading and backtesting consistent, use this method to obtain bar data).

        :param Stock stk: the specified security
        :param Datetime start_date: the start date
        :param Datetime end_date: the end date; if it is Null or later than the current time, nextDatetime() is used automatically
        :param KQuery.KType ktype: the bar type
        :param KQuery.RecoverType recover_type: the price adjustment type; defaults to KQuery.NO_RECOVER
        :return: the bar data
        :rtype: KData

    .. py:method:: order(self, stock, num, remark='')

        Place an order by quantity (a positive value buys, a negative value sells).

        The actual traded quantity is constrained by the security's minimum and maximum tradable quantity:

        - When buying, if the order quantity exceeds the maximum tradable quantity, it is filled at the maximum tradable quantity.
        - When selling, if the order quantity exceeds the maximum tradable quantity and is not equal to MAX_DOUBLE, it is filled at the maximum tradable quantity.
        - When selling, if the order quantity is below the minimum tradable quantity, the entire position is sold.

        :param Stock stock: the specified security
        :param float num: the order quantity
        :param str remark: the order remark
        :return: the trade record
        :rtype: TradeRecord

    .. py:method:: order_value(self, stock, value, remark='')

        Place an order by target market value, i.e. the amount of capital to allocate to buying the security (a positive value buys, a negative value sells).

        This method calculates the quantity to trade from the current price and takes fees and other factors into account, ensuring that the available funds are not exceeded.
        If the funds are insufficient, the buy quantity is reduced automatically; if even the minimum tradable quantity cannot be bought, no order is placed.

        :param Stock stock: the specified security
        :param float value: the funds to invest when buying
        :param str remark: the order remark
        :return: the trade record
        :rtype: TradeRecord

    .. py:method:: buy(self, stock, price, num, stoploss=0.0, goal_price=0.0, part=SystemPart.PART_SIGNAL, remark='')

        The buy operation.

        :param Stock stock: the specified security
        :param price_t price: the buy price; 0 means using the current market price
        :param float num: the buy quantity
        :param float stoploss: the stop-loss price; defaults to 0
        :param float goal_price: the take-profit target price; defaults to 0
        :param SystemPart part: the system part; defaults to PART_SIGNAL
        :param str remark: the remark
        :return: the trade record
        :rtype: TradeRecord

    .. py:method:: sell(self, stock, price, num, stoploss=0.0, goal_price=0.0, part=SystemPart.PART_SIGNAL, remark='')

        The sell operation.

        :param Stock stock: the specified security
        :param price_t price: the sell price; 0 means using the current market price
        :param float num: the sell quantity
        :param price_t stoploss: the stop-loss price; defaults to 0
        :param price_t goal_price: the take-profit target price; defaults to 0
        :param SystemPart part: the system part; defaults to PART_SIGNAL
        :param str remark: the remark
        :return: the trade record
        :rtype: TradeRecord


.. py:function:: start_spot_agent(print=False, worker_num=1, addr="")

    Start the market-data receiving agent.

    An exception is raised if it is already running.

    :param bool print: whether to print logs; defaults to False
    :param int worker_num: the number of worker threads; defaults to 1
    :param str addr: the market-data collection service address; when empty, the market server address in the hikyuu configuration file is used

.. py:function:: stop_spot_agent()

    Stop the market-data receiving agent.

.. py:function:: spot_agent_is_running()

    Check whether the market-data receiving agent is running.

    :return: True or False

.. py:function:: spot_agent_is_connected()

    Check whether the market-data receiving agent is connected.

    :return: True or False


.. py:function:: crtBrokerTM(broker, cost_func=TC_Zero(), name="SYS", other_brokers=[])

    Create a broker-backed trade manager.

    :param broker: the broker instance
    :param TradeCost cost_func: the trade cost function; defaults to TC_Zero()
    :param str name: the name; defaults to "SYS"
    :param list other_brokers: the list of other order brokers; defaults to an empty list
    :return: the trade manager instance
    :rtype: TradeManagerPtr


.. py:function:: run_in_strategy(sys, stock, query, broker, cost_func, other_brokers=[])

    Form 1: run the trading system SYS within the strategy runtime.

    run_in_strategy(sys, stock, query, broker, cost_func, other_brokers=[])
    Currently, only systems with both buy_delay and sell_delay set to false are supported, i.e. trades are executed at the close.

    :param sys: the trading system
    :param stock: the trading object
    :param query: the query condition
    :param broker: the order broker (the order broker dedicated to synchronizing with the account assets)
    :param cost_func: the cost function
    :param list other_brokers: the other order brokers; defaults to an empty list



.. py:function:: crt_sys_strategy(sys, stk_market_code, query, broker, cost_func, other_brokers=[], name="SYSStrategy", config="")

    Create a system strategy.

    :param sys: the trading system
    :param str stk_market_code: the security market code
    :param query: the query condition
    :param broker: the order broker
    :param cost_func: the cost function
    :param list other_brokers: the other order brokers; defaults to an empty list
    :param str name: the strategy name; defaults to "SYSStrategy"
    :param str config: the configuration file path; defaults to an empty string


.. py:function:: crt_multi_sys_strategy(ms, stk_market_code, query, broker, cost_func, name="MultiSYSStrategy", other_brokers=[], config="")

    Create an aggregate system strategy (the MultiSystem live-trading entry point).

    The parent account uses the ``BrokerTM`` synchronized with the broker, while the sub-systems use their own shadow/virtual accounts (mode A/B is decided internally by MultiSystem).
    Currently, only sub-systems with both buy_delay and sell_delay set to false are supported, i.e. trades are executed at the close.

    :param MultiSystem ms: the aggregate trading system
    :param str stk_market_code: the driving instrument (e.g. "SH000001"), used to align the time axis; it should cover the trading days of every sub-system
    :param query: the query condition
    :param broker: the order broker (synchronized with the parent account's assets)
    :param cost_func: the cost function
    :param str name: the strategy name; defaults to "MultiSYSStrategy"
    :param list other_brokers: the other order brokers; defaults to an empty list
    :param str config: the configuration file path; defaults to an empty string


Portfolio Strategy Migration Guide (PF -> MultiSystem)
------------------------------------------------------

Since 2.8.x, the original portfolio backtest building blocks ``Portfolio`` / ``AllocateFunds`` have been removed; :class:`MultiSystem` now uniformly handles both portfolio backtesting and live trading.

**Backtest migration**

.. code-block:: python

    # Old approach (PF + AF, removed)
    # pf = crtPF(tm, mm, se, af, adjust_cycle=10)
    # pf.run(query)

    # New approach (MultiSystem)
    sys1 = SYS_Simple(tm=tm1, sg=sg1, mm=mm1)   # each holds its own SG/MM
    sys2 = SYS_Simple(tm=tm2, sg=sg2, mm=mm2)
    ms = MultiSystem()          # or MultiSystem(name="Combo")
    ms.tm = crtTM(init_cash=1000000)
    ms.set_mode("A")            # the default is fine: A = signal aggregation; B = fund allocation (FOF-MOM)
    ms.set_adjust_cycle(10)     # the rebalancing cycle (days); 1 by default means rebalancing on every close day
    ms.add(sys1)
    ms.add(sys2)
    ms.run(sh000001.get_kdata(Query(-200)))   # drive with a time axis covering the trading days of every sub-system

    # Mode B: the parent allocates equal-weight quotas to the sub-systems, which trade autonomously within their quotas (L2 pass-through)
    ms.set_mode("B")

**Live trading migration**

.. code-block:: python

    # Old approach (removed)
    # stg = crt_pf_strategy(pf, query, broker, cost_func)

    # New approach
    stg = crt_multi_sys_strategy(ms, "SH000001", query, broker, cost_func)
    stg.start()

**Notes**

- Each sub-system must hold independent SG/MM instances (different securities need to compute their signals separately; sharing the same SG causes signals to overwrite each other).
- Mode A: the parent uniformly allocates and places orders by weight (equal weight by default), and the sub-systems are pure signal sources; Mode B: the parent allocates real quotas, and the sub-systems decide autonomously.
- SE (trading object selection) is optional: after ``ms.set_se(se)``, only the selected sub-systems run on the rebalancing day, while the unselected ones can be force-liquidated (``ms.set_sell_at_not_selected(True)``).
- Portfolio risk control (the concentration limit) is controlled by the parent MM parameter ``max-single-position`` (1.0 by default, which means no limit).
