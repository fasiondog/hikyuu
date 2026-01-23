# API参考

<cite>
**本文档中引用的文件**   
- [__init__.py](file://hikyuu/__init__.py)
- [core.py](file://hikyuu/core.py)
- [extend.py](file://hikyuu/extend.py)
- [hub.py](file://hikyuu/hub.py)
- [interactive.py](file://hikyuu/interactive.py)
- [indicator.py](file://hikyuu/indicator/indicator.py)
- [trade.py](file://hikyuu/trade_manage/trade.py)
- [trade_sys.py](file://hikyuu/trade_sys/trade_sys.py)
- [KData.cpp](file://hikyuu_cpp/hikyuu/KData.cpp)
- [KData.h](file://hikyuu_cpp/hikyuu/KData.h)
</cite>

## 目录
1. [StockManager API](#stockmanager-api)
2. [KData API](#kdata-api)
3. [Indicator API](#indicator-api)
4. [TradeManager API](#trademanager-api)

## StockManager API

StockManager是Hikyuu框架的核心组件，负责管理所有证券信息。通过全局变量`sm`可直接访问其实例。

### get_stock
根据"市场简称证券代码"获取对应的证券实例。

**函数签名**
```python
def get_stock(self, querystr: str) -> Stock
```

**参数说明**
- `querystr` (str): 格式为"市场简称证券代码"，如"sh000001"

**返回值描述**
- 返回对应的证券实例，如果实例不存在，则返回Null<Stock>()，不抛出异常

**使用示例**
```python
# 获取上证指数
stock = sm.get_stock("sh000001")
print(stock.name)  # 输出: 上证指数

# 获取平安银行
stock = sm.get_stock("sz000001")
print(stock.market_code)  # 输出: sz000001
```

### get_plugin_path
获取插件路径。

**函数签名**
```python
def get_plugin_path(self) -> str
```

**返回值描述**
- 返回插件路径字符串

### set_language_path
设置多语言支持的翻译文件所在路径，仅在初始化之前设置有效。

**函数签名**
```python
def set_language_path(self, path: str) -> None
```

**参数说明**
- `path` (str): 翻译文件所在路径

### get_market_list
获取市场简称列表。

**函数签名**
```python
def get_market_list(self) -> StringList
```

**返回值描述**
- 返回市场简称列表

### get_market_info
获取相应的市场信息。

**函数签名**
```python
def get_market_info(self, market: str) -> MarketInfo
```

**参数说明**
- `market` (str): 指定的市场标识（市场简称）

**返回值描述**
- 返回相应的市场信息，如果相应的市场信息不存在，则返回Null<MarketInfo>()

### get_stock_type_info
获取相应的证券类型详细信息。

**函数签名**
```python
def get_stock_type_info(self, stk_type: int) -> StockTypeInfo
```

**参数说明**
- `stk_type` (int): 证券类型

**返回值描述**
- 返回指定证券类型的详细信息

### get_stock_type_info_list
获取所有证券类型详细信息。

**函数签名**
```python
def get_stock_type_info_list(self) -> DataFrame
```

**返回值描述**
- 返回包含所有证券类型详细信息的DataFrame

### get_stock_list
获取证券列表。

**函数签名**
```python
def get_stock_list(self, filter: Callable = None) -> StockList
```

**参数说明**
- `filter` (Callable, 可选): 输入参数为stock，返回True | False的过滤函数

**返回值描述**
- 返回证券列表，可通过filter函数进行过滤

### get_category_list
获取所有板块分类。

**函数签名**
```python
def get_category_list(self) -> StringList
```

**返回值描述**
- 返回所有板块分类的列表

### get_block
获取预定义的板块。

**函数签名**
```python
def get_block(self, category: str, name: str) -> Block
```

**参数说明**
- `category` (str): 板块分类
- `name` (str): 板块名称

**返回值描述**
- 返回指定的板块，如找不到返回空Block

**Section sources**
- [__init__.py](file://hikyuu/__init__.py#L85)

## KData API

KData类用于管理K线数据，提供多种方式获取和操作K线数据。

### getKData (Stock方法)
根据查询条件获取K线数据。

**函数签名**
```python
def getKData(self, query: KQuery) -> KData
```

**参数说明**
- `query` (KQuery): 查询条件

**返回值描述**
- 返回符合查询条件的K线数据

**使用示例**
```python
# 获取最近100个交易日的日线数据
stock = sm.get_stock("sh000001")
kdata = stock.getKData(Query(-100))
print(f"数据长度: {len(kdata)}")
print(f"最新收盘价: {kdata[-1].close}")
```

### getKData (KData方法)
从现有KData对象获取新的K线数据。

**函数签名**
```python
def getKData(self, start: Datetime, end: Datetime) -> KData
def getKData(self, start: int, end: int) -> KData
def getKData(self, ktype: KQuery.KType) -> KData
```

**参数说明**
- `start` (Datetime或int): 起始日期或索引位置
- `end` (Datetime或int): 结束日期或索引位置
- `ktype` (KQuery.KType): K线类型

**返回值描述**
- 返回新的KData对象，包含指定范围或类型的K线数据

### getKRecord
根据日期获取单条K线记录。

**函数签名**
```python
def getKRecord(self, datetime: Datetime) -> KRecord
```

**参数说明**
- `datetime` (Datetime): 指定的日期时间

**返回值描述**
- 返回指定日期的K线记录，如果不存在则返回NullKRecord

### getTradingCalendar
获取交易日历。

**函数签名**
```python
def getTradingCalendar(self, query: KQuery) -> DatetimeList
```

**参数说明**
- `query` (KQuery): 查询条件

**返回值描述**
- 返回指定条件下的交易日历列表

### 全局getKData函数
提供便捷方式直接获取K线数据。

**函数签名**
```python
def getKData(market_code: str, query: KQuery) -> KData
def getKData(market_code: str, start: Datetime = Datetime.min(), end: Datetime = None, ktype: KQuery.KType = KQuery.DAY, recoverType: KQuery.RecoverType = KQuery.NO_RECOVER) -> KData
```

**参数说明**
- `market_code` (str): 市场简称+证券代码
- `start` (Datetime, 可选): 起始日期，默认为最小日期
- `end` (Datetime, 可选): 结束日期，默认为最大日期
- `ktype` (KQuery.KType, 可选): K线类型，默认为日线
- `recoverType` (KQuery.RecoverType, 可选): 复权类型，默认为不复权

**使用示例**
```python
# 使用全局函数获取K线数据
kdata = getKData("sh000001", Query(-50))
print(f"数据点数量: {len(kdata)}")

# 指定日期范围获取数据
start_date = Datetime(202301010000)
end_date = Datetime(202312310000)
kdata = getKData("sh000001", start_date, end_date, KQuery.WEEK)
print(f"周线数据点数量: {len(kdata)}")
```

**Section sources**
- [KData.cpp](file://hikyuu_cpp/hikyuu/KData.cpp#L93-L213)
- [KData.h](file://hikyuu_cpp/hikyuu/KData.h#L193-L237)

## Indicator API

Indicator模块提供了一系列技术指标的创建和操作函数。

### 预定义指标函数
框架提供了一系列预定义的指标函数，可直接使用：

- `CLOSE()`: 收盘价指标
- `OPEN()`: 开盘价指标
- `HIGH()`: 最高价指标
- `LOW()`: 最低价指标
- `AMO()`: 成交额指标
- `VOL()`: 成交量指标
- `KDATA()`: K线数据指标

### concat_to_df
将多个指标合并到一个DataFrame中。

**函数签名**
```python
def concat_to_df(dates: DatetimeList, ind_list: Sequence, head_stock_code: bool = True, head_ind_name: bool = False) -> DataFrame
```

**参数说明**
- `dates` (DatetimeList): 指定的日期列表
- `ind_list` (Sequence): 已计算的指标列表
- `head_stock_code` (bool, 可选): 表标题是否使用证券代码，默认为True
- `head_ind_name` (bool, 可选): 表标题是否使用指标名称，默认为False

**返回值描述**
- 返回合并后的DataFrame，以dates为index

**使用示例**
```python
# 创建多个股票的MA指标并合并
query = Query(-200)
k_list = [stk.get_kdata(query) for stk in [sm['sz000001'], sm['sz000002']]]
ma_list = [MA(CLOSE(k)) for k in k_list]
df = concat_to_df(sm.get_trading_calendar(query), ma_list, head_stock_code=True)
print(df.head())
```

### df_to_ind
将pandas.DataFrame的指定列转换为Indicator。

**函数签名**
```python
def df_to_ind(df: DataFrame, col_name: str, col_date: str = None) -> Indicator
```

**参数说明**
- `df` (DataFrame): pandas.DataFrame对象
- `col_name` (str): 指定列名
- `col_date` (str, 可选): 指定日期列名，为None时忽略

**返回值描述**
- 返回转换后的Indicator对象

**使用示例**
```python
import akshare as ak
# 从akshare获取美国国债10年期收益率
df = ak.bond_zh_us_rate("19901219")
x = df_to_ind(df, '美国国债收益率10年', '日期')
```

### 同名指标别名
为常用指标提供别名：

- `VALUE = PRICELIST`: 价格列表
- `CAPITAL = LIUTONGPAN`: 流通盘
- `CONST = LASTVALUE`: 最后值
- `STD = STDEV`: 标准差

**Section sources**
- [indicator.py](file://hikyuu/indicator/indicator.py#L39-L120)

## TradeManager API

TradeManager用于管理交易过程，提供交易执行和性能分析功能。

### run
运行交易系统。

**函数签名**
```python
def run(self, system: System, datetime: Datetime) -> None
```

**参数说明**
- `system` (System): 要运行的交易系统
- `datetime` (Datetime): 运行日期

**使用示例**
```python
# 创建并运行交易系统
tm = TradeManager()
system = crtSG(some_signal_func)  # 创建信号系统
tm.run(system, Datetime.now())
```

### Performance API
提供交易性能分析功能。

#### to_df
将Performance统计结果转换为DataFrame格式。

**函数签名**
```python
def to_df(self) -> DataFrame
```

**返回值描述**
- 返回包含性能统计结果的DataFrame

**使用示例**
```python
# 获取交易性能并转换为DataFrame
performance = tm.getPerformance()
df = performance.to_df()
print(df)
```

### 交易记录转换
提供将交易记录转换为常用数据格式的功能。

#### to_np 和 to_df (TradeRecordList)
将交易记录列表转换为numpy数组或pandas DataFrame。

**函数签名**
```python
def to_np(self) -> np.ndarray
def to_df(self) -> DataFrame
```

**使用示例**
```python
# 获取交易记录并转换为DataFrame
trades = tm.getTradeList()
df = trades.to_df()
print(df.head())
```

#### to_np 和 to_df (PositionRecordList)
将持仓记录列表转换为numpy数组或pandas DataFrame。

**函数签名**
```python
def to_np(self) -> np.ndarray
def to_df(self) -> DataFrame
```

**使用示例**
```python
# 获取持仓记录并转换为DataFrame
positions = tm.getPositionList()
df = positions.to_df()
print(df.head())
```

### TradeManager创建函数
提供快速创建交易管理组件的函数。

#### crtOB
创建订单经纪人。

**函数签名**
```python
def crtOB(func, params={}, name='crtOB') -> OrderBrokerBase
```

**参数说明**
- `func`: 订单处理函数
- `params` (dict, 可选): 参数字典
- `name` (str, 可选): 自定义名称

**返回值描述**
- 返回自定义订单经纪人实例

#### crtMM
创建资金管理策略。

**函数签名**
```python
def crtMM(get_buy_num, get_sell_num=None, params={}, name='crtMM', buy_notify=None, sell_notify=None) -> MoneyManagerBase
```

**参数说明**
- `get_buy_num`: 买入数量接口
- `get_sell_num` (可选): 卖出数量接口，默认为None（卖出全部）
- `params` (dict, 可选): 参数字典
- `name` (str, 可选): 自定义名称
- `buy_notify` (可选): 接收买入交易记录通知
- `sell_notify` (可选): 接收卖出交易记录通知

**返回值描述**
- 返回自定义资金管理策略实例

#### crtSG
创建信号指示器。

**函数签名**
```python
def crtSG(func, params={}, name='crtSG') -> SignalBase
```

**参数说明**
- `func`: 信号策略函数
- `params` (dict, 可选): 参数字典
- `name` (str, 可选): 自定义名称

**返回值描述**
- 返回自定义信号指示器实例

#### crtCN
创建系统有效条件。

**函数签名**
```python
def crtCN(func, params={}, name='crtCN') -> ConditionBase
```

**参数说明**
- `func`: 系统有效条件函数
- `params` (dict, 可选): 参数字典
- `name` (str, 可选): 自定义名称

**返回值描述**
- 返回自定义系统有效条件实例

#### crtEV
创建市场环境判断策略。

**函数签名**
```python
def crtEV(func, params={}, name='crtEV') -> EnvironmentBase
```

**参数说明**
- `func`: 市场环境判断策略函数
- `params` (dict, 可选): 参数字典
- `name` (str, 可选): 自定义名称

**返回值描述**
- 返回自定义市场环境判断策略实例

#### crtPG
创建盈利目标策略。

**函数签名**
```python
def crtPG(get_goal, calculate=None, params={}, name='crtPG', buy_notify=None, sell_notify=None) -> ProfitGoalBase
```

**参数说明**
- `get_goal`: 获取目标价格接口
- `calculate` (可选): 内部计算接口
- `params` (dict, 可选): 参数字典
- `name` (str, 可选): 自定义名称
- `buy_notify` (可选): 接收买入交易记录通知
- `sell_notify` (可选): 接收卖出交易记录通知

**返回值描述**
- 返回盈利目标策略实例

#### crtSE
创建交易对象选择算法。

**函数签名**
```python
def crtSE(calculate, get_selected, is_match_af=None, params={}, name='crtSE') -> SelectorBase
```

**参数说明**
- `calculate`: 计算函数
- `get_selected`: 选择算法
- `is_match_af` (可选): 匹配算法
- `params` (dict, 可选): 参数字典
- `name` (str, 可选): 自定义名称

**返回值描述**
- 返回自定义交易对象选择算法实例

#### crtAF
创建资产分配算法。

**函数签名**
```python
def crtAF(allocate_weight_func, params={}, name='crtAF') -> AllocateFundsBase
```

**参数说明**
- `allocate_weight_func`: 资产分配算法
- `params` (dict, 可选): 参数字典
- `name` (str, 可选): 自定义名称

**返回值描述**
- 返回自定义资产分配算法实例

**Section sources**
- [trade.py](file://hikyuu/trade_manage/trade.py#L37-L54)
- [trade_sys.py](file://hikyuu/trade_sys/trade_sys.py#L48-L200)