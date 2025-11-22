# 技术指标API

<cite>
**本文档中引用的文件**   
- [__init__.py](file://hikyuu/indicator/__init__.py)
- [indicator.py](file://hikyuu/indicator/indicator.py)
- [pyind.py](file://hikyuu/indicator/pyind.py)
- [MA.h](file://hikyuu_cpp/hikyuu/indicator/crt/MA.h)
- [EMA.h](file://hikyuu_cpp/hikyuu/indicator/crt/EMA.h)
- [MACD.h](file://hikyuu_cpp/hikyuu/indicator/crt/MACD.h)
- [RSI.h](file://hikyuu_cpp/hikyuu/indicator/crt/RSI.h)
- [RSI.cpp](file://hikyuu_cpp/hikyuu/indicator/crt/RSI.cpp)
- [_build_in.cpp](file://hikyuu_pywrap/indicator/_build_in.cpp)
- [_ta_lib.cpp](file://hikyuu_pywrap/indicator/_ta_lib.cpp)
- [build_in.h](file://hikyuu_cpp/hikyuu/indicator/build_in.h)
- [ta_crt.h](file://hikyuu_cpp/hikyuu/indicator_talib/ta_crt.h)
</cite>

## 目录
1. [简介](#简介)
2. [核心指标创建函数](#核心指标创建函数)
3. [TA-Lib集成指标](#ta-lib集成指标)
4. [指标对象属性与方法](#指标对象属性与方法)
5. [指标间运算](#指标间运算)
6. [指标与KData关联](#指标与kdata关联)
7. [完整代码示例](#完整代码示例)
8. [策略中高效使用指标](#策略中高效使用指标)

## 简介
Hikyuu技术分析系统提供了全面的技术指标API，支持创建各种经典技术指标和TA-Lib集成指标。本文档系统性地列出所有内置技术指标的创建函数，包括MA、EMA、MACD、RSI等经典指标以及TA-Lib集成指标。详细说明每个指标构造函数的参数（如计算周期、价格类型）、返回的Indicator对象属性和方法。提供从原始数据创建指标、指标间运算（如REF、CROSS）以及指标结果提取的完整代码示例。特别说明Indicator与KData的关联方式，以及如何在策略中高效使用指标结果。

## 核心指标创建函数

Hikyuu提供了多种经典技术指标的创建函数，这些函数位于`hikyuu/indicator`模块中，主要通过C++实现并在Python层面进行封装。

### 移动平均线（MA）
简单移动平均线（Simple Moving Average）的创建函数：

```cpp
Indicator HKU_API MA(int n = 22);
Indicator HKU_API MA(const IndParam& n);
inline Indicator HKU_API MA(const Indicator& ind, int n = 22);
```

**参数说明：**
- `n`：计算均值的周期窗口，n为0时从第一个有效数据开始计算
- `ind`：待计算的数据源

**返回值：**
- 返回一个Indicator对象，表示简单移动平均线

**Section sources**
- [MA.h](file://hikyuu_cpp/hikyuu/indicator/crt/MA.h#L1-L45)

### 指数移动平均线（EMA）
指数移动平均线（Exponential Moving Average）的创建函数：

```cpp
Indicator HKU_API EMA(int n = 22);
Indicator HKU_API EMA(const IndParam& n);
inline Indicator HKU_API EMA(const Indicator& data, int n = 22);
```

**参数说明：**
- `n`：计算均值的周期窗口，必须为大于0的整数
- `data`：待计算的源数据

**返回值：**
- 返回一个Indicator对象，表示指数移动平均线

**Section sources**
- [EMA.h](file://hikyuu_cpp/hikyuu/indicator/crt/EMA.h#L1-L45)

### 平滑异同移动平均线（MACD）
MACD（Moving Average Convergence Divergence）的创建函数：

```cpp
Indicator HKU_API MACD(int n1 = 12, int n2 = 26, int n3 = 9);
Indicator HKU_API MACD(const IndParam& n1, const IndParam& n2, const IndParam& n3);
inline Indicator MACD(const Indicator& data, int n1 = 12, int n2 = 26, int n3 = 9);
```

**参数说明：**
- `n1`：短期EMA时间窗，默认12
- `n2`：长期EMA时间窗，默认26
- `n3`：（短期EMA-长期EMA）EMA平滑时间窗，默认9
- `data`：待计算数据

**返回值：**
- 具有三个结果集的Indicator对象：
  - `result(0)`：MACD_BAR（MACD直柱，即MACD快线－MACD慢线）
  - `result(1)`：DIFF（快线，即短期EMA-长期EMA）
  - `result(2)`：DEA（慢线，即快线的n3周期EMA平滑）

**Section sources**
- [MACD.h](file://hikyuu_cpp/hikyuu/indicator/crt/MACD.h#L1-L63)

### 相对强弱指数（RSI）
相对强弱指数（Relative Strength Index）的创建函数：

```cpp
Indicator HKU_API RSI(int n = 14);
Indicator HKU_API RSI(const Indicator& data, int n = 14);
```

**参数说明：**
- `n`：计算周期，默认14
- `data`：待计算的数据源

**计算公式：**
1. 计算价格变化：`diff = REF(0) - REF(1)`
2. 分离上涨和下跌：`u = IF(diff > 0, diff, 0)`，`d = IF(diff < 0, (-1) * diff, 0)`
3. 计算EMA：`ema_u = EMA(u, n)`，`ema_d = EMA(d, n)`
4. 计算RS：`rs = ema_u / ema_d`
5. 计算RSI：`rsi = (1 - 1 / (1 + rs)) * 100`

**返回值：**
- 返回一个Indicator对象，表示相对强弱指数

**Section sources**
- [RSI.h](file://hikyuu_cpp/hikyuu/indicator/crt/RSI.h#L1-L28)
- [RSI.cpp](file://hikyuu_cpp/hikyuu/indicator/crt/RSI.cpp#L1-L38)

## TA-Lib集成指标

Hikyuu集成了TA-Lib库，提供了丰富的技术指标函数。这些函数通过`TA_`前缀标识，位于`hikyuu/indicator_talib`模块中。

### TA-Lib MACD指标
TA-Lib版本的MACD指标创建函数：

```cpp
m.def("TA_MACD", py::overload_cast<int, int, int>(TA_MACD), py::arg("fast_n") = 12,
      py::arg("slow_n") = 26, py::arg("signal_n") = 9);
m.def("TA_MACD", py::overload_cast<const Indicator&, int, int, int>(TA_MACD), py::arg("data"),
      py::arg("fast_n") = 30, py::arg("slow_n") = 26, py::arg("signal_n") = 9);
```

**参数说明：**
- `data`：输入数据
- `fast_n`：快速EMA周期（2-100000）
- `slow_n`：慢速EMA周期（2-100000）
- `signal_n`：信号线平滑周期（1-100000）

**Section sources**
- [_ta_lib.cpp](file://hikyuu_pywrap/indicator/_ta_lib.cpp#L335-L357)

### TA-Lib RSI指标
TA-Lib版本的RSI指标创建函数：

```cpp
m.def("TA_RSI", py::overload_cast<int>(TA_RSI), py::arg("n") = 14);
m.def("TA_RSI", py::overload_cast<const Indicator&, int>(TA_RSI), py::arg("data"),
      py::arg("n") = 14, R"(TA_RSI - Relative Strength Index)");
```

**参数说明：**
- `data`：输入数据
- `n`：计算周期（2-100000）

**Section sources**
- [_ta_lib.cpp](file://hikyuu_pywrap/indicator/_ta_lib.cpp#L510-L528)

### 其他TA-Lib指标
Hikyuu还集成了多种TA-Lib指标，包括：

- `TA_SMA`：简单移动平均
- `TA_EMA`：指数移动平均
- `TA_WMA`：加权移动平均
- `TA_TEMA`：三重指数移动平均
- `TA_BOLL`：布林带
- `TA_KDJ`：随机指标
- `TA_STOCH`：随机振荡器
- `TA_WILLR`：威廉指标
- `TA_ADX`：平均趋向指数

这些指标的创建函数都遵循类似的模式，通常包含数据源和计算周期参数。

**Section sources**
- [ta_crt.h](file://hikyuu_cpp/hikyuu/indicator_talib/ta_crt.h)
- [_ta_lib.cpp](file://hikyuu_pywrap/indicator/_ta_lib.cpp)

## 指标对象属性与方法

Indicator对象是Hikyuu技术指标系统的核心，封装了指标计算结果和相关操作。

### 基本属性
- `name`：指标名称
- `size()`：指标结果的大小
- `discard()`：丢弃的初始数据点数量
- `getResult(index)`：获取指定索引的结果集

### 主要方法
Indicator对象提供了多种方法用于数据转换和操作：

```python
# 转换为numpy数组
Indicator.to_numpy = Indicator.to_np
Indicator.value_to_numpy = Indicator.value_to_np

# 转换为pandas DataFrame
Indicator.to_pandas = Indicator.to_df
Indicator.value_to_pandas = Indicator.value_to_df

# 设置上下文
Indicator.set_context(kdata)
```

### 结果提取
Indicator对象可以包含多个结果集，通过`getResult()`方法访问：

```cpp
Indicator macd = MACD(data, 12, 26, 9);
Indicator bar = macd.getResult(0);  // MACD直柱
Indicator diff = macd.getResult(1); // DIFF线
Indicator dea = macd.getResult(2);  // DEA线
```

**Section sources**
- [indicator.py](file://hikyuu/indicator/indicator.py#L32-L36)
- [build_in.h](file://hikyuu_cpp/hikyuu/indicator/build_in.h)

## 指标间运算

Hikyuu支持丰富的指标间运算操作，允许用户创建复杂的指标组合。

### 基本运算
支持标准的数学运算符：
- 加法：`+`
- 减法：`-`
- 乘法：`*`
- 除法：`/`
- 取模：`%`

```python
# 示例：创建自定义指标
ma5 = MA(CLOSE(), 5)
ma10 = MA(CLOSE(), 10)
ma_diff = ma5 - ma10  # 两条均线的差值
```

### 逻辑运算
支持逻辑运算符：
- 大于：`>`
- 小于：`<`
- 等于：`==`
- 不等于：`!=`
- 与：`&`
- 或：`|`
- 非：`~`

### 特殊运算函数
Hikyuu提供了专门的指标运算函数：

#### REF函数
引用过去某个周期的值：

```cpp
Indicator HKU_API REF(int n);
Indicator HKU_API REF(const IndParam& n);
inline Indicator HKU_API REF(const Indicator& ind, int n);
```

**参数说明：**
- `n`：回溯周期数
- `ind`：源指标

#### CROSS函数
判断是否发生交叉：

```cpp
Indicator HKU_API CROSS(const Indicator& ind1, const Indicator& ind2);
```

**参数说明：**
- `ind1`：第一个指标
- `ind2`：第二个指标

当`ind1`从下方向上穿越`ind2`时返回True。

#### 其他运算函数
- `HHV`：N周期内最高值
- `LLV`：N周期内最低值
- `SUM`：N周期内求和
- `COUNT`：N周期内满足条件的次数
- `EVERY`：N周期内是否一直满足条件
- `EXIST`：N周期内是否存在满足条件的情况

**Section sources**
- [build_in.h](file://hikyuu_cpp/hikyuu/indicator/build_in.h)
- [_build_in.cpp](file://hikyuu_pywrap/indicator/_build_in.cpp)

## 指标与KData关联

指标与KData的关联是Hikyuu技术分析系统的关键特性，确保指标计算与原始K线数据保持同步。

### 上下文设置
通过`set_context()`方法将指标与KData关联：

```python
def KDJ(kdata=None, n=9, m1=3, m2=3):
    """ 经典 KDJ 随机指标 """
    rsv = (CLOSE() - LLV(LOW(), n)) / (HHV(HIGH(), n) - LLV(LOW(), n)) * 100
    k = SMA(rsv, m1, 1)
    d = SMA(k, m2, 1)
    j = 3 * k - 2 * d
    if kdata is not None:
        k.set_context(kdata)
        j.set_context(kdata)
        d.set_context(kdata)
    return k, d, j
```

### 价格数据引用
Hikyuu提供了便捷的价格数据引用函数：

```python
# 避免 python 中公式原型必须加括号
KDATA = C_KDATA()
CLOSE = C_CLOSE()
OPEN = C_OPEN()
HIGH = C_HIGH()
LOW = C_LOW()
AMO = C_AMO()
VOL = C_VOL()
```

这些函数可以直接在指标公式中使用，自动关联到当前的KData上下文。

### 同名指标别名
为了兼容不同习惯，Hikyuu提供了同名指标的别名：

```python
# 同名指标
VALUE = PRICELIST
CAPITAL = LIUTONGPAN
CONST = LASTVALUE
STD = STDEV
```

**Section sources**
- [indicator.py](file://hikyuu/indicator/indicator.py#L107-L119)
- [pyind.py](file://hikyuu/indicator/pyind.py#L30-L47)

## 完整代码示例

以下是一些完整的代码示例，展示如何在实际应用中使用技术指标API。

### 创建基本指标
```python
from hikyuu import *

# 获取股票数据
stock = sm['sh000001']
kdata = stock.get_kdata(Query(-100))

# 创建基本技术指标
ma5 = MA(CLOSE(kdata), 5)
ma10 = MA(CLOSE(kdata), 10)
ema12 = EMA(CLOSE(kdata), 12)
ema26 = EMA(CLOSE(kdata), 26)

# 创建MACD指标
macd = MACD(CLOSE(kdata), 12, 26, 9)
diff = macd.getResult(1)
dea = macd.getResult(2)
bar = macd.getResult(0)

# 创建RSI指标
rsi = RSI(CLOSE(kdata), 14)
```

### 指标间运算示例
```python
# 均线交叉信号
golden_cross = CROSS(ma5, ma10)
death_cross = CROSS(ma10, ma5)

# RSI超买超卖信号
rsi_overbought = RSI(kdata, 14) > 70
rsi_oversold = RSI(kdata, 14) < 30

# MACD柱状图变化
macd_increasing = REF(bar, 1) < bar
macd_decreasing = REF(bar, 1) > bar

# 复合条件
buy_signal = golden_cross & (rsi_oversold | (rsi < 50))
sell_signal = death_cross & (rsi_overbought | (rsi > 50))
```

### 数据合并与导出
```python
def concat_to_df(dates, ind_list, head_stock_code=True, head_ind_name=False):
    """将列表中的指标至合并在一张 pandas DataFrame 中"""
    df = dates.to_df('ms')
    if not ind_list:
        return df
    for i in range(len(ind_list)):
        ind = ind_list[i]
        if head_ind_name and head_stock_code:
            name = f"{ind.name}/{ind.get_context().get_stock().market_code}"
        elif head_ind_name:
            name = f'{ind.name}{i}'
        else:
            name = ind.get_context().get_stock().market_code
        df = pd.merge(df, ind.to_df()[['datetime', 'value1']].rename(
            columns={'value1': name}), on='datetime', how='left')
    return df

# 使用示例
query = Query(-200)
k_list = [stk.get_kdata(query) for stk in [sm['sz000001'], sm['sz000002']]]
ma_list = [MA(CLOSE(k)) for k in k_list]
result_df = concat_to_df(sm.get_trading_calendar(query), ma_list, 
                        head_stock_code=True, head_ind_name=False)
```

### 自定义指标函数
```python
def BOLL(kdata, n=20, p=2):
    """布林带指标"""
    mid = MA(CLOSE(kdata), n)
    std = STDEV(CLOSE(kdata), n)
    up = mid + p * std
    down = mid - p * std
    up.set_context(kdata)
    mid.set_context(kdata)
    down.set_context(kdata)
    return up, mid, down

def KDJ(kdata, n=9, m1=3, m2=3):
    """KDJ随机指标"""
    rsv = (CLOSE(kdata) - LLV(LOW(kdata), n)) / \
          (HHV(HIGH(kdata), n) - LLV(LOW(kdata), n)) * 100
    k = SMA(rsv, m1, 1)
    d = SMA(k, m2, 1)
    j = 3 * k - 2 * d
    k.set_context(kdata)
    d.set_context(kdata)
    j.set_context(kdata)
    return k, d, j
```

**Section sources**
- [indicator.py](file://hikyuu/indicator/indicator.py#L39-L81)
- [pyind.py](file://hikyuu/indicator/pyind.py#L30-L47)

## 策略中高效使用指标

在交易策略中高效使用技术指标是实现量化交易的关键。以下是最佳实践和优化建议。

### 批量计算优化
避免重复计算相同的指标，采用批量计算方式：

```python
# 错误做法：重复计算
def strategy_bad(stock):
    ma5 = MA(CLOSE(stock.get_kdata()), 5)
    ma10 = MA(CLOSE(stock.get_kdata()), 10)
    rsi = RSI(CLOSE(stock.get_kdata()), 14)
    # 每次都重新获取KData并计算指标

# 正确做法：批量计算
def strategy_good(stock, kdata):
    close = CLOSE(kdata)
    ma5 = MA(close, 5)
    ma10 = MA(close, 10)
    rsi = RSI(close, 14)
    # 复用KData和中间结果
```

### 内存管理
合理管理指标对象的生命周期，避免内存泄漏：

```python
# 及时释放不再需要的指标
def process_indicators(stock_list, query):
    results = []
    for stock in stock_list:
        kdata = stock.get_kdata(query)
        if len(kdata) == 0:
            continue
            
        # 创建指标
        indicators = {
            'ma5': MA(CLOSE(kdata), 5),
            'ma10': MA(CLOSE(kdata), 10),
            'macd': MACD(CLOSE(kdata), 12, 26, 9),
            'rsi': RSI(CLOSE(kdata), 14)
        }
        
        # 提取所需结果
        result = {
            'code': stock.market_code,
            'ma5_last': indicators['ma5'].to_np()[-1],
            'ma10_last': indicators['ma10'].to_np()[-1],
            'macd_bar': indicators['macd'].getResult(0).to_np()[-1],
            'rsi_value': indicators['rsi'].to_np()[-1]
        }
        results.append(result)
        
        # 显式释放指标对象（Python会自动处理，但明确更好）
        del indicators
        
    return results
```

### 性能优化技巧
1. **复用中间结果**：避免重复计算相同的基础指标
2. **延迟计算**：只在需要时才计算指标结果
3. **批量处理**：对多个股票使用相同的计算逻辑
4. **合理设置discard**：避免不必要的数据丢弃

```python
class IndicatorCache:
    """指标缓存类，避免重复计算"""
    
    def __init__(self):
        self._cache = {}
    
    def get_indicator(self, key, creator_func):
        """获取指标，如果已存在则返回缓存版本"""
        if key not in self._cache:
            self._cache[key] = creator_func()
        return self._cache[key]
    
    def clear(self):
        """清空缓存"""
        self._cache.clear()

# 使用示例
indicator_cache = IndicatorCache()

def get_ma_indicator(kdata, period):
    key = f"MA_{kdata.get_stock().market_code}_{period}"
    return indicator_cache.get_indicator(key, 
                                       lambda: MA(CLOSE(kdata), period))
```

### 实时策略中的指标使用
在实时交易策略中，需要特别注意指标的实时更新：

```python
class RealTimeStrategy:
    """实时交易策略示例"""
    
    def __init__(self):
        self.indicator_cache = {}
        self.last_update_time = None
    
    def update_indicators(self, kdata):
        """更新指标"""
        stock_code = kdata.get_stock().market_code
        
        # 只有当有新数据时才重新计算
        if (stock_code not in self.indicator_cache or 
            self.last_update_time != kdata[-1].datetime):
            
            close = CLOSE(kdata)
            
            # 计算所需指标
            self.indicator_cache[stock_code] = {
                'ma5': MA(close, 5),
                'ma20': MA(close, 20),
                'macd': MACD(close, 12, 26, 9),
                'rsi': RSI(close, 14)
            }
            
            self.last_update_time = kdata[-1].datetime
    
    def get_signals(self, kdata):
        """获取交易信号"""
        self.update_indicators(kdata)
        stock_code = kdata.get_stock().market_code
        
        if stock_code not in self.indicator_cache:
            return None
            
        indicators = self.indicator_cache[stock_code]
        ma5 = indicators['ma5'].to_np()
        ma20 = indicators['ma20'].to_np()
        macd_bar = indicators['macd'].getResult(0).to_np()
        rsi = indicators['rsi'].to_np()
        
        # 生成交易信号
        signals = {
            'golden_cross': ma5[-1] > ma20[-1] and ma5[-2] <= ma20[-2],
            'death_cross': ma5[-1] < ma20[-1] and ma5[-2] >= ma20[-2],
            'macd_positive': macd_bar[-1] > 0,
            'rsi_overbought': rsi[-1] > 70,
            'rsi_oversold': rsi[-1] < 30
        }
        
        return signals
```

**Section sources**
- [indicator.py](file://hikyuu/indicator/indicator.py)
- [pyind.py](file://hikyuu/indicator/pyind.py)