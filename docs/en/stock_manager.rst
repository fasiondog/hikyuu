.. py:currentmodule:: hikyuu
.. highlight:: python

Security Management
===================

Building Bar (Candlestick) Query Conditions
--------------------------------------------

.. py:class:: Query

    Query criteria for bar (candlestick) data. In Python, ``Query`` is readily available in the
    hikyuu namespace and can be used directly.

    Shortcut aliases for the :py:data:`Query.KType` enumeration values

    - Query.DAY - daily bar
    - Query.WEEK - weekly bar
    - Query.MONTH - monthly bar
    - Query.QUARTER - quarterly bar
    - Query.HALFYEAR - half-yearly bar
    - Query.YEAR - yearly bar
    - Query.MIN - 1-minute bar
    - Query.MIN5 - 5-minute bar
    - Query.MIN15 - 15-minute bar
    - Query.MIN30 - 30-minute bar
    - Query.MIN60 - 60-minute bar

    Shortcut aliases for the :py:data:`Query.RecoverType` enumeration values

    - Query.NO_RECOVER      - no adjustment (raw prices)
    - Query.FORWARD         - forward adjustment
    - Query.BACKWARD        - backward adjustment
    - Query.EQUAL_FORWARD   - proportional forward adjustment
    - Query.EQUAL_BACKWARD  - proportional backward adjustment

    .. py:attribute:: start

        Start index. Invalid when the query is created by date, in which case it is
        constant.null_int64

    .. py:attribute:: end

        End index. Invalid when the query is created by date, in which case it is
        constant.null_int64

    .. py:attribute:: start_datetime

        Start date/time. Invalid when the query is created by positional index, in which case it
        is constant.null_datetime

    .. py:attribute:: end_datetime

        End date/time. Invalid when the query is created by positional index, in which case it is
        constant.null_datetime

    .. py:attribute:: query_type

        Query mode (by date or by positional index)

    .. py:attribute:: ktype

        Requested bar type (candlestick period)

    .. py:attribute:: recover_type

        Requested price adjustment type

    .. py:attribute:: ktype_in_sec

        Duration, in seconds, of the bar type given by ``ktype``; returned as a TimeDelta object

        :rtype: TimeDelta

    .. py:data:: QueryType

        Available query modes

        - DATE  - query by date
        - INDEX - query by positional index

    .. py:data:: KType

        Bar type (candlestick period) enumeration

        - DAY      - daily bar
        - WEEK     - weekly bar
        - MONTH    - monthly bar
        - QUARTER  - quarterly bar
        - HALFYEAR - half-yearly bar
        - YEAR     - yearly bar
        - MIN      - 1-minute bar
        - MIN5     - 5-minute bar
        - MIN15    - 15-minute bar
        - MIN30    - 30-minute bar
        - MIN60    - 60-minute bar

    .. py:data:: RecoverType

        Price adjustment type enumeration for bar data

        - NO_RECOVER      - no adjustment (raw prices)
        - FORWARD         - forward adjustment
        - BACKWARD        - backward adjustment
        - EQUAL_FORWARD   - proportional forward adjustment
        - EQUAL_BACKWARD  - proportional backward adjustment

    .. py:method:: is_right_opening(self)

        Return whether the query covers a right-open interval, i.e. whether no end time is
        specified

    .. py:method:: is_valid_ktype(self, ktype)

        Check whether the given bar type is valid

        :param KType ktype: bar type
        :return: True if the bar type is valid
        :rtype: bool

    .. py:staticmethod:: is_base_ktype(ktype)

        Check whether the given bar type is a built-in (base) bar type

        :param KType ktype: bar type
        :return: True if it is a built-in bar type
        :rtype: bool

    .. py:staticmethod:: is_extra_ktype(ktype)

        Check whether the given bar type is an extended bar type

        :param KType ktype: bar type
        :return: True if it is an extended bar type
        :rtype: bool

    .. py:staticmethod:: get_base_ktype_list()

        Return all built-in (base) bar types

        :return: list of built-in bar types
        :rtype: list[Query.KType]

    .. py:staticmethod:: get_extra_ktype_list()

        Return all extended bar types

        :return: list of extended bar types
        :rtype: list[Query.KType]

    .. py:staticmethod:: get_ktype_in_min(ktype)

        Return the duration, in minutes, of the given bar type

        :rtype: int


