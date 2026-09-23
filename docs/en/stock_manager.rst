.. py:currentmodule:: hikyuu
.. highlight:: python

Security Management
===================

Building the K-line Query Condition
-----------------------------------
    
.. py:class:: Query

    The K-line data query condition; generally in Python, using Query directly is enough, without specifying Query.

    The simplified :py:data:`Query.KType` enumeration values
    
    - Query.DAY - the daily line type
    - Query.WEEK - the weekly line type
    - Query.MONTH - the monthly line type
    - Query.QUARTER - the quarterly line type
    - Query.HALFYEAR - the half-year line type
    - Query.YEAR - the yearly line type
    - Query.MIN - the 1-minute line type
    - Query.MIN5 - the 5-minute line type
    - Query.MIN15 - the 15-minute line type
    - Query.MIN30 - the 30-minute line type
    - Query.MIN60 - the 60-minute line type
    
    The simplified :py:data:`Query.RecoverType` enumeration values
    
    - Query.NO_RECOVER      - no adjustment
    - Query.FORWARD         - the forward adjustment
    - Query.BACKWARD        - the backward adjustment
    - Query.EQUAL_FORWARD   - the equal-ratio forward adjustment
    - Query.EQUAL_BACKWARD  - the equal-ratio backward adjustment

    .. py:attribute:: start 
    
        The start index; it is invalid when created with the date query way, being constant.null_int64
        
    .. py:attribute:: end

        The end index; it is invalid when created with the date query way, being constant.null_int64
        
    .. py:attribute:: start_datetime
    
        The start date; it is invalid when created with the index query way, being constant.null_datetime
        
    .. py:attribute:: end_datetime
    
        The end date; it is invalid when created with the index query way, being constant.null_datetime
        
    .. py:attribute:: query_type
    
        The query way
        
    .. py:attribute:: ktype
    
        The K-line type queried
        
    .. py:attribute:: recover_type
    
        The adjustment type queried
        
    .. py:attribute:: ktype_in_sec
    
        Get the number of the seconds corresponding to the ktype, returning a TimeDelta object
        
        :rtype: TimeDelta
    
    .. py:data:: QueryType
    
        The definition of the query ways
        
        - DATE  - query by the date way
        - INDEX - query by the index way
    
    .. py:data:: KType
    
        The definition of the K-line type enumerations
        
        - DAY      - the daily line type
        - WEEK     - the weekly line type
        - MONTH    - the monthly line type
        - QUARTER  - the quarterly line type 
        - HALFYEAR - the half-year line type 
        - YEAR     - the yearly line type 
        - MIN      - the 1-minute line type
        - MIN5     - the 5-minute line type
        - MIN15    - the 15-minute line type
        - MIN30    - the 30-minute line type
        - MIN60    - the 60-minute line type    
        
    .. py:data:: RecoverType
    
        The definition of the K-line adjustment type enumerations
    
        - NO_RECOVER      - no adjustment
        - FORWARD         - the forward adjustment
        - BACKWARD        - the backward adjustment
        - EQUAL_FORWARD   - the equal-ratio forward adjustment
        - EQUAL_BACKWARD  - the equal-ratio backward adjustment

    .. py:method:: is_right_opening(self)

        Judge whether it is a right-open interval, i.e. the end time is not specified

    .. py:method:: is_valid_ktype(self, ktype)

        Judge whether the specified K-line type is valid

        :param KType ktype: the K-line type
        :return: whether it is valid
        :rtype: bool

    .. py:staticmethod:: is_base_ktype(ktype)

        Judge whether the specified K-line type is a basic K-line type

        :param KType ktype: the K-line type
        :return: whether it is a basic K-line type
        :rtype: bool

    .. py:staticmethod:: is_extra_ktype(ktype)

        Judge whether the specified K-line type is an extended K-line type

        :param KType ktype: the K-line type
        :return: whether it is an extended K-line type
        :rtype: bool

    .. py:staticmethod:: get_base_ktype_list()

        Get all the basic K-line types

        :return: the list of the basic K-line types
        :rtype: list[Query.KType]

    .. py:staticmethod:: get_extra_ktype_list()

        Get all the extended K-line types

        :return: the list of the extended K-line types
        :rtype: list[Query.KType]

    .. py:staticmethod:: get_ktype_in_min(ktype)

        Get the number of the minutes corresponding to the specified K-line type

        :rtype: int


