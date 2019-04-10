.. py:currentmodule:: hikyuu
.. highlightlang:: python

证券管理
========

构建K线查询条件
-----------------

.. py:function:: QueryByDate([start=None, end=None, kType=Query.DAY, recoverType=Query.NO_RECOVER])        

    构建按日期 [start, end) 方式获取K线数据条件
    
    :param Datetime start: 起始日期
    :param Datetime end: 结束日期
    :param KQuery.KType kType: K线数据类型（如日线、分钟线等）
    :param KQuery.RecoverType recoverType: 复权类型
    :return: 查询条件
    :rtype: KQuery

    
.. py:class:: QueryByIndex([start=0, end=None, kType=KQuery.KType.DAT, recoverType=KQuery.RecoverType.NO_RECOVER])

    构建按索引 [start, end) 方式获取K线数据条件，等同于直接使用 Query 构造
    
    :param ind start: 起始日期
    :param ind end: 结束日期
    :param KQuery.KType kType: K线数据类型（如日线、分钟线等）
    :param KQuery.RecoverType recoverType: 复权类型
    :return: 查询条件
    :rtype: KQuery
    
    
.. py:class:: Query

    对 KQuery 的简单包装，并简化定义相关常量，可视同为 :py:class:`KQuery`
    
    简化 :py:data:`KQuery.KType` 枚举值
    
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
    
    简化 :py:data:`KQuery.RecoverType` 枚举值
    
    - Query.NO_RECOVER      - 不复权
    - Query.FORWARD         - 前向复权
    - Query.BACKWARD        - 后向复权
    - Query.EQUAL_FORWARD   - 等比前向复权
    - Query.EQUAL_BACKWARD  - 等比后向复权

    
.. py:class:: KQuery

    K线数据查询条件，一般在Python中使用 Query 即可，不用指明 KQuery。

    .. py:attribute:: start 
    
        起始索引，当按日期查询方式创建时无效，为 constant.null_int64
        
    .. py:attribute:: end

        结束索引，当按日期查询方式创建时无效，为 constant.null_int64
        
    .. py:attribute:: startDatetime
    
        起始日期，当按索引查询方式创建时无效，为 constant.null_datetime
        
    .. py:attribute:: endDatetime
    
        结束日期，当按索引查询方式创建时无效，为 constant.null_datetime
        
    .. py:attribute:: queryType
    
        查询方式
        
    .. py:attribute:: kType
    
        查询的K线类型
        
    .. py:attribute:: recoverType
    
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

    .. py:staticmethod:: getQueryTypeName(queryType)
    
        获取queryType名称，用于显示输出
        
        :param KQuery.QueryType queryType: 查询类型
        :rtype: str
    
    .. py:staticmethod:: getKTypeName(kType)
    
        获取KType名称，用于显示输出
        
        :param KQuery.KType kType: K线类型
        :rtype: str
    
    .. py:staticmethod:: getRecoverTypeName(recoverType)
    
        获取recoverType名称，用于显示输出
        
        :param KQuery.RecoverType recoverType: 复权类型
        :rtype: str
        
    .. py:staticmethod:: getQueryTypeEnum(queryType)
    
        根据字符串名称获取相应的queryType枚举值
        
        :param str queryType: 字符串名称，如“DATE”
        :rtype: KQuery.QueryType
        
    .. py:staticmethod:: getKTypeEnum(ktype)
    
        根据字符串名称，获取相应的枚举值 
        
        :param str ktype: 字符串名称，如“DAY”
        :rtype: KQuery.KType
        
    .. py:staticmethod:: getRecoverTypeEnum(recoverType)

        根据字符串名称，获取相应的枚举值
        
        :param str recoverType: 字符串名称，如“NO_RECOVER”
        :rtype: KQuery.RecoverType
        
    
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
       
    .. py:method:: getBaseInfoDriverParameter(self)
    
        :return: 基础信息数据驱动参数
        :rtype: Parameter
        
    .. py:method:: getBlockDriverParameter(self)

        :return: 板块信息数据驱动参数
        :rtype: Parameter
        
    .. py:method:: getKDataDriverParameter(self)
    
        :return: K线数据驱动参数
        :rtype: Parameter
        
    .. py:method:: getPreloadParameter(self)
    
        :return: 预加载参数
        :rtype: Parameter
        
    .. py:method:: getHikyuuParameter(self)
    
        :return: 其他hikyuu参数
        :rtype: Parameter
    
    .. py:method:: tmpdir(self)
    
        获取用于保存零时变量等的临时目录，如未配置则为当前目录 由m_config中的“tmpdir”指定
    
    .. py:method:: getAllMarket(self)
    
        获取市场简称列表
        
        :rtype: StringList
    
    .. py:method:: getMarketInfo(self, market)
    
        获取相应的市场信息
        
        :param string market: 指定的市场标识（市场简称）
        :return: 相应的市场信息，如果相应的市场信息不存在，则返回Null<MarketInfo>()
        :rtype: MarketInfo
    
    .. py:method:: getStockTypeInfo(self, stk_type)
    
        获取相应的证券类型详细信息
        
        :param int stk_type: 证券类型，参见： :py:data:`constant`
        :return: 对应的证券类型信息，如果不存在，则返回Null<StockTypeInfo>()
        :rtype: StockTypeInfo
        
    .. py:method:: size(self)
    
        获取证券数量
        
    .. py:method:: getStock(self, querystr)
    
        根据"市场简称证券代码"获取对应的证券实例
        
        :param str querystr: 格式：“市场简称证券代码”，如"sh000001"
        :return: 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
        :rtype: Stock
    
    .. py:method:: __getitem__

        同 getStock
    
    .. py:method:: getBlock(self, category, name)
    
        获取预定义的板块
        
        :param str category: 板块分类
        :param str name: 板块名称
        :return: 板块，如找不到返回空Block
        :rtype: Block
        
    .. py:method:: getBlockList(self[, category])
    
        获取指定分类的板块列表
        
        :param str category: 板块分类
        :return: 板块列表
        :rtype: BlockList
    
    .. py:method:: getTradingCalendar(self, query[, market='SH'])
    
        获取指定市场的交易日日历
        
        :param KQuery query: Query查询条件
        :param str market: 市场简称
        :return: 日期列表
        :rtype: DatetimeList
        
    .. py:method:: addTempCsvStock(self, code, day_filename, min_filename[, tick=0.01, tickValue=0.01, precision=2, minTradeNumber = 1, maxTradeNumber=1000000])

        从CSV文件（K线数据）增加临时的Stock，可用于只有CSV格式的K线数据时，进行临时测试。        
        
        CSV文件第一行为标题，需含有 Datetime（或Date、日期）、OPEN（或开盘价）、HIGH（或最高价）、LOW（或最低价）、CLOSE（或收盘价）、AMOUNT（或成交金额）、VOLUME（或VOL、COUNT、成交量）。
        
        :param str code: 自行编号的证券代码，不能和已有的Stock相同，否则将返回Null<Stock>
        :param str day_filename: 日线CSV文件名
        :param str min_filename: 分钟线CSV文件名
        :param float tick: 最小跳动量，默认0.01
        :param float tickValue: 最小跳动量价值，默认0.01
        :param int precision: 价格精度，默认2
        :param int minTradeNumber: 单笔最小交易量，默认1
        :param int maxTradeNumber: 单笔最大交易量，默认1000000
        :return: 加入的Stock
        :rtype: Stock

    .. py:method:: removeTempCsvStock(self, code)
    
        移除增加的临时Stock
        
        :param str code: 创建时自定义的编码
   