StockManager/Block/Stock
-------------------------

.. py:class:: StockManager

    The security information manager

    .. py:attribute:: data_ready

        Whether all data has finished loading and is ready to use

    .. py:staticmethod:: instance()

        Return the StockManager singleton instance

    .. py:method:: init(self, base_info_param, block_param, kdata_param, preload_param, hikyuu_param[, context])

        Initialize the manager. This method must be called once at program startup

        :param Parameter base_info_param: parameters for the base-information data driver
        :param Parameter block_param: parameters for the sector-information data driver
        :param Parameter kdata_param: parameters for the bar-data driver
        :param Parameter preload_param: preloading parameters
        :param Parameter hikyuu_param: other hikyuu parameters
        :param StrategyContext context: strategy context; all securities are loaded by default

    .. py:method:: wait_data_ready(self)

        Block until all data has finished loading and is ready to use

    .. py:method:: cancel_load(self)

        Cancel all in-progress data loading

    .. py:method:: get_base_info_parameter(self)

        :return: parameters for the base-information data driver
        :rtype: Parameter

    .. py:method:: get_block_parameter(self)

        :return: parameters for the sector-information data driver
        :rtype: Parameter

    .. py:method:: get_kdata_parameter(self)

        :return: parameters for the bar-data driver
        :rtype: Parameter

    .. py:method:: get_preload_parameter(self)

        :return: preloading parameters
        :rtype: Parameter

    .. py:method:: get_hikyuu_parameter(self)

        :return: other hikyuu parameters
        :rtype: Parameter

    .. py:method:: get_context(self)

        :return: the current strategy context
        :rtype: StrategyContext

    .. py:method:: set_plugin_path(self, path)

        Set the plugin search path. Only takes effect when called before initialization

    .. py:method:: get_plugin_path(self)

        :return: the plugin search path
        :rtype: str

    .. py:method:: set_language_path(self, path)

        Set the directory containing the translation files used for multi-language support. Only
        takes effect when called before initialization

    .. py:method:: reload(self)

        Reload all security data

    .. py:method:: reload_with(self, context)

        Reload using the given strategy context. If the security list in the context is empty,
        the previous context is retained

        :param StrategyContext context: the strategy context

    .. py:method:: tmpdir(self)

        Return the temporary directory used for temporary variables and similar files. When not
        configured, it defaults to the current working directory; it is set via the "tmpdir" key
        in m_config

    .. py:method:: datadir(self)

        Return the directory containing the financial data

    .. py:method:: get_market_list(self)

        Return the list of known market abbreviations

        :rtype: StringList

    .. py:method:: get_market_info(self, market)

        Return information about the given market

        :param string market: market identifier (the market abbreviation)
        :return: market information for the given market; returns Null<MarketInfo>() if no such
                 market exists
        :rtype: MarketInfo

    .. py:method:: get_market_stock(self, market)

        Return the representative benchmark index of the given market (may be null)

        :param string market: market identifier (the market abbreviation)
        :return: the representative index for the market; returns Null<Stock>() if no such market
                 exists
        :rtype: Stock

    .. py:method:: get_stock_type_info(self, stk_type)

        Return the detailed information record for the given security type

        :param int stk_type: security type; see :py:data:`constant`
        :return: information for the given security type; returns Null<StockTypeInfo>() if it
                 does not exist
        :rtype: StockTypeInfo

    .. py:method:: get_stock_type_list(self)

        Return the detailed information records for all security types

        :return: detailed information for all security types
        :rtype: DataFrame

    .. py:method:: get_stock(self, querystr)

        Return the security identified by "market abbreviation + security code"

        :param str querystr: market abbreviation followed by the security code, e.g. "sh000001"
        :return: the matching security; returns Null<Stock>() if it does not exist, without
                 raising an exception
        :rtype: Stock

    .. py:method:: get_stock_list(self[, filter=None])

        Return the list of securities

        :param func filter: predicate function that takes a stock as its argument and returns
                            True | False

    .. py:method:: __getitem__

        Equivalent to get_stock

    .. py:method:: __len__

        Return the number of securities

    .. py:method:: __iter__

        Iterate over all securities

    .. py:method:: get_category_list(self)

        Return all sector categories

        :return: all sector categories
        :rtype: StringList

    .. py:method:: get_block(self, category, name)

        Return a predefined sector

        :param str category: the sector category
        :param str name: the sector name
        :return: the sector; returns an empty Block if it cannot be found
        :rtype: Block

    .. py:method:: add_block(self, block)

        Add a standalone sector to the database. Sectors are keyed by category + name, so an
        existing sector with the same key will be overwritten. Note that after modifying a sector
        you must call save_block to persist the changes.

        :param Block block: the sector to add

    .. py:method:: save_block(self, block)

        Persist a modified sector to the database

        :param Block block: the sector instance

    .. py:method:: remove_block(self, block)

        Delete a sector from the database

        :param Block block: the sector to delete

    .. py:method:: get_block_list(self[, category])

        Return the sectors belonging to the given category

        :param str category: the sector category
        :return: the sector list
        :rtype: BlockList

    .. py:method:: get_block_list_by_index_stock(self, index_stk)

        Return the sectors associated with the given index

        :param Stock index_stk: the index
        :return: the sector list
        :rtype: BlockList

    .. py:method:: get_trading_calendar(self, query[, market='SH'])
                  get_trading_calendar(self, stk_list, query)

        Return the trading calendar

        **Form 1:** return the trading calendar of the specified market

        :param Query query: the Query criteria
        :param str market: market abbreviation, defaults to 'SH'
        :return: the list of trading dates
        :rtype: DatetimeList

        **Form 2:** return the union of the trading calendars of the given securities (mainly
        used when the list contains securities from different markets)

        :param StockList stk_list: the security list
        :param Query query: the Query criteria
        :return: the list of trading dates
        :rtype: DatetimeList

    .. py:method:: is_holiday(self, d)

        Return whether the date of the given time is a market holiday (the A-share calendar
        only)

        :param Datetime d: the specified time
        :rtype: bool

    .. py:method:: is_trading_hours(self, d: Datetime, market: str)

        Return whether the given time falls within trading hours

        :param Datetime d: the time to check
        :param str market: the market abbreviation
        :return: True if the time is within trading hours
        :rtype: bool


    .. py:method:: add_temp_csv_stock(self, code, day_filename, min_filename[, tick=0.01, tick_value=0.01, precision=2, min_trade_num = 1, max_trade_num=1000000])

        Register a temporary Stock backed by CSV files of bar data. This is useful for ad-hoc
        testing when the only market data available is a set of CSV bars.

        The added stock is placed on the "TMP" market; prefix its code with "tmp" to retrieve it
        through the manager, e.g. sm['tmp0001']

        The first line of each CSV file is a header and must contain the following columns:
        Datetime (Date or the Chinese alias 日期 is also accepted), OPEN (开盘价),
        HIGH (最高价), LOW (最低价), CLOSE (收盘价), AMOUNT (成交金额), and VOLUME
        (VOL, COUNT, or 成交量 are also accepted). The Chinese names are literal header aliases
        recognized by the CSV loader.

        :param str code: a self-assigned security code; it must not clash with an existing
                         Stock, otherwise Null<Stock> is returned
        :param str day_filename: path to the daily-bar CSV file
        :param str min_filename: path to the minute-bar CSV file
        :param float tick: minimum price tick, defaults to 0.01
        :param float tick_value: monetary value of one tick, defaults to 0.01
        :param int precision: price precision in decimal places, defaults to 2
        :param int min_trade_num: minimum order quantity, defaults to 1
        :param int max_trade_num: maximum order quantity, defaults to 1000000
        :return: the newly added Stock
        :rtype: Stock

    .. py:method:: remove_temp_csv_stock(self, code)

        Remove a previously added temporary Stock

        :param str code: the custom code specified when it was created

    .. py:method:: add_stock(self, stock)

        Use with caution!!! Intended only for registering temporary external Stocks; typically
        used together with the Stock.set_krecord_list method so that data from external sources
        can be consumed directly

        :param Stock stock: a Stock created outside the manager

    .. py:method:: remove_stock(self, market_code)

        Remove the security identified by market_code from the manager. Use with caution!!!
        Typically used to remove temporary external Stocks that were added earlier

        :param str market_code: the market-qualified security identifier

    .. py:method:: get_history_finance_all_fields(self)

        Return all historical financial-report fields together with their indices

    .. py:method:: get_history_finance_field_index(self, name)

        Return the index of the historical financial field with the given name

    .. py:method:: get_history_finance_field_name(self, index)

        Return the name of the historical financial field at the given index


