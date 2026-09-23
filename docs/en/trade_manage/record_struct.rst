.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Basic Data Structures
=====================

Trade Business Types
--------------------

.. py:function:: get_business_name(business)

    :param BUSINESS business: the trade business type
    :return: the trade business type name ("INIT"|"BUY"|"SELL"|"GIFT"|"BONUS"|"CHECKIN"|"CHECKOUT"|"UNKNOWN"
    :rtype: string

.. py:class:: BUSINESS    
    
    - BUSINESS.INIT     - Create the initial account
    - BUSINESS.BUY      - Buy
    - BUSINESS.SELL     - Sell
    - BUSINESS.GIFT     - Bonus shares
    - BUSINESS.BONUS    - Dividend
    - BUSINESS.CHECKIN  - Deposit cash
    - BUSINESS.CHECKOUT - Withdraw cash
    - BUSINESS.INVALID  - Invalid type




Trade Cost Record
-----------------

The return result of the trade cost calculation.

.. py:class:: CostRecord

    The trade cost record

    .. py:attribute:: commission  Commission (float)
    .. py:attribute:: stamptax    Stamp tax (float)
    .. py:attribute:: transferfee Transfer fee (float)
    .. py:attribute:: others      Other fees (float)
    .. py:attribute:: total       Total cost (float), = commission + stamp tax + transfer fee + other fees
        
        
Trade Records
-------------

.. py:class:: TradeRecordList

    The trade record list, a wrapper of the C++ std::vector<TradeRecord>
    
    .. py:method:: to_numpy()
    
        Takes effect only when the numpy module is installed; converts to numpy.array
    
    .. py:method:: to_pandas()
    
        Takes effect only when the pandas module is installed; converts to pandas.DataFrame

    .. py:method:: to_pyarrow()

        Converts to pyarrow.Table

.. py:class:: TradeRecord([stock, datetime, business, planPrice, realPrice, goalPrice, number, cost, stoploss, cash, part])

    The trade record
    
    .. py:attribute:: stock     Stock (Stock)
    .. py:attribute:: datetime  Trade time (Datetime)
    .. py:attribute:: business  Trade type
    .. py:attribute:: plan_price Planned trade price (float)
    .. py:attribute:: real_price Actual trade price (float)
    .. py:attribute:: goal_price Goal price (float); 0 means no goal is set
    .. py:attribute:: number    Traded number (float)
    .. py:attribute:: cost      Trade cost

        Type: :py:class:`CostRecord`
        
    .. py:attribute:: stoploss Stop-loss price (float)
    .. py:attribute:: cash     Cash balance (float)
    .. py:attribute:: part     
    
        The source of the trade instruction, distinguishing which part of the trade system issued the instruction; see: :py:class:`System.Part`



Position Records
----------------

.. py:class:: PositionRecordList

    The position record list, a wrapper of the C++ std::vector<PositionRecord>
    
    .. py:method:: to_numpy()
    
        Takes effect only when the numpy module is installed; converts to numpy.array
    
    .. py:method:: to_pandas()
    
        Takes effect only when the pandas module is installed; converts to pandas.DataFrame

    .. py:method:: to_pyarrow()

        Converts to pyarrow.Table
        

.. py:class:: PositionRecord([stock, take_datetime, clean_datetime, number, stoploss, goal_price, total_number, buy_money, total_cost, total_risk, sell_money])

    The position record
    
    .. py:attribute:: stock          Trading object (Stock)
    .. py:attribute:: take_datetime  The moment of the initial position opening (Datetime)
    .. py:attribute:: clean_datetime The position closing date; it is constant.null_datetime in the current position record
    .. py:attribute:: number       The current position number (float)
    .. py:attribute:: stoploss     The current stop-loss price (float)
    .. py:attribute:: goal_price   The current goal price (float)
    .. py:attribute:: total_number The accumulated position number (float)
    .. py:attribute:: buy_money    The accumulated buy money (float)
    .. py:attribute:: total_cost   The accumulated total trade cost (float)
    .. py:attribute:: total_risk   The accumulated trade risk (float) = the sum of (buy price - stop loss) * buy number, excluding the trade cost
    .. py:attribute:: sell_money   The accumulated sell money (float)

    
Funds Records
-------------

Returned by TradeManager::getFunds.

.. py:class:: FundsRecord([cash, market_value, short_market_value, base_cash, base_asset, borrow_cash, borrow_asset])

    The current funds record, returned by :py:meth:`TradeManager.getFunds`
    
    .. py:attribute:: cash               The current cash (float)
    .. py:attribute:: market_value       The current long market value (float)
    .. py:attribute:: short_market_value The current short position market value (float)
    .. py:attribute:: base_cash          The current invested principal (float)
    .. py:attribute:: base_asset         The current invested asset value (float)
    .. py:attribute:: borrow_cash        The currently borrowed money (float), i.e. the debt
    .. py:attribute:: borrow_asset       The current borrowed securities asset value (float)

    Read-only attributes, the results calculated automatically from the attributes above:

    .. py:attribute:: total_assets  Total assets
    .. py:attribute:: net_assets  Net assets
    .. py:attribute:: total_borrow  Total debt
    .. py:attribute:: total_base  The invested principal (capital)
    .. py:attribute:: profit  The profit
