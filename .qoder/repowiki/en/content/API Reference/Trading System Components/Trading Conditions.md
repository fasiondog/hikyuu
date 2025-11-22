# Trading Conditions

<cite>
**Referenced Files in This Document**   
- [ConditionBase.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/ConditionBase.h)
- [ConditionBase.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/ConditionBase.cpp)
- [CN_Bool.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_Bool.h)
- [BoolCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/BoolCondition.h)
- [BoolCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/BoolCondition.cpp)
- [CN_Logic.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_Logic.h)
- [AndCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/logic/AndCondition.h)
- [AndCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/logic/AndCondition.cpp)
- [OrCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/logic/OrCondition.h)
- [OrCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/logic/OrCondition.cpp)
- [CN_Manual.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_Manual.h)
- [ManualCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/ManualCondition.h)
- [ManualCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/ManualCondition.cpp)
- [CN_OPLine.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_OPLine.h)
- [OPLineCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/OPLineCondition.h)
- [OPLineCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/OPLineCondition.cpp)
- [Condition.py](file://hikyuu/test/Condition.py)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [ConditionBase Interface](#conditionbase-interface)
3. [CN_Bool Conditions](#cn_bool-conditions)
4. [CN_Logic Conditions](#cn_logic-conditions)
5. [CN_Manual Conditions](#cn_manual-conditions)
6. [CN_OPLine Conditions](#cn_opline-conditions)
7. [Usage Patterns](#usage-patterns)
8. [Performance Considerations](#performance-considerations)
9. [Troubleshooting Guide](#troubleshooting-guide)

## Introduction
This document provides comprehensive API documentation for trading conditions in the Hikyuu quantitative trading framework. Trading conditions are essential components that determine when a trading system is valid and can execute trades. The framework supports multiple condition types including CN_Bool, CN_Logic, CN_Manual, and CN_OPLine, each serving different purposes in filtering trades based on market state, technical indicators, or custom logic. These conditions work in conjunction with other system components like signals, money managers, and trade managers to form complete trading systems.

## ConditionBase Interface

The ConditionBase class serves as the abstract base class for all trading conditions in the Hikyuu framework. It defines the core interface and common functionality that all condition types must implement.

```mermaid
classDiagram
class ConditionBase {
+string name()
+void name(string)
+size_t size()
+price_t at(size_t)
+price_t const* data()
+void reset()
+void setTO(KData)
+KData getTO()
+void setTM(TradeManagerPtr)
+TradeManagerPtr getTM()
+void setSG(SGPtr)
+SGPtr getSG()
+DatetimeList getDatetimeList()
+Indicator getValues()
+void _addValid(Datetime, price_t)
+ConditionPtr clone()
+bool isValid(Datetime)
+virtual void _calculate() = 0
+virtual void _reset()
+virtual ConditionPtr _clone() = 0
}
class BoolCondition {
-Indicator m_ind
+BoolCondition()
+BoolCondition(Indicator)
+~BoolCondition()
+void _calculate() override
+ConditionPtr _clone() override
}
class ManualCondition {
+ManualCondition()
+~ManualCondition()
+void _calculate() override
+ConditionPtr _clone() override
}
class OPLineCondition {
-Indicator m_op
+OPLineCondition()
+OPLineCondition(Indicator)
+~OPLineCondition()
+void _calculate() override
+ConditionPtr _clone() override
}
class AndCondition {
-ConditionPtr m_left
-ConditionPtr m_right
+AndCondition(ConditionPtr, ConditionPtr)
+~AndCondition()
+void _calculate() override
+ConditionPtr _clone() override
}
class OrCondition {
-ConditionPtr m_left
-ConditionPtr m_right
+OrCondition(ConditionPtr, ConditionPtr)
+~OrCondition()
+void _calculate() override
+ConditionPtr _clone() override
}
ConditionBase <|-- BoolCondition
ConditionBase <|-- ManualCondition
ConditionBase <|-- OPLineCondition
ConditionBase <|-- AndCondition
ConditionBase <|-- OrCondition
```

**Diagram sources**
- [ConditionBase.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/ConditionBase.h#L24-L250)
- [BoolCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/BoolCondition.h#L15-L40)
- [ManualCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/ManualCondition.h#L14-L23)
- [OPLineCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/OPLineCondition.h#L17-L45)
- [AndCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/logic/AndCondition.h#L15-L27)
- [OrCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/logic/OrCondition.h#L15-L27)

**Section sources**
- [ConditionBase.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/ConditionBase.h#L19-L250)
- [ConditionBase.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/ConditionBase.cpp#L1-L100)

## CN_Bool Conditions

CN_Bool conditions are used to create trading conditions based on boolean indicators. These conditions evaluate an indicator that returns boolean values (or numeric values where >0 indicates true) and determine system validity based on the indicator's output.

The CN_Bool condition implementation processes an indicator and marks dates as valid when the indicator value is greater than zero. This allows users to leverage any technical indicator that produces boolean signals as a trading condition.

```mermaid
sequenceDiagram
participant User as "User Code"
participant CN as "CN_Bool"
participant Ind as "Indicator"
participant KData as "KData"
User->>CN : CN_Bool(indicator)
CN->>CN : Store indicator reference
User->>CN : setTO(kdata)
CN->>CN : Trigger _calculate()
CN->>Ind : setContext(kdata)
CN->>Ind : data()
loop For each date in KData
Ind-->>CN : indicator value
CN->>CN : Check if value > 0
alt value > 0
CN->>CN : _addValid(date)
end
end
User->>CN : isValid(datetime)
CN->>CN : Check date_index map
CN-->>User : true/false
```

**Diagram sources**
- [CN_Bool.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_Bool.h#L15-L22)
- [BoolCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/BoolCondition.cpp#L26-L35)

**Section sources**
- [CN_Bool.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_Bool.h#L15-L22)
- [BoolCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/BoolCondition.h#L15-L40)
- [BoolCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/BoolCondition.cpp#L1-L41)

## CN_Logic Conditions

CN_Logic conditions provide logical operations (AND, OR, etc.) to combine multiple conditions. These conditions enable the creation of complex trading rules by combining simpler conditions using boolean logic.

The framework implements logical conditions through operator overloading, allowing intuitive syntax for combining conditions. The AND condition (operator&) returns true only when both constituent conditions are valid, while the OR condition (operator|) returns true when either condition is valid.

```mermaid
classDiagram
class ConditionBase {
<<abstract>>
}
class AndCondition {
-ConditionPtr m_left
-ConditionPtr m_right
+AndCondition(ConditionPtr, ConditionPtr)
+void _calculate() override
+ConditionPtr _clone() override
}
class OrCondition {
-ConditionPtr m_left
-ConditionPtr m_right
+OrCondition(ConditionPtr, ConditionPtr)
+void _calculate() override
+ConditionPtr _clone() override
}
ConditionBase <|-- AndCondition
ConditionBase <|-- OrCondition
AndCondition --> ConditionBase : "left"
AndCondition --> ConditionBase : "right"
OrCondition --> ConditionBase : "left"
OrCondition --> ConditionBase : "right"
```

**Diagram sources**
- [CN_Logic.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_Logic.h#L14-L35)
- [AndCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/logic/AndCondition.h#L15-L27)
- [OrCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/logic/OrCondition.h#L15-L27)

**Section sources**
- [CN_Logic.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_Logic.h#L14-L35)
- [AndCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/logic/AndCondition.cpp#L1-L30)
- [OrCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/logic/OrCondition.cpp#L1-L30)

## CN_Manual Conditions

CN_Manual conditions provide a way to manually specify valid trading dates. This type of condition is particularly useful for testing, backtesting specific scenarios, or implementing discretionary trading rules that cannot be expressed through automated indicators.

The CN_Manual condition has an empty _calculate() implementation, meaning it doesn't automatically determine validity based on market data. Instead, validity is determined by manually added dates using the _addValid() method inherited from ConditionBase.

```mermaid
flowchart TD
Start([Create CN_Manual]) --> Create["CN_Manual cn = CN_Manual()"]
Create --> AddDates["cn._addValid(datetime)"]
AddDates --> SetTO["cn.setTO(kdata)"]
SetTO --> Calculate["cn._calculate()"]
Calculate --> Check["cn.isValid(datetime)"]
Check --> Valid{"Valid?"}
Valid --> |Yes| True["Return true"]
Valid --> |No| False["Return false"]
True --> End([End])
False --> End
```

**Diagram sources**
- [CN_Manual.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_Manual.h#L14-L20)
- [ManualCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/ManualCondition.cpp#L1-L24)

**Section sources**
- [CN_Manual.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_Manual.h#L14-L20)
- [ManualCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/ManualCondition.h#L14-L23)
- [ManualCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/ManualCondition.cpp#L1-L24)

## CN_OPLine Conditions

CN_OPLine conditions determine system validity based on the relationship between the profit curve and a reference line (OP line). This condition is particularly useful for implementing equity curve trading strategies, where trading is only allowed when the system's performance is above a certain threshold.

The implementation creates a simulated trading system with fixed parameters (using the minimum trade quantity) to generate a profit curve, then compares this curve to the provided OP line indicator. When the profit curve is above the OP line, the system is considered valid.

```mermaid
sequenceDiagram
participant User as "User Code"
participant CN as "CN_OPLine"
participant Sys as "Simulated System"
participant TM as "TradeManager"
participant Profit as "Profit Curve"
User->>CN : CN_OPLine(op_indicator)
User->>CN : setTO(kdata)
CN->>CN : Trigger _calculate()
CN->>Sys : Create SYS_Simple()
CN->>TM : crtTM(initial_capital, TC_Zero)
CN->>Sys : setTM(TM)
CN->>Sys : setMM(MM_FixedCount(minTradeNumber))
CN->>Sys : setSG(signal)
CN->>Sys : run(kdata)
Sys-->>CN : Profit curve
CN->>CN : Calculate profit - op
loop For each date
CN->>CN : Check if difference > 0
alt difference > 0
CN->>CN : _addValid(date)
end
end
User->>CN : isValid(datetime)
CN-->>User : Result
```

**Diagram sources**
- [CN_OPLine.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_OPLine.h#L17-L27)
- [OPLineCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/OPLineCondition.cpp#L31-L57)

**Section sources**
- [CN_OPLine.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/crt/CN_OPLine.h#L17-L27)
- [OPLineCondition.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/OPLineCondition.h#L17-L45)
- [OPLineCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/OPLineCondition.cpp#L1-L64)

## Usage Patterns

### Creating Boolean Conditions
Boolean conditions can be created from any indicator that produces boolean-like output. The condition is valid when the indicator value is greater than zero.

```mermaid
flowchart TD
A[Create Indicator] --> B[Apply to KData]
B --> C[Create CN_Bool]
C --> D[Set to Trading System]
D --> E[Validate Trades]
```

### Combining Conditions with Logic Operators
Multiple conditions can be combined using logical operators to create complex trading rules.

```mermaid
flowchart TD
A[Condition 1] --> C[AND]
B[Condition 2] --> C
C --> D[Combined Condition]
D --> E[Trading System]
```

### Manual Condition for Specific Scenarios
Manual conditions allow precise control over when the system is valid, useful for testing specific market scenarios.

```mermaid
flowchart TD
A[Create CN_Manual] --> B[Add Specific Dates]
B --> C[Set to System]
C --> D[Execute on Specified Dates]
```

### OPLine for Equity Curve Filtering
OPLine conditions help filter trades based on system performance, preventing trading during drawdown periods.

```mermaid
flowchart TD
A[Generate Profit Curve] --> B[Compare to OP Line]
B --> C{Profit > OP?}
C --> |Yes| D[Valid: Allow Trading]
C --> |No| E[Invalid: Suspend Trading]
```

**Section sources**
- [Condition.py](file://hikyuu/test/Condition.py#L16-L57)
- [BoolCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/BoolCondition.cpp#L37-L39)
- [OPLineCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/OPLineCondition.cpp#L59-L61)

## Performance Considerations

When working with complex condition trees, several performance considerations should be kept in mind:

1. **Condition Evaluation Order**: The framework evaluates conditions in the order they are combined. Place computationally expensive conditions later in the evaluation chain when possible, as earlier conditions may short-circuit the evaluation.

2. **Indicator Caching**: Indicators used in conditions are recalculated when the trading object (KData) changes. Reusing the same indicator across multiple conditions can benefit from internal caching mechanisms.

3. **Memory Usage**: Complex condition trees with many nested logical operations can consume significant memory. Consider simplifying complex conditions or breaking them into smaller, reusable components.

4. **Timing Synchronization**: Conditions are evaluated based on the KData time series they are associated with. Ensure that all conditions in a trading system use compatible time frames and that market data is properly synchronized.

5. **Discard Periods**: Indicators often have a discard period at the beginning of the time series where values are unreliable. The condition framework respects these discard periods, but users should be aware of their impact on backtesting results.

**Section sources**
- [ConditionBase.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/ConditionBase.h#L75-L74)
- [BoolCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/BoolCondition.cpp#L30-L31)
- [OPLineCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/OPLineCondition.cpp#L44-L45)

## Troubleshooting Guide

### Common Issues and Solutions

**Issue**: Condition always returns invalid
- **Cause**: Indicator values never exceed zero or no valid dates were manually added
- **Solution**: Verify the indicator is producing expected values and check for proper condition setup

**Issue**: Unexpected valid dates in logical combinations
- **Cause**: Incorrect logical operator usage (using OR when AND was intended)
- **Solution**: Review the condition combination logic and ensure proper operator precedence

**Issue**: Performance degradation with complex conditions
- **Cause**: Deeply nested condition trees or computationally expensive indicators
- **Solution**: Simplify condition logic, cache intermediate results, or optimize indicator calculations

**Issue**: Timing mismatches between conditions
- **Cause**: Using KData with different time frames or frequencies
- **Solution**: Ensure all conditions use compatible time series data and consider resampling when necessary

**Issue**: OPLine condition not behaving as expected
- **Cause**: The OP line indicator may not be properly aligned with the profit curve
- **Solution**: Verify the OP line calculation and ensure it uses the same time frame as the trading system

**Section sources**
- [Condition.py](file://hikyuu/test/Condition.py#L37-L57)
- [ConditionBase.h](file://hikyuu_cpp/hikyuu/trade_sys/condition/ConditionBase.h#L91-L92)
- [BoolCondition.cpp](file://hikyuu_cpp/hikyuu/trade_sys/condition/imp/BoolCondition.cpp#L26-L35)