StockManager/Block/Stock
-------------------------

.. py:class:: StockManager

    The security information management class
    
    .. py:attribute:: data_ready
    
        Whether all the data is ready (loaded)
        
    .. py:staticmethod:: instance()
    
        Get the StockManager singleton instance
        
    .. py:method:: init(self, base_info_param, block_param, kdata_param, preload_param, hikyuu_param[, context])
    
        The initialization function, which must be called at the program entry
        
        :param Parameter base_info_param: the basic information data driver parameters
        :param Parameter block_param: the block information data driver parameters
        :param Parameter kdata_param: the K-line data driver parameters
        :param Parameter preload_param: the preloading parameters
        :param Parameter hikyuu_param: the other hikyuu parameters
        :param StrategyContext context: the strategy context, loading all the securities by default
        
    .. py:method:: wait_data_ready(self)
    
        A simple block, waiting for all the data to be ready (loaded)
        
    .. py:method:: cancel_load(self)
    
        Cancel all the data loading
       
    .. py:method:: get_base_info_parameter(self)
    
        :return: the basic information data driver parameters
        :rtype: Parameter
        
    .. py:method:: get_block_parameter(self)

        :return: the block information data driver parameters
        :rtype: Parameter
        
    .. py:method:: get_kdata_parameter(self)
    
        :return: the K-line data driver parameters
        :rtype: Parameter
        
    .. py:method:: get_preload_parameter(self)
    
        :return: the preloading parameters
        :rtype: Parameter
        
    .. py:method:: get_hikyuu_parameter(self)
    
        :return: the other hikyuu parameters
        :rtype: Parameter

    .. py:method:: get_context(self)

        :return: get the current context
        :rtype: StrategyContext

    .. py:method:: set_plugin_path(self, path)
    
        Set the plugin path, which is valid only when set before the initialization
        
    .. py:method:: get_plugin_path(self)
    
        :return: get the plugin path
        :rtype: str
        
    .. py:method:: set_language_path(self, path)
    
        Set the path of the translation files for the multi-language support, which is valid only when set before the initialization
    
    .. py:method:: reload(self)
    
        Reload all the security data
        
    .. py:method:: reload_with(self, context)
    
        Reload with the strategy context parameter; if the security list in the context is empty, the original context will be used
        
        :param StrategyContext context: the strategy context
    
    .. py:method:: tmpdir(self)
    
        Get the temporary directory used to save the temporary variables, etc.; if not configured, it is the current directory, specified by "tmpdir" in m_config

    .. py:method:: datadir(self)

        Get the finance data directory
    
    .. py:method:: get_market_list(self)
    
        Get the list of the market abbreviations
        
        :rtype: StringList
    
    .. py:method:: get_market_info(self, market)
    
        Get the corresponding market information
        
        :param string market: the specified market identifier (the market abbreviation)
        :return: the corresponding market information; if the corresponding market information does not exist, return Null<MarketInfo>()
        :rtype: MarketInfo
    
    .. py:method:: get_market_stock(self, market)
    
        Get the representative index of the specified market (may be empty)
        
        :param string market: the specified market identifier (the market abbreviation)
        :return: the corresponding market representative index; if the corresponding market information does not exist, return Null<Stock>()
        :rtype: Stock
    
    .. py:method:: get_stock_type_info(self, stk_type)
    
        Get the detailed information of the corresponding security type
        
        :param int stk_type: the security type, see: :py:data:`constant`
        :return: the corresponding security type information; if it does not exist, return Null<StockTypeInfo>()
        :rtype: StockTypeInfo

    .. py:method:: get_stock_type_list(self)

        Get the detailed information of all the security types

        :return: the detailed information of all the security types
        :rtype: DataFrame
        
    .. py:method:: get_stock(self, querystr)
    
        Get the corresponding security instance by "market abbreviation + security code"
        
        :param str querystr: the format: "market abbreviation + security code", e.g. "sh000001"
        :return: the corresponding security instance; if the instance does not exist, return Null<Stock>() without raising an exception
        :rtype: Stock
    
    .. py:method:: get_stock_list(self[, filter=None])
    
        Get the security list
        
        :param func filter: a filter function whose input parameter is the stock and which returns True | False
        
    .. py:method:: __getitem__

        The same as get_stock
        
    .. py:method:: __len__
    
        Return the number of the securities
        
    .. py:method:: __iter__
    
        Traverse all the securities
        
    .. py:method:: get_category_list(self)
    
        Get all the block categories
        
        :return: all the block categories
        :rtype: StringList
    
    .. py:method:: get_block(self, category, name)
    
        Get the predefined block
        
        :param str category: the block category
        :param str name: the block name
        :return: the block; if it cannot be found, return an empty Block
        :rtype: Block

    .. py:method:: add_block(self, block)

        Add the independent block to the database; the blocks are distinguished by category+name, and the same block in the database will be overwritten. Note, if the block has changed, you need to call save_block to save it again.

        :param Block block: the newly added block
        
    .. py:method:: save_block(self, block)
    
        Save the changed block to the database
        
        :param Block block: the block instance

    .. py:method:: remove_block(self, block)

        Delete the block from the database system

        :param Block block: the block to delete
        
    .. py:method:: get_block_list(self[, category])
    
        Get the block list of the specified category
        
        :param str category: the block category
        :return: the block list
        :rtype: BlockList

    .. py:method:: get_block_list_by_index_stock(self, index_stk)

        Get the block list of the specified index

        :param Stock index_stk: the index
        :return: the block list
        :rtype: BlockList        
    
    .. py:method:: get_trading_calendar(self, query[, market='SH'])
                  get_trading_calendar(self, stk_list, query)
    
        Get the trading calendar
        
        **Way 1:** get the trading calendar of the specified market
        
        :param Query query: the Query condition
        :param str market: the market abbreviation, defaulting to 'SH'
        :return: the date list
        :rtype: DatetimeList
        
        **Way 2:** get the superimposed trading calendar according to the specified security list (mainly used when the securities of the different markets are included)
        
        :param StockList stk_list: the stock list
        :param Query query: the Query condition
        :return: the date list
        :rtype: DatetimeList
        
    .. py:method:: is_holiday(self, d)

        Judge whether the date corresponding to the time is a holiday (only using the A-share market)

        :param Datetime d: the specified time
        :rtype: bool

    .. py:method:: is_trading_hours(self, d: Datetime, market: str)

        Judge whether the date corresponding to the specified time is the trading time

        :param Datetime d: the time to judge
        :param str market: the market abbreviation
        :return: whether it is the trading time
        :rtype: bool
    

    .. py:method:: add_temp_csv_stock(self, code, day_filename, min_filename[, tick=0.01, tick_value=0.01, precision=2, min_trade_num = 1, max_trade_num=1000000])

        Add a temporary Stock from the CSV files (the K-line data), which can be used for the temporary testing when there are only the K-line data in the CSV format.

        The market of the added stock is "TMP"; if it needs to be obtained through sm, you need to add tmp, e.g.: sm['tmp0001']
        
        The first line of the CSV file is the header, which needs to contain Datetime (or Date, 日期), OPEN (or 开盘价), HIGH (or 最高价), LOW (or 最低价), CLOSE (or 收盘价), AMOUNT (or 成交金额), VOLUME (or VOL, COUNT, 成交量).
        
        :param str code: the security code numbered by yourself; it cannot be the same as the existing Stock, otherwise it will return Null<Stock>.
        :param str day_filename: the daily-line CSV file name
        :param str min_filename: the minute-line CSV file name
        :param float tick: the minimum tick, defaulting to 0.01
        :param float tick_value: the minimum tick value, defaulting to 0.01
        :param int precision: the price precision, defaulting to 2
        :param int min_trade_num: the minimum trading quantity per order, defaulting to 1
        :param int min_trade_num: the maximum trading quantity per order, defaulting to 1000000
        :return: the added Stock
        :rtype: Stock

    .. py:method:: remove_temp_csv_stock(self, code)
    
        Remove the added temporary Stock
        
        :param str code: the custom code at the creation

    .. py:method:: add_stock(self, stock)

        Call with caution!!! It is only used to add some temporary external Stocks, usually used together with the Stock.set_krecord_list method to directly use the data from the external sources

        :param Stock stock: the Stock created by yourself outside sm

    .. py:method:: remove_stock(self, market_code)

        Remove the security represented by the market_code from sm; use with caution!!! It is usually used to remove the temporary external Stocks added

        :param str market_code: the security market identifier

    .. py:method:: get_history_finance_all_fields(self)

        Get all the historical finance information fields and their indexes

    .. py:method:: get_history_finance_field_index(self, name)

        Get the index of the corresponding field of the historical finance information by the field name

    .. py:method:: get_history_finance_field_name(self, index)

        Get the corresponding field name of the historical finance information by the field index


