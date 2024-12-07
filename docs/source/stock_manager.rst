.. py:currentmodule:: hikyuu
.. highlight:: python

证券管理
========

构建K线查询条件
-----------------
    
.. py:class:: Query

    K线数据查询条件，一般在Python中使用 Query 即可，不用指明 Query。

    简化 :py:data:`Query.KType` 枚举值
    
    - Query.DAY - 日线类型
    - Query.WEEK - 周线类型
    - Query.MONTH - 月线类型
    - Query.QUARTER - 季线类型
    - Query.HALFYEAR - 半年线类型
    - Query.YEAR - 年线类型
    - Query.MIN - 1分钟线类型
    - Query.MIN5 - 5分钟线类型
    - Query.MIN15 - 15分钟线类型
    - Query.MIN30 - 30分钟线类型
    - Query.MIN60 - 60分钟线类型
    
    简化 :py:data:`Query.RecoverType` 枚举值
    
    - Query.NO_RECOVER      - 不复权
    - Query.FORWARD         - 前向复权
    - Query.BACKWARD        - 后向复权
    - Query.EQUAL_FORWARD   - 等比前向复权
    - Query.EQUAL_BACKWARD  - 等比后向复权

    .. py:attribute:: start 
    
        起始索引，当按日期查询方式创建时无效，为 constant.null_int64
        
    .. py:attribute:: end

        结束索引，当按日期查询方式创建时无效，为 constant.null_int64
        
    .. py:attribute:: start_datetime
    
        起始日期，当按索引查询方式创建时无效，为 constant.null_datetime
        
    .. py:attribute:: end_datetime
    
        结束日期，当按索引查询方式创建时无效，为 constant.null_datetime
        
    .. py:attribute:: query_type
    
        查询方式
        
    .. py:attribute:: ktype
    
        查询的K线类型
        
    .. py:attribute:: recover_type
    
        查询的复权类型
    
    .. py:data:: QueryType
    
        查询方式定义
        
        - DATE  - 按日期方式查询
        - INDEX - 按索引方式查询
    
    .. py:data:: KType
    
        K线类型枚举定义
        
        - DAY      - 日线类型
        - WEEK     - 周线类型
        - MONTH    - 月线类型
        - QUARTER  - 季线类型 
        - HALFYEAR - 半年线类型 
        - YEAR     - 年线类型 
        - MIN      - 1分钟线类型
        - MIN5     - 5分钟线类型
        - MIN15    - 15分钟线类型
        - MIN30    - 30分钟线类型
        - MIN60    - 60分钟线类型    
        
    .. py:data:: RecoverType
    
        K线复权类别枚举定义
    
        - NO_RECOVER      - 不复权
        - FORWARD         - 前向复权
        - BACKWARD        - 后向复权
        - EQUAL_FORWARD   - 等比前向复权
        - EQUAL_BACKWARD  - 等比后向复权  

        
    
StockManager/Block/Stock
-----------------------------