.. py:class:: Stock

    A tradable security

    .. py:attribute:: id : internal id, typically used as a map key
    .. py:attribute:: market : market abbreviation of the market it belongs to; the market abbreviation uniquely identifies a market
    .. py:attribute:: code : the security code
    .. py:attribute:: market_code : market abbreviation + security code, e.g. sh000001
    .. py:attribute:: name : the security name
    .. py:attribute:: type

        The security type; see :py:data:`constant`

    .. py:attribute:: valid : whether the security is currently valid (tradable)
    .. py:attribute:: start_datetime : the first date for which data is available
    .. py:attribute:: last_datetime : the last date for which data is available
    .. py:attribute:: tick : the minimum price tick
    .. py:attribute:: tick_value : the monetary value of one tick
    .. py:attribute:: unit : value per unit = tickValue / tick
    .. py:attribute:: precision : the price precision
    .. py:attribute:: atom : the minimum tradable quantity; the same as minTradeNumber
    .. py:attribute:: min_trade_number : the minimum tradable quantity
    .. py:attribute:: max_trade_number : the maximum tradable quantity

    .. py:method:: is_null(self)

        Whether this is a null instance

        :rtype: bool

    .. py:method:: get_kdata(self, query)

        Return the bars matching the query

        :param Query query: the query criteria
        :return: the bar data satisfying the query
        :rtype: KData

    .. py:method:: get_count(self[, ktype=Query.DAY])

        Return the number of available bars of the specified type

        :param Query.KType ktype: the bar type
        :return: the number of bar records
        :rtype: int

    .. py:method:: get_market_value(self, date, ktype)

        Return the market value at the specified time, i.e. the close price of the last bar whose
        time is less than or equal to the given time

        :param Datetime date: the specified time
        :param Query.KType ktype: the bar type
        :return: the market value at the specified time
        :rtype: float

    .. py:method:: get_krecord(self, pos[, ktype=Query.DAY])

        Return the bar record at the specified index, without bounds checking

        :param int pos | Datetime datetime: the zero-based index position, or a date
        :param Query.KType ktype: the bar type
        :return: the bar record
        :rtype: KRecord


    .. py:method:: get_krecord_list(self, start, end, ktype)

        Return the bar records in the half-open range [start, end). Rarely called directly;
        use get_kdata instead

        :param int start: the start position
        :param int end: the end position
        :param Query.KType ktype: the bar type
        :return: the list of bar records
        :rtype: KRecordList

    .. py:method:: get_datetime_list(self, query)

        Return the list of bar dates

        :param Query query: the query criteria
        :rtype: DatetimeList

    .. py:method:: get_timeline_list(self, query)

        Return the intraday timeline data

        :param Query query: the query criteria (the bar type and price adjustment type carried by
                            the query are ignored here)
        :rtype: TimeLineList

    .. py:method:: get_trans_list(self, query)

        Return the historical tick-by-tick transaction data

        :param Query query: the query criteria (the bar type and price adjustment type carried by
                            the query are ignored here)
        :rtype: TransList

    .. py:method:: get_weight(self[, start, end])

        Return the dividend and corporate-action records within the half-open range
        [start, end). When neither bound is specified, all records are returned.

        :param Datetime start: the start time
        :param Datetime end: the end time
        :rtype: StockWeightList

    .. py:method:: get_finance_info(self)

        Return the current fundamental data

        :rtype: Parameter

    .. py:method:: get_history_finance(self)

        Return all historical fundamental records. For the field layout, refer to the related
        StockManager methods: get_history_finance_all_fields /
        get_history_finance_field_index / get_history_finance_field_name

        For day-to-day work, the FINANCE indicator is the more convenient way to access
        fundamental data

        :param Datetime date: the report date must be one of 0331, 0630, 0930, 1231,
                              e.g. Datetime(201109300000)
        :rtype: list

    .. py:method:: set_krecord_list(self, krecord_list[, ktype=Query.DAY])

        Use with caution!!! Directly replaces the in-memory KRecordList. Intended only for
        supplying bar data to temporary external Stocks.
        If the data is a pandas.DataFrame, use the set_kdata_from_df method instead.

        :param sequence krecord_list: an iterable that yields KRecord instances, e.g. a list
                                      containing only KRecord instances
        :param Query.KType ktype: the bar type

    .. py:method:: set_kdata_from_df(self, df, cols, [ktype=Query.DAY])

        Use with caution!!! Directly replaces the in-memory data, which means the underlying data
        of the Stock is changed.
        Builds a KRecordList from the DataFrame and assigns it to this Stock. df must provide the
        column names in the given order; the default columns are:
        ("datetime", "open", "high", "low", "close", "amount", "volume")

        .. code-block:: python

            import baostock as bs
            import pandas as pd
            lg = bs.login()

            rs = bs.query_history_k_data_plus("sh.600246",
                                            "date,code,open,high,low,close,volume,amount,adjustflag",
                                            start_date='2020-01-01', end_date='2025-12-31')
            print('query_history_k_data_plus respond error_code:'+rs.error_code)
            print('query_history_k_data_plus respond  error_msg:'+rs.error_msg)

            #### Print the result set ####
            data_list = []
            while (rs.error_code == '0') & rs.next():
                # Fetch one row at a time and accumulate the rows
                data_list.append(rs.get_row_data())
            result = pd.DataFrame(data_list, columns=rs.fields)
            print(result)
            result['datetime'] = pd.to_datetime(result['date'])
            print(result)

            stock = Stock('TMP', '600246', 'test')
            stock.set_kdata_from_df(result)
            print(stock)

        :param DataFrame df: the input data
        :param list cols: the column names
        :param Query.KType ktype: the bar type


    .. py:method:: realtime_update(self, krecord)

        (Temporary helper) Only updates the daily-bar data in the in-memory cache

        In client mode of the standalone data server, updates for ordinary securities (which
        have no local buffer) are forwarded over IPC to the master process, applied there, and
        mirrored into shared memory where every client can read them; temporary securities
        (those with a local buffer created through set_krecord_list) are updated in place in the
        local cache and are not forwarded.

        :param KRecord krecord: the newly arrived real-time bar record

    .. py:method:: get_last_update_time(self[, ktype=Query.DAY])

        Return the last update time for bars of the specified type. In client mode of the
        standalone data server, ordinary securities are forwarded to the master process and
        return its buffer refresh time; temporary securities (external data supplied through
        set_krecord_list) return the local write time.

        :param Query.KType ktype: the bar type
        :rtype: Datetime

    .. py:method:: load_kdata_to_buffer(self, ktype)

        Load bars of the specified type into the in-memory cache

        :param Query.KType ktype: the bar type

    .. py:method:: release_kdata_buffer(self, ktype)

        Release the in-memory bars of the specified type from the cache

        :param Query.KType ktype: the bar type

    .. py:method:: get_belong_to_block_list(self[, category=None])

        Return the sectors this security belongs to

        :param str category: restrict the result to this sector category; when None, sectors from
                             all categories are returned
        :rtype: list