.. py:class:: Stock

    The security object

    .. py:attribute:: id : the internal id, generally used as the key of a map
    .. py:attribute:: market : get the market abbreviation it belongs to; the market abbreviation is the unique identifier of the market
    .. py:attribute:: code : get the security code
    .. py:attribute:: market_code : the market abbreviation + the security code, e.g.: sh000001
    .. py:attribute:: name : get the security name
    .. py:attribute:: type 
    
        Get the security type, see: :py:data:`constant`
        
    .. py:attribute:: valid : whether the security is currently valid
    .. py:attribute:: start_datetime : the start date of the security
    .. py:attribute:: last_datetime : the last date of the security
    .. py:attribute:: tick : the minimum tick
    .. py:attribute:: tick_value : the minimum tick value
    .. py:attribute:: unit : the per-unit value = tickValue / tick
    .. py:attribute:: precision : the price precision
    .. py:attribute:: atom : the minimum trading quantity, the same as minTradeNumber
    .. py:attribute:: min_trade_number : the minimum trading quantity
    .. py:attribute:: max_trade_number : the maximum trading quantity

    .. py:method:: is_null(self)
    
        Whether it is Null
    
        :rtype: bool
    
    .. py:method:: get_kdata(self, query)
    
        Get the K-line data
        
        :param Query query: the query condition
        :return: the K-line data satisfying the query condition
        :rtype: KData
    
    .. py:method:: get_count(self[, ktype=Query.DAY])
    
        Get the amount of the K-line data of the different types
        
        :param Query.KType ktype: the K-line data category
        :return: the number of the K-line records
        :rtype: int
    
    .. py:method:: get_market_value(self, date, ktype)
    
        Get the market value at the specified moment, i.e. the close price of the last record less than or equal to the specified moment
        
        :param Datetime date: the specified moment
        :param Query.KType ktype: the K-line data category
        :return: the market value at the specified moment
        :rtype: float
    
    .. py:method:: get_krecord(self, pos[, ktype=Query.DAY])
    
        Get the K-line data record at the specified index, without the out-of-bounds check
        
        :param int pos | Datetime datetime: the specified index position, or the date
        :param Query.KType ktype: the K-line data category
        :return: the K-line record
        :rtype: KRecord
    
    
    .. py:method:: get_krecord_list(self, start, end, ktype)
    
        Get the K-line records [start, end); it is generally not used directly, replaced with getKData
        
        :param int start: the start position
        :param int end: the end position
        :param Query.KType ktype: the K-line category
        :return: the K-line record list
        :rtype: KRecordList
    
    .. py:method:: get_datetime_list(self, query)
    
        Get the date list
        
        :param Query query: the query condition
        :rtype: DatetimeList

    .. py:method:: get_timeline_list(self, query)
    
        Get the time-line data
        
        :param Query query: the query condition (the K-line type and the adjustment type parameters in the query condition are useless at this time)
        :rtype: TimeLineList
    
    .. py:method:: get_trans_list(self, query)
    
        Get the historical tick data
        
        :param Query query: the query condition (the K-line type and the adjustment type parameters in the query condition are useless at this time)
        :rtype: TransList

    .. py:method:: get_weight(self[, start, end])
    
        Get the dividend information within the specified time range [start, end). When the start and the end moments are not specified, get all the dividend records.
        
        :param Datetime start: the start moment
        :param Datetime end: the end moment
        :rtype: StockWeightList
        
    .. py:method:: get_finance_info(self)
    
        Get the current finance information
        
        :rtype: Parameter
        
    .. py:method:: get_history_finance(self)
    
        Get the list of all the historical finance information; for the field information, refer to the related methods in StockManager: the get_history_finance_all_fields/get_history_finance_field_index/get_history_finance_field_name methods
        
        For the daily use, it is recommended to use the FINANCE indicator directly to get the finance data
        
        :param Datetime date: the specified date must be 0331, 0630, 0930, 1231, e.g. Datetime(201109300000)
        :rtype: list
    
    .. py:method:: set_krecord_list(self, krecord_list[, ktype=Query.DAY])

        Call with caution!!! Set the current memory KRecordList directly; it is only used to set the K-line data for the external Stocks that need to be added temporarily.
        If the data format is a pandas.DataFrame, you can use the set_kdata_from_df method.

        :param sequence krecord_list: an object of an iterable variable to get the KRecord instances, e.g.: a list (containing only the KRecord instances)
        :param Query.KType ktype: the K-line category

    .. py:method:: set_kdata_from_df(self, df, cols, [ktype=Query.DAY])

        Call with caution!!! Set the current memory data directly, which means the basic data of the Stock is changed.
        Get the KRecordList from the DataFrame and set it to the current Stock. df must specify the column names in order, defaulting to: ("datetime", "open", "high", "low", "close", "amount", "volume"))")

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
                # Get a record and merge the records together
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
        :param Query.KType ktype: the K-line category


    .. py:method:: realtime_update(self, krecord)
    
        (A temporary function) only used to update the daily-line data in the memory cache

        In the client mode of the single-machine data server: the update of the ordinary securities (no local buffer) is forwarded through IPC to the master process to apply and
        mirrored into the shared memory (readable by all the clients); the temporary securities (with a local buffer created by set_krecord_list) are
        updated in place in the local cache, without being sent out.
        
        :param KRecord krecord: the newly added real-time K-line record
        
    .. py:method:: get_last_update_time(self[, ktype=Query.DAY])

        Get the last update moment of the specified type of the K-line data. In the client mode of the single-machine data server, the ordinary securities are forwarded to
        the master process to take its buffer refresh moment; the temporary securities (with the external data specified by set_krecord_list) return the local writing moment.

        :param Query.KType ktype: the K-line type
        :rtype: Datetime

    .. py:method:: load_kdata_to_buffer(self, ktype)
    
        Load the K-line data of the specified category into the memory cache
        
        :param Query.KType ktype: the K-line type

    .. py:method:: release_kdata_buffer(self, ktype)
    
        Release the memory K-line data of the specified category
        
        :param Query.KType ktype: the K-line type

    .. py:method:: get_belong_to_block_list(self[, category=None])
    
        Get the list of the belonging blocks

        :param str category: the specified block category; when it is None, return the belonging blocks under all the block categories
        :rtype: list    
    
    
