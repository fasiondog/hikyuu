# 数据管理API

<cite>
**本文档引用的文件**   
- [Stock.h](file://hikyuu_cpp/hikyuu/Stock.h)
- [KData.h](file://hikyuu_cpp/hikyuu/KData.h)
- [KQuery.h](file://hikyuu_cpp/hikyuu/KQuery.h)
- [Block.h](file://hikyuu_cpp/hikyuu/Block.h)
- [StockManager.h](file://hikyuu_cpp/hikyuu/StockManager.h)
- [pytdx_to_h5.py](file://hikyuu/data/pytdx_to_h5.py)
- [common_h5.py](file://hikyuu/data/common_h5.py)
- [common_pytdx.py](file://hikyuu/data/common_pytdx.py)
</cite>

## 目录
1. [核心数据类](#核心数据类)
2. [关键函数说明](#关键函数说明)
3. [K线查询条件构建](#k线查询条件构建)
4. [板块数据操作](#板块数据操作)
5. [数据导入导出接口](#数据导入导出接口)

## 核心数据类

### StockManager类
StockManager是证券信息统一管理类，提供单例实例访问。它负责初始化、重新加载和释放资源，管理基础信息驱动、板块驱动和K线数据驱动参数。

**公开方法：**
- `init(baseInfoParam, blockParam, kdataParam, preloadParam, hikyuuParam, context)`：初始化函数，必须在程序入口调用
- `reload()`：重新加载数据
- `quit()`：主动退出并释放资源
- `getStock(querystr)`：根据"市场简称证券代码"获取对应的证券实例
- `getBlock(category, name)`：获取预定义的板块
- `getMarketInfo(market)`：获取相应的市场信息
- `getStockTypeInfo(type)`：获取相应的证券类型详细信息
- `getTradingCalendar(query, market)`：获取交易日历
- `getZhBond10()`：获取10年期中国国债收益率
- `isHoliday(d)`：判断指定时间对应的日期是否为节假日
- `isTradingHours(d, market)`：粗略判断指定时间是否为交易时间
- `addStock(stock)`：添加Stock，仅供临时增加的特殊Stock使用
- `removeStock(market_code)`：从StockManager中移除相应的Stock

**Section sources**
- [StockManager.h](file://hikyuu_cpp/hikyuu/StockManager.h#L29-L50)

### Stock类
Stock类是证券基类，提供对单个证券的详细信息访问和操作。

**公开方法：**
- `id()`：获取内部id，一般用于作为map的键值使用
- `market()`：获取所属市场简称
- `code()`：获取证券代码
- `market_code()`：获取市场简称+证券代码
- `name()`：获取证券名称
- `type()`：获取证券类型
- `valid()`：该证券当前是否有效
- `startDatetime()`：获取证券起始日期
- `lastDatetime()`：获取证券最后日期
- `tick()`：获取最小跳动量
- `tickValue()`：最小跳动量价值
- `unit()`：每单位价值 = tickValue / tick
- `precision()`：获取价格精度
- `minTradeNumber()`：获取最小交易数量
- `maxTradeNumber()`：获取最大交易量
- `getWeight(start, end)`：获取指定时间段内的权息信息
- `getCount(dataType)`：获取不同类型K线数据量
- `getMarketValue(datetime, ktype)`：获取指定日期时刻的市值
- `getIndexRange(query, out_start, out_end)`：根据KQuery指定的条件，获取对应的K线位置范围
- `getKRecord(pos, dataType)`：获取指定索引的K线数据记录
- `getKData(query)`：获取K线数据
- `getDatetimeList(query)`：获取日期列表
- `getTimeLineList(query)`：获取分时线
- `getTransList(query)`：获取历史分笔数据
- `getFinanceInfo()`：获取当前财务信息
- `getBelongToBlockList(category)`：获取所属板块列表
- `getHistoryFinance()`：获取历史财务信息
- `getTradingCalendar(query)`：获取自身市场的交易日日历

**Section sources**
- [Stock.h](file://hikyuu_cpp/hikyuu/Stock.h#L38-L258)

### KData类
KData类表示K线数据，提供对K线数据的各种操作。

**公开方法：**
- `size()`：获取K线数据大小
- `empty()`：判断K线数据是否为空
- `getDatetimeList()`：获取日期列表
- `getKRecord(pos)`：获取指定位置的KRecord
- `getKRecord(datetime)`：按日期查询KRecord
- `front()`：获取第一条KRecord
- `back()`：获取最后一条KRecord
- `getKData(start, end)`：通过当前KData获取一个保持数据类型、复权类型不变的新的KData
- `getKData(ktype)`：获取相同时间范围内的其他类型K线数据
- `getKData(start, end)`：通过索引获取其子集
- `getPos(datetime)`：按日期查询对应的索引位置
- `getPosInStock(datetime)`：按日期获取在原始K线记录中的位置
- `getQuery()`：获取关联的KQuery
- `getStock()`：获取关联的Stock
- `startPos()`：获取在原始K线记录中对应的起始位置
- `lastPos()`：获取在原始K线记录中对应的最后一条记录的位置
- `endPos()`：获取在原始K线记录中对应范围的下一条记录的位置
- `tocsv(filename)`：输出数据到指定的文件中
- `open()`：获取开盘价指标
- `high()`：获取最高价指标
- `close()`：获取收盘价指标
- `low()`：获取最低价指标
- `vol()`：获取成交量指标
- `amo()`：获取成交金额指标

**Section sources**
- [KData.h](file://hikyuu_cpp/hikyuu/KData.h#L22-L129)

### KQuery类
KQuery类定义了K线数据的查询条件。

**公开方法：**
- `start()`：按索引方式查询时，返回指定的起始索引
- `end()`：按索引方式查询时，返回指定的结束索引
- `startDatetime()`：按日期方式查询时，返回指定的起始日期
- `endDatetime()`：按日期方式查询时，返回指定的结束日期
- `queryType()`：获取查询条件类型
- `kType()`：获取K线数据类型
- `recoverType()`：获取复权类型
- `recoverType(recoverType)`：设置复权类型
- `isRightOpening()`：判断是否为右开区间，即未指定结束时间
- `hash()`：获取哈希值

**静态成员：**
- `MIN`：1分钟线
- `MIN5`：5分钟线
- `MIN15`：15分钟线
- `MIN30`：30分钟线
- `MIN60`：60分钟线
- `HOUR2`：2小时线
- `DAY`：日线
- `WEEK`：周线
- `MONTH`：月线
- `QUARTER`：季线
- `HALFYEAR`：半年线
- `YEAR`：年线
- `DAY3`：3日线
- `DAY5`：5日线
- `DAY7`：7日线
- `MIN3`：3分钟线
- `HOUR4`：4小时线
- `HOUR6`：6小时线
- `HOUR12`：12小时线
- `TIMELINE`：分时
- `TRANS`：分笔

**枚举类型：**
- `QueryType`：查询方式（INDEX按索引，DATE按日期）
- `RecoverType`：复权类型（NO_RECOVER不复权，FORWARD前向复权，BACKWARD后向复权，EQUAL_FORWARD等比前向复权，EQUAL_BACKWARD等比后向复权）

**Section sources**
- [KQuery.h](file://hikyuu_cpp/hikyuu/KQuery.h#L20-L188)

### Block类
Block类表示板块，可视为证券的容器。

**公开方法：**
- `category()`：获取板块类别
- `name()`：获取板块名称
- `category(category)`：设置板块类别
- `name(name)`：设置名称
- `have(market_code)`：判断是否包含指定的证券
- `have(stock)`：判断是否包含指定的证券
- `get(market_code)`：获取指定的证券
- `getStockList(filter)`：获取板块下所有证券
- `add(stock)`：加入指定证券
- `add(market_code)`：加入指定证券
- `add(stocks)`：加入指定的证券列表
- `add(market_codes)`：加入指定的证券列表
- `remove(market_code)`：移除指定证券
- `remove(stock)`：移除指定证券
- `size()`：获取包含的证券数量
- `empty()`：判断是否为空
- `clear()`：清除包含的所有证券
- `getIndexStock()`：获取对应的指数
- `setIndexStock(stk)`：设置对应的指数
- `strongHash()`：获取强哈希值

**Section sources**
- [Block.h](file://hikyuu_cpp/hikyuu/Block.h#L20-L156)

## 关键函数说明

### get_stock函数
`get_stock`函数用于根据"市场简称证券代码"获取对应的证券实例。

**参数：**
- `querystr`：格式为"市场简称证券代码"，如"sh000001"

**返回值：**
- 对应的证券实例，如果实例不存在，则返回Null<Stock>()，不抛出异常

**使用示例：**
```python
stock = get_stock("sh000001")  # 获取上证指数
```

**Section sources**
- [Stock.h](file://hikyuu_cpp/hikyuu/Stock.h#L341-L342)

### get_kdata函数
`get_kdata`函数用于根据股票标识按指定的查询条件查询K线数据。

**参数：**
- `market_code`：股票标识，如"sh000001"
- `query`：KQuery查询条件

**返回值：**
- KData对象，包含查询到的K线数据

**使用示例：**
```python
from hikyuu import KQuery
query = KQuery(Datetime(20200101), Datetime(20201231), KQuery.DAY)
kdata = get_kdata("sh000001", query)
```

**Section sources**
- [KData.h](file://hikyuu_cpp/hikyuu/KData.h#L223-L224)

### query_history_finance函数
`query_history_finance`函数用于查询历史财务信息。

**参数：**
- `stk`：Stock对象
- `start`：起始日期
- `end`：结束日期

**返回值：**
- 包含历史财务信息的vector<HistoryFinanceInfo>对象

**使用示例：**
```python
from hikyuu import Datetime
stock = get_stock("sh600000")
history_finance = stock.getHistoryFinance()
```

**Section sources**
- [StockManager.h](file://hikyuu_cpp/hikyuu/StockManager.h#L206-L207)

## K线查询条件构建

KQuery类提供了多种方式来构建K线查询条件：

### 按索引查询
使用`KQueryByIndex`函数构建按索引方式的K线查询：
```python
from hikyuu import KQuery
query = KQueryByIndex(0, 100, KQuery.DAY, KQuery.NO_RECOVER)
```

### 按日期查询
使用`KQueryByDate`函数构建按日期方式的K线查询：
```python
from hikyuu import KQuery, Datetime
query = KQueryByDate(Datetime(20200101), Datetime(20201231), KQuery.DAY, KQuery.NO_RECOVER)
```

### 查询参数说明
- **市场代码**：使用市场简称，如"SH"表示上海证券交易所，"SZ"表示深圳证券交易所
- **股票代码**：6位数字代码，如"000001"
- **查询周期**：KQuery类中定义的K线类型，如DAY（日线）、MIN5（5分钟线）、WEEK（周线）等
- **价格复权方式**：KQuery.RecoverType枚举，包括：
  - `NO_RECOVER`：不复权
  - `FORWARD`：前向复权
  - `BACKWARD`：后向复权
  - `EQUAL_FORWARD`：等比前向复权
  - `EQUAL_BACKWARD`：等比后向复权

**Section sources**
- [KQuery.h](file://hikyuu_cpp/hikyuu/KQuery.h#L91-L116)

## 板块数据操作

### 获取板块
使用`getBlock`函数获取预定义的板块：
```python
from hikyuu import getBlock
block = getBlock("行业板块", "银行")
```

### 板块操作
Block类提供了丰富的板块操作方法：

```python
# 创建板块
block = Block("行业板块", "银行")

# 添加证券
block.add("sh600000")  # 通过市场代码添加
block.add(stock)       # 通过Stock对象添加

# 移除证券
block.remove("sh600000")

# 查询证券
if block.have("sh600000"):
    stock = block.get("sh600000")

# 获取所有证券
stocks = block.getStockList()

# 设置对应指数
index_stock = get_stock("sh000001")
block.setIndexStock(index_stock)
```

**Section sources**
- [Block.h](file://hikyuu_cpp/hikyuu/Block.h#L179-L180)

## 数据导入导出接口

### pytdx_to_h5.py模块
`pytdx_to_h5.py`模块提供了从通达信数据源导入数据到HDF5文件的功能。

**主要函数：**
- `import_data(connect, market, ktype, quotations, api, dest_dir, startDate, progress)`：导入通达信指定盘后数据路径中的K线数据
- `import_trans(connect, market, quotations, api, dest_dir, max_days, progress)`：导入分笔数据
- `import_time(connect, market, quotations, api, dest_dir, max_days, progress)`：导入分时数据
- `import_stock_name(connect, api, market, quotations)`：更新每只股票的名称、当前是否有效性、起始日期及结束日期
- `import_index_name(connect)`：导入所有指数代码表

**使用示例：**
```python
import sqlite3
from pytdx.hq import TdxHq_API
from hikyuu.data.pytdx_to_h5 import import_data, import_trans, import_time

# 连接数据库
connect = sqlite3.connect("stock.db")

# 连接通达信API
api = TdxHq_API()
api.connect('180.101.48.170', 7709)

# 导入日线数据
import_data(connect, 'SH', 'DAY', ['stock', 'fund'], api, 'd:\\stock')

# 导入5分钟线数据
import_data(connect, 'SH', '5MIN', ['stock', 'fund'], api, 'd:\\stock')

# 导入分笔数据
import_trans(connect, 'SH', ['stock', 'fund'], api, 'd:\\stock', max_days=30)

# 导入分时数据
import_time(connect, 'SH', ['stock', 'fund'], api, 'd:\\stock', max_days=9000)

api.disconnect()
connect.close()
```

**Section sources**
- [pytdx_to_h5.py](file://hikyuu/data/pytdx_to_h5.py#L384-L423)

### 数据准备流程
数据导入导出接口在实际数据准备流程中的应用：

1. **初始化数据库**：创建SQLite数据库并初始化表结构
2. **导入基础信息**：导入股票和指数代码表，更新股票名称和状态
3. **导入K线数据**：从通达信服务器获取日线、分钟线等K线数据
4. **导入权息数据**：下载并导入权息数据，用于复权计算
5. **导入分笔数据**：获取历史分笔交易数据
6. **导入分时数据**：获取历史分时线数据
7. **生成扩展K线**：基于基础K线数据生成周线、月线等扩展K线数据

**Section sources**
- [pytdx_to_h5.py](file://hikyuu/data/pytdx_to_h5.py#L622-L710)
- [common_h5.py](file://hikyuu/data/common_h5.py#L77-L291)