.. py:class:: StockManager

    证券信息管理类
    
    .. py:staticmethod:: instance()
    
        获取StockManager单例实例
        
    .. py:method:: init(self, baseInfoParam, blockParam, kdataParam, preloadParam, hkuParam)
    
        初始化
        
        :param Parameter baseInfoParam: 基础信息数据驱动参数
        :param Parameter blockParam: 板块信息数据驱动参数
        :param Parameter kdataParam: K线数据驱动参数
        :param Parameter preloadParam: 预加载参数
        :param Parameter hkuParam: 其他hikyuu参数
       
    .. py:method:: get_base_info_parameter(self)
    
        :return: 基础信息数据驱动参数
        :rtype: Parameter
        
    .. py:method:: get_block_parameter(self)

        :return: 板块信息数据驱动参数
        :rtype: Parameter
        
    .. py:method:: get_kdata_parameter(self)
    
        :return: K线数据驱动参数
        :rtype: Parameter
        
    .. py:method:: get_preload_parameter(self)
    
        :return: 预加载参数
        :rtype: Parameter
        
    .. py:method:: get_hikyuu_parameter(self)
    
        :return: 其他hikyuu参数
        :rtype: Parameter

    .. py:method:: get_context(self)

        :return: 获取当前上下文
        :rtype: StrategyContext
    
    .. py:method:: tmpdir(self)
    
        获取用于保存零时变量等的临时目录，如未配置则为当前目录 由m_config中的“tmpdir”指定

    .. py:method:: datadir(self)

        获取财务数据目录
    
    .. py:method:: get_market_list(self)
    
        获取市场简称列表
        
        :rtype: StringList
    
    .. py:method:: get_market_info(self, market)
    
        获取相应的市场信息
        
        :param string market: 指定的市场标识（市场简称）
        :return: 相应的市场信息，如果相应的市场信息不存在，则返回Null<MarketInfo>()
        :rtype: MarketInfo
    
    .. py:method:: get_stock_type_info(self, stk_type)
    
        获取相应的证券类型详细信息
        
        :param int stk_type: 证券类型，参见： :py:data:`constant`
        :return: 对应的证券类型信息，如果不存在，则返回Null<StockTypeInfo>()
        :rtype: StockTypeInfo
        
    .. py:method:: get_stock(self, querystr)
    
        根据"市场简称证券代码"获取对应的证券实例
        
        :param str querystr: 格式：“市场简称证券代码”，如"sh000001"
        :return: 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
        :rtype: Stock
    
    .. py:method:: __getitem__

        同 get_stock
    
    .. py:method:: get_block(self, category, name)
    
        获取预定义的板块
        
        :param str category: 板块分类
        :param str name: 板块名称
        :return: 板块，如找不到返回空Block
        :rtype: Block

    .. py:method:: add_block(self, block)

        将新增独立的板块加入到数据库系统中。注意，如 block 发生变化，需要调用 save_block 进行保存。

        :param Block block: 新增的板块

    .. py:method:: remove_block(self, block)

        从数据库系统中删除板块

        :param Block block: 要删除的板块
        
    .. py:method:: get_block_list(self[, category])
    
        获取指定分类的板块列表
        
        :param str category: 板块分类
        :return: 板块列表
        :rtype: BlockList
    
    .. py:method:: get_trading_calendar(self, query[, market='SH'])
    
        获取指定市场的交易日日历
        
        :param Query query: Query查询条件
        :param str market: 市场简称
        :return: 日期列表
        :rtype: DatetimeList
        
    .. py:method:: is_holiday(self, d)

        判断日期是否为节假日

        :param Datetime d: 待判定的日期

    .. py:method:: add_temp_csv_stock(self, code, day_filename, min_filename[, tick=0.01, tick_value=0.01, precision=2, min_trade_num = 1, max_trade_num=1000000])

        从CSV文件（K线数据）增加临时的Stock，可用于只有CSV格式的K线数据时，进行临时测试。

        添加的 stock 对应的 market 为 "TMP", 如需通过 sm 获取，需加入 tmp，如：sm['tmp0001']
        
        CSV文件第一行为标题，需含有 Datetime（或Date、日期）、OPEN（或开盘价）、HIGH（或最高价）、LOW（或最低价）、CLOSE（或收盘价）、AMOUNT（或成交金额）、VOLUME（或VOL、COUNT、成交量）。
        
        :param str code: 自行编号的证券代码，不能和已有的Stock相同，否则将返回Null<Stock>。
        :param str day_filename: 日线CSV文件名
        :param str min_filename: 分钟线CSV文件名
        :param float tick: 最小跳动量，默认0.01
        :param float tick_value: 最小跳动量价值，默认0.01
        :param int precision: 价格精度，默认2
        :param int min_trade_num: 单笔最小交易量，默认1
        :param int min_trade_num: 单笔最大交易量，默认1000000
        :return: 加入的Stock
        :rtype: Stock

    .. py:method:: remove_temp_csv_stock(self, code)
    
        移除增加的临时Stock
        
        :param str code: 创建时自定义的编码

    .. py:method:: add_stock(self, stock)

        谨慎调用！！！仅供增加某些临时的外部 Stock, 通常配合 Stock.set_krecord_list 方法直接使用外部来源的数据

        :param Stock stock: sm 外部自行创建的 Stock

    .. py:method:: remove_stock(self, market_code)

        从 sm 中移除 market_code 代表的证券，谨慎使用！！！通常用于移除临时增加的外部 Stock

        :param str market_code: 证券市场标识

    .. py:method:: get_history_finance_all_fields(self)

        获取所有历史财务信息字段及其索引

    .. py:method:: get_history_finance_field_index(self, name)

        根据字段名称，获取历史财务信息相应字段索引

    .. py:method:: get_history_finance_field_name(self, index)

        根据字段索引，获取历史财务信息相应字段名