.. py:class:: Block

    The block class, which can be regarded as a container of the securities
    
    .. py:attribute:: category : the block category
    .. py:attribute:: name : the block name
    .. py:attribute:: index_stock: the corresponding index (may be an empty Stock)
    
    .. py:method:: __init__(self, category, name):
    
        Build a new block instance and specify its block category and block name
    
        :param str category: the block category
        :param srt name: the block name

    .. py:method:: __init__(self, block):
    
        Build a new block instance from another block instance
    
        :param Block block: the block instance
    
    .. py:method:: size(self)
    
        The number of the contained securities
        
    .. py:method:: empty(self)
    
        Whether it is empty
        
    .. py:method:: get(self, market_code)

        Get the corresponding security instance by "market abbreviation + security code"

        :param str market_code: the format: "market abbreviation + security code", e.g. "sh000001"
        :return: the corresponding security instance; if the instance does not exist, return Null<Stock>() without raising an exception
        :rtype: Stock

    .. py:method:: add(self, stock)
    
        Add the specified security
        
        :param Stock stock: the security to add
        :return: whether it was added successfully
        :rtype: bool
        
        add(self, market_code)
    
        Add the specified security by "market abbreviation + security code"
        
        :param str market_code: the market abbreviation + the security code
        :return: whether it was added successfully
        :rtype: bool

    .. py:method:: remove(self, stock)
    
        Remove the specified security
        
        :param Stock stock: the specified security
        :return: whether it was successful
        :rtype: bool
        
        remove(self, market_code)
    
        Remove the specified security
        
        :param str market_code: the market abbreviation + the security code
        :return: whether it was successful
        :rtype: bool
        
    .. py:method:: clear(self)

        Remove all the contained securities
        
    .. py:method:: __len__(self)  

        The number of the contained securities
        
    .. py:method:: __getitem__(self, market_code)
    
        Get the corresponding security instance by "market abbreviation + security code"
        
        :param str market_code: the format: "market abbreviation + security code", e.g. "sh000001"
        :return: the corresponding security instance; if the instance does not exist, return Null<Stock>() without raising an exception
        :rtype: Stock        
     

