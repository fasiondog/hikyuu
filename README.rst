.. image:: http://fasiondog.gitee.io/hikyuu/images/00000_title.png
    :target: http://fasiondog.gitee.io/hikyuu
    :align: left
    :alt: Hikyuu

-----------

.. image:: https://github.com/fasiondog/hikyuu/workflows/win-build/badge.svg
    :target: https://github.com/fasiondog/hikyuu/actions

.. image:: https://img.shields.io/github/license/mashape/apistatus.svg
    :target: https://github.com/fasiondog/hikyuu/blob/master/LICENSE.txt
    :alt: GitHub


Hikyuu Quant Framework是一款基于C++/Python的开源量化交易研究框架，用于策略分析及回测（目前主要用于国内A股市场）。其核心思想基于当前成熟的系统化交易方法，将整个系统化交易抽象为由市场环境判断策略、系统有效条件、信号指示器、止损/止盈策略、资金管理策略、盈利目标策略、移滑价差算法七大组件，你可以分别构建这些组件的策略资产库，在实际研究中对它们自由组合来观察系统的有效性、稳定性以及单一种类策略的效果。

详细文档： `<https://hikyuu.org/>`_

如果上述网站无法访问，请戳这里： `<http://fasiondog.gitee.io/hikyuu/>`_

祝贺 HIKYUU 入选 GITEE 最有价值开源项目 GVP
-----------------------------------------------

.. image:: http://fasiondog.gitee.io/hikyuu/images/gitee_GVP.png
    :target: https://gitee.com/gvp
    :alt: Gitee
    

**给作者加点油，每天扫扫红包，或者请作者喝杯咖啡**

.. image:: http://fasiondog.gitee.io/hikyuu/images/juanzeng.jpg


示例：

::

    #创建模拟交易账户进行回测，初始资金30万
    my_tm = crtTM(init_cash = 300000)

    #创建信号指示器（以5日EMA为快线，5日EMA自身的10日EMA作为慢线，快线向上穿越慢线时买入，反之卖出）
    my_sg = SG_Flex(OP(EMA(n=5)), slow_n=10)

    #固定每次买入1000股
    my_mm = MM_FixedCount(1000)

    #创建交易系统并运行
    sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
    sys.run(sm['sz000001'], Query(-150))

.. figure:: http://fasiondog.gitee.io/hikyuu/images/10000-overview.png
        :width: 600px

完整示例参见：`<https://nbviewer.jupyter.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True>`_


为什么选择 Hikyuu？
--------------------

- **组合灵活，分类构建策略资产库** Hikyuu对系统化交易方法进行了良好的抽象，包含了九大策略组件：市场环境判断策略、系统有效条件、信号指示器、止损/止盈策略、资金管理策略、盈利目标策略、移滑价差算法、交易对象选择策略、资金分配策略。可以在此基础上构建自己的策略库，并进行灵活的组合和测试。在进行策略探索时，可以更加专注于某一方面的策略性能与影响。其主要功能模块如下：

  .. figure:: http://fasiondog.gitee.io/hikyuu/images/10002-function-arc.png
        :width: 600px

- **性能保障，打造自己的专属应用** 目前项目包含了3个主要组成部分：基于C++的核心库、对C++进行包装的Python库(hikyuu)、基于Python的交互式工具。

    - 百万级别 K 线数据，2~3秒内完成 A 股全市场回测

    - C++核心库，提供了整体的策略框架，在保证性能的同时，已经考虑了对多线程和多核处理的支持，在未来追求更高运算速度提供便利。C++核心库，可以单独剥离使用，自行构建自己的客户端工具。

    - Python库（hikyuu），提供了对C++库的包装，同时集成了talib库（如TA_SMA，对应talib.SMA），可以与numpy、pandas数据结构进行互相转换，为使用其他成熟的python数据分析工具提供了便利。
    
    - hikyuu.interactive 交互式探索工具，提供了K线、指标、系统信号等的基本绘图功能，用于对量化策略的探索和回测。

- **代码简洁，探索更便捷、自由** 同时支持面向对象和命令行编程范式。其中，命令行在进行策略探索时，代码简洁、探索更便捷、自由。
    
- **安全、自由、隐私，搭建自己的专属云量化平台** 结合 Python + Jupyter 的强大能力与云服务器，可以搭建自己专属的云量化平台。将Jupyter部署在云服务器上，随时随地的访问自己的云平台，即刻实现自己新的想法，如下图所示通过手机访问自己的云平台。结合Python强大成熟的数据分析、人工智能工具（如 numpy、scipy、pandas、TensorFlow)搭建更强大的人工智能平台。
 
- **数据存储方式可扩展** 目前支持本地HDF5格式、MySQL存储。默认使用HDF5，数据文件体积小、速度更快、备份更便利。截止至2017年4月21日，沪市日线数据文件149M、深市日线数据文件184M、5分钟线数据各不到2G。

.. image:: https://api.star-history.com/svg?repos=fasiondog/hikyuu&type=Date
    :target: https://star-history.com/#fasiondog/hikyuu&Date
    :alt: Star History Chart
