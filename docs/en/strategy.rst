.. currentmodule:: hikyuu
.. highlight:: python

Live Trading
============

Programmatic trading, i.e. automated trading, is the various quantitative frameworks commonly seen; its essence is the scheduled task scheduling + the notification callbacks.

Hikyuu mainly focuses on the fast strategy analysis; it does not provide live trading itself. The Strategy runtime is only for everyone to learn how to connect with the live trading; you are responsible for the profits and losses caused by yourself.

For the details, see the related demos in the strategy subdirectory of the installation directory.


Common parameters:

    * **spot_worker_num=1** *(int)* : the number of the internal threads when receiving the market data
    * **quotation_server=""** *(string)* : specify the market data service address; when empty, the local default configuration (hikyuu.ini) is used


.. py:class:: Strategy

    The strategy runtime
    
    There are the following ways to create a strategy runtime:
    
    .. code-block:: python
    
        # Way 1: create with the default parameters
        stg = Strategy()
        
        # Way 2: specify the security code list and the K-line types
        stg = Strategy(
            code_list=["sz000001", "sz000002"],
            ktype_list=["day", "min"],
            preload_num={"day_max": 1000, "min_max": 2000},
            name="MyStrategy",
            config=""  # the configuration file path; when empty, the default hikyuu configuration file is used
        )
        
        # Way 3: create with a context
        context = StrategyContext(stock_list=["sz000001"], ktype_list=["day"])
        stg = Strategy(context, name="MyStrategy", config="")

    .. py:attribute:: name
        
        The strategy name, readable and writable
        
    .. py:attribute:: context
        
        The strategy context, a read-only attribute, containing the security code list, the K-line types, etc.
        
    .. py:attribute:: tm
        
        The associated trade manager instance, readable and writable, used to manage the account assets and the orders
        
    .. py:attribute:: sp
        
        The slippage algorithm, readable and writable, used only in the backtest state
        
    .. py:attribute:: running
        
        Get the current running state, a read-only attribute, returning True or False
        
    .. py:attribute:: is_backtesting
        
        Get the backtest state, a read-only attribute

    .. py:method:: start(self, auto_recieve_spot=True)

        Start the strategy execution; please execute it after completing the related callback settings.
        
        Note: the strategy cannot be started in the Python interactive mode.

        :param bool auto_recieve_spot: whether to receive the market data automatically, defaulting to True

    .. py:method:: on_change(self, func)

        Set the callback notification of the security data update
        
        This callback is triggered when the market data changes, usually used for debugging. It is triggered whenever a market data collection message is received, without being restricted by the market open/close times.

        :param func: a callable object, which needs to receive three parameters: func(stg: Strategy, stock: Stock, spot: SpotRecord)

    .. py:method:: on_received_spot(self, func)

        Set the callback notification of the security data update
        
        Notified after a batch of the market data has been received, usually used only for the debugging printing. This batch of the market data does not necessarily contain the stocks in the context,
        and it is triggered whenever a market data collection message is received, without being restricted by the market open/close times.

        :param func: a callable object, which needs to receive two parameters: func(stg: Strategy, revTime: Datetime)

    .. py:method:: run_daily(self, func, time, market="SH", ignore_market=False)
        
        Set the callback executed in a loop within the day. If the market open/close is ignored, it loops from the start moment by the interval time;
        otherwise, at the first execution, the time interval is aligned with the market open time, and the execution stops in the non-trading time.

        :param func: a callable object, which needs to receive one parameter: func(stg: Strategy)
        :param TimeDelta time: the interval time, e.g. an interval of 3 seconds: TimeDelta(0, 0, 0, 3) or Seconds(3)
        :param str market: which market's open/close times to use, defaulting to "SH"
        :param ignore_market: ignore the market open/close times, defaulting to False

    .. py:method:: run_daily_at(self, func, time, ignore_holiday=True)

        Set the callback executed at a fixed time every day

        :param func: a callable object, which needs to receive one parameter: func(stg: Strategy)
        :param TimeDelta time: the execution moment, e.g. at 15 o'clock every day: TimeDelta(0, 15), which must be less than 1 day
        :param ignore_holiday: do not execute on the holidays, defaulting to True
       
        .. note:: only one task can be registered at the same moment; registering repeatedly will raise an exception

    .. py:method:: today(self)

        Get the current trading day date (use this method instead of Datetime.today(), so that the backtest and the live trading are consistent)

        :return: the current trading day date
        :rtype: Datetime

    .. py:method:: now(self)   

        Get the current time (use this method instead of Datetime.now(), so that the backtest and the live trading are consistent)

        :return: the current time
        :rtype: Datetime

    .. py:method:: next_datetime(self)

        The next trading time point (used for the backtest)
        
        :return: the next trading time point; in live trading, return Null<Datetime>()
        :rtype: Datetime

    .. py:method:: get_current_price(self, stk, ktype)
    
        Get the current price
        
        :param Stock stk: the specified security
        :param KQuery.KType ktype: the K-line type
        :return: the current price; when invalid, return constant.null_price
        :rtype: price_t

    .. py:method:: get_last_kdata(self, stk, start_date, ktype, recover_type)

        Way 1: get the K-line data of the specified security from the specified date to the current time (to keep the live trading and the backtest consistent, please use this method to get the K-line data)
        get_last_kdata(self, stk, start_date, ktype, recover_type)

        Way 2: get the last last_num K-line records of the specified security that can currently be obtained (to keep the live trading and the backtest consistent, please use this method to get the K-line data)
        get_last_kdata(self, stk, lastnum, ktype, recover_type)

        :param Stock stk: the specified security
        :param int lastnum: the last N records
        :param Datetime start_date: the start date
        :param KQuery.KType ktype: the K-line type
        :param KQuery.RecoverType recover_type: the recovery type, defaulting to KQuery.NO_RECOVER
        :return: the K-line data
        :rtype: KData


    .. py:method:: get_kdata(self, stk, start_date, end_date, ktype, recover_type)

        Get the K-line data of the specified security within the specified date range (to keep the live trading and the backtest consistent, please use this method to get the K-line data)

        :param Stock stk: the specified security
        :param Datetime start_date: the start date
        :param Datetime end_date: the end date; if it is Null or greater than the current time, nextDatetime() is used automatically
        :param KQuery.KType ktype: the K-line type
        :param KQuery.RecoverType recover_type: the recovery type, defaulting to KQuery.NO_RECOVER
        :return: the K-line data
        :rtype: KData

    .. py:method:: order(self, stock, num, remark='')

        Place an order by the quantity (a positive number is a buy, a negative number is a sell)
        
        The actual trading quantity is restricted by the minimum/maximum trading quantity of the security:
        
        - When buying, if the order quantity exceeds the maximum trading quantity, it is traded by the maximum trading quantity
        - When selling, if the order quantity exceeds the maximum trading quantity and is not equal to MAX_DOUBLE, it is traded by the maximum trading quantity
        - When selling, if the order quantity is less than the minimum trading quantity, all are sold

        :param Stock stock: the specified security
        :param float num: the order quantity
        :param str remark: the order remark
        :return: the trade record
        :rtype: TradeRecord

    .. py:method:: order_value(self, stock, value, remark='')

        Place an order by the expected security market value, i.e. how much money of the security you want to buy (a positive number is a buy, a negative number is a sell)
        
        This method calculates the quantity to buy by the current price, and takes the fees and other factors into account, ensuring that it will not exceed the available funds.
        If the funds are insufficient, the buy quantity will be reduced automatically; if even the minimum trading quantity cannot be bought, no order is placed.

        :param Stock stock: the specified security
        :param float value: the funds invested to buy
        :param str remark: the order remark
        :return: the trade record
        :rtype: TradeRecord
        
    .. py:method:: buy(self, stock, price, num, stoploss=0.0, goal_price=0.0, part=SystemPart.PART_SIGNAL, remark='')
    
        The buy operation
        
        :param Stock stock: the specified security
        :param price_t price: the buy price; 0 means using the current market price
        :param float num: the buy quantity
        :param float stoploss: the stop-loss price, defaulting to 0
        :param float goal_price: the target price, defaulting to 0
        :param SystemPart part: the system part, defaulting to PART_SIGNAL
        :param str remark: the remark information
        :return: the trade record
        :rtype: TradeRecord
        
    .. py:method:: sell(self, stock, price, num, stoploss=0.0, goal_price=0.0, part=SystemPart.PART_SIGNAL, remark='')
    
        The sell operation
        
        :param Stock stock: the specified security
        :param price_t price: the sell price; 0 means using the current market price
        :param float num: the sell quantity
        :param price_t stoploss: the stop-loss price, defaulting to 0
        :param price_t goal_price: the target price, defaulting to 0
        :param SystemPart part: the system part, defaulting to PART_SIGNAL
        :param str remark: the remark information
        :return: the trade record
        :rtype: TradeRecord


