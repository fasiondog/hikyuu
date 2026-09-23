.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Trade Management
================

Trade management can be understood as a simulated brokerage account that executes simulated trades. In general, use crtTM to create a trade manager instance.

Common parameters:

    * **precision=2** *(int)* : price calculation precision
    * **support_borrow_cash=False** *(bool)* : whether to automatically borrow cash (financing / margin)
    * **support_borrow_stock=False** *(bool)* : whether to automatically borrow securities for short selling
    * **save_action=True** *(bool)* : whether to save the Python command sequences


.. py:function:: crtTM([date = Datetime(199001010000), init_cash = 100000, cost_func = TC_Zero(), name = "SYS"])

    Create the trade manager module, which manages the account's trade records and fund usage
    
    :param Datetime date:  the account establishment date
    :param float init_cash:    the initial capital
    :param TradeCost cost_func: the trade cost algorithm
    :param string name:        the account name
    :rtype: TradeManager
    
    
.. py:class:: TradeManager

    The trade manager class, which can be understood as a simulated brokerage account that executes simulated trades. In general, use crtTM to create a trade manager instance.

    .. py:attribute:: name
        
        The account name
        
    .. py:attribute:: cost_func 
        
        The trade cost algorithm
        
    .. py:attribute:: init_cash
        
        (Read-only) the initial capital
        
    .. py:attribute:: current_cash
    
        (Read-only) the current cash balance
        
    .. py:attribute:: init_datetime
        
        (Read-only) the account establishment date
        
    .. py:attribute:: first_datetime 
        
        (Read-only) the date of the first buy trade; returns Datetime() if no trade has occurred
        
    .. py:attribute:: last_datetime
        
        (Read-only) the date of the last trade (regardless of trade type); returns the account establishment date if no trade has occurred
        
    .. py:attribute:: precision 
        
        (Read-only) the price precision, same as the common parameter "precision"
        
    .. py:attribute:: broker_last_datetime
    
        The moment from which the order broker actually starts operating.
        
        By default, when the TradeManager executes buy/sell operations, it invokes the order broker to perform the broker's buy/sell actions. This causes a problem in live trading. To obtain the latest signal, the signal generator must backtrack through historical data, so the TradeManager ends up executing buy/sell operations at historical moments. If the order broker itself does not control when buy/sell instructions are issued, it will send out erroneous instructions. To prevent this, you must specify a moment after which the order broker's buy/sell operations are allowed. The broker_last_datetime attribute is used to specify that moment.
        

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
    
        Reset, clearing all trade and position records
        
    .. py:method:: clone(self)

        Clone (deep copy) the instance
        
        :rtype: TradeManager
        
    .. py:method:: checkin(self, datetime, cash)
    
        Deposit cash into the account
    
        :param Datetime datetime: the trading time
        :param float cash: the amount of cash deposited
        :rtype: TradeRecord
        
    .. py:method:: checkout(self, datetime, cash)
    
        Withdraw cash from the account
        
        :param Datetime datetime: the trading time
        :param float cash: the amount of funds withdrawn
        :rtype: TradeRecord
        
    .. py:method:: checkin_stock(self, datetime, stock, price, number)
    
        Deposit securities into the account
    
        :param Datetime datetime: the trading time
        :param Stock stock: the security to deposit
        :param float price: the per-share price of the deposited security
        :param float number: the quantity of the deposited security
        :rtype: TradeRecord
        
    .. py:method:: checkout_stock(self, datetime, stock, price, number)
    
        Withdraw securities from the account
        
        :param Datetime datetime: the trading time
        :param Stock stock: the security to withdraw
        :param float price: the per-share price of the withdrawn security
        :param float number: the quantity withdrawn
        :rtype: TradeRecord
        
    .. py:method:: borrow_cash(self, datetime, cash)
    
        Borrow cash (financing / margin)
    
        :param Datetime datetime: the trading time
        :param float cash: the amount of cash borrowed
        :rtype: TradeRecord
        
    .. py:method:: return_cash(self, datetime, cash)
    
        Repay borrowed cash
        
        :param Datetime datetime: the trading time
        :param float cash: the amount of cash repaid
        :rtype: TradeRecord
        
    .. py:method:: borrow_stock(self, datetime, stock, price, number)
    
        Borrow securities (for short selling)
    
        :param Datetime datetime: the trading time
        :param Stock stock: the security borrowed
        :param float price: the per-share price at the time of borrowing
        :param float number: the quantity borrowed
        :rtype: TradeRecord
        
    .. py:method:: return_stock(self, datetime, stock, price, number)
    
        Return borrowed securities
        
        :param Datetime datetime: the trading time
        :param Stock stock: the security returned
        :param float price: the per-share price at the time of return
        :param float number: the quantity returned
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
        :param str remark:        the remark
        :rtype: TradeRecord
        
    .. py:method:: sell(self, datetime, stock, real_price[, number=constant.max_double, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
        The sell operation
        
        :param Datetime datetime: the sell time
        :param Stock stock:       the security to sell
        :param float real_price:  the actual sell price
        :param float number:      the sell quantity; if it equals constant.max_double, sell all
        :param float stoploss:    the new stop-loss price
        :param float goal_price:  the new target price
        :param float plan_price:  the originally planned sell price
        :param SystemPart part:   the source of the trading instruction
        :param str remark:        the remark
        :rtype: TradeRecord
        
    .. py:method:: buy_short(self, datetime, stock, real_price, number[, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
        The short selling operation (sell first, buy back later)
        
        :param Datetime datetime: the short selling time
        :param Stock stock:       the security to short sell
        :param float real_price:  the actual short selling price
        :param float number:      the short selling quantity
        :param float stoploss:    the stop-loss price
        :param float goal_price:  the target price
        :param float plan_price:  the planned short selling price
        :param SystemPart part:   the source of the trading instruction
        :param str remark:        the remark
        :rtype: TradeRecord
        
    .. py:method:: sell_short(self, datetime, stock, real_price[, number=constant.max_double, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])
    
        The short covering operation (buy back to close the position)
        
        :param Datetime datetime: the covering time
        :param Stock stock:       the security to cover
        :param float real_price:  the actual covering price
        :param float number:      the covering quantity; if it equals constant.max_double, cover all
        :param float stoploss:    the stop-loss price
        :param float goal_price:  the target price
        :param float plan_price:  the planned covering price
        :param SystemPart part:   the source of the trading instruction
        :param str remark:        the remark
        :rtype: TradeRecord
        
    .. py:method:: have(self, stock)
    
        Whether the specified security is currently held (long position)
        
        :param Stock stock: the specified security
        :rtype: bool
        
    .. py:method:: have_short(self, stock)
    
        Whether the specified security is currently held in the short position
        
        :param Stock stock: the specified security
        :rtype: bool
        
    .. py:method:: cash(self, datetime[, ktype=Query.KType.DAY])
    
        Get the cash balance on the specified date. (Note: without the date parameter, positions cannot be adjusted according to dividend information.)
        
        :param Datetime datetime: the specified moment
        :param ktype: the K-line type
        :rtype: float
        
    .. py:method:: get_stock_num(self)
    
        The number of distinct securities currently held, i.e. how many different securities are currently held (not the position size of each security)
        
        :rtype: int
        
    .. py:method:: get_short_stock_num(self)
    
        The number of distinct securities currently held in the short position
        
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
    
        Get the number of borrowed shares of the specified security at the specified moment
        
        :param Datetime datetime: the specified moment
        :param Stock stock: the specified security
        :rtype: float
        
    .. py:method:: get_debt_cash(self, datetime)
    
        Get the amount of borrowed cash at the specified moment
        
        :param Datetime datetime: the specified moment
        :rtype: float
        
    .. py:method:: get_position(self, date, stock)

        Get the position record of the security at the specified time; returns PositionRecord() if the security is not currently held
        
        :param Datetime date: the specified time
        :param Stock stock: the specified security
        :rtype: PositionRecord
        
    .. py:method:: get_short_position(self, stock)
    
        Get the current short position record of the specified security; returns PositionRecord() if the security is not currently held
        
        :param Stock stock: the specified security
        :rtype: PositionRecord
        
    .. py:method:: get_position_list(self)
    
        Get all current position records (long)
        
        :rtype: PositionRecordList
        
    .. py:method:: get_positions(self)
    
        Get all current position records as a dictionary, with the stock as the key and the PositionRecord as the value
        
        :rtype: dict
        
    .. py:method:: get_history_position_list(self)
    
        Get all historical position records, i.e. the closed records (long)
        
        :rtype: PositionRecordList

    .. py:method:: get_short_position_list(self)
    
        Get all current short position records
        
        :rtype: PositionRecordList
        
    .. py:method:: get_short_history_position_list(self)
    
        Get all historical short position records
        
        :rtype: PositionRecordList
        
    .. py:method:: get_borrow_stock_list(self)
    
        Get the list of currently borrowed securities
        
        :rtype: BorrowRecordList
        
    .. py:method:: get_trade_list(self[, start, end])
    
        Get the trade records; when no parameters are specified, get all trade records
        
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
    
        Calculate the cost of borrowing cash
        
        :param Datetime datetime: the trading time
        :param float cash: the amount of cash borrowed
        :rtype: CostRecord
        
    .. py:method:: get_return_cash_cost(self, datetime, cash)
    
        Calculate the cost of repaying borrowed cash
        
        :param Datetime datetime: the trading time
        :param float cash: the amount of cash repaid
        :rtype: CostRecord
        
    .. py:method:: get_borrow_stock_cost(self, datetime, stock, price, num)
    
        Calculate the cost of borrowing securities
        
        :param Datetime datetime: the trading time
        :param Stock stock: the security borrowed
        :param float price: the per-share price at the time of borrowing
        :param float num: the quantity borrowed
        :rtype: CostRecord
        
    .. py:method:: get_return_stock_cost(self, datetime, stock, price, num)
    
        Calculate the cost of returning borrowed securities
        
        :param Datetime datetime: the trading time
        :param Stock stock: the security returned
        :param float price: the per-share price at the time of return
        :param float num: the quantity returned
        :rtype: CostRecord
     
    .. py:method:: get_funds(self[, ktype = Query.DAY])

        Get the asset market value details at the specified moment
    
        Form 1: get_funds(self[, ktype = Query.DAY])

        Form 2: get_funds(self, datetime[, ktype = Query.DAY])
    
        get_funds(self, datetime[, ktype = Query.DAY])
        Get the asset market value details at the specified moment
        
        :param Query.KType ktype: the K-line type
        :rtype: FundsRecord

    .. py:method:: get_funds_list(self, dates[, ktype = Query.DAY])
    
        Get the daily asset records for the specified date list
        
        :param DatetimeList dates: the date list
        :param Query.KType ktype: the K-line type
        :rtype: FundsList

    .. py:method:: get_funds_curve(self, dates[, ktype = Query.DAY])
    
        Get the net asset value (NAV) curve
        
        :param DatetimeList dates: the date list; the corresponding NAV curve is obtained according to this date list
        :param Query.KType ktype: the K-line type, which must match the date list
        :return: the NAV list
        :rtype: PriceList
        
    .. py:method:: get_profit_curve(self, dates[, ktype = Query.DAY])
    
        Get the profit curve, i.e. the NAV curve after deducting all previous cash deposits
        
        :param DatetimeList dates: the date list; the corresponding profit curve is obtained according to this date list, which must be in ascending order
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

        Add a trade record directly. If an account initialization record is added, all existing trade and position records are cleared.

        :param TradeRecord tr: the trade record
        :return: True (success) | False (failure)
        :rtype: bool
        
    .. py:method:: add_position(self, position)
    
        After the initial account is established, add a position record directly. It is only used to build an account that already has initial positions.
        
        :param PositionRecord position: the position record
        :return: True | False
        :rtype: bool
        
    .. py:method:: tocsv(self, path)
    
        Output the trade records, open position records, closed position records, and NAV curve in CSV format
        
        :param str path: the directory of the output files
        
    .. py:method:: reg_broker(self, broker)
    
        Register an order broker. This command can be called multiple times to register multiple order brokers.
        
        :param OrderBrokerBase broker: the order broker instance
        
    .. py:method:: clear_broker(self)

        Clear all registered order brokers

    .. py:method:: get_margin_rate(self, datetime, stock)
    
        Get the margin rate of the specified object
        
        :param Datetime datetime: the date
        :param Stock stock: the specified object
        :rtype: float
        
    .. py:method:: update_with_weight(self, date)
    
        Update the current positions and trade records according to dividend/equity adjustment information; must be called in chronological order
        
        :param Datetime date: the current moment
        
    .. py:method:: fetch_asset_info_from_broker(self, broker[, date=Datetime.now()])
    
        Synchronize asset information at the current moment from the broker; must be called in chronological order
        
        :param OrderBrokerBase broker: the order broker instance
        :param Datetime date: when synchronizing, usually the current time (Null); can also be forced to a specified time point
        
    .. py:method:: get_performance(self[, datetime=Datetime.now(), ktype=Query.DAY, ext=False]) -> Performance
        
        Get the account performance at the specified moment

        :param Datetime datetime: the specified moment
        :param Query.KType ktype: the K-line type
        :param bool ext: whether to return the extended statistics items (requires donor user permission); otherwise only the basic statistics items are returned
        :return: the account performance
        :rtype: Performance

    .. py:method:: get_max_pull_back(self, date, ktype=Query.DAY) -> float
    
        Get the maximum drawdown percentage of the account at the specified moment (a negative number)

        :param Datetime date: the specified date (this moment included)
        :param Query.KType ktype: the K-line type
        :return: the maximum drawdown percentage

    .. py:method:: get_position_ext_info_list(self, current_time, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo]
          
        Get the position details (open position records) at the specified time after the account's last trading moment
    
        :param Datetime current_time: the current moment (must be greater than or equal to the last trading moment)
        :param Query.KType ktype: the K-line type
        :param int trade_mode: the trading mode, affecting some statistics items: 0-trade at the close, 1-trade at the next open
        :return: the list of extended position details

    .. py:method:: get_position_ext_info(self, stock, current_time, ktype=Query.DAY, trade_mode=0) -> PositionExtInfo
    
        Get the extended position details at the specified moment (for the specified security only)

        :param Stock stock: the specified security
        :param Datetime current_time: the current moment (must be greater than or equal to the last trading moment)
        :param Query.KType ktype: the K-line type, defaulting to the daily line
        :param int trade_mode: the trading mode, affecting some statistics items: 0-trade at the close, 1-trade at the next open, defaulting to 0
        :return: the extended position details, containing the following fields:
        
            - position (PositionRecord): the basic position record
            - max_high_price (float): the maximum of the highest prices in the period
            - min_low_price (float): the minimum of the lowest prices in the period
            - max_close_price (float): the highest close price in the period
            - min_close_price (float): the lowest close price in the period
            - current_close_price (float): the current close price
            - max_pull_back1 (float): maximum drawdown percentage 1 (computed using only the maximum close price and the lowest close price) (a negative number)
            - max_pull_back2 (float): maximum drawdown percentage 2 (computed using the maximum of the highest prices and the minimum of the lowest prices in the period) (a negative number)
            - current_profit (float): the current floating profit and loss (excluding the estimated sell cost)
            
            And the following calculation methods:
            
            - current_pull_back1(): current drawdown percentage 1 (computed using only the maximum close price and the current close price)
            - current_pull_back2(): current drawdown percentage 2 (computed using the maximum of the highest prices in the period and the current close price)
            - max_floating_profit1(): maximum floating profit percentage 1 in the period (computed using only the close price, excluding the estimated sell cost; statistics may be inaccurate with multiple buys and sells)
            - max_floating_profit2(): maximum floating profit percentage 2 in the period (computed using the maximum of the highest prices, excluding the estimated sell cost; statistics may be inaccurate with multiple buys and sells)
            - min_loss_profit1(): maximum floating loss percentage 1 in the period (computed using only the close price, excluding the estimated sell cost; statistics may be inaccurate with multiple buys and sells)
            - min_loss_profit2(): maximum floating loss percentage 2 in the period (computed using the lowest price in the period, excluding the estimated sell cost; statistics may be inaccurate with multiple buys and sells)
            
        :note: this function is only suitable for the case of one buy and one sell; for the case of one buy and multiple sells, some statistics may be inaccurate — for reference only

    .. py:method:: get_history_position_ext_info_list(self, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo]
          
        Get the historical extended position details of the account (closed records)
    
        :param Query.KType ktype: the K-line type
        :param int trade_mode: the trading mode, affecting some statistics items: 0-trade at the close, 1-trade at the next open
        :return: the list of extended position details

    .. py:method:: get_profit_percent_monthly(self[, datetime=Datetime.now()]) -> list[tuple[Datetime, double]]

        Get the account's monthly profit percentage as of the specified deadline

        :param Datetime datetime: the specified deadline
        :return: the account's monthly profit percentage, a list of (date, return rate) tuples

    .. py:method:: get_profit_percent_yearly(self[, datetime=Datetime.now()]) -> list[tuple[Datetime, double]]

        Get the account's yearly profit percentage as of the specified deadline

        :param Datetime datetime: the specified deadline
        :return: the account's yearly profit percentage, a list of (date, return rate) tuples
