# API Reference

<cite>
**Referenced Files in This Document**   
- [__init__.py](file://hikyuu/__init__.py)
- [core.py](file://hikyuu/core.py)
- [extend.py](file://hikyuu/extend.py)
- [hub.py](file://hikyuu/hub.py)
- [interactive.py](file://hikyuu/interactive.py)
- [indicator.py](file://hikyuu/indicator/indicator.py)
- [main.cpp](file://hikyuu_pywrap/main.cpp)
- [hikyuu.h](file://hikyuu_cpp/hikyuu/hikyuu.h)
- [Indicator.h](file://hikyuu_cpp/hikyuu/indicator/Indicator.h)
- [IndicatorImp.h](file://hikyuu_cpp/hikyuu/indicator/IndicatorImp.h)
- [IndParam.h](file://hikyuu_cpp/hikyuu/indicator/IndParam.h)
- [OrderBrokerBase.h](file://hikyuu_cpp/hikyuu/trade_manage/OrderBrokerBase.h)
- [BrokerTradeManager.cpp](file://hikyuu_cpp/hikyuu/strategy/BrokerTradeManager.cpp)
- [_IndicatorImp.cpp](file://hikyuu_pywrap/indicator/_IndicatorImp.cpp)
- [_TradeManager.cpp](file://hikyuu_pywrap/trade_manage/_TradeManager.cpp)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Data Management](#data-management)
3. [Indicators](#indicators)
4. [Trading Components](#trading-components)
5. [Backtesting](#backtesting)
6. [Utility Functions](#utility-functions)
7. [C++ Core API](#c-core-api)
8. [Python Interface](#python-interface)

## Introduction
The Hikyuu framework is a comprehensive quantitative trading system that provides tools for financial data management, technical indicator calculation, trading strategy development, and backtesting. This API reference documents the public interfaces exposed by both the C++ core and Python interface, organized by functional areas. The framework follows an object-oriented design with clear separation of concerns between data management, analysis components, and trading systems.

The framework's architecture is built around several key components:
- **StockManager**: Central repository for all stock and market data
- **Indicator System**: Comprehensive technical analysis engine with over 100 built-in indicators
- **Trading System**: Modular backtesting framework with configurable components
- **Data Drivers**: Flexible data access layer supporting multiple storage backends

The Python interface provides a high-level, user-friendly API that wraps the high-performance C++ core, allowing for both rapid prototyping and production-grade quantitative analysis.

## Data Management

The data management system in Hikyuu provides comprehensive functionality for accessing and manipulating financial market data. The core component is the StockManager, which serves as the central repository for all stock and market information.

```mermaid
classDiagram
class StockManager {
+instance() StockManager
+getStock(market_code) Stock
+get_block(category, name) Block
+get_trading_calendar(query, market) DatetimeList
+init(base_param, block_param, kdata_param, preload_param, hku_param, context) void
}
class Stock {
+market_code() string
+name() string
+type() int
+market() string
+get_kdata(query) KData
+realtime_update(krecord) void
+is_null() bool
}
class Block {
+name() string
+category() string
+add(stock) void
+remove(stock) void
+size() int
}
class KData {
+size() int
+open() Indicator
+close() Indicator
+high() Indicator
+low() Indicator
+vol() Indicator
+amo() Indicator
+get_datetime_list() DatetimeList
}
class Query {
+Query(start, end, ktype, recover_type)
+DATE QueryType
+INDEX QueryType
+DAY string
+WEEK string
+MONTH string
+NO_RECOVER RecoverType
+FORWARD RecoverType
+BACKWARD RecoverType
}
StockManager --> Stock : "contains"
StockManager --> Block : "contains"
Stock --> KData : "has"
KData --> Query : "uses"
```

**Diagram sources**
- [__init__.py](file://hikyuu/__init__.py#L85)
- [core.py](file://hikyuu/core.py#L1)
- [main.cpp](file://hikyuu_pywrap/main.cpp#L99)
- [hikyuu.h](file://hikyuu_cpp/hikyuu/hikyuu.h#L14)

**Section sources**
- [__init__.py](file://hikyuu/__init__.py#L85)
- [core.py](file://hikyuu/core.py#L1)
- [main.cpp](file://hikyuu_pywrap/main.cpp#L99)

## Indicators

The indicator system in Hikyuu provides a comprehensive set of technical analysis tools for financial market analysis. Indicators are implemented as C++ classes wrapped with Python interfaces, providing high-performance calculations with a user-friendly API.

```mermaid
classDiagram
class Indicator {
+size() int
+discard() int
+name() string
+get_context() KData
+set_context(kdata) void
+to_numpy() numpy.ndarray
+to_pandas() pandas.DataFrame
+get_parameter() Parameter
+haveIndParam(name) bool
+setIndParam(name, indicator) void
+getIndParam(name) IndParam
}
class IndicatorImp {
+name() string
+discard() int
+getResultNumber() int
+getParameter() Parameter
+isNeedContext() bool
+supportIndParam() bool
+_clone() IndicatorImpPtr
+_calculate(ind) void
}
class IndParam {
+IndParam()
+IndParam(indicator)
+getImp() IndicatorImpPtr
+get() Indicator
}
class Parameter {
+set(name, value) void
+get(name) any
+has(name) bool
+get_name_list() list
+to_dict() dict
}
Indicator --> IndicatorImp : "implements"
Indicator --> IndParam : "uses"
IndicatorImp --> Parameter : "has"
IndParam --> IndicatorImp : "contains"
```

**Diagram sources**
- [indicator.py](file://hikyuu/indicator/indicator.py#L32)
- [Indicator.h](file://hikyuu_cpp/hikyuu/indicator/Indicator.h#L1)
- [IndicatorImp.h](file://hikyuu_cpp/hikyuu/indicator/IndicatorImp.h#L1)
- [IndParam.h](file://hikyuu_cpp/hikyuu/indicator/IndParam.h#L1)
- [extend.py](file://hikyuu/extend.py#L259)

## Trading Components

The trading components in Hikyuu provide a modular framework for implementing trading strategies and managing trade execution. The system is designed with a component-based architecture that allows for flexible strategy construction.

```mermaid
classDiagram
class TradeManagerBase {
+currentCash() price_t
+getTotalNumber(stock) double
+getAvailableNumber(stock) double
+getBuyCost(datetime, stock, price, number) CostRecord
+getSellCost(datetime, stock, price, number) CostRecord
+buy(datetime, stock, real_price, number, stoploss, goal_price, plan_price, part, remark) TradeRecord
+sell(datetime, stock, real_price, number, stoploss, goal_price, plan_price, part, remark) TradeRecord
+getTradeList() TradeRecordList
+getPerformance() Performance
}
class OrderBrokerBase {
+name() string
+buy(tm, trade_record) bool
+sell(tm, trade_record) bool
+get_position(stock) BrokerPositionRecord
+get_all_positions() list[BrokerPositionRecord]
}
class TradeRecord {
+datetime Datetime
+stock Stock
+business int
+planPrice price_t
+realPrice price_t
+goalPrice price_t
+number double
+cost CostRecord
+stoploss price_t
+cash price_t
+from SystemPart
+remark string
}
class CostRecord {
+realPrice price_t
+number double
+total price_t
+brokage price_t
+stamp_tax price_t
+transfer_fee price_t
}
class BrokerPositionRecord {
+stock Stock
+number double
+available double
+buy_date Datetime
+cost_price price_t
}
TradeManagerBase --> TradeRecord : "manages"
TradeManagerBase --> CostRecord : "calculates"
OrderBrokerBase --> BrokerPositionRecord : "tracks"
TradeManagerBase --> OrderBrokerBase : "notifies"
```

**Diagram sources**
- [BrokerTradeManager.cpp](file://hikyuu_cpp/hikyuu/strategy/BrokerTradeManager.cpp#L109)
- [OrderBrokerBase.h](file://hikyuu_cpp/hikyuu/trade_manage/OrderBrokerBase.h#L31)
- [_TradeManager.cpp](file://hikyuu_pywrap/trade_manage/_TradeManager.cpp#L509)

## Backtesting

The backtesting system in Hikyuu provides a comprehensive framework for evaluating trading strategies. The system is designed with a modular architecture that allows for flexible strategy construction and evaluation.

```mermaid
classDiagram
class System {
+run(stock, query) void
+getTM() TradeManagerBase
+getSG() Signal
+getMM() MoneyManager
+getST() Stoploss
+getTP() TakeProfit
+getPG() ProfitGoal
+getEV() Environment
+setTM(tm) void
+setSG(sg) void
+setMM(mm) void
+setST(st) void
+setTP(tp) void
+setPG(pg) void
+setEV(ev) void
+readyForRun() bool
}
class Signal {
+isValid(datetime) bool
+getBuySignal(datetime) int
+getSellSignal(datetime) int
}
class MoneyManager {
+getNumber(datetime, stock, price, risk) double
}
class Stoploss {
+getStoploss(datetime, stock, price) price_t
}
class TakeProfit {
+getGoalPrice(datetime, stock, price) price_t
}
class ProfitGoal {
+getGoalPrice(datetime, stock, price) price_t
}
class Environment {
+isValid(datetime) bool
}
System --> Signal : "uses"
System --> MoneyManager : "uses"
System --> Stoploss : "uses"
System --> TakeProfit : "uses"
System --> ProfitGoal : "uses"
System --> Environment : "uses"
System --> TradeManagerBase : "controls"
```

**Diagram sources**
- [trade_sys/__init__.py](file://hikyuu/trade_sys/__init__.py#L28)
- [trade_manage/__init__.py](file://hikyuu/trade_manage/__init__.py#L27)

## Utility Functions

Hikyuu provides a comprehensive set of utility functions for common tasks in quantitative analysis. These functions are designed to simplify common operations and provide a consistent interface across the framework.

```mermaid
classDiagram
class UtilityFunctions {
+load_hikyuu(**kwargs) void
+hku_save(var, filename) void
+hku_load(filename) object
+set_global_context(stk, query) void
+get_global_context() KData
+select(cond, start, end, print_out) list[Stock]
+select2(inds, start, end, stks) pandas.DataFrame
+realtime_update(source, delta, stk_list) void
+get_part(name, **kwargs) object
+get_part_info(name) dict
+print_part_info(name) void
+search_part(name, hub, part_type, label) list[string]
}
class GlobalVariables {
+sm StockManager
+O Indicator
+C Indicator
+H Indicator
+L Indicator
+A Indicator
+V Indicator
+Q Query
+blocka Block
+zsbk_a Block
+blocksh Block
+zsbk_sh Block
+blocksz Block
+zsbk_sz Block
+blockbj Block
+zsbk_bj Block
+blockg Block
+zsbk_cyb Block
+blockstart Block
+blockzxb Block
+zsbk_zxb Block
+zsbk_sh50 Block
+zsbk_sh180 Block
+zsbk_hs300 Block
+zsbk_zz100 Block
}
UtilityFunctions --> GlobalVariables : "modifies"
```

**Diagram sources**
- [__init__.py](file://hikyuu/__init__.py#L219)
- [hub.py](file://hikyuu/hub.py#L646)
- [interactive.py](file://hikyuu/interactive.py#L27)

## C++ Core API

The C++ core of Hikyuu provides high-performance implementations of all framework components. The API is designed with a focus on performance and memory efficiency, while providing a clean, object-oriented interface.

```mermaid
classDiagram
class HikyuuCore {
+hikyuu_init(config_file_name, ignore_preload, context) void
+getConfigFromIni(config_file_name, baseParam, blockParam, kdataParam, preloadParam, hkuParam) void
+get_version() string
+get_stock(market_code) Stock
+get_block(category, name) Block
+getKData(market_code, query) KData
+setPythonInJupyter(flag) void
+setPythonInInteractive(flag) void
}
HikyuuCore --> StockManager : "initializes"
HikyuuCore --> Stock : "creates"
HikyuuCore --> Block : "creates"
HikyuuCore --> KData : "creates"
```

**Diagram sources**
- [hikyuu.h](file://hikyuu_cpp/hikyuu/hikyuu.h#L35)
- [main.cpp](file://hikyuu_pywrap/main.cpp#L131)

## Python Interface

The Python interface for Hikyuu provides a high-level, user-friendly API that wraps the high-performance C++ core. The interface is designed to be intuitive and easy to use, while providing access to all the functionality of the underlying C++ implementation.

```mermaid
classDiagram
class PythonInterface {
+from hikyuu import *
+load_hikyuu(**kwargs) void
+hku_save(var, filename) void
+hku_load(filename) object
+set_global_context(stk, query) void
+get_global_context() KData
+select(cond, start, end, print_out) list[Stock]
+select2(inds, start, end, stks) pandas.DataFrame
+realtime_update(source, delta, stk_list) void
+get_part(name, **kwargs) object
+get_part_info(name) dict
+print_part_info(name) void
+search_part(name, hub, part_type, label) list[string]
}
PythonInterface --> CppCore : "wraps"
PythonInterface --> UtilityFunctions : "exposes"
PythonInterface --> GlobalVariables : "exposes"
```

**Diagram sources**
- [__init__.py](file://hikyuu/__init__.py#L53)
- [core.py](file://hikyuu/core.py#L9)
- [extend.py](file://hikyuu/extend.py#L6)
- [hub.py](file://hikyuu/hub.py#L739)