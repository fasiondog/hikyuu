#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# pylint: disable=E0602

from hikyuu._hikyuu import *

#------------------------------------------------------------------
# from _DataType.cpp
#------------------------------------------------------------------

DatetimeList.__doc__ = """日期序列，对应C++中的std::vector<Datetime>"""

PriceList.__doc__ = """价格序列，其中价格使用double表示，对应C++中的std::vector<double>。"""

toPriceList.__doc__ = """
将Python的可迭代对象如 list、tuple 转化为 PriceList
    
    :param arg: 待转化的Python序列
    :rtype: PriceList
"""

StringList.__doc__ = """字符串列表，对应C++中的std::vector<String>"""

#------------------------------------------------------------------
# from _Datetime.cpp
#------------------------------------------------------------------

Datetime.__doc__ = """日期时间类，精确到微秒"""

Datetime.isNull.__doc__ = """是否是Null值, 即是否等于 constant.null_datetime"""

Datetime.year.__doc__ = """年，如果是 Null 将抛出异常"""
Datetime.month.__doc__ = """月，如果是 Null 将抛出异常"""
Datetime.day.__doc__ = """日，如果是 Null 将抛出异常"""
Datetime.hour.__doc__ = """时，如果是 Null 将抛出异常"""
Datetime.minute.__doc__ = """分，如果是 Null 将抛出异常"""
Datetime.second.__doc__ = """秒，如果是 Null 将抛出异常"""
Datetime.millisecond.__doc__ = """毫秒，如果是 Null 将抛出异常"""
Datetime.microsecond.__doc__ = """微秒，如果是 Null 将抛出异常"""
Datetime.number.__doc__ = """YYYYMMDDHHMM 形式的整数，精度只到分钟"""

Datetime.dayOfWeek.__doc__ = """
dayOfWeek(self)
    
    返回是一周中的第几天，周日为0，周一为1
        
    :rtype: int
"""

Datetime.dateOfWeek.__doc__ = """
dateOfWeek(self, day)
    
    返回指定的本周中第几天的日期，周日为0天，周六为第6天
        
    :param int day: 指明本周的第几天，如小于则认为为第0天，如大于6则认为为第6天
    :rtype: Datetime
"""

Datetime.dayOfYear.__doc__ = """
dayOfYear(self)
    
    返回一年中的第几天，1月1日为一年中的第1天
        
    :rtype: int
"""

Datetime.startOfDay.__doc__ = """
startOfDay(self)

    返回当天 0点0分0秒
"""

Datetime.endOfDay.__doc__ = """
endOfDay(self)

    返回当日 23点59分59秒
"""

Datetime.startOfWeek.__doc__ = """
startOfWeek(self)
    
    返回周起始日期（周一）
"""

Datetime.endOfWeek.__doc__ = """
endOfWeek(self)
    
    返回周结束日期（周日）
"""

Datetime.startOfMonth.__doc__ = """
startOfMonth(self)
    
    返回月度起始日期
"""

Datetime.endOfMonth.__doc__ = """
endOfMonth(self)
    
    返回月末最后一天日期
"""

Datetime.startOfQuarter.__doc__ = """
startOfQuarter(self)
    
    返回季度起始日期
"""

Datetime.endOfQuarter.__doc__ = """
endOfQuarter(self)
    
    返回季度结束日期
"""

Datetime.startOfHalfyear.__doc__ = """
startOfHalfyear(self)
    
    返回半年度起始日期
"""

Datetime.endOfHalfyear.__doc__ = """
endOfHalfyear(self)
    
    返回半年度结束日期
"""

Datetime.startOfYear.__doc__ = """
startOfYear(self)
    
    返回年度起始日期
"""

Datetime.endOfYear.__doc__ = """
endOfYear(self)
    
    返回年度结束日期
"""

Datetime.nextDay.__doc__ = """
nextDay(self)
    
    返回下一自然日
"""

Datetime.nextWeek.__doc__ = """
nextWeek(self)
    
    返回下周周一日期
"""

Datetime.nextMonth.__doc__ = """
nextMonth(self)
    
    返回下月首日日期
"""

Datetime.nextQuarter.__doc__ = """
nextQuarter(self)
    
    返回下一季度首日日期
"""

