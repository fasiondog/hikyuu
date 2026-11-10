.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Basic Data Structures
=====================

Trade Business Types
--------------------

.. py:function:: get_business_name(business)

    :param BUSINESS business: trade business type
    :return: name of the trade business type, one of ("INIT" | "BUY" | "SELL" | "BUY_SHORT" | "SELL_SHORT" | "GIFT" | "BONUS" | "CHECKIN" | "CHECKOUT" | "CHECKIN_STOCK" | "CHECKOUT_STOCK" | "BORROW_CASH" | "RETURN_CASH" | "BORROW_STOCK" | "RETURN_STOCK" | "SUOGU" | "BORROW_ADJUST" | "SHORT_ADJUST" | "DIVIDEND_COMPENSATION" | "SETTLEMENT" | "INTEREST" | "UNKNOWN")
    :rtype: string

.. py:class:: BUSINESS

    - BUSINESS.INIT           - Initialize the account
    - BUSINESS.BUY            - Buy
    - BUSINESS.SELL            - Sell
    - BUSINESS.BUY_SHORT      - Buy short (return the borrowed stock)
    - BUSINESS.SELL_SHORT     - Sell short (sell the borrowed stock)
    - BUSINESS.GIFT           - Bonus shares (stock dividend)
    - BUSINESS.BONUS          - Cash dividend
    - BUSINESS.CHECKIN        - Deposit cash
    - BUSINESS.CHECKOUT       - Withdraw cash
    - BUSINESS.CHECKIN_STOCK  - Deposit stock
    - BUSINESS.CHECKOUT_STOCK - Withdraw stock
    - BUSINESS.BORROW_CASH    - Borrow cash (margin financing)
    - BUSINESS.RETURN_CASH    - Return the borrowed cash
    - BUSINESS.BORROW_STOCK   - Borrow stock (securities lending)
    - BUSINESS.RETURN_STOCK   - Return the borrowed stock
    - BUSINESS.SUOGU          - Share consolidation (stock squeeze)
    - BUSINESS.BORROW_ADJUST  - Borrowed stock quantity adjustment (ex-rights)
    - BUSINESS.SHORT_ADJUST   - Short position quantity adjustment (ex-rights)
    - BUSINESS.DIVIDEND_COMPENSATION - Dividend compensation paid to the lender (short)
    - BUSINESS.SETTLEMENT    - Daily settlement of a margin holding (the settlement price against the mark-to-market basis), see :doc:`margin`
    - BUSINESS.INTEREST      - Interest accrued on a liability
    - BUSINESS.INVALID        - Invalid type




Trade Cost Record
-----------------

Result returned by a transaction-cost calculation.

.. py:class:: CostRecord

    Transaction cost record.

    .. py:attribute:: commission  Commission (float)
    .. py:attribute:: stamptax    Stamp duty (float)
    .. py:attribute:: transferfee Transfer fee (float)
    .. py:attribute:: others      Other fees (float)
    .. py:attribute:: total       Total cost (float) = commission + stamp duty + transfer fee + other fees


Trade Records
-------------

.. py:class:: TradeRecordList

    A list of trade records; a thin wrapper around the C++ std::vector<TradeRecord>.

    .. py:method:: to_numpy()

        Available only when NumPy is installed; converts the list to a numpy.array.

    .. py:method:: to_pandas()

        Available only when pandas is installed; converts the list to a pandas.DataFrame.

    .. py:method:: to_pyarrow()

        Converts the list to a pyarrow.Table.