.. py:function:: start_spot_agent(print=False, worker_num=1, addr="")
    
    Start the market data receiving agent
    
    If it is already in the running state, an exception will be raised.

    :param bool print: whether to print the logs, defaulting to False
    :param int worker_num: the number of the working threads, defaulting to 1
    :param str addr: the market data collection service address; when empty, the market server address in the hikyuu configuration file is used

.. py:function:: stop_spot_agent()

    Stop the market data receiving agent

.. py:function:: spot_agent_is_running()

    Judge whether the market data receiving agent is running

    :return: True or False

.. py:function:: spot_agent_is_connected()

    Judge whether the market data receiving agent is connected

    :return: True or False


.. py:function:: crtBrokerTM(broker, cost_func=TC_Zero(), name="SYS", other_brokers=[])

    Create the broker trade manager
    
    :param broker: the broker instance
    :param TradeCost cost_func: the trade cost function, defaulting to TC_Zero()
    :param str name: the name, defaulting to "SYS"
    :param list other_brokers: the list of the other order brokers, defaulting to empty
    :return: the trade manager instance
    :rtype: TradeManagerPtr


.. py:function:: run_in_strategy(sys, stock, query, broker, cost_func, other_brokers=[])
          
    Way 1: execute the system trading SYS in the strategy runtime

    run_in_strategy(sys, stock, query, broker, cost_func, other_brokers=[])   
    Currently only the systems with both buy_delay|sell_delay being false are supported, i.e. trading at the close
     
    :param sys: the trading system
    :param stock: the trading object
    :param query: the query condition
    :param broker: the order broker (dedicated to the order broker synchronizing with the account assets)
    :param cost_func: the cost function
    :param list other_brokers: the other order brokers, defaulting to an empty list