Datetime.nextHalfyear.__doc__ = """
nextHalfyear(self)
    
    返回下一半年度首日日期
"""

Datetime.nextYear.__doc__ = """
nextYear(self)
    
    返回下一年度首日日期
"""

Datetime.preDay.__doc__ = """
preDay(self)
    
    返回前一自然日日期
"""

Datetime.preWeek.__doc__ = """
preWeek(self)
    
    返回上周周一日期
"""

Datetime.preMonth.__doc__ = """
preMonth(self)
    
    返回上月首日日期
"""

Datetime.preQuarter.__doc__ = """
preQuarter(self)
    
    返回上一季度首日日期
"""

Datetime.preHalfyear.__doc__ = """
preHalfyear(self)
    
    返回上一半年度首日日期
"""

Datetime.preYear.__doc__ = """
preYear(self)
    
    返回上一年度首日日期
"""

Datetime.max.__doc__ = """
max()
    
    获取支持的最大日期
"""
Datetime.min.__doc__ = """
min()

    获取支持的最小日期
"""

Datetime.now.__doc__ = """
now()
    
    获取当前日期时间
"""

Datetime.today.__doc__ = """
today()
    
    获取当前的日期
"""

getDateRange.__doc__ = """\n
获取指定 [start, end) 日期时间范围的自然日日历日期列表，仅支持到日

:param Datetime start: 起始日期
:param Datetime end: 结束日期
:rtype: DatetimeList
"""

#------------------------------------------------------------------
# from _TimeDelta.cpp
#------------------------------------------------------------------

TimeDelta.__doc__ = """
    时间时长，用于时间计算。可通过以下方式构建：

    - 通过 datetime.timedelta 构建。TimdeDelta(timedelta实例)
    - TimeDelta(days=0, hours=0, minutes=0, seconds=0, milliseconds=0, microseconds=0)

        - -99999999 <= days <= 99999999
        - -100000 <= hours <= 100000
        - -100000 <= minutes <= 100000
        - -8639900 <= seconds <= 8639900
        - -86399000000 <= milliseconds <= 86399000000
        - -86399000000 <= microseconds <= 86399000000

    以上参数限制，主要为防止求总微秒数时可能出现溢出的情况。如只使用一个参数不希望存在上述限制时，可使用快捷函数：
    Days, Hours, Minutes, Seconds, Milliseconds, Microseconds
"""

TimeDelta.days.__doc__ = """天数 [-99999999, 99999999]"""
TimeDelta.hours.__doc__ = """小时数 [0, 23]"""
TimeDelta.minutes.__doc__ = """分钟数 [0, 59]"""
TimeDelta.seconds.__doc__ = """秒数 [0, 59]"""
TimeDelta.milliseconds.__doc__ = """毫秒数 [0, 999]"""
TimeDelta.microseconds.__doc__ = """微秒数 [0, 999]"""
TimeDelta.ticks.__doc__ = """同总微秒数"""

TimeDelta.isNegative.__doc__ = """
isNegative()

    是否为负时长

    :rtype: bool
"""

TimeDelta.max.__doc__ = """
max()

    支持的最大时长

    :return: TimeDelta(99999999, 23, 59, 59, 999, 999)
"""

TimeDelta.min.__doc__ = """
min()

    支持的最小时长

    :return: TimeDelta(-99999999, 0, 0, 0, 0, 0)
"""

TimeDelta.resolution.__doc__ = """
resolution()

    支持的最小精度
        
    :return: TimeDelta(0, 0, 0, 0, 0, 1)
"""

TimeDelta.maxTicks.__doc__ = """
maxTicks()

    支持的最大 ticks （即微秒数）

    :rtype: int
"""

TimeDelta.minTicks.__doc__ = """
minTicks()

    支持的最小 ticks （即微秒数）

    :rtype: int
"""

TimeDelta.fromTicks.__doc__ = """
fromTicks(ticks)

    使用 ticks（即微秒数） 值创建

    :param int ticks: 微秒数
    :rtype: TimeDelta
"""

#------------------------------------------------------------------
# from _util.cpp
#------------------------------------------------------------------

