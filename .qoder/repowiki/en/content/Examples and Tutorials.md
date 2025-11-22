# Examples and Tutorials

<cite>
**Referenced Files in This Document**   
- [000-Index.ipynb](file://hikyuu/examples/notebook/000-Index.ipynb)
- [001-overview.ipynb](file://hikyuu/examples/notebook/001-overview.ipynb)
- [002-HowToGetStock.ipynb](file://hikyuu/examples/notebook/002-HowToGetStock.ipynb)
- [003-HowToGetKDataAndDraw.ipynb](file://hikyuu/examples/notebook/003-HowToGetKDataAndDraw.ipynb)
- [004-IndicatorOverview.ipynb](file://hikyuu/examples/notebook/004-IndicatorOverview.ipynb)
- [006-TradeManager.ipynb](file://hikyuu/examples/notebook/006-TradeManager.ipynb)
- [007-SystemDetails.ipynb](file://hikyuu/examples/notebook/007-SystemDetails.ipynb)
- [008-Pickle.ipynb](file://hikyuu/examples/notebook/008-Pickle.ipynb)
- [009-RealData.ipynb](file://hikyuu/examples/notebook/009-RealData.ipynb)
- [010-Portfolio.ipynb](file://hikyuu/examples/notebook/010-Portfolio.ipynb)
- [Turtle_SG.py](file://hikyuu/examples/Turtle_SG.py)
- [quick_crtsg.py](file://hikyuu/examples/quick_crtsg.py)
- [examples_init.py](file://hikyuu/examples/examples_init.py)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Getting Started Guide](#getting-started-guide)
3. [Strategy Examples](#strategy-examples)
4. [Backtesting Walkthrough](#backtesting-walkthrough)
5. [Real-Time Trading Example](#real-time-trading-example)
6. [Educational Value and Learning Progression](#educational-value-and-learning-progression)

## Introduction
The Hikyuu framework provides a comprehensive suite of examples and tutorials designed to help users quickly become productive with its powerful quantitative trading capabilities. These resources are structured to guide users from basic setup to advanced trading strategies, offering practical, hands-on guidance through interactive Jupyter notebooks and code examples. The tutorial ecosystem is built around real-world applications, demonstrating how to use the framework for data analysis, strategy development, backtesting, and live trading. By following these examples, users can gain a deep understanding of the framework's architecture and implementation choices while building their own trading systems. The examples are carefully designed to build upon each other in complexity, creating a natural learning progression from beginner to advanced concepts.

## Getting Started Guide
The getting started guide provides a comprehensive introduction to the Hikyuu framework, walking users through the basic setup and first steps. The guide begins with importing the necessary modules and initializing the environment, as demonstrated in the `examples_init.py` file which sets up the global stock manager and configuration. Users learn to access stock data through the global management object `sm` or the `get_stock` function, with stock identifiers formatted as "market identifier + stock code" (e.g., "sh000001" for the Shanghai Composite Index). The guide covers fundamental operations such as retrieving stock information, viewing weight information (dividends and splits), and understanding the structure of stock data. It also introduces the concept of blocks (groups of stocks) which allows users to work with specific market segments like the CSI 300 index. The interactive nature of the Jupyter notebooks enables users to immediately experiment with these concepts, making the learning process intuitive and engaging.

**Section sources**
- [002-HowToGetStock.ipynb](file://hikyuu/examples/notebook/002-HowToGetStock.ipynb)
- [examples_init.py](file://hikyuu/examples/examples_init.py)

## Strategy Examples
The strategy examples demonstrate various trading approaches and techniques, ranging from simple moving average crossovers to more sophisticated systems like the Turtle trading strategy. The Turtle strategy example, implemented in `Turtle_SG.py`, showcases a classic trend-following system that buys when the price exceeds the highest high of the past N days and sells when it falls below the lowest low of the past N days. This example illustrates the object-oriented approach to strategy development by defining a custom `TurtleSignal` class that inherits from `SignalBase`. An alternative implementation in `quick_crtsg.py` demonstrates a more concise functional approach using the `crtSG` function, allowing users to define strategies with minimal code. The examples also cover various money management techniques, such as fixed share counting (`MM_FixedCount`) and position sizing based on account equity. These strategy examples serve as templates that users can modify and extend to create their own trading systems, demonstrating the framework's flexibility and extensibility.

**Section sources**
- [Turtle_SG.py](file://hikyuu/examples/Turtle_SG.py)
- [quick_crtsg.py](file://hikyuu/examples/quick_crtsg.py)
- [007-SystemDetails.ipynb](file://hikyuu/examples/notebook/007-SystemDetails.ipynb)

## Backtesting Walkthrough
The backtesting walkthrough demonstrates the complete process from data loading to result analysis, providing users with a comprehensive understanding of how to evaluate trading strategies. The process begins with creating a simulated trading account using `crtTM` with an initial capital, as shown in the overview notebook. Users then create signal indicators, such as the `SG_Flex` indicator which uses exponential moving averages (EMA) to generate buy and sell signals when a fast EMA crosses above or below a slow EMA. The walkthrough shows how to combine these components into a complete trading system using `SYS_Simple`, which integrates the trading manager, signal generator, and money manager. After running the system on historical data with the `run` method, users learn to visualize the results by plotting the system signals and price data. The walkthrough culminates in performance analysis, where users generate detailed reports on key metrics such as total return, win rate, maximum drawdown, and risk-adjusted returns. This comprehensive approach enables users to thoroughly evaluate their strategies before deploying them in live trading.

**Section sources**
- [001-overview.ipynb](file://hikyuu/examples/notebook/001-overview.ipynb)
- [006-TradeManager.ipynb](file://hikyuu/examples/notebook/006-TradeManager.ipynb)

## Real-Time Trading Example
The real-time trading example illustrates how to connect to live data and execute trades, bridging the gap between backtesting and actual trading. This example demonstrates the use of the `realtime_update` function to fetch current market data from various sources such as Sina and QQ, as shown in the `009-RealData.ipynb` notebook. The framework's architecture supports real-time data integration through fetcher modules that can be easily extended to support additional data providers. The example shows how the same trading logic used in backtesting can be applied to real-time data, enabling users to transition their strategies from historical analysis to live execution. It also covers important considerations for real-time trading, such as update frequency and data latency. The integration with order brokers, as demonstrated in the trade manager notebook, shows how to connect the framework to actual brokerage accounts for automated trade execution, making it possible to implement fully automated trading systems.

**Section sources**
- [009-RealData.ipynb](file://hikyuu/examples/notebook/009-RealData.ipynb)
- [006-TradeManager.ipynb](file://hikyuu/examples/notebook/006-TradeManager.ipynb)

## Educational Value and Learning Progression
The educational value of the Hikyuu examples lies in their structured learning progression and practical focus, which helps users build expertise incrementally. The examples are organized in a logical sequence that mirrors the natural development of trading system knowledge, starting with basic data access in `002-HowToGetStock.ipynb` and progressing to more complex topics like indicator usage in `004-IndicatorOverview.ipynb` and portfolio management in `010-Portfolio.ipynb`. Each notebook serves as an interactive tutorial that combines explanatory text with executable code, allowing users to immediately apply what they've learned. The examples cover both conceptual overviews for beginners and technical details for experienced developers, providing insights into implementation choices such as the use of object-oriented design for extensibility and performance optimizations for handling large datasets. By working through these examples, users not only learn how to use the framework but also gain a deeper understanding of quantitative trading principles and best practices in algorithmic trading system development.

**Section sources**
- [000-Index.ipynb](file://hikyuu/examples/notebook/000-Index.ipynb)
- [004-IndicatorOverview.ipynb](file://hikyuu/examples/notebook/004-IndicatorOverview.ipynb)
- [010-Portfolio.ipynb](file://hikyuu/examples/notebook/010-Portfolio.ipynb)