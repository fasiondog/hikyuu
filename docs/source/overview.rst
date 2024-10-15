.. figure:: _static/00000-title.png

Hikyuu 简介
============

👉 项目首页：[https://hikyuu.org/](https://hikyuu.org/)

👉 帮助文档：[https://hikyuu.readthedocs.io/zh-cn/latest/index.html](https://hikyuu.readthedocs.io/zh-cn/latest/index.html)

👉 入门示例:  [https://nbviewer.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True](https://nbviewer.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True)

👉 策略部件库：[https://gitee.com/fasiondog/hikyuu_hub](https://gitee.com/fasiondog/hikyuu_hub)

👉 感谢网友提供的 Hikyuu Ubuntu虚拟机环境, 百度网盘下载(提取码: ht8j): [https://pan.baidu.com/s/1CAiUWDdgV0c0VhPpe4AgVw?pwd=ht8j](https://pan.baidu.com/s/1CAiUWDdgV0c0VhPpe4AgVw?pwd=ht8j)

Hikyuu Quant Framework是一款基于C++/Python的开源量化交易研究框架，用于策略分析及回测（仅受限于数据，如有数据也可用于期货等）。其核心思想基于当前成熟的系统化交易方法，将整个系统化交易抽象为由市场环境判断策略、系统有效条件、信号指示器、止损/止盈策略、资金管理策略、盈利目标策略、移滑价差算法七大组件，你可以分别构建这些组件的策略资产库，在实际研究中对它们自由组合来观察系统的有效性、稳定性以及单一种类策略的效果。例如：

::

    #创建模拟交易账户进行回测，初始资金30万
    my_tm = crtTM(init_cash = 300000)

    #创建信号指示器（以5日EMA为快线，5日EMA自身的10日EMA作为慢线，快线向上穿越慢线时买入，反之卖出）
    my_sg = SG_Flex(EMA(CLOSE(), n=5), slow_n=10)

    #固定每次买入1000股
    my_mm = MM_FixedCount(1000)

    #创建交易系统并运行
    sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
    sys.run(sm['sz000001'], Query(-150))

.. figure:: _static/10000-overview.png
        :width: 600px

完整示例参见：`<https://nbviewer.jupyter.org/github/fasiondog/hikyuu_examples/blob/master/001-overview.ipynb?flush_cache=True>`_

Maybe，你已经注意到了，上面没有“选股策略”？！是的，选股策略是股票交易的重要方面，肯定不会少。事实上，之前所述的交易系统都是针对一个交易对象的，也就是经常听到的策略，但很多所谓的“策略”其实仅仅只是买入、卖出的指示信号而已，并非完整的交易策略。为了区别，在这里直接以系统指称，表示一个完整的系统化交易方法或策略。而在系统之上，称为Portfolio资产组合，选股策略则是Portfolio的组件，Portfolio的另一重要组成则是资金分配策略，比如选股策略选定了4个交易对象（股票或基金等），那么如何在它们之间进行合理的资金分配？

所以，Hikyuu Quant Framework其实是在System和Portfolio基础之上、包含了九大策略组件：市场环境判断策略、系统有效条件、信号指示器、止损/止盈策略、资金管理策略、盈利目标策略、移滑价差算法、交易对象选择策略、资金分配策略。可以在此基础上构建自己的策略库，并进行灵活的组合和测试，甚至可以更进一步，在选择交易对象的同时，选取与之匹配的最优系统交易策略（System）。


为什么选择 Hikyuu？
--------------------

- **组合灵活，分类构建策略资产库** Hikyuu对系统化交易方法进行了良好的抽象，将完整的系统交易分为不同的策略组件接口，在进行策略探索时，可以更加专注于某一方面的策略性能与影响，可以构建自己的策略库累计资产，并灵活组合。其主要功能模块如下：

  .. figure:: _static/10002-function-arc.png
         :width: 600px

- **性能保障，打造自己的专属应用** 目前项目包含了3个主要组成部分：基于C++的核心库、对C++进行包装的Python库(hikyuu)、基于Python的交互式工具。

    - C++核心库，提供了整体的策略框架，在保证性能的同时，已经考虑了对多线程和多核处理的支持，在未来追求更高运算速度提供便利。C++核心库，可以单独剥离使用，自行构建自己的客户端工具。

    - Python库（hikyuu），提供了对C++库的包装，同时集成了talib库（如TA_SMA，对应talib.SMA），可以与numpy、pandas数据结构进行互相转换，为使用其他成熟的python数据分析工具提供了便利。
    
    - hikyuu.interactive 交互式探索工具，提供了K线、指标、系统信号等的基本绘图功能，用于对量化策略的探索和回测。

- **代码简洁，探索更便捷、自由** 同时支持面向对象和命令行编程范式。其中，命令行在进行策略探索时，代码简洁、探索更便捷、自由。
    
- **安全、自由、隐私，搭建自己的专属云量化平台** 结合 Python + Jupyter 的强大能力与云服务器，可以搭建自己专属的云量化平台。将Jupyter部署在云服务器上，随时随地的访问自己的云平台，即刻实现自己新的想法，如下图所示通过手机访问自己的云平台。结合Python强大成熟的数据分析、人工智能工具（如 numpy、scipy、pandas、TensorFlow)搭建更强大的人工智能平台。
 
- **数据存储方式可扩展** 目前支持本地HDF5格式、MySQL存储。默认使用HDF5，数据文件体积小、速度更快、备份更便利。截止至2017年4月21日，沪市日线数据文件149M、深市日线数据文件184M、5分钟线数据各不到2G。


想要更多了解Hikyuu？请使用以下方式联系：
---------------------------------------------------------------

更多示例与程序化交易分享（您的加入将视为对项目的捐赠），只保证对星球用户的有问必答，其它群视情况而定。

.. figure:: _static/zhishixingqiu.png

关注公众号：

.. figure:: _static/weixin_gongzhonghao.jpg

微信群（请备注：加入 Hikyuu)：

.. figure:: _static/weixin.jpg

QQ交流群（逐渐废弃）：114910869, 或扫码加入

.. figure:: _static/10003-qq.png