roundUp.__doc__ = """
roundUp(arg1[, arg2=0])

    向上截取，如10.1截取后为11
    
    :param float arg1: 待处理数据
    :param int arg2: 保留小数位数
    :return: 处理过的数据
"""

roundDown.__doc__ = """
roundDown(arg1[, arg2=0])

    向下截取，如10.1截取后为10
    
    :param float arg1: 待处理数据
    :param int arg2: 保留小数位数
    :return: 处理过的数据
"""

#------------------------------------------------------------------
# from _Log.cpp
#------------------------------------------------------------------

get_log_level.__doc__ = """获取当前日志打印级别"""

set_log_level.__doc__ = """设置当前日志打印级别"""

LOG_LEVEL.__doc__ = """
    - DEBUG
    - TRACE
    - INFO
    - WARN
    - ERROR
    - FATAL
    - NO_PRINT
"""

#------------------------------------------------------------------
# from _MarketInfo.cpp
#------------------------------------------------------------------

MarketInfo.__doc__ = """
    市场信息记录
    
    .. py:attribute:: market : 市场简称（如：沪市“SH”, 深市“SZ”）
    .. py:attribute:: name : 市场全称
    .. py:attribute:: description :描述说明
    .. py:attribute:: code : 该市场对应的主要指数，用于获取交易日历
    .. py:attribute:: lastDate : 该市场K线数据最后交易日期
"""

#------------------------------------------------------------------
# from _StockTypeInfo.cpp
#------------------------------------------------------------------

StockTypeInfo.__doc__ = """
    股票类型详情记录
    
    .. py:attribute:: type : 证券类型
    .. py:attribute:: description : 描述信息
    .. py:attribute:: tick : 最小跳动量
    .. py:attribute:: tickValue : 每一个tick价格
    .. py:attribute:: unit : 每最小变动量价格，即单位价格 = tickValue/tick
    .. py:attribute:: precision : 价格精度
    .. py:attribute:: minTradeNumber : 每笔最小交易量
    .. py:attribute:: maxTradeNumber : 每笔最大交易量
"""

#------------------------------------------------------------------
# from _StockWeight.cpp
#------------------------------------------------------------------

StockWeight.__doc__ = """
    权息记录
    
    .. py:attribute:: datetime : 权息日期
    .. py:attribute:: countAsGift : 每10股送X股
    .. py:attribute:: countForSell : 每10股配X股
    .. py:attribute:: priceForSell : 配股价
    .. py:attribute:: bonus : 每10股红利
    .. py:attribute:: increasement : 每10股转增X股
    .. py:attribute:: totalCount : 总股本（万股）
    .. py:attribute:: freeCount : 流通股（万股）
"""

StockWeightList.__doc__ = """std::vector<StockWeight> 包装"""

#------------------------------------------------------------------
# from _StockManager.cpp
#------------------------------------------------------------------

StockManager.__doc__ = """证券信息管理类"""

StockManager.instance.__doc__ = """
instance()
    
    获取StockManager单例实例
"""

StockManager.tmpdir.__doc__ = """
tmpdir()

    获取用于保存零时变量等的临时目录，如未配置则为当前目录 由m_config中的“tmpdir”指定
"""

StockManager.getAllMarket.__doc__ = """
getAllMarket()
    
    获取市场简称列表
        
    :rtype: StringList
"""

StockManager.getMarketInfo.__doc__ = """
getMarketInfo(market)
    
    获取相应的市场信息
       
    :param string market: 指定的市场标识（市场简称）
    :return: 相应的市场信息，如果相应的市场信息不存在，则返回Null<MarketInfo>()
    :rtype: MarketInfo
"""

StockManager.getStockTypeInfo.__doc__ = """
getStockTypeInfo(stk_type)

    获取相应的证券类型详细信息
            
    :param int stk_type: 证券类型，参见： :py:data:`constant`
    :return: 对应的证券类型信息，如果不存在，则返回Null<StockTypeInfo>()
    :rtype: StockTypeInfo
"""

StockManager.size.__doc__ = """\n
size()
    
    获取证券数量
"""

StockManager.__len__.__doc__ = """\n
    获取证券数量
"""

StockManager.getStock.__doc__ = """
getStock(querystr)

    根据"市场简称证券代码"获取对应的证券实例
            
    :param str querystr: 格式：“市场简称证券代码”，如"sh000001"
    :return: 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
    :rtype: Stock
"""

