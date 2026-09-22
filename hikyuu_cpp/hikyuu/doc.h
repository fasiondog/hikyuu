/*
 * doc.h
 *
 *  Created on: 2010-10-14
 *      Author: fasiondog
 */

#ifndef DOC_H_
#define DOC_H_

/******************************************************************************
 * This file is used by doxygen to generate the documentation only
 *****************************************************************************/

/**
 * @mainpage Hikyuu Quant Framework
 * Hikyuu Quant Framework is an open-source C++/Python quantitative trading research framework for
 * strategy analysis and backtesting (currently aimed at the domestic securities market). Its core
 * idea is based on the current mature systematic trading method: the whole systematic trading is
 * abstracted into seven components, namely the market environment judgment strategy, the system
 * valid condition, the signal generator, the stop-loss / take-profit strategy, the money management
 * strategy, the profit goal strategy and the slippage algorithm. You can build the strategy asset
 * library of these components separately and combine them freely in practical research to observe
 * the effectiveness and stability of the system as well as the effect of a single kind of strategy.
 *
 * For more information, please visit: <a href="http://hikyuu.org">http://hikyuu.org</a>
 */

/**
 * The core library, it contains the stock data management, the indicator implementation, the
 * trading system framework, etc.
 * @defgroup Hikyuu Hikyuu core engine library
 * @note Boost is used here as the basic C++ library, so it may depend heavily on Boost, which needs
 *       attention when porting
 *
 * @defgroup Base Base infrastructure
 * The base library, responsible for the Stock instance management, the K-line data reading, the
 * block management, etc.
 * @details The base library, responsible for the Stock instance management, the K-line data
 * reading, the block management, etc.
 * @ingroup Hikyuu
 *
 * @defgroup DataType DataType basic data type definitions
 * Define the required basic data types
 * @details Define the required basic data types
 * @ingroup Base
 *
 * @defgroup StockManage StockManage security management
 * The security management class, the security class, etc.
 * @details The security management class, the security class, etc.
 * @ingroup Base
 *
 * @defgroup DataDriver Data-Driver data driver engine
 * Read the market information, the security information, the block data, the K-line data, etc.
 * @details Read the market information, the security information, the block data, the K-line data,
 *          etc.; a custom data driver engine can be implemented as needed.
 * @ingroup Base
 *
 * @defgroup Indicator Indicator indicator library
 * The built-in common indicators and the infrastructure for defining and implementing new
 * indicators
 * @details It contains the indicator base class definition, some common indicators and the
 * indicator creation functions
 * @note
 * In a client program it is recommended to use the indicator creation functions as much as possible
 * to create the concrete indicator to be called, so as to avoid wrong memory allocation and release
 * \n Example: \n Indicator ma = MA(); \n std::cout << ma.name() << std::endl; \n
 * @ingroup Hikyuu
 *
 * @defgroup TradeManager TradeManager trade management
 * Trade management, responsible for recording every trade record and the current positions
 * @details Trade management, responsible for recording every trade record and the current positions
 * @ingroup Hikyuu
 *
 * @defgroup TradeCost TradeCost trade cost algorithm
 * The trade cost algorithms, such as the A-share cost calculation (stamp duty / commission, etc.)
 * @details The trade cost algorithms, such as the A-share cost calculation (stamp duty /
 * commission, etc.)
 * @ingroup TradeManager
 *
 * @defgroup TradeManagerClass TradeManager trade management class
 * Trade management can be understood as a simulated account for simulated trading. crtTM is
 * generally used to create a trade management instance.
 * @ingroup TradeManager
 *
 * @defgroup OrderBroker OrderBroker order broker
 * The order broker, it implements the actual order operations and the programmatic orders
 * @ingroup TradeManager
 *
 * @defgroup Performance Performance performance statistics
 * The performance statistics of the trades
 * @ingroup TradeManager
 *
 * @defgroup TradeSystem TradeSystem systematic trading framework
 * @details Detailed description of the systematic trading framework
 * @ingroup Hikyuu
 *
 * @defgroup Portfolio Portfolio portfolio management
 * @details The portfolio management system
 * @ingroup Hikyuu
 *
 * @defgroup Selector Selector trading object selection algorithm
 * @details Select the trading object
 * @ingroup Portfolio
 *
 * @defgroup AllocateFunds Allocate Funds asset allocation algorithm module
 * @details Allocate the funds
 * @ingroup Portfolio
 *
 * @defgroup Environment Environment external environment judgment
 * @details The external environment judgment module, used to judge whether the current market
 *          environment is valid; a buy operation happens only when the market is in a valid state.
 *          When the market enters an invalid state, the general strategy of the system is to
 *          immediately force a liquidation.
 * @ingroup TradeSystem
 *
 * @defgroup Condition Condition system valid condition judgment
 * @details It judges the precondition for the current system to be valid; when the system is in an
 *          invalid state no buy is suggested, and the held stocks are generally forced to be
 *          liquidated (the concrete behavior is decided by the concrete system strategy)
 * @ingroup TradeSystem
 *
 * @defgroup MoneyManager MoneyManager money management strategy
 * @details The money management strategy, it decides the quantity of every trade
 * @ingroup TradeSystem
 *
 * @defgroup Signal Signal signal generator
 * @details The signal generator module, including the various signal generator constructors. \n
 *          The signal generator is responsible for producing the buy and sell signals.
 * @ingroup TradeSystem
 *
 * @defgroup Stoploss Stoploss stop-loss / take-profit strategy
 * @details A strategy that stops the loss in time when the market trend goes against the direction
 *          predicted by the signal generator.
 * @ingroup TradeSystem
 *
 * @defgroup ProfitGoal ProfitGoal profit goal strategy
 * @details Before every trade is executed the trading goal is determined, so that the system
 *          suggests a sell trade after the price reaches the goal price
 * @ingroup TradeSystem
 *
 * @defgroup Slippage Slippage slippage algorithm
 * @details
 * The price indicated by a buy or sell signal generated by the system usually cannot be used to
 * actually buy or sell at exactly that price in a real environment, due to the operation delay, the
 * jump of the quoted price, and so on. The deviation between such a theoretical price and the price
 * at which the buy may actually happen is called "slippage". In a system simulation the influence
 * of the slippage should be fully considered, and it is also a test of the system stability, i.e. a
 * small deviation does not have a large influence on the final return of the system.
 * @ingroup TradeSystem
 *
 * @defgroup System System trading system
 * @details The trading system framework
 * @ingroup TradeSystem
 *
 * @defgroup MultiFactor MultiFactor multi-factor synthesis
 * @details Synthesize the multiple factors
 * @ingroup TradeSystem
 *
 * @defgroup Strategy Strategy strategy runtime
 * @details The strategy runtime
 * @ingroup Hikyuu
 *
 * @defgroup Agent Agent external data receiving and sending agent
 * @details Used to receive and send external data
 * @ingroup Hikyuu
 *
 * @defgroup Utilities Utilities program utility collection
 * A collection of independent small functions and object utilities
 * @details An additional public utility collection
 * @ingroup Hikyuu
 *
 * @defgroup DBConnect DB Connect database connection
 * The database connection utilities
 * @ingroup Utilities
 *
 * @defgroup ThreadPool Thread Pool thread pool
 * @ingroup Utilities
 *
 * @defgroup Views Data views
 * Data views, used to display and print the data
 * @ingroup Views
 */

/**
 * The Hikyuu core namespace, it contains the stock data management, the indicator implementation,
 * the trading system framework, etc.
 */
namespace hku {}

#endif /* DOC_H_ */
