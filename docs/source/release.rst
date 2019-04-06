版本发布说明
===============

1.1.0 - 2019年2月28日
-------------------------

1. 复权增加周线及其以上支持
2. 支持历史分笔、分时数据
3. 添加日志打印的等级控制
4. MoneyManagerBase增加对成本计算
5. Datetime增加 dateOfWeek,startOfWeek,endOfWeek,nextWeek,preWeek等系列便捷方法
6. fix：Stock.realtimeUpdate中未判断缓存未空的情况
7. fix：io重定向中未进行重复open的判定
8. fix：Block分类显示乱码
9. 简化源码安装方式，支持 python setup.py
10. 全新的快速数据下载工具（支持GUI及命令行，如下图所示），下载当日权息、日线、分钟线、分笔、分时数据耗时2~4分钟（视个人网络有所不同），同时不再需要通过证券客户端下载盘后数据。具体参见：`<https://hikyuu.readthedocs.io/zh_CN/latest/quickstart.html>`_

.. figure:: _static/install-20190228.png


1.0.9 - 2018年10月23日
-------------------------

1. 更新周线、月线等周线及其之上的K线BAR记录，从以开始时间为准，改为以结束时间为准。（如从老版本升级，需手工删除sh_day.h5、sz_day.h5文件中的week、month等目录，只保留data目录。可运行 tools/delelte_index.py 完成删除，运行前请自行修改相关文件路径等信息）。
2. 实现将C++中的日志输出重定向至Python，使Jupyter notebook可以看到C++部分的打印信息提示。注意：部分情景可能导致notebook因打印信息过多失去响应，此时可在产生较多打印信息的命令之前运行“iodog.close()”关闭重定向，后续可以再使用“iodog.open()”重新打开重定向信息输出。
3. Datetime增加nextDay、dayOfWeek、dayOfYear、endOfMonth方法。
4. TradeManager增加直接加入交易记录的方法（addTradeRecord）。
5. 升级使用的依赖库 boost、libmysql、hdf5
6. 使用xmake重构编译工程并调整代码结构
7. 试验linux下pip打包安装。linux下可使用 pip install hikyuu 命令完成安装，安装前需安装依赖的软件包（sudo apt-get install -y libhdf5-dev libhdf5-serial-dev libmysqlclient-dev）
8. 支持MacOSX下源码编译



1.0.8 - 2018年1月22日
-------------------------

1. 实现一个简单资产组合回测框架 PF_Simple（多标的、相同策略），因目标是多标的、多策略的资产组合框架，所以后续接口可能变化！
2. 新增固定列表选择器 SE_Fixed 配合 PF_Simple 使用。
3. 新增一个固定持仓天数的盈利目标策略 PG_FixedHoldDays。
4. Datetime增加 dayOfWeek、dayOfYear、endOfMonth 方法。
5. System增加 ev_open_position、cn_open_position参数，控制是否使用环境判断和系统有效性策略作为建仓信号，默认为False。
6. 资金管理策略（MoneyManagerBase)加入公共参数disable_ev_force_clean_position、disable_cn_force_clean_position，控制是否禁用市场环境及系统条件强制清仓。
7. 资金管理策略（MoneyManagerBase）中，获取买入/卖出数量接口中增加系统来源组件参数。
8. 所有系统策略组件clone方法增加保护，在子类clone失败时返回自身。
9. 合入网友哥本哈根达斯反馈的复权修改。
10. matplotlib调整默认绘图窗口大小。
11. 解决echarts绘制macd缺失缩放的问题。
12. TradeManager缺失引出currentCash函数至python。
13. MoneyManager缺失引出getTM函数至python。



1.0.7 - 2017年12月15日
-------------------------

1、合入网友哥本哈根达斯提供的修改，复权时不处理只有股本变化的权息记录，和通达信等软件处理保持一致。

2、增加使用 pyecharts 的绘图引擎，可在 notebook 或 网页 环境中使用。echarts 绘图速度比 matplotlib 快，尤其是在K线数据较大时，提速明显，且可以自由缩放和拖动。在 notebook 环境中，可使用如下语句切换绘图引擎：

::

    use_draw_engine('echarts')  #默认为 use_draw_engine('matplotlib')



1.0.6 - 2017年11月20日
-------------------------

1. 完善Python帮助，以便在Shell中直接使用 help(cmd) 查询
2. 修改数据驱动，支持直接使用Python编写数据驱动。实现使用 pytdx 作为K线数据驱动的示例，详见安装目录下“data_driver\pytdx_data_driver.py”。如有需要使用MySQL、CSV等存储K线数据的，可参考该示例自行实现。
3. 优化了初始化过程，可不使用ini文件进行初始化，如实现自己的客户端，可参考“interactive.interactive.py”中初始化过程。
4. 简化了数据配置文件， **如安装了1.0.5及其之前的版本，需要重新运行 python hku_config.py 进行配置，或手工修改配置文件** 。 
5. 修复Bug，TradeManager::getProfitCurve未对长度为0的dates进行保护
6. 修正系统止损策略部件的缩写不一致问题


1.0.5 - 2017年9月25日
-------------------------

1. 增加载入临时的CSV K线数据功能，可用于期货或A股之外的数据测试。详情参见 StockManager 的 addTempCsvStock、removeTempCsvStock 方法帮助。
2. CVAL指标支持创建指定长度的固定数值指标
3. Datetime 的方法 maxDatetime、minDatetime 更名为 max、min
4. 增加 getDateRange 函数，获取指定的日历日期列表
5. 调整部分 Python 代码结构，补充和完善帮助信息