StockManager.__getitem__.__doc__ = """
同 getStock
"""

StockManager.getBlock.__doc__ = """
getBlock(category, name)

    获取预定义的板块
            
    :param str category: 板块分类
    :param str name: 板块名称
    :return: 板块，如找不到返回空Block
    :rtype: Block
"""

StockManager.getBlockList.__doc__ = """
getBlockList([category])

    获取指定分类的板块列表
            
    :param str category: 板块分类
    :return: 板块列表
    :rtype: BlockList
"""

StockManager.getTradingCalendar.__doc__ = """
getTradingCalendar(query[, market='SH'])
    
    获取指定市场的交易日日历
        
    :param KQuery query: Query查询条件
    :param str market: 市场简称
    :return: 日期列表
    :rtype: DatetimeList
"""

StockManager.addTempCsvStock.__doc__ = """
addTempCsvStock(code, day_filename, min_filename[, tick=0.01, tickValue=0.01, precision=2, minTradeNumber = 1, maxTradeNumber=1000000])

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
"""

StockManager.removeTempCsvStock.__doc__ = """
removeTempCsvStock(code)
    
    移除增加的临时Stock
        
    :param str code: 创建时自定义的编码
"""

#------------------------------------------------------------------
# from _KQuery.cpp
#------------------------------------------------------------------

KQueryByDate.__doc__ = """
KQueryByDate(start, end, kType, recoverType)

    构建按日期 [start, end) 方式获取K线数据条件
    
    :param Datetime start: 起始日期
    :param Datetime end: 结束日期
    :param KQuery.KType kType: K线数据类型（如日线、分钟线等）
    :param KQuery.RecoverType recoverType: 复权类型
    :return: 查询条件
    :rtype: KQuery
"""

KQueryByIndex.__doc__ = """
KQueryByIndex(start, end, kType, recoverType)

    构建按索引 [start, end) 方式获取K线数据条件，等同于直接使用 Query 构造
    
    :param ind start: 起始日期
    :param ind end: 结束日期
    :param KQuery.KType kType: K线数据类型（如日线、分钟线等）
    :param KQuery.RecoverType recoverType: 复权类型
    :return: 查询条件
    :rtype: KQuery
"""

KQuery.__doc__ = """K线数据查询条件，一般在Python中使用 Query 即可，不用指明 KQuery"""

KQuery.getQueryTypeName.__doc__ = """
getQueryTypeName(queryType)

    获取queryType名称，用于显示输出
        
    :param KQuery.QueryType queryType: 查询类型
    :rtype: str
"""

KQuery.getKTypeName.__doc__ = """
getKTypeName(kType)
    
    获取KType名称，用于显示输出
        
    :param KQuery.KType kType: K线类型
    :rtype: str
"""

KQuery.getRecoverTypeName.__doc__ = """
getRecoverTypeName(recoverType)
    
    获取recoverType名称，用于显示输出
        
    :param KQuery.RecoverType recoverType: 复权类型
    :rtype: str
"""

KQuery.getQueryTypeEnum.__doc__ = """
getQueryTypeEnum(queryType)
    
    根据字符串名称获取相应的queryType枚举值
        
    :param str queryType: 字符串名称，如“DATE”
    :rtype: KQuery.QueryType
"""

KQuery.getKTypeEnum.__doc__ = """
getKTypeEnum(ktype)
    
    根据字符串名称，获取相应的枚举值 
        
    :param str ktype: 字符串名称，如“DAY”
    :rtype: KQuery.KType
"""

KQuery.getRecoverTypeEnum.__doc__ = """
getRecoverTypeEnum(recoverType)

    根据字符串名称，获取相应的枚举值
        
    :param str recoverType: 字符串名称，如“NO_RECOVER”
    :rtype: KQuery.RecoverType
"""

KQuery.QueryType.__doc__ = """
- DATE  - 按日期方式查询
- INDEX - 按索引方式查询
"""

KQuery.RecoverType.__doc__ = """
K线复权类别枚举定义

- NO_RECOVER      - 不复权
- FORWARD         - 前向复权
- BACKWARD        - 后向复权
- EQUAL_FORWARD   - 等比前向复权
- EQUAL_BACKWARD  - 等比后向复权 
"""

