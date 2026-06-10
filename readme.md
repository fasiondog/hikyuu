![title](docs/source/_static/00000-title.png)

---

![img](https://static.pepy.tech/badge/hikyuu) ![img](https://static.pepy.tech/badge/hikyuu/month) ![img](https://static.pepy.tech/badge/hikyuu/week) ![img](https://github.com/fasiondog/hikyuu/workflows/win-build/badge.svg) ![img](https://github.com/fasiondog/hikyuu/workflows/ubuntu-build/badge.svg) ![License](https://img.shields.io/github/license/fasiondog/hikyuu.svg)

Hikyuu Quant Framework 是一款基于 C++/Python 开发的**开源超高速量化研究框架**，免费开放、无使用限制。项目深耕国内量化金融技术研究，深度适配 A 股市场数据体系，聚焦策略建模、量化模拟回测与金融数据分析，核心能力覆盖量化模型研发、高速金融计算、标准化回测体系、研究能力拓展四大方向。

框架依托成熟的系统化交易研究理念，将量化分析体系拆解为**市场环境研判、策略生效条件判定、信号指标解析、盈亏风控模型、资金配比模型、收益目标测算、滑点模拟算法、多因子建模、投资组合分析、资金分配**等独立模块化组件。用户可自由组合模块、搭建专属策略模型库，通过模拟回测验证策略稳定性与有效性，完成量化策略研究与数据分析工作。同时框架预留拓展接口，支持开发者自主开发、对接合规的第三方交易接口（如 QMT 等官方合规终端接口），满足个性化技术拓展与私有适配需求。

**免责声明** ：本项目为开源金融技术研究工具，仅供个人学习、学术研究与数据分析使用，不构成任何投资建议与交易指导，不提供、不内置证券交易服务。框架仅提供通用接口拓展能力，仅建议用户对接持牌机构提供的合规交易终端接口；用户自主新增、对接各类交易接口、开发拓展功能以及对应的实操行为，均由用户自行承担全部风险与法律责任，严禁对接非法交易通道、用于违规交易场景。

安装及使用，请先看帮助文档，谢谢！[https://hikyuu.readthedocs.io/zh-cn/latest/index.html](https://hikyuu.readthedocs.io/zh-cn/latest/index.html)

👉 **项目地址：**

* [https://github.com/fasiondog/hikyuu](https://github.com/fasiondog/hikyuu)
* [https://gitee.com/fasiondog/hikyuu](https://gitee.com/fasiondog/hikyuu)
* [https://gitcode.com/hikyuu/hikyuu](https://gitcode.com/hikyuu/hikyuu)

👉 **项目首页：**[https://hikyuu.org/](https://hikyuu.org/)

👉 **帮助文档：**[https://hikyuu.readthedocs.io/zh-cn/latest/index.html](https://hikyuu.readthedocs.io/zh-cn/latest/index.html)

👉 **入门示例:**[https://nbviewer.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True](https://nbviewer.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True)

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

# 🔥 为什么选择 Hikyuu？

**💹 组合灵活，分类构建策略资产库**

Hikyuu 对系统化交易方法进行了轻量化抽象，涵盖市场环境判断、系统有效条件、信号指示器、止损 / 止盈策略、资金管理、盈利目标、滑点算法、交易对象筛选、资金分配等核心组件。你可以基于这些组件自由搭建专属策略库，灵活组合、高效回测，并在策略探索时专注于单一模块的效果与影响，大幅提升研究效率。主要功能架构如下：![img](docs/source/_static/10002-function-arc.png)

**🚀 极致性能，轻松构建专属量化应用**

项目由三大部分构成：高性能 C++ 核心库、Python 接口层（hikyuu）、以及交互式探索工具。

* AMD 7950x 实测：A 股全市场 1913 万日 K 线，首次加载 + 计算 20 日均线并求和，仅需 6 秒；数据预热后，同操作耗时仅需 166 毫秒。性能实测: [性能实测](https://mp.weixin.qq.com/s?__biz=MzkwMzY1NzYxMA==&mid=2247483768&idx=1&sn=33e40aa9633857fa7b4c7ded51c95ae7&chksm=c093a09df7e4298b3f543121ba01334c0f8bf76e75c643afd6fc53aea1792ebb92de9a32c2be&mpshare=1&scene=23&srcid=05297ByHT6DEv6XAmyje1oOr&sharer_shareinfo=b38f5f91b4efd8fb60303a4ef4774748&sharer_shareinfo_first=b38f5f91b4efd8fb60303a4ef4774748#rd)
* C++ 核心库：内置完整策略框架，原生支持多线程与多核加速，为超高算力场景预留扩展空间；核心库可独立剥离使用，帮助开发者快速构建自定义量化工具。
* Python接口层（hikyuu）对 C++ 核心进行轻量化封装，集成 TA-Lib，支持与 numpy、pandas 无缝互转，轻松对接主流 Python 数据分析生态。
* hikyuu.interactive 交互式探索工具，内置 K 线、指标、信号可视化能力，适合快速策略验证与回测分析。

**🍳 语法简洁，策略探索更高效自由**

同时支持面向对象与命令行两种编程范式。尤其在策略探索阶段，命令行风格语法极简、表达直观，让你更快验证想法、迭代策略。

**🔐 自主可控，搭建专属云量化平台**

结合 Python + Jupyter 与云服务器，即可搭建完全自主可控的云量化平台。部署后随时随地访问（手机、平板、电脑均可使用），快速落地新想法。同时可无缝对接 numpy、scipy、pandas、TensorFlow 等成熟 AI 与数据分析工具，构建智能量化系统。也可按需自定义界面、实现服务化部署。

**🎁 模块化可扩展数据存储**

目前支持本地 HDF5、MySQL 两种存储方式，默认采用 HDF5（文件体积小、读写速度快、备份便捷）。截至 2017 年 4 月 21 日，沪市日线数据文件仅 149MB，深市 184MB，5 分钟线数据整体小于 2GB。通过插件可扩展 ClickHouse 存储，其读写速度优于 HDF5、空间占用远低于 MySQL，更适配分钟级及以下粒度的高频数据存储。

# 🍺 想要更多了解Hikyuu？请使用以下方式联系：

**因作者精力有限，对捐赠用户的问题会优先解答、确保响应；非捐赠用户的咨询会在时间允许的情况下尽力回复😁**

交流渠道以微信群为主、QQ 群为辅。**加入任一社群请备注 “加入 hikyuu”，未备注将无法通过验证哦～**

![img](docs/source/_static/qun.png)

## 🎉 感谢捐赠，让 Hikyuu 走得更远

🎁 [**捐赠计划与附赠参见**：https://hikyuu.readthedocs.io/zh-cn/latest/vip/donate-plan.html](https://hikyuu.readthedocs.io/zh-cn/latest/vip/donate-plan.html)

| 说明                                                                                       | 扫码方式               | 捐赠链接（与下方二维码同）                                      |
| ------------------------------------------------------------------------------------------ | ---------------------- | --------------------------------------------------------------- |
| 请作者喝杯☕️（30元）                                                                     | 支付宝                 | [https://pay.ldxp.cn/item/gflv3v](https://pay.ldxp.cn/item/gflv3v) |
| 订阅180天（50元）                                                                          | 支付宝                 | [https://pay.ldxp.cn/item/du4h8s](https://pay.ldxp.cn/item/du4h8s) |
| 订阅365天（100元）                                                                         | 支付宝                 | [https://pay.ldxp.cn/item/ehbz9b](https://pay.ldxp.cn/item/ehbz9b) |
| 加入星球<br />(3台设备及其他，<br />首年300元， 续费半价）<br />另有单独星球微信群及部件库 | 微信/<br />知识星球app | [https://t.zsxq.com/YSATD](https://t.zsxq.com/YSATD)               |

![img](docs/source/_static/dingyue.png)

## ⭐️ 支持项目，点亮你的星

![img](https://api.star-history.com/svg?repos=fasiondog/hikyuu&type=Date "Star History Chart")

## 私域定制

![img](docs/source/_static/hikyuu_ads_640px.png)

## 项目依赖说明

Hikyuu 的 C++ 核心模块直接依赖以下开源项目（间接依赖项及 Python 侧依赖未列出；Python 依赖可参考 requirements.txt 文件）。在此感谢所有开源作者的贡献

| 名称          | 项目地址                                                                    | License                                                                               |
| ------------- | --------------------------------------------------------------------------- | ------------------------------------------------------------------------------------- |
| xmake         | [https://github.com/xmake-io/xmake](https://github.com/xmake-io/xmake)         | Apache 2.0                                                                            |
| hdf5          | [https://github.com/HDFGroup/hdf5](https://github.com/HDFGroup/hdf5)           | [hdf5 license](https://github.com/HDFGroup/hdf5?tab=License-1-ov-file#License-1-ov-file) |
| mysql(client) | [https://github.com/mysql/mysql-server]()                                      | [mysql license](https://github.com/mysql/mysql-server?tab=License-1-ov-file#readme)      |
| fmt           | [https://github.com/fmtlib/fmt](https://github.com/fmtlib/fmt)                 | [fmt license](https://github.com/fmtlib/fmt?tab=License-1-ov-file#readme)                |
| spdlog        | [https://github.com/gabime/spdlog](https://github.com/gabime/spdlog)           | MIT                                                                                   |
| sqlite        | [https://www.sqlite.org/](https://www.sqlite.org/)                             | [sqlite license](https://www.sqlite.org/copyright.html)                                  |
| flatbuffers   | [https://github.com/google/flatbuffers](https://github.com/google/flatbuffers) | Apache 2.0                                                                            |
| nng           | [https://github.com/nanomsg/nng](https://github.com/nanomsg/nng)               | MIT                                                                                   |
| nlohmann_json | [https://github.com/nlohmann/json](https://github.com/nlohmann/json)           | MIT                                                                                   |
| boost         | [https://www.boost.org/](https://www.boost.org/)                               | [Boost Software License](https://www.boost.org/users/license.html)                       |
| python        | [https://www.python.org/](https://www.python.org/)                             | [Python license](https://docs.python.org/3/license.html)                                 |
| pybind11      | [https://github.com/pybind/pybind11](https://github.com/pybind/pybind11)       | [pybind11 license](https://github.com/pybind/pybind11?tab=License-1-ov-file#readme)      |
| gzip-hpp      | [https://github.com/mapbox/gzip-hpp](https://github.com/mapbox/gzip-hpp)       | BSD-2-Clause license                                                                  |
| doctest       | [https://github.com/doctest/doctest](https://github.com/doctest/doctest)       | MIT                                                                                   |
| ta-lib        | [https://github.com/TA-Lib/ta-lib.git]()                                       | BSD-3-Clause license                                                                  |
| clickhouse    | [https://github.com/ClickHouse/ClickHouse]()                                   | Apache 2.0                                                                            |
| xxhash        | [https://github.com/Cyan4973/xxHash]()                                         | BSD 2-Clause License                                                                  |
| utf8proc      | [https://github.com/JuliaStrings/utf8proc]()                                   | MIT                                                                                   |
| arrow         | [https://github.com/apache/arrow]()                                            | Apache 2.0                                                                            |
| eigen         | [https://gitlab.com/libeigen/eigen](https://gitlab.com/libeigen/eigen)         | Apache 2.0                                                                            |
| mimalloc      | [https://github.com/microsoft/mimalloc](https://github.com/microsoft/mimalloc) | MIT                                                                                   |
