/*
 * doc.h
 *
 *  Created on: 2010-10-14
 *      Author: fasiondog
 */

#ifndef DOC_H_
#define DOC_H_

/******************************************************************************
 * 该文件仅用于doxygen生成文档
 *****************************************************************************/

/**
 * @mainpage Hikyuu Quant Framework
 * Hikyuu Quant Framework是一款基于C++/Python的开源量化交易研究框架，用于策略分析及回测 （目前
 * 用于国内证券市场）。其核心思想基于当前成熟的系统化交易方法，将整个系统化交易抽象为由市场环境判断策略、
 * 系统有效条件、信号指示器、止损/止盈策略、资金管理策略、盈利目标策略、移滑价差算法七大组件，你可以分别
 * 构建这些组件的策略资产库，在实际研究中对它们自由组合来观察系统的有效性、稳定性以及单一种类策略的效果。
 *
 * 更多信息，请访问：<a href="http://hikyuu.org">http://hikyuu.org</a>
 */

/**
 * 核心库，包含股票数据的管理、指标实现、交易系统框架等
 * @defgroup Hikyuu Hikyuu 核心引擎库
 * @note 这里把Boost库作为C++的基础库使用，可能严重依赖于Boost库，移植时需要注意
 *
 * @defgroup Base Base 基础设施
 * 基础库，负责Stock实例管理、K线数据读取、板块管理等
 * @details 基础库，负责Stock实例管理、K线数据读取、板块管理等
 * @ingroup Hikyuu
 *
 * @defgroup DataType DataType 基础数据类型定义
 * 定义所需的基础数据类型
 * @details 定义所需的基础数据类型
 * @ingroup Base
 *
 * @defgroup StockManage StockManage 证券管理
 * 证券管理类、证券类等
 * @details 证券管理类、证券类等
 * @ingroup Base
 *
 * @defgroup DataDriver Data-Driver 数据驱动引擎
 * 读取市场信息、证券信息、板块数据、K线数据等
 * @details 读取市场信息、证券信息、板块数据、K线数据等；可根据需要实现自定义的数据驱动引擎。
 * @ingroup Base
 *
 * @defgroup Indicator Indicator 指标库
 * 内建常用指标，及新指标定义实现基础设施
 * @details 包含指标基类定义、部分常用指标以及指标的生成函数
 * @note
 * 在客户端程序中，建议尽可能使用指标的生成函数，生成具体的指标进行调用，以避免错误的内存申请和释放
 * \n 示例： \n Indicator ma = MA(); \n std::cout << ma.name() << std::endl; \n
 * @ingroup Hikyuu
 *
 * @defgroup TradeManager TradeManager 交易管理
 * 交易管理，负责记录每一笔交易记录及当前的持仓情况
 * @details 交易管理，负责记录每一笔交易记录及当前的持仓情况
 * @ingroup Hikyuu
 *
 * @defgroup TradeCost TradeCost 交易成本算法
 * 交易成本算法，如A股成本计算（印花税/佣金等）
 * @details 交易成本算法，如A股成本计算（印花税/佣金等）
 * @ingroup TradeManager
 *
 * @defgroup TradeManagerClass TradeManager 交易管理类
 * 交易管理可理解为一个模拟账户进行模拟交易。一般使用 crtTM 创建交易管理实例。
 * @ingroup TradeManager
 *
 * @defgroup OrderBroker OrderBroker 订单代理
 * 订单代理，实现实际的订单操作及程序化的订单
 * @ingroup TradeManager
 *
 * @defgroup Performance Performance 绩效统计
 * 对交易进行绩效统计
 * @ingroup TradeManager
 *
 * @defgroup TradeSystem TradeSystem 系统交易框架
 * @details 系统交易框架详细说明
 * @ingroup Hikyuu
 *
 * @defgroup Portfolio Portfolio 投资组合管理
 * @details 投资组合管理系统
 * @ingroup Hikyuu
 *
 * @defgroup Selector Selector 交易对象选择算法
 * @details 选择交易对象
 * @ingroup Portfolio
 *
 * @defgroup AllocateFunds Allocate Funds 资产分配算法模块
 * @details 进行资金分配
 * @ingroup Portfolio
 *
 * @defgroup Environment Environment 外部环境判断
 * @details 外部环境判断模块，用于判断当前的市场环境是否有效，只有在市场
 *          处于有效状态中，才会发生买入操作。当市场进入失效状态下，系统
 *          的一般策略是立即强行清仓。
 * @ingroup TradeSystem
 *
 * @defgroup Condition Condition 系统有效条件判断
 * @details 用于当前系统有效的前提条件判断，当系统处于失效状态时，不会提示买入，
 *          已持仓的股票一般会被强行指示清仓（具体行为有具体的系统策略决定）
 * @ingroup TradeSystem
 *
 * @defgroup MoneyManager MoneyManager 资金管理策略
 * @details 资金管理策略，决定每次交易的数量
 * @ingroup TradeSystem
 *
 * @defgroup Signal Signal 信号指示器
 * @details 信号指示器模块，包括各种信号指示器构造函数。\n
 *         信号指示器负责产生买入、卖出信号。
 * @ingroup TradeSystem
 *
 * @defgroup Stoploss Stoploss 止损、止赢策略
 * @details 在市场走势和信号指示器预测的方向相反时，及时止住损失的策略。
 * @ingroup TradeSystem
 *
 * @defgroup ProfitGoal ProfitGoal 盈利目标策略
 * @details 每笔交易执行前，确定交易目标，用于系统在价格达到目标价位后提示卖出交易
 * @ingroup TradeSystem
 *
 * @defgroup Slippage Slippage 移滑价差算法
 * @details
 * 系统产生的买入或卖出信号所指示的价格，在真实的环境中通常无法准确的以该价格进行实际的买入或卖出
 *         活动，如操作的延时、出价的跳跃等等。这种理论价格和直接可能买入的价格之间的偏差称为“移滑价差”。
 *         在系统模拟中，应充分考虑移滑价差带来的影响，同时也是对系统稳定性的一种检验，即较小的偏差，不会
 *         对系统最终的收益存在较大的影响。
 * @ingroup TradeSystem
 *
 * @defgroup System System 交易系统
 * @details 交易系统框架
 * @ingroup TradeSystem
 *
 * @defgroup MultiFactor MultiFactor 合成多因子
 * @details 合成多因子
 * @ingroup TradeSystem
 *
 * @defgroup Strategy Strategy 策略运行时
 * @details 策略运行时
 * @ingroup Hikyuu
 *
 * @defgroup Agent Agent 对外数据接收发送代理
 * @details 用于接收和发送对外数据
 * @ingroup Hikyuu
 *
 * @defgroup Utilities Utilities 程序工具集
 * 独立的一些小函数、对象工具集合
 * @details 附加的公共工具集
 * @ingroup Hikyuu
 *
 * @defgroup DBConnect DB Connect 数据库连接
 * 数据库连接工具
 * @ingroup Utilities
 *
 * @defgroup ThreadPool Thread Pool 线程池
 * @ingroup Utilities
 */

/**
 * Hikyuu核心命名空间，包含股票数据的管理、指标实现、交易系统框架等
 */
namespace hku {}

#endif /* DOC_H_ */