KQuery.start.__doc__ = """
起始索引，当按日期查询方式创建时无效，为 constant.null_int64
"""

KQuery.end.__doc__ = """
结束索引，当按日期查询方式创建时无效，为 constant.null_int64
"""

KQuery.startDatetime.__doc__ = """
起始日期，当按索引查询方式创建时无效，为 constant.null_datetime
"""

KQuery.endDatetime.__doc__ = """
结束日期，当按索引查询方式创建时无效，为 constant.null_datetime
"""

KQuery.queryType.__doc__ = """
查询方式
"""

KQuery.kType.__doc__ = """
查询的K线类型
"""

KQuery.recoverType.__doc__ = """
查询的复权类型
"""

#------------------------------------------------------------------
# from _KRecord.cpp
#------------------------------------------------------------------

KRecord.__doc__ = """K线记录，组成K线数据，属性可读写"""

KRecord.datetime.__doc__ = """日期时间"""
KRecord.openPrice.__doc__ = """开盘价"""
KRecord.highPrice.__doc__ = """最高价"""
KRecord.lowPrice.__doc__ = """最低价"""
KRecord.closePrice.__doc__ = """收盘价"""
KRecord.transAmount.__doc__ = """成交金额"""
KRecord.transCount.__doc__ = """成交量"""

KRecordList.__doc__ = """C++ std::vector<KRecord>包装"""

#------------------------------------------------------------------
# from _KData.cpp
#------------------------------------------------------------------

KData.__doc__ = """
通过 Stock.getKData 获取的K线数据，由 KRecord 组成的数组，可象 list 一样进行遍历
"""

KData.startPos.__doc__ = """
获取在原始K线记录中对应的起始位置，如果KData为空返回0
"""

KData.endPos.__doc__ = """
获取在原始K线记录中对应范围的下一条记录的位置，如果为空返回0,其他等于lastPos + 1
"""

KData.lastPos.__doc__ = """
获取在原始K线记录中对应的最后一条记录的位置，如果为空返回0,其他等于endPos - 1
"""

KData.getDatetimeList.__doc__ = """
getDatetimeList()
    
    返回交易日期列表

    :rtype: DatetimeList
"""

KData.getKRecord.__doc__ = """
getKRecord(pos)
    
    获取指定索引位置的K线记录
        
    :param int pos: 位置索引
    :rtype: KRecord
"""

KData.get.__doc__ = """
get(pos)

    同 getKRecord。获取指定索引位置的K线记录
        
    :param int pos: 位置索引
    :rtype: KRecord   
"""

KData.getKRecordByDate.__doc__ = """
getKRecordByDate(datetime)

    获取指定时间的K线记录
    
    :param Datetime datetime: 指定的日期
    :rtype: KRecord
"""

KData.getByDate.__doc__ = """
getByDate(datetime)    

    获取指定时间的K线记录。同 getKRecordByDate。
    
    :param Datetime datetime: 指定的日期
    :rtype: KRecord
"""

KData.size.__doc__ = """
size()
    
    K线记录数量，同 __len__
"""

KData.empty.__doc__ = """
empty()
    
    判断是否为空
    
    :rtype: bool
"""

KData.getQuery.__doc__ = """
getQuery()
    
    获取关联的查询条件
    
    :rtype: KQuery
"""

KData.getStock.__doc__ = """
getStock()
    
    获取关联的Stock
    
    :rtype: Stock
"""

KData.tocsv.__doc__ = """
tocsv(filename)
    
    将数据保存至CSV文件
    
    :param str filename: 指定保存的文件名称
"""

#------------------------------------------------------------------
# from _TimeLineRecord.cpp
#------------------------------------------------------------------

TimeLineRecord.__doc__ = """分时线记录，属性可读写。"""

TimeLineRecord.datetime.__doc__ = """时间"""
TimeLineRecord.price.__doc__ = """价格"""
TimeLineRecord.vol.__doc__ = """成交量"""

TimeLineList.__doc__ = """
通过 Stock.getTimeLineList 获取的分时线数据，由 TimeLineRecord 组成的数组，可象 list 一样进行遍历
"""