.. py:class:: Block

    The sector class; a Block can be viewed as a container of securities

    .. py:attribute:: category : the sector category
    .. py:attribute:: name : the sector name
    .. py:attribute:: index_stock: the associated index (may be a null Stock)

    .. py:method:: __init__(self, category, name):

        Construct a new empty sector with the given category and name

        :param str category: the sector category
        :param str name: the sector name

    .. py:method:: __init__(self, block):

        Construct a new sector as a copy of another sector

        :param Block block: the source sector instance

    .. py:method:: size(self)

        The number of securities contained in the sector

    .. py:method:: empty(self)

        Whether the sector is empty

    .. py:method:: get(self, market_code)

        Return the security identified by "market abbreviation + security code"

        :param str market_code: market abbreviation followed by the security code, e.g.
                                "sh000001"
        :return: the matching security; returns Null<Stock>() if it does not exist, without
                 raising an exception
        :rtype: Stock

    .. py:method:: add(self, stock)

        Add the specified security to the sector

        :param Stock stock: the security to add
        :return: True if it was added successfully
        :rtype: bool

        add(self, market_code)

        Add the specified security by "market abbreviation + security code"

        :param str market_code: market abbreviation + security code
        :return: True if it was added successfully
        :rtype: bool

    .. py:method:: remove(self, stock)

        Remove the specified security from the sector

        :param Stock stock: the security to remove
        :return: True on success
        :rtype: bool

        remove(self, market_code)

        Remove the specified security from the sector

        :param str market_code: market abbreviation + security code
        :return: True on success
        :rtype: bool

    .. py:method:: clear(self)

        Remove all securities from the sector

    .. py:method:: __len__(self)

        The number of securities contained in the sector

    .. py:method:: __getitem__(self, market_code)

        Return the security identified by "market abbreviation + security code"

        :param str market_code: market abbreviation followed by the security code, e.g.
                                "sh000001"
        :return: the matching security; returns Null<Stock>() if it does not exist, without
                 raising an exception
        :rtype: Stock


