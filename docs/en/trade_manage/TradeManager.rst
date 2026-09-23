.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Trade Management
================

The trade management can be understood as a simulated account performing the simulated trades. Generally, use crtTM to create the trade manager instance.

Common parameters:

    * **precision=2** *(int)* : the price calculation accuracy
    * **support_borrow_cash=False** *(bool)* : whether to finance automatically
    * **support_borrow_stock=False** *(bool)* : whether to short the securities automatically
    * **save_action=True** *(bool)* : whether to save the Python command sequences


.. py:function:: crtTM([date = Datetime(199001010000), init_cash = 100000, cost_func = TC_Zero(), name = "SYS"])

    Create the trade manager module, managing the trade records and the fund usage of the account
    
    :param Datetime date:  the account establishment date
    :param float init_cash:    the initial capital
    :param TradeCost cost_func: the trade cost algorithm
    :param string name:        the account name
    :rtype: TradeManager
    
    
.. py:class:: TradeManager

    The trade manager class, which can be understood as a simulated account performing the simulated trades. Generally, use crtTM to create the trade manager instance.

    .. py:attribute:: name
        
        The name
        
    .. py:attribute:: cost_func 
        
        The trade cost algorithm
        
    .. py:attribute:: init_cash
        
        (Read-only) the initial capital
        
    .. py:attribute:: current_cash
    
        (Read-only) the current cash
        
    .. py:attribute:: init_datetime
        
        (Read-only) the account establishment date
        
    .. py:attribute:: first_datetime 
        
        (Read-only) the date when the first buy trade occurred; if no trade has occurred, return Datetime()
        
    .. py:attribute:: last_datetime
        
        (Read-only) the date of the last trade; note that it is unrelated to the trade type; if no trade has occurred, return the account establishment date
        
    .. py:attribute:: precision 
        
        (Read-only) the price precision, the same as the common parameter "precision"
        
    .. py:attribute:: broker_last_datetime
    
        The moment when the order broker operations actually start.
        
        By default, when the TradeManager executes the buy/sell operations, it calls the order broker to execute the broker's buy/sell actions, but there will be a problem in the live trading operation. Because the system needs to backtrack the historical data to get the latest signal when calculating the signal indicator, the TradeManager will execute the buy/sell operations at the historical moments; at this time, if the order broker itself does not control the moment of issuing the buy/sell instructions, it will cause the broker to send the wrong instructions. At this time, it is necessary to specify that only after a certain moment are the buy/sell operations of the order broker allowed to be specified. The attribute brokeLastDatetime is used to specify that moment.
        

    .. py:method:: __init__()
    
        The initialization constructor
        

    .. py:method:: get_param(self, name)

        Get the specified parameter
    
        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: no such parameter
        
    .. py:method:: set_param(self, name, value)
    
        Set the parameter
        
        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string | Query | KData | Stock | DatetimeList
        :raises logic_error: Unsupported type! The parameter type is not supported
        
    .. py:method:: have_param(self, name)
    
        Check whether the specified parameter exists
        
        :param str name: the parameter name
        :rtype: bool
        
    .. py:method:: reset(self)
    
        Reset, clearing the trade and the position records
        
    .. py:method:: clone(self)

        Clone (deep copy) the instance
        
        :rtype: TradeManager
        
    .. py:method:: checkin(self, datetime, cash)
    
    Deposit the cash into the account
    
        :param Datetime datetime: the trading time
        :param float cash: the amount of the cash deposited
        :rtype: TradeRecord
        
    .. py:method:: checkout(self, datetime, cash)
    
    Withdraw the cash from the account
        
        :param Datetime datetime: the trading time
        :param float cash: the amount of the funds withdrawn
        :rtype: TradeRecord
        
    .. py:method:: checkin_stock(self, datetime, stock, price, number)
    
    Deposit the stock assets
    
        :param Datetime datetime: the trading time
        :param Stock stock: the stock to deposit
        :param float price: the per-share price of the deposited stock
        :param float number: the quantity of the deposited stock
        :rtype: TradeRecord
        
    .. py:method:: checkout_stock(self, datetime, stock, price, number)
    
    Withdraw the stock assets
        
        :param Datetime datetime: the trading time
        :param Stock stock: the stock to withdraw
        :param float price: the per-share price of the withdrawn stock
        :param float number: the withdrawn quantity
        :rtype: TradeRecord
        
    .. py:method:: borrow_cash(self, datetime, cash)
    
    Borrow the funds (financing)
    
        :param Datetime datetime: the trading time
        :param float cash: the amount of the cash borrowed
        :rtype: TradeRecord
        
    .. py:method:: return_cash(self, datetime, cash)
    
    Return the borrowed funds
        
        :param Datetime datetime: the trading time
        :param float cash: the amount of the cash returned
        :rtype: TradeRecord
        
    .. py:method:: borrow_stock(self, datetime, stock, price, number)
    
    Borrow the stocks (short selling the securities)
    
        :param Datetime datetime: the trading time
        :param Stock stock: the stock borrowed
        :param float price: the per-share price at the borrowing
        :param float number: the borrowed quantity
        :rtype: TradeRecord
        
    .. py:method:: return_stock(self, datetime, stock, price, number)
    
    Return the borrowed stocks
        
        :param Datetime datetime: the trading time
        :param Stock stock: the stock returned
        :param float price: the per-share price at the return
        :param float number: the returned quantity
        :rtype: TradeRecord
        
    .. py:method:: buy(self, datetime, stock, real_price, number[, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
    The buy operation
        
        :param Datetime datetime: the buy time
        :param Stock stock:       the security to buy
        :param float real_price:  the actual buy price
        :param float number:      the buy quantity
        :param float stoploss:    the stop-loss price
        :param float goal_price:  the target price
        :param float plan_price:  the planned buy price
        :param SystemPart part:   the source of the trading instruction
        :param str remark:        the remark information
        :rtype: TradeRecord
        
    .. py:method:: sell(self, datetime, stock, real_price[, number=constant.max_double, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
    The sell operation
        
        :param Datetime datetime: the sell time
        :param Stock stock:       the security to sell
        :param float real_price:  the actual sell price
        :param float number:      the sell quantity; if it equals constant.max_double, it means selling all
        :param float stoploss:    the new stop-loss price
        :param float goal_price:  the new target price
        :param float plan_price:  the originally planned sell price
        :param SystemPart part:   the source of the trading instruction
        :param str remark:        the remark information
        :rtype: TradeRecord
        
    .. py:method:: buy_short(self, datetime, stock, real_price, number[, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
    The short selling operation (sell first, buy later)
        
        :param Datetime datetime: the short selling time
        :param Stock stock:       the security to short sell
        :param float real_price:  the actual short selling price
        :param float number:      the short selling quantity
        :param float stoploss:    the stop-loss price
        :param float goal_price:  the target price
        :param float plan_price:  the planned short selling price
        :param SystemPart part:   the source of the trading instruction
        :param str remark:        the remark information
        :rtype: TradeRecord
        
    .. py:method:: sell_short(self, datetime, stock, real_price[, number=constant.max_double, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
    The short covering operation (buy back to close the position)
        
        :param Datetime datetime: the covering time
        :param Stock stock:       the security to cover
        :param float real_price:  the actual covering price
        :param float number:      the covering quantity; if it equals constant.max_double, it means covering all
        :param float stoploss:    the stop-loss price
        :param float goal_price:  the target price
        :param float plan_price:  the planned covering price
        :param SystemPart part:   the source of the trading instruction
        :param str remark:        the remark information
        :rtype: TradeRecord
        
    .. py:method:: have(self, stock)
    
        Whether the specified security is currently held (the long position)
        
        :param Stock stock: the specified security
        :rtype: bool
        
    .. py:method:: have_short(self, stock)
    
        Whether the current short position holds the specified security
        
        :param Stock stock: the specified security
        :rtype: bool
        
    .. py:method:: cash(self, datetime[, ktype=Query.KType.DAY])
    
        Get the cash on the specified date. (Note: without the date parameter, the positions cannot be adjusted according to the dividend information.)
        
        :param Datetime datetime: the specified moment
        :param ktype: the K-line type
        :rtype: float
        
    .. py:method:: get_stock_num(self)
    
        The number of the kinds of the securities currently held, i.e. how many stocks are currently held (not the position size of each stock)
        
        :rtype: int
        
    .. py:method:: get_short_stock_num(self)
    
        The number of the kinds of the securities currently held in the short position
        
        :rtype: int
        
    .. py:method:: get_hold_num(self, datetime, stock)

        Get the long holding quantity of the specified security at the specified moment
        
        :param Datetime datetime: the specified moment
        :param Stock stock: the specified security
        :rtype: float

    .. py:method:: get_short_hold_num(self, datetime, stock)

        Get the short holding quantity of the specified security at the specified moment
        
        :param Datetime datetime: the specified moment
        :param Stock stock: the specified security
        :rtype: float
        
    .. py:method:: get_debt_number(self, datetime, stock)
    
        Get the number of the borrowed stocks at the specified moment
        
        :param Datetime datetime: the specified moment
        :param Stock stock: the specified security
        :rtype: float
        
    .. py:method:: get_debt_cash(self, datetime)
    
        Get the amount of the borrowed cash at the specified moment
        
        :param Datetime datetime: the specified moment
        :rtype: float
        
    .. py:method:: get_position(self, date, stock)

        Get the position record of the security at the specified time; if the stock is not currently held, return PositionRecord()
        
        :param Datetime date: the specified time
        :param Stock stock: the specified security
        :rtype: PositionRecord
        
    .. py:method:: get_short_position(self, stock)
    
        Get the current short position record of the specified security; if the stock is not currently held, return PositionRecord()
        
        :param Stock stock: the specified security
        :rtype: PositionRecord
        
    .. py:method:: get_position_list(self)
    
        Get all the current position records (long)
        
        :rtype: PositionRecordList
        
    .. py:method:: get_positions(self)
    
    Get all the current position records as a dictionary, with the stock as the key and the PositionRecord as the value
        
        :rtype: dict
        
    .. py:method:: get_history_position_list(self)
    
        Get all the historical position records, i.e. the closed records (long)
        
        :rtype: PositionRecordList

    .. py:method:: get_short_position_list(self)
    
        Get all the current short position records
        
        :rtype: PositionRecordList
        
    .. py:method:: get_short_history_position_list(self)
    
        Get all the historical short position records
        
        :rtype: PositionRecordList
        
    .. py:method:: get_borrow_stock_list(self)
    
        Get the list of the currently borrowed stocks
        
        :rtype: BorrowRecordList
        
    .. py:method:: get_trade_list(self[, start, end])
    
        Get the trade records; when the parameters are not specified, get all the trade records
        
        :param Datetime start: the start date
        :param Datetime end: the end date
        :rtype: TradeRecordList
        
    .. py:method:: get_buy_cost(self, datetime, stock, price, num)
    
        Calculate the buy cost
        
        :param Datetime datetime: the trading time
        :param Stock stock:       the security traded
        :param float price:       the buy price
        :param float num:         the buy quantity
        :rtype: CostRecord
        
    .. py:method:: get_sell_cost(self, datetime, stock, price, num)
    
        Calculate the sell cost

        :param Datetime datetime: the trading time
        :param Stock stock:       the security traded
        :param float price:       the sell price
        :param float num:         the sell quantity
        :rtype: CostRecord
        
    .. py:method:: get_borrow_cash_cost(self, datetime, cash)
    
        Calculate the cost of borrowing the funds
        
        :param Datetime datetime: the trading time
        :param float cash: the amount of the cash borrowed
        :rtype: CostRecord
        
    .. py:method:: get_return_cash_cost(self, datetime, cash)
    
        Calculate the cost of returning the borrowed funds
        
        :param Datetime datetime: the trading time
        :param float cash: the amount of the cash returned
        :rtype: CostRecord
        
    .. py:method:: get_borrow_stock_cost(self, datetime, stock, price, num)
    
        Calculate the cost of borrowing the stocks
        
        :param Datetime datetime: the trading time
        :param Stock stock: the stock borrowed
        :param float price: the per-share price at the borrowing
        :param float num: the borrowed quantity
        :rtype: CostRecord
        
    .. py:method:: get_return_stock_cost(self, datetime, stock, price, num)
    
        Calculate the cost of returning the borrowed stocks
        
        :param Datetime datetime: the trading time
        :param Stock stock: the stock returned
        :param float price: the per-share price at the return
        :param float num: the returned quantity
        :rtype: CostRecord
     
    .. py:method:: get_funds(self[, ktype = Query.DAY])

        Get the asset market value details at the specified moment
    
        Way 1: get_funds(self[, ktype = Query.DAY])

        Way 2: get_funds(self, datetime[, ktype = Query.DAY])
    
        get_funds(self, datetime[, ktype = Query.DAY])
        Get the asset market value details at the specified moment
        
        :param Query.KType ktype: the K-line type
        :rtype: FundsRecord

    .. py:method:: get_funds_list(self, dates[, ktype = Query.DAY])
    
        Get the daily asset records of the specified date list
        
        :param DatetimeList dates: the date list
        :param Query.KType ktype: the K-line type
        :rtype: FundsList

    .. py:method:: get_funds_curve(self, dates[, ktype = Query.DAY])
    
        Get the net asset value curve
        
        :param DatetimeList dates: the date list; get the corresponding net asset value curve according to this date list
        :param Query.KType ktype: the K-line type, which must match the date list
        :return: the net asset value list
        :rtype: PriceList
        
    .. py:method:: get_profit_curve(self, dates[, ktype = Query.DAY])
    
        Get the profit curve, i.e. the net asset value curve after deducting the previous deposits
        
        :param DatetimeList dates: the date list; get the corresponding profit curve according to this date list, which should be in the increasing order
        :param Query.KType ktype: the K-line type, which must match the date list
        :return: the profit curve
        :rtype: PriceList
        
    .. py:method:: get_profit_cum_change_curve(self, dates[, ktype = Query.DAY])
    
        Get the cumulative return curve
        
        :param DatetimeList dates: the date list
        :param Query.KType ktype: the K-line type, which must match the date list
        :rtype: PriceList
        
    .. py:method:: get_base_assets_curve(self, dates[, ktype = Query.DAY])
    
        Get the invested principal asset curve (the invested capital)
        
        :param DatetimeList dates: the date list
        :param Query.KType ktype: the K-line type, which must match the date list
        :rtype: PriceList
        
    .. py:method:: add_trade_record(self, tr)

        Add the trade record directly; if the initialization account record is added, all the existing trade and position records will be cleared.

        :param TradeRecord tr: the trade record
        :return: True (success) | False (failure)
        :rtype: bool
        
    .. py:method:: add_position(self, position)
    
        After establishing the initial account, add the position record directly; it is only used to build an account with the initial positions
        
        :param PositionRecord position: the position record
        :return: True | False
        :rtype: bool
        
    .. py:method:: tocsv(self, path)
    
    Output the trade records, the open position records, the closed position records and the net asset value curve in the csv format
        
        :param str path: the directory of the output files
        
    .. py:method:: reg_broker(self, broker)
    
    Register the order broker. This command can be executed multiple times to register multiple order brokers.
        
        :param OrderBrokerBase broker: the order broker instance
        
    .. py:method:: clear_broker(self)

        Clear all the registered order brokers

    .. py:method:: get_margin_rate(self, datetime, stock)
    
        Get the margin ratio of the specified object
        
        :param Datetime datetime: the date
        :param Stock stock: the specified object
        :rtype: float
        
    .. py:method:: update_with_weight(self, date)
    
        Update the current positions and the trade records according to the dividend information; it must be called in the chronological order
        
        :param Datetime date: the current moment
        
    .. py:method:: fetch_asset_info_from_broker(self, broker[, date=Datetime.now()])
    
        Synchronize the asset information at the current moment from the Broker; it must be called in the chronological order
        
        :param OrderBrokerBase broker: the order broker instance
        :param Datetime date: when synchronizing, it is usually the current time (Null); it can also be forced to a specified time point
        
    .. py:method:: get_performance(self[, datetime=Datetime.now(), ktype=Query.DAY, ext=False]) -> Performance
        
        Get the account performance at the specified moment of the account

        :param Datetime datetime: the specified moment
        :param Query.KType ktype: the K-line type
        :param bool ext: whether to get the extended statistics items (requiring the donating user permission); otherwise, only the basic statistics items
        :return: the account performance
        :rtype: Performance

    .. py:method:: get_max_pull_back(self, date, ktype=Query.DAY) -> float
    
        Get the maximum drawdown percentage of the account at the specified moment (a negative number)

        :param Datetime date: the specified date (including this moment)
        :param Query.KType ktype: the K-line type
        :return: the maximum drawdown percentage

    .. py:method:: get_position_ext_info_list(self, current_time, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo]
          
        Get the position details (the open position records) of the specified time after the last trading moment of the account
    
        :param Datetime current_time: the current moment (it needs to be greater than or equal to the last trading moment)
        :param Query.KType ktype: the K-line type
        :param int trade_mode: the trading mode, affecting some statistics items: 0-trading at the close, 1-trading at the next open
        :return: the list of the extended position details

    .. py:method:: get_position_ext_info(self, stock, current_time, ktype=Query.DAY, trade_mode=0) -> PositionExtInfo
    
        Get the extended position details at the specified moment of the account (only for the specified stock)

        :param Stock stock: the specified stock
        :param Datetime current_time: the current moment (it needs to be greater than or equal to the last trading moment)
        :param Query.KType ktype: the K-line type, defaulting to the daily line
        :param int trade_mode: the trading mode, affecting some statistics items: 0-trading at the close, 1-trading at the next open, defaulting to 0
        :return: the extended position details, containing the following fields:
        
            - position (PositionRecord): the basic position record
            - max_high_price (float): the maximum of the highest prices in the period
            - min_low_price (float): the minimum of the lowest prices in the period
            - max_close_price (float): the highest close price in the period
            - min_close_price (float): the lowest close price in the period
            - current_close_price (float): the current close price
            - max_pull_back1 (float): the maximum drawdown percentage 1 (calculated only with the maximum close price and the lowest close price) (a negative number)
            - max_pull_back2 (float): the maximum drawdown percentage 2 (calculated with the maximum of the highest prices and the minimum of the lowest prices in the period) (a negative number)
            - current_profit (float): the current floating profit and loss (excluding the estimated sell cost)
            
            And the following calculation methods:
            
            - current_pull_back1(): the current drawdown percentage 1 (calculated only with the maximum close price and the current close price)
            - current_pull_back2(): the current drawdown percentage 2 (calculated with the maximum of the highest prices in the period and the current close price)
            - max_floating_profit1(): the maximum floating profit percentage 1 in the period (calculated only with the close price, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)
            - max_floating_profit2(): the maximum floating profit percentage 2 in the period (calculated with the maximum of the highest prices, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)
            - min_loss_profit1(): the maximum floating loss percentage 1 in the period (calculated only with the close price, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)
            - min_loss_profit2(): the maximum floating loss percentage 2 in the period (calculated only with the lowest price in the period, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)
            
        :note: this function is only suitable for the case of one buy and one sell; for the case of one buy and multiple sells, some statistics may be inaccurate, for reference only

    .. py:method:: get_history_position_ext_info_list(self, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo]
          
        Get the historical position extended details of the account (the closed records)
    
        :param Query.KType ktype: the K-line type
        :param int trade_mode: the trading mode, affecting some statistics items: 0-trading at the close, 1-trading at the next open
        :return: the list of the extended position details

    .. py:method:: get_profit_percent_monthly(self[, datetime=Datetime.now()]) -> list[tuple[Datetime, double]]

        Get the account profit percentage (monthly) of the account at the specified deadline

        :param Datetime datetime: the specified deadline
        :return: the account profit percentage (monthly), a list of the tuples of (date, return rate)

    .. py:method:: get_profit_percent_yearly(self[, datetime=Datetime.now()]) -> list[tuple[Datetime, double]]

        Get the account profit percentage (yearly) of the account at the specified deadline

        :param Datetime datetime: the specified deadline
        :return: the account profit percentage (yearly), a list of the tuples of (date, return rate)