#------------------------------------------------------------------
# from _TransRecord.cpp
#------------------------------------------------------------------

TransRecord.__doc__ = """历史分笔成交记录，属性可读写。"""

TransRecord.datetime.__doc__ = """时间"""
TransRecord.price.__doc__ = """价格"""
TransRecord.vol.__doc__ = """成交量"""
TransRecord.direct.__doc__ = """买卖盘方向"""

TransList.__doc__ = """
历史分笔成交记录列表
"""

#------------------------------------------------------------------
# from _Stock.cpp
#------------------------------------------------------------------

Stock.market.__doc__ = """获取所属市场简称，市场简称是市场的唯一标识"""
Stock.code.__doc__ = """获取证券代码"""
Stock.market_code.__doc__ = """市场简称+证券代码，如: sh000001"""
Stock.name.__doc__ = """获取证券名称"""
Stock.type.__doc__ = """获取证券类型，参见：:py:data:`constant`"""
Stock.valid.__doc__ = """该证券当前是否有效"""
Stock.startDatetime.__doc__ = """证券起始日期"""
Stock.lastDatetime.__doc__ = """证券最后日期"""
Stock.tick.__doc__ = """最小跳动量"""
Stock.tickValue.__doc__ = """最小跳动量价值"""
Stock.unit.__doc__ = """每单位价值 = tickValue / tick"""
Stock.precision.__doc__ = """价格精度"""
Stock.atom.__doc__ = """最小交易数量，同minTradeNumber"""
Stock.minTradeNumber.__doc__ = """最小交易数量"""
Stock.maxTradeNumber.__doc__ = """最大交易数量"""

Stock.isNull.__doc__ = """
isNull()
    
    是否为Null
    
    :rtype: bool
"""

Stock.getKData.__doc__ = """
getKData(query)
    
    获取K线数据
        
    :param Query query: 查询条件
    :return: 满足查询条件的K线数据
    :rtype: KData
"""

Stock.getCount.__doc__ = """
getCount([ktype=Query.DAY])
    
    获取不同类型K线数据量
        
    :param KQuery.KType ktype: K线数据类别
    :return: K线记录数
    :rtype: int
"""

Stock.getMarketValue.__doc__ = """
getMarketValue(datetime, ktype)
    
    获取指定时刻的市值，即小于等于指定时刻的最后一条记录的收盘价
        
    :param Datetime datetime: 指定时刻
    :param KQuery.KType ktype: K线数据类别
    :return: 指定时刻的市值
    :rtype: float
"""

Stock.getKRecord.__doc__ = """
getKRecord(pos[, ktype=Query.DAY])
    
    获取指定索引的K线数据记录，未作越界检查
        
    :param int pos: 指定的索引位置
    :param KQuery.KType ktype: K线数据类别
    :return: K线记录
    :rtype: KRecord
"""

Stock.getKRecordByDate.__doc__ = """
getKRecordByDate(datetime[, ktype=Query.DAY])
    
    根据数据类型（日线/周线等），获取指定时刻的KRecord
        
    :param Datetime datetime: 指定日期时刻
    :param KQuery.KType ktype: K线数据类别
    :return: K线记录
    :rtype: KRecord
"""

Stock.getKRecordList.__doc__ = """
getKRecordList(start, end, ktype)
    
    获取K线记录 [start, end)，一般不直接使用，用getKData替代
        
    :param int start: 起始位置
    :param int end: 结束位置
    :param KQuery.KType ktype: K线类别
    :return: K线记录列表
    :rtype: KRecordList
"""

Stock.getDatetimeList.__doc__ = """
getDatetimeList(query)
    
    获取日期列表
        
    :param Query query: 查询条件
    :rtype: DatetimeList
        
getDatetimeList(start, end, ktype)
    
    获取日期列表
        
    :param int start: 起始位置
    :param ind end: 结束位置
    :param KQuery.KType ktype: K线类型
    :rtype: DatetimeList        
"""

Stock.getTimeLineList.__doc__ = """
getTimeLineList(self, query)
    
    获取分时线
        
    :param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）
    :rtype: TimeLineList
"""

Stock.getTransList.__doc__ = """
getTransList(self, query)
    
    获取历史分笔数据
        
    :param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）
    :rtype: TransList
"""