.. py:class:: Stock

    Stock基类

    .. py:attribute:: id : 内部id，一般用于作为map的键值使用
    .. py:attribute:: market : 获取所属市场简称，市场简称是市场的唯一标识
    .. py:attribute:: code : 获取证券代码
    .. py:attribute:: market_code : 市场简称+证券代码，如: sh000001
    .. py:attribute:: name : 获取证券名称
    .. py:attribute:: type 
    
        获取证券类型，参见：:py:data:`constant`
        
    .. py:attribute:: valid : 该证券当前是否有效
    .. py:attribute:: startDatetime : 证券起始日期
    .. py:attribute:: lastDatetime : 证券最后日期
    .. py:attribute:: tick : 最小跳动量
    .. py:attribute:: tickValue : 最小跳动量价值
    .. py:attribute:: unit : 每单位价值 = tickValue / tick
    .. py:attribute:: precision : 价格精度
    .. py:attribute:: atom : 最小交易数量，同minTradeNumber
    .. py:attribute:: minTradeNumber : 最小交易数量
    .. py:attribute:: maxTradeNumber : 最大交易数量

    .. py:method:: isNull(self)
    
        是否为Null
    
        :rtype: bool
    
    .. py:method:: getKData(self, query)
    
        获取K线数据
        
        :param Query query: 查询条件
        :return: 满足查询条件的K线数据
        :rtype: KData
    
    .. py:method:: getCount(self[, ktype=Query.DAY])
    
        获取不同类型K线数据量
        
        :param KQuery.KType ktype: K线数据类别
        :return: K线记录数
        :rtype: int
    
    .. py:method:: getMarketValue(self, datetime, ktype)
    
        获取指定时刻的市值，即小于等于指定时刻的最后一条记录的收盘价
        
        :param Datetime datetime: 指定时刻
        :param KQuery.KType ktype: K线数据类别
        :return: 指定时刻的市值
        :rtype: float
    
    .. py:method:: getKRecord(self, pos[, ktype=Query.DAY])
    
        获取指定索引的K线数据记录，未作越界检查
        
        :param int pos: 指定的索引位置
        :param KQuery.KType ktype: K线数据类别
        :return: K线记录
        :rtype: KRecord
    
    .. py:method:: getKRecordByDate(self, datetime[, ktype=Query.DAY])
    
        根据数据类型（日线/周线等），获取指定时刻的KRecord
        
        :param Datetime datetime: 指定时刻
        :param KQuery.KType ktype: K线数据类别
        :return: K线记录
        :rtype: KRecord
    
    .. py:method:: getKRecordList(self, start, end, ktype)
    
        获取K线记录 [start, end)，一般不直接使用，用getKData替代
        
        :param int start: 起始位置
        :param int end: 结束位置
        :param KQuery.KType ktype: K线类别
        :return: K线记录列表
        :rtype: KRecordList
    
    .. py:method:: getDatetimeList(self, query)
    
        获取日期列表
        
        :param Query query: 查询条件
        :rtype: DatetimeList

    .. py:method:: getDatetimeList(self, start, end, ktype)
    
        获取日期列表
        
        :param int start: 起始位置
        :param ind end: 结束位置
        :param KQuery.KType ktype: K线类型
        :rtype: DatetimeList
        
    .. py:method:: getTimeLineList(self, query)
    
        获取分时线数据
        
        :param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）
        :rtype: TimeLineList
    
    .. py:method:: getTransList(self, query)
    
        获取历史分笔数据
        
        :param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）
        :rtype: TransList

    .. py:method:: getWeight(self[, start, end])
    
        获取指定时间段[start,end)内的权息信息。未指定起始、结束时刻时，获取全部权息记录。
        
        :param Datetime start: 起始时刻
        :param Datetime end: 结束时刻
        :rtype: StockWeightList
        
    .. py:method:: getFinanceInfo(self)
    
        获取当前财务信息
        
        :rtype: Parameter
        
    .. py:method:: getHistoryFinanceInfo(self, date)
    
        获取历史财务信息, 字段含义参见：`<https://hikyuu.org/finance_fields.html>`_
        
        :param Datetime date: 指定日期必须是0331、0630、0930、1231，如 Datetime(201109300000)
        :rtype: PriceList
    
    .. py:method:: realtimeUpdate(self, krecord)
    
        （临时函数）只用于更新内存缓存中的日线数据
        
        :param KRecord krecord: 新增的实时K线记录
        
    .. py:method:: loadKDataToBuffer(self, ktype)
    
        将指定类别的K线数据加载至内存缓存
        
        :param KQuery.KType ktype: K线类型

    .. py:method:: releaseKDataBuffer(self, ktype)
    
        释放指定类别的内存K线数据
        
        :param KQuery.KType ktype: K线类型
    
    