Other Security Information Definitions
---------------------------------------

.. py:class:: StockTypeInfo

    The detailed record describing a security type

    .. py:attribute:: type : the security type
    .. py:attribute:: description : descriptive information
    .. py:attribute:: tick : the minimum price tick
    .. py:attribute:: tick_value : the price of one tick
    .. py:attribute:: unit : the price per minimum increment, i.e. the unit price = tickValue/tick
    .. py:attribute:: precision : the price precision
    .. py:attribute:: min_trade_num : the minimum order quantity
    .. py:attribute:: max_trade_num : the maximum order quantity


.. py:class:: StockWeight

    The dividend and corporate-action record

    .. py:attribute:: datetime : the dividend/corporate-action date
    .. py:attribute:: count_as_gift : X bonus shares granted per 10 shares held
    .. py:attribute:: count_for_sell : X rights-issue shares allotted per 10 shares held
    .. py:attribute:: price_for_sell : the rights-issue subscription price
    .. py:attribute:: bonus : the cash dividend per 10 shares
    .. py:attribute:: increasement : X shares converted from capital reserves per 10 shares
    .. py:attribute:: total_count : total share capital (in 10,000 shares)
    .. py:attribute:: free_count : freely tradable (floating) shares (in 10,000 shares)


.. py:class:: StockWeightList

    A wrapper around std::vector<StockWeight>; see :py:class:`StockWeight`

    .. py:method:: to_numpy(self)

        Convert to a numpy array

    .. py:method:: to_pandas(self)

        Convert to a pandas DataFrame

    .. py:method:: to_pyarrow(self)

        Convert to a pyarrow Table


.. py:class:: MarketInfo

    The market information record

    .. py:attribute:: market : the market abbreviation (e.g. "SH" for the Shanghai market, "SZ" for the Shenzhen market)
    .. py:attribute:: name : the full name of the market
    .. py:attribute:: description : descriptive information
    .. py:attribute:: code : the main benchmark index of this market, used to derive the trading calendar
    .. py:attribute:: last_datetime : the latest trading date for which bar data is available in this market