.. py:class:: Stock

    证券对象

    .. py:attribute:: id : 内部id，一般用于作为map的键值使用
    .. py:attribute:: market : 获取所属市场简称，市场简称是市场的唯一标识
    .. py:attribute:: code : 获取证券代码
    .. py:attribute:: market_code : 市场简称+证券代码，如: sh000001
    .. py:attribute:: name : 获取证券名称
    .. py:attribute:: type 
    
        获取证券类型，参见：:py:data:`constant`
        
    .. py:attribute:: valid : 该证券当前是否有效
    .. py:attribute:: start_datetime : 证券起始日期
    .. py:attribute:: last_datetime : 证券最后日期
    .. py:attribute:: tick : 最小跳动量
    .. py:attribute:: tick_value : 最小跳动量价值
    .. py:attribute:: unit : 每单位价值 = tickValue / tick
    .. py:attribute:: precision : 价格精度
    .. py:attribute:: atom : 最小交易数量，同minTradeNumber
    .. py:attribute:: min_trade_number : 最小交易数量
    .. py:attribute:: max_trade_number : 最大交易数量

    .. py:method:: is_null(self)
    
        是否为Null
    
        :rtype: bool
    
    .. py:method:: get_kdata(self, query)
    
        获取K线数据
        
        :param Query query: 查询条件
        :return: 满足查询条件的K线数据
        :rtype: KData
    
    .. py:method:: get_count(self[, ktype=Query.DAY])
    
        获取不同类型K线数据量
        
        :param Query.KType ktype: K线数据类别
        :return: K线记录数
        :rtype: int
    
    .. py:method:: get_market_value(self, date, ktype)
    
        获取指定时刻的市值，即小于等于指定时刻的最后一条记录的收盘价
        
        :param Datetime date: 指定时刻
        :param Query.KType ktype: K线数据类别
        :return: 指定时刻的市值
        :rtype: float
    
    .. py:method:: get_krecord(self, pos[, ktype=Query.DAY])
    
        获取指定索引的K线数据记录，未作越界检查
        
        :param int pos | Datetime datetime: 指定的索引位置，或日期
        :param Query.KType ktype: K线数据类别
        :return: K线记录
        :rtype: KRecord
    
    
    .. py:method:: get_krecord_list(self, start, end, ktype)
    
        获取K线记录 [start, end)，一般不直接使用，用getKData替代
        
        :param int start: 起始位置
        :param int end: 结束位置
        :param Query.KType ktype: K线类别
        :return: K线记录列表
        :rtype: KRecordList
    
    .. py:method:: get_datetime_list(self, query)
    
        获取日期列表
        
        :param Query query: 查询条件
        :rtype: DatetimeList

        get_datetime_list(self, start, end, ktype)
    
        获取日期列表
        
        :param int start: 起始位置
        :param ind end: 结束位置
        :param Query.KType ktype: K线类型
        :rtype: DatetimeList
        
    .. py:method:: get_timeline_list(self, query)
    
        获取分时线数据
        
        :param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）
        :rtype: TimeLineList
    
    .. py:method:: get_trans_list(self, query)
    
        获取历史分笔数据
        
        :param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）
        :rtype: TransList

    .. py:method:: get_weight(self[, start, end])
    
        获取指定时间段[start,end)内的权息信息。未指定起始、结束时刻时，获取全部权息记录。
        
        :param Datetime start: 起始时刻
        :param Datetime end: 结束时刻
        :rtype: StockWeightList
        
    .. py:method:: get_finance_info(self)
    
        获取当前财务信息
        
        :rtype: Parameter
        
    .. py:method:: get_history_finance_info(self, date)
    
        获取历史财务信息, 字段含义参见：`<https://hikyuu.org/finance_fields.html>`_
        
        :param Datetime date: 指定日期必须是0331、0630、0930、1231，如 Datetime(201109300000)
        :rtype: list
    
    .. py:method:: set_krecord_list(self, krecord_list)

        谨慎调用！！！直接设置当前内存 KRecordList, 仅供需临时增加的外部 Stock 设置 K 线数据

        :param sequence krecord_list: 一个可迭代变量获取 KRecord 实例的对象，如: list (仅包含 KRecord 实例)

    .. py:method:: realtime_update(self, krecord)
    
        （临时函数）只用于更新内存缓存中的日线数据
        
        :param KRecord krecord: 新增的实时K线记录
        
    .. py:method:: load_kdata_to_buffer(self, ktype)
    
        将指定类别的K线数据加载至内存缓存
        
        :param Query.KType ktype: K线类型

    .. py:method:: release_kdata_buffer(self, ktype)
    
        释放指定类别的内存K线数据
        
        :param Query.KType ktype: K线类型

    .. py:method:: get_belong_to_block_list(self[, category=None])
    
        获取所属板块列表

        :param str category: 指定的板块分类，为 None 时，返回所有板块分类下的所属板块
        :rtype: list    
    
    