.. py:class:: Block

    板块类，可视为证券的容器
    
    .. py:attribute:: category : 板块分类
    .. py:attribute:: name : 板块名称
    
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
        
    .. py:method:: add(self, market_code)
    
        根据"市场简称证券代码"加入指定的证券
        
        :param str market_code: 市场简称证券代码
        :return: 是否成功加入
        :rtype: bool

    .. py:method:: remove(self, stock)
    
        移除指定证券
        
        :param Stock stock: 指定的证券
        :return: 是否成功
        :rtype: bool
        
    .. py:method:: remove(self, market_code)
    
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
    .. py:attribute:: tickValue : 每一个tick价格
    .. py:attribute:: unit : 每最小变动量价格，即单位价格 = tickValue/tick
    .. py:attribute:: precision : 价格精度
    .. py:attribute:: minTradeNumber : 每笔最小交易量
    .. py:attribute:: maxTradeNumber : 每笔最大交易量


.. py:class:: StockWeight

    权息记录
    
    .. py:attribute:: datetime : 权息日期
    .. py:attribute:: countAsGift : 每10股送X股
    .. py:attribute:: countForSell : 每10股配X股
    .. py:attribute:: priceForSell : 配股价
    .. py:attribute:: bonus : 每10股红利
    .. py:attribute:: increasement : 每10股转增X股
    .. py:attribute:: totalCount : 总股本（万股）
    .. py:attribute:: freeCount : 流通股（万股）
    

.. py:class:: StockWeightList

    std::vector<StockWeight> 包装，见 :py:class:`StockWeight`


.. py:class:: MarketInfo

    市场信息记录
    
    .. py:attribute:: market : 市场简称（如：沪市“SH”, 深市“SZ”）
    .. py:attribute:: name : 市场全称
    .. py:attribute:: description :描述说明
    .. py:attribute:: code : 该市场对应的主要指数，用于获取交易日历
    .. py:attribute:: lastDate : 该市场K线数据最后交易日期