Stock.getWeight.__doc__ = """
getWeight([start, end])
    
    获取指定时间段[start,end)内的权息信息。未指定起始、结束时刻时，获取全部权息记录。
        
    :param Datetime start: 起始时刻
    :param Datetime end: 结束时刻
    :rtype: StockWeightList
"""

Stock.getFinanceInfo.__doc__ = """
getFinanceInfo(self)
    
    获取当前财务信息
       
    :rtype: Parameter
"""

Stock.getHistoryFinanceInfo.__doc__ = """
getHistoryFinanceInfo(self, date)
    
    获取历史财务信息, 字段含义参见：https://hikyuu.org/finance_fields.html
        
    :param Datetime date: 指定日期必须是0331、0630、0930、1231，如 Datetime(201109300000)
    :rtype: PriceList
"""

Stock.realtimeUpdate.__doc__ = """
realtimeUpdate(krecord)
    
    （临时函数）只用于更新缓存中的日线数据
        
    :param KRecord krecord: 新增的实时K线记录
"""

Stock.loadKDataToBuffer.__doc__ = """
loadKDataToBuffer(ktype)
    
    将指定类别的K线数据加载至内存缓存
        
    :param KQuery.KType ktype: K线类型
"""

Stock.releaseKDataBuffer.__doc__ = """
releaseKDataBuffer(ktype)
    
    释放指定类别的内存K线数据
        
    :param KQuery.KType ktype: K线类型
"""

#------------------------------------------------------------------
# from _Block.cpp
#------------------------------------------------------------------

BlockList.__doc__ = """C++ std::vector<Block>包装"""

Block.__doc__ = """板块类，可视为证券的容器"""

Block.category.__doc__ = """板块分类，可读写"""
Block.name.__doc__ = """板块名称，可读写"""

Block.size.__doc__ = """
size()
    
    包含的证券数量
"""

Block.empty.__doc__ = """
empty()
    
    是否为空
"""

Block.add.__doc__ = """
add(stock)
    
    加入指定的证券
        
    :param Stock stock: 待加入的证券
    :return: 是否成功加入
    :rtype: bool
    
add(market_code)
    
    根据"市场简称证券代码"加入指定的证券
        
    :param str market_code: 市场简称证券代码
    :return: 是否成功加入
    :rtype: bool     
"""

Block.remove.__doc__ = """
remove(stock)
    
    移除指定证券
        
    :param Stock stock: 指定的证券
    :return: 是否成功
    :rtype: bool
    
remove(market_code)
    
    移除指定证券
        
    :param str market_code: 市场简称证券代码
    :return: 是否成功
    :rtype: bool    
"""

Block.clear.__doc__ = """移除包含的所有证券"""

Block.__len__.__doc__ = """包含的证券数量"""

#------------------------------------------------------------------
# from _save_load.cpp
#------------------------------------------------------------------

hku_save.__doc__ = """
hku_save(var, filename)

    序列化，将hikyuu内建类型的变量（如Stock、TradeManager等）保存在指定的文件中，格式为XML。
    
    :param var: hikyuu内建类型的变量
    :param str filename: 指定的文件名
"""

hku_load.__doc__ = """
hku_load(var, filename)

    将通过 hku_save 保存的变量，读取到var中。
    
    :param var: 指定的变量
    :param str filename: 待载入的序列化文件。
"""

#------------------------------------------------------------------
# from iostream.cpp
#------------------------------------------------------------------

OstreamRedirect.__doc__ = """
OstreamRedirect([stdout=True, stderr=True])

    重定向C++ std::cout、std::cerr至python。在非命令行方式下，某些App无法显示C++ iostream的输出信息，如Jupyter notebook。默认构造时，只是指定是否需要重定向std::cout或std::cerr，必须使用open方法或with语法才会启用重定向。
    
    使用with示例：
    with OstreamRedirect():
        your_function() #被封装的C++函数，其中使用了std::iostream输出
    
    :param bool stdout: 是否重定向C++ std::cout
    :param bool stderr: 是否重定向C++ std::cerr
"""

OstreamRedirect.open.__doc__ = """
open()

    启用重定向
"""

OstreamRedirect.close.__doc__ = """
close()

    关闭重定向
"""