.. py:class:: Block

    板块类，可视为证券的容器
    
    .. py:attribute:: category : 板块分类
    .. py:attribute:: name : 板块名称
    .. py:attribute:: index_stock: 对应指数（可能为空 Stock）
    
    .. py:method:: __init__(self, category, name):
    
        构建一个新的板块实例，并指定其板块分类及板块名称
    
        :param str category: 板块分类
        :param srt name: 板块名称

    .. py:method:: __init__(self, block):
    
        通过其他板块实例构建新的板块实例
    
        :param Block block: 板块实例
    
    .. py:method:: size(self)
    
        包含的证券数量
        
    .. py:method:: empty(self)
    
        是否为空
        
    .. py:method:: get(self, market_code)

        根据"市场简称证券代码"获取对应的证券实例
        
        :param str querystr: 格式：“市场简称证券代码”，如"sh000001"
        :return: 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
        :rtype: Stock        

    .. py:method:: add(self, stock)
    
        加入指定的证券
        
        :param Stock stock: 待加入的证券
        :return: 是否成功加入
        :rtype: bool
        
        add(self, market_code)
    
        根据"市场简称证券代码"加入指定的证券
        
        :param str market_code: 市场简称证券代码
        :return: 是否成功加入
        :rtype: bool

    .. py:method:: remove(self, stock)
    
        移除指定证券
        
        :param Stock stock: 指定的证券
        :return: 是否成功
        :rtype: bool
        
        remove(self, market_code)
    
        移除指定证券
        
        :param str market_code: 市场简称证券代码
        :return: 是否成功
        :rtype: bool
        
    .. py:method:: clear(self)

        移除包含的所有证券
        
    .. py:method:: __len__(self)  

        包含的证券数量
        
    .. py:method:: __getitem__(self, market_code)
    
        根据"市场简称证券代码"获取对应的证券实例
        
        :param str querystr: 格式：“市场简称证券代码”，如"sh000001"
        :return: 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
        :rtype: Stock        

     

其它证券信息定义
------------------

.. py:class:: StockTypeInfo

    股票类型详情记录
    
    .. py:attribute:: type : 证券类型
    .. py:attribute:: description : 描述信息
    .. py:attribute:: tick : 最小跳动量
    .. py:attribute:: tick_value : 每一个tick价格
    .. py:attribute:: unit : 每最小变动量价格，即单位价格 = tickValue/tick
    .. py:attribute:: precision : 价格精度
    .. py:attribute:: min_trade_num : 每笔最小交易量
    .. py:attribute:: max_trade_num : 每笔最大交易量


.. py:class:: StockWeight

    权息记录
    
    .. py:attribute:: datetime : 权息日期
    .. py:attribute:: count_as_gift : 每10股送X股
    .. py:attribute:: count_for_sell : 每10股配X股
    .. py:attribute:: price_for_sell : 配股价
    .. py:attribute:: bonus : 每10股红利
    .. py:attribute:: increasement : 每10股转增X股
    .. py:attribute:: total_count : 总股本（万股）
    .. py:attribute:: free_count : 流通股（万股）
    

.. py:class:: StockWeightList

    std::vector<StockWeight> 包装，见 :py:class:`StockWeight`


.. py:class:: MarketInfo

    市场信息记录
    
    .. py:attribute:: market : 市场简称（如：沪市“SH”, 深市“SZ”）
    .. py:attribute:: name : 市场全称
    .. py:attribute:: description :描述说明
    .. py:attribute:: code : 该市场对应的主要指数，用于获取交易日历
    .. py:attribute:: last_datetime : 该市场K线数据最后交易日期