.. py:class:: TradeRecord([stock, datetime, business, planPrice, realPrice, goalPrice, number, cost, stoploss, cash, part])

    A single trade record representing one executed fill.

    .. py:attribute:: stock     Traded instrument (Stock)
    .. py:attribute:: datetime  Fill timestamp (Datetime)
    .. py:attribute:: business  Trade business type (see BUSINESS)
    .. py:attribute:: plan_price Planned price for the order (float)
    .. py:attribute:: real_price Actual fill price (float)
    .. py:attribute:: goal_price Target price (float); 0 means no target price is set
    .. py:attribute:: number    Filled share quantity (float)
    .. py:attribute:: cost      Transaction costs of the fill

        Type: :py:class:`CostRecord`

    .. py:attribute:: stoploss Stop-loss price (float)
    .. py:attribute:: cash     Cash balance after the fill (float)
    .. py:attribute:: margin   The margin this opening locked (float); non-zero only for a margin-accounted buy or sell short, 0 for every other business, so a replay reproduces the books of that moment; on a daily settlement record (``SETTLEMENT``) it is the relocked margin after the settlement
    .. py:attribute:: part

        Identifies the trading-system part that issued the instruction (see the SystemPart enumeration, also exposed as System.Part).

    .. note::

        A daily settlement record (``BUSINESS.SETTLEMENT``) is not a fill: ``real_price`` is the settlement
        price, ``number`` carries the direction (positive long, negative short, so a security held on both
        sides is told apart), ``cash`` is the cash balance after the posting, ``cost`` is 0 and ``part`` is
        the invalid part (``System.Part.INVALID``).



Position Records
----------------

.. py:class:: PositionRecordList

    A list of position records; a thin wrapper around the C++ std::vector<PositionRecord>.

    .. py:method:: to_numpy()

        Available only when NumPy is installed; converts the list to a numpy.array.

    .. py:method:: to_pandas()

        Available only when pandas is installed; converts the list to a pandas.DataFrame.

    .. py:method:: to_pyarrow()

        Converts the list to a pyarrow.Table.


.. py:class:: PositionRecord([stock, take_datetime, clean_datetime, number, stoploss, goal_price, total_number, buy_money, total_cost, total_risk, sell_money])

    Position record.

    .. py:attribute:: stock          Instrument held (Stock)
    .. py:attribute:: take_datetime  Initial entry time (Datetime)
    .. py:attribute:: clean_datetime Exit time; equal to constant.null_datetime while the position is still open
    .. py:attribute:: number         Current position size, in shares/units (float)
    .. py:attribute:: stoploss       Current stop-loss price (float)
    .. py:attribute:: goal_price     Current target price (float)
    .. py:attribute:: total_number   Cumulative acquired share quantity (float)
    .. py:attribute:: buy_money      Cumulative total buy amount (float)
    .. py:attribute:: total_cost     Cumulative total transaction cost (float)
    .. py:attribute:: total_risk     Cumulative risk (float): the sum over all entries of (entry price - stop-loss price) x entry quantity, excluding transaction costs
    .. py:attribute:: sell_money     Cumulative sale proceeds (float)
    .. py:attribute:: margin         The margin currently locked (float); 0 means the plain cash accounting (see :doc:`margin`)
    .. py:attribute:: open_basis     The opening notional of the still-held lots (float) = opening price x number x unit, released proportionally as the lots close; the cost reference
    .. py:attribute:: settle_basis   The mark-to-market basis of the still-held lots (float); rolled to the settlement price by the daily settlement and equal to open_basis until the first settlement, the basis of the realized profit and of the equity of a margin holding


Funds Records
-------------

Snapshot of the account's current cash, holdings, and borrowings. Returned by :py:meth:`TradeManager.getFunds`.

.. py:class:: FundsRecord([cash, market_value, short_market_value, base_cash, base_asset, borrow_cash, borrow_asset])

    Current funds record, returned by :py:meth:`TradeManager.getFunds`.

    .. py:attribute:: cash               Current cash balance (float)
    .. py:attribute:: market_value       Current market value of long positions (float)
    .. py:attribute:: short_market_value Current market value of short positions (float)
    .. py:attribute:: base_cash          Cumulative cash principal invested (float)
    .. py:attribute:: base_asset         Current value of assets contributed in kind (float)
    .. py:attribute:: borrow_cash        Cash borrowed (margin loan), i.e. outstanding debt (float)
    .. py:attribute:: borrow_asset       Market value of borrowed securities (float)
    .. py:attribute:: occupied_margin    The margin locked by the margin-accounted holdings (float); its equity contribution is already included in market_value

    Read-only attributes, computed automatically from the attributes above:

    .. py:attribute:: total_assets  Total assets (float)
    .. py:attribute:: net_assets  Net assets (float)
    .. py:attribute:: total_borrow  Total borrowings (total debt) (float)
    .. py:attribute:: total_base  Total invested capital (float)
    .. py:attribute:: profit  Cumulative profit (float)