1.0.4 - 2017年7月5日
----------------------

1、Indicator、Operand 支持直接AND和OR操作，如：

::

    c = CLOSE(c)
    #由于语法问题，不能直接使用关键字and，采用&、|来表达与、或的操作
    x = c & 1

2、实现邮件发送订单代理，如：

::

    #创建模拟交易账户进行回测，初始资金30万
    my_tm = crtTM(initCash = 300000)

    #可以同时注册多个订单代理，同时实现打印、发送邮件、实盘下单动作
    #TestOerderBroker是测试用订单代理对象，只打印
    my_tm.regBroker(crtOB(TestOrderBroker())) 

    #注册邮件订单代理，在发出买入、卖出信号时，给自己发邮件，同时指示买入、卖出的数量
    my_tm.regBroker(crtOB(MailOrderBroker("smtp.sina.com", "yourmail@sina.com", "yourpwd", "receivermail@XXX.yy)))

    #Puppet为内建的扯线木偶实盘下单对象
    my_tm.regBroker(crtOB(Puppet()))

3、TradeManager中增加保存执行操作命令的功能，便于用于实盘时进行校准和修正，可直接在python客户端中重新执行买入、卖出动作便于复盘。可使用TM的公共参数“save_action”进行设置（默认为True）。保存的命令序列示例如下：

::

    my_tm = crtTM(datetime=Datetime('2017-Jan-01 00:00:00'), initCash=100000, costFunc=TC_Zero(), name='SYS')
    td = my_tm.buy(Datetime('2017-Jan-03 00:00:00'), sm['SZ000001'], 9.11, 100, 0, 0, 0, 8)
    td = my_tm.sell(Datetime('2017-Feb-21 00:00:00'),sm['SZ000001'], 9.6, 100, 0, 0, 0, 8)
    
4、修正hku_config.py在指定的数据目录已经存在的情况下出现的错误。

5、上传并修改直接从网络下载权息文件的importdata.py（代替使用钱龙下载权限数据），方便用户使用。使用前提，需要在系统PATH中能够找到unrar.exe文件（通常在winrar安装路径下）。通过在cmd中执行 python importdata.py 命令，代替直接执行importdata.exe。

6、解决Ubuntu下的编译问题，配合网友 pchaos 生成 docker 解决方案，如希望在Linux环境下运行hikyuu，可使用pchaos提供的docker解决方案，地址：`<https://gitee.com/pchaos/Docker-hikyuu>`_


1.0.3 - 2017年7月3日
------------------------

1、Indicator、Operand 支持直接和数字进行四则运算及比较运算，如：

::

    c = CLOSE(k)
    x = c + 100

2、增加 SG_Bool 布尔信号指示器，直接分别通过类似bool数据的方式指定买入、卖出信号，进一步简化信号指示器创建方式。如，海龟通道突破系统（大于20日买入、小于10日卖出），可简化为以下写法： 

::

    h = OP(OP(REF(1)),OP(HHV(n=20)))
    l = OP(OP(REF(1)),OP(LLV(n=10)))
    my_sg = SG_Bool(OP(CLOSE()) > h, OP(CLOSE()) < l)

3、支持实盘交易，可轻易绑定其他实盘下单程序，只要下单对象拥有 buy 和 sell 方法。本次发布内建了实盘下单交易程序“扯线木偶”，可直接使用，感谢“睿瞳深邃”的共享。也可以借助easytrader和easyquant的事件处理框架自行实现自动化交易。示例见下，只需使用“my_tm.regBroker(crtOB(Puppet()))”类似方法向TradeManager实例注册订单代理程序即可。更具体的使用方法，欢迎入群讨论。

::

    #创建模拟交易账户进行回测，初始资金30万
    my_tm = crtTM(initCash = 300000)

    #注册实盘交易订单代理
    my_tm.regBroker(crtOB(TestOrderBroker())) #TestOerderBroker是测试用订单代理对象，只打印
    #my_tm.regBroker(crtOB(Puppet()))  #Puppet为内建的扯线木偶实盘下单对象

    #根据需要修改订单代理最后的时间戳，后续只有大于该时间戳时，订单代理才会实际发出订单指令
    my_tm.brokeLastDatetime=Datetime(201706010000)

    #创建信号指示器（以5日EMA为快线，5日EMA自身的10日EMA最为慢线，快线向上穿越慢线时买入，反之卖出）
    my_sg = SG_Flex(OP(EMA(n=5)), slow_n=10)

    #固定每次买入1000股
    my_mm = MM_FixedCount(1000)

    #创建交易系统并运行
    sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
    sys.run(sm['sz000001'], Query(-150))


1.0.2 - 2017年6月19日
-----------------------

修复延迟操作情况下止损未按预期卖出的BUG（建议升级）

其他开发工程调整：

- 建立VS2010工程，供VS开发爱好者使用
- 删除notebook示例代码，移至单独的项目，方便普通用户打包下载
- 优化Boost.Build编译工程，完成Linux gcc编译


1.0.1 - 2017年5月30日
-----------------------

1. 改变安装方式，支持 pip install hikyuu
2. 完善快速配置脚本 hku_config.py
3. 增加特殊的资金管理策略 MM_Nothing（不做资金管理，方便对比测试）
4. 修复 tushare 升级后，无法从 tushare 获取实时日线更新的问题
5. 修改 realtimeUpdate，将允许的更新间隔作为函数参数，防止被sina或qq设为黑名单


1.0.0 - 2017年4月28日
-----------------------

2017年4月28日发布初始版本
2017年5月12日发布32位安装包