Other Security Information Definitions
--------------------------------------

.. py:class:: StockTypeInfo

    The stock type detail record
    
    .. py:attribute:: type : the security type
    .. py:attribute:: description : the description information
    .. py:attribute:: tick : the minimum tick
    .. py:attribute:: tick_value : the price of each tick
    .. py:attribute:: unit : the price of each minimum change, i.e. the unit price = tickValue/tick
    .. py:attribute:: precision : the price precision
    .. py:attribute:: min_trade_num : the minimum trading quantity per order
    .. py:attribute:: max_trade_num : the maximum trading quantity per order


.. py:class:: StockWeight

    The dividend record
    
    .. py:attribute:: datetime : the dividend date
    .. py:attribute:: count_as_gift : X shares sent per 10 shares
    .. py:attribute:: count_for_sell : X shares allotted per 10 shares
    .. py:attribute:: price_for_sell : the allotment price
    .. py:attribute:: bonus : the dividend per 10 shares
    .. py:attribute:: increasement : X shares converted per 10 shares
    .. py:attribute:: total_count : the total share capital (10,000 shares)
    .. py:attribute:: free_count : the circulating shares (10,000 shares)
    

.. py:class:: StockWeightList

    A wrapper of std::vector<StockWeight>, see :py:class:`StockWeight`

    .. py:method:: to_numpy(self)

        Convert to a numpy array

    .. py:method:: to_pandas(self)

        Convert to a pandas DataFrame

    .. py:method:: to_pyarrow(self)

        Convert to a pyarrow Table


.. py:class:: MarketInfo

    The market information record
    
    .. py:attribute:: market : the market abbreviation (e.g.: the Shanghai market "SH", the Shenzhen market "SZ")
    .. py:attribute:: name : the full name of the market
    .. py:attribute:: description : the description
    .. py:attribute:: code : the main index corresponding to this market, used to get the trading calendar
    .. py:attribute:: last_datetime : the last trading date of the K-line data of this market