.. py:function:: crt_sys_strategy(sys, stk_market_code, query, broker, cost_func, other_brokers=[], name="SYSStrategy", config="")

    Create the system strategy
    
    :param sys: the trading system
    :param str stk_market_code: the security market code
    :param query: the query condition
    :param broker: the order broker
    :param cost_func: the cost function
    :param list other_brokers: the other order brokers, defaulting to an empty list
    :param str name: the strategy name, defaulting to "SYSStrategy"
    :param str config: the configuration file path, defaulting to empty


.. py:function:: crt_multi_sys_strategy(ms, stk_market_code, query, broker, cost_func, name="MultiSYSStrategy", other_brokers=[], config="")

    Create the aggregate system strategy (the MultiSystem live trading entry)
    
    The parent account uses the ``BrokerTM`` synchronized with the broker, the sub-systems use their own shadow/virtual accounts (mode A/B is decided internally by MultiSystem).
    Currently only the sub-systems with both buy_delay|sell_delay being false are supported, i.e. the trade is executed at the close.
    
    :param MultiSystem ms: the aggregate trading system
    :param str stk_market_code: the driving instrument (e.g. "SH000001", used to align the time axis, it should cover the trading days of every sub-system)
    :param query: the query condition
    :param broker: the order broker (the order broker synchronized with the parent account assets)
    :param cost_func: the cost function
    :param str name: the strategy name, defaulting to "MultiSYSStrategy"
    :param list other_brokers: the other order brokers, defaulting to an empty list
    :param str config: the configuration file path, defaulting to empty


Portfolio Strategy Migration Guide (PF -> MultiSystem)
------------------------------------------------------

Since 2.8.x, the original portfolio backtest components ``Portfolio`` / ``AllocateFunds`` have been removed, :class:`MultiSystem` uniformly takes over the portfolio backtest and the live trading.

**Backtest migration**

.. code-block:: python

    # Old writing (PF + AF, removed)
    # pf = crtPF(tm, mm, se, af, adjust_cycle=10)
    # pf.run(query)

    # New writing (MultiSystem)
    sys1 = SYS_Simple(tm=tm1, sg=sg1, mm=mm1)   # each has its own SG/MM
    sys2 = SYS_Simple(tm=tm2, sg=sg2, mm=mm2)
    ms = MultiSystem()          # or MultiSystem(name="Combo")
    ms.tm = crtTM(init_cash=1000000)
    ms.set_mode("A")            # the default is fine: A=signal aggregation; B=fund allocation (FOF-MOM)
    ms.set_adjust_cycle(10)     # the rebalancing cycle (days), 1 by default = rebalance on every close day
    ms.add(sys1)
    ms.add(sys2)
    ms.run(sh000001.get_kdata(Query(-200)))   # drive with the time axis covering the trading days of every sub-system

    # Mode B: the parent allocates the quota to the sub-systems by equal weight, the sub-systems trade autonomously within the quota (L2 pass-through)
    ms.set_mode("B")

**Live trading migration**

.. code-block:: python

    # Old writing (removed)
    # stg = crt_pf_strategy(pf, query, broker, cost_func)

    # New writing
    stg = crt_multi_sys_strategy(ms, "SH000001", query, broker, cost_func)
    stg.start()

**Notes**

- The sub-systems must each hold independent SG/MM instances (different securities need to calculate their signals respectively, sharing the same SG will overwrite each other).
- Mode A: the parent allocates and orders uniformly by the weight (the equal weight by default), the sub-systems are pure signal sources; mode B: the parent allocates the real quota, the sub-systems decide autonomously.
- SE (trading object selection) is optional: after ``ms.set_se(se)`` only the selected sub-systems run on the rebalancing day, the unselected ones can be force liquidated (``ms.set_sell_at_not_selected(True)``).
- The portfolio risk control (the concentration upper limit) is controlled by the parent MM parameter ``max-single-position`` (1.0 by default meaning no limit).
