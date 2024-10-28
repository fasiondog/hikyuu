![title](docs/source/_static/00000-title.png)

---

![img](https://static.pepy.tech/badge/hikyuu) ![img](https://static.pepy.tech/badge/hikyuu/month) ![img](https://static.pepy.tech/badge/hikyuu/week) ![img](https://github.com/fasiondog/hikyuu/workflows/win-build/badge.svg) ![img](https://github.com/fasiondog/hikyuu/workflows/ubuntu-build/badge.svg) ![img](https://img.shields.io/github/license/mashape/apistatus.svg)

Hikyuu Quant Framework是一款基于C++/Python的开源量化交易研究框架，用于策略分析及回测（目前主要用于国内A股市场）。其核心思想基于当前成熟的系统化交易方法，将整个系统化交易抽象为由市场环境判断策略、系统有效条件、信号指示器、止损/止盈策略、资金管理策略、盈利目标策略、移滑价差算法七大组件，你可以分别构建这些组件的策略资产库，在实际研究中对它们自由组合来观察系统的有效性、稳定性以及单一种类策略的效果。

👉 **项目地址：**

* [https://github.com/fasiondog/hikyuu](https://github.com/fasiondog/hikyuu)
* [https://gitee.com/fasiondog/hikyuu](https://gitee.com/fasiondog/hikyuu)
* [https://gitcode.com/hikyuu/hikyuu](https://gitcode.com/hikyuu/hikyuu)

👉 **项目首页：**[https://hikyuu.org/](https://hikyuu.org/)

👉 **帮助文档：**[https://hikyuu.readthedocs.io/zh-cn/latest/index.html](https://hikyuu.readthedocs.io/zh-cn/latest/index.html)

👉 **入门示例:**  [https://nbviewer.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True](https://nbviewer.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True)

👉 **策略部件库：**[https://gitee.com/fasiondog/hikyuu_hub](https://gitee.com/fasiondog/hikyuu_hub)

👉 感谢网友提供的 Hikyuu Ubuntu虚拟机环境, 百度网盘下载(提取码: ht8j): [https://pan.baidu.com/s/1CAiUWDdgV0c0VhPpe4AgVw?pwd=ht8j](https://pan.baidu.com/s/1CAiUWDdgV0c0VhPpe4AgVw?pwd=ht8j)

示例：

```python
    #创建模拟交易账户进行回测，初始资金30万
    my_tm = crtTM(init_cash = 300000)

    #创建信号指示器（以5日EMA为快线，5日EMA自身的10日EMA作为慢线，快线向上穿越慢线时买入，反之卖出）
    my_sg = SG_Flex(EMA(CLOSE(), n=5), slow_n=10)

    #固定每次买入1000股
    my_mm = MM_FixedCount(1000)

    #创建交易系统并运行
    sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
    sys.run(sm['sz000001'], Query(-150))
```

![img](docs/source/_static/10000-overview.png)

完整示例参见：[https://nbviewer.jupyter.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True](https://nbviewer.jupyter.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True)

# 为什么选择 Hikyuu？

* **组合灵活，分类构建策略资产库** Hikyuu对系统化交易方法进行了良好的抽象，包含了九大策略组件：市场环境判断策略、系统有效条件、信号指示器、止损/止盈策略、资金管理策略、盈利目标策略、移滑价差算法、交易对象选择策略、资金分配策略。可以在此基础上构建自己的策略库，并进行灵活的组合和测试。在进行策略探索时，可以更加专注于某一方面的策略性能与影响。其主要功能模块如下：![img](docs/source/_static/10002-function-arc.png)
* **性能保障，打造自己的专属应用** 目前项目包含了3个主要组成部分：基于C++的核心库、对C++进行包装的Python库(hikyuu)、基于Python的交互式工具。
  * AMD 7950x 实测：A股全市场（1913万日K线）仅加载全部日线计算 20日 MA 并求最后 MA 累积和，首次执行含数据加载 耗时 6秒，数据加载完毕后计算耗时 166 毫秒，详见: [性能实测](https://mp.weixin.qq.com/s?__biz=MzkwMzY1NzYxMA==&mid=2247483768&idx=1&sn=33e40aa9633857fa7b4c7ded51c95ae7&chksm=c093a09df7e4298b3f543121ba01334c0f8bf76e75c643afd6fc53aea1792ebb92de9a32c2be&mpshare=1&scene=23&srcid=05297ByHT6DEv6XAmyje1oOr&sharer_shareinfo=b38f5f91b4efd8fb60303a4ef4774748&sharer_shareinfo_first=b38f5f91b4efd8fb60303a4ef4774748#rd)
  * C++核心库，提供了整体的策略框架，在保证性能的同时，已经考虑了对多线程和多核处理的支持，在未来追求更高运算速度提供便利。C++核心库，可以单独剥离使用，自行构建自己的客户端工具。
  * Python库（hikyuu），提供了对C++库的包装，同时集成了talib库（如TA_SMA，对应talib.SMA），可以与numpy、pandas数据结构进行互相转换，为使用其他成熟的python数据分析工具提供了便利。
  * hikyuu.interactive 交互式探索工具，提供了K线、指标、系统信号等的基本绘图功能，用于对量化策略的探索和回测。
* **代码简洁，探索更便捷、自由** 同时支持面向对象和命令行编程范式。其中，命令行在进行策略探索时，代码简洁、探索更便捷、自由。
* **安全、自由、隐私，搭建自己的专属云量化平台** 结合 Python + Jupyter 的强大能力与云服务器，可以搭建自己专属的云量化平台。将Jupyter部署在云服务器上，随时随地的访问自己的云平台，即刻实现自己新的想法，如下图所示通过手机访问自己的云平台。结合Python强大成熟的数据分析、人工智能工具（如 numpy、scipy、pandas、TensorFlow)搭建更强大的人工智能平台。
* **数据存储方式可扩展** 目前支持本地HDF5格式、MySQL存储。默认使用HDF5，数据文件体积小、速度更快、备份更便利。截止至2017年4月21日，沪市日线数据文件149M、深市日线数据文件184M、5分钟线数据各不到2G。

![img](https://api.star-history.com/svg?repos=fasiondog/hikyuu&type=Date "Star History Chart")

# 想要更多了解Hikyuu？请使用以下方式联系：

## **加入知识星球**

更多示例与程序化交易的分享（您的加入将视为对项目的捐赠）。**作者只保证对知识星球用户有问必答，其他渠道视情况。(公众号文章末尾可能不定期发放优惠券)**

![知识星球](docs/source/_static/zhishixingqiu.png)

## 关注公众号：

![img](docs/source/_static/weixin_gongzhonghao.jpg)

## 加入微信群（请注明“加入hikyuu”）：

![weixin](docs/source/_static/weixin.jpg)

## QQ交流群：114910869, 或扫码加入

![img](docs/source/_static/10003-qq.png)
