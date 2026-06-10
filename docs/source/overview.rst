.. figure:: _static/00000-title.png

Hikyuu 简介
============

Hikyuu Quant Framework 是一款基于 C++/Python 开发的开源超高速量化交易研究框架，聚焦策略分析、回测与实盘能力扩展（目前深度适配国内 A 股市场）。

项目深耕量化交易核心技术领域，核心能力覆盖四大维度：**交易模型研发、极速计算引擎、高效回测体系、实盘交易拓展**。

框架以成熟的系统化交易理念为核心，将完整交易体系拆解为**市场环境判断、系统有效条件、信号指示器、止损 / 止盈策略、资金管理策略、盈利目标策略、滑点算法、多因子模型、投资组合、资金分配策略**等独立组件。你可针对每个模块搭建专属策略库，在研究中自由组合、灵活回测，精准分析单一策略的有效性、稳定性，以及组合策略的整体收益表现。

👉 **项目地址：**

* `GitHub <https://github.com/fasiondog/hikyuu>`_
* `Gitee <https://gitee.com/fasiondog/hikyuu>`_
* `GitCode <https://gitcode.com/hikyuu/hikyuu>`_

👉 **项目首页：** `https://hikyuu.org/ <https://hikyuu.org/>`_

👉 **帮助文档：** `https://hikyuu.readthedocs.io/zh-cn/latest/index.html <https://hikyuu.readthedocs.io/zh-cn/latest/index.html>`_

👉 **Wiki文档（AI生成）：** `https://github.com/fasiondog/hikyuu/wiki <https://github.com/fasiondog/hikyuu/wiki>`_

👉 **入门示例:** `https://nbviewer.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True <https://nbviewer.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True>`_

👉 **策略部件库：** `https://gitee.com/fasiondog/hikyuu_hub <https://gitee.com/fasiondog/hikyuu_hub>`_

👉 感谢网友提供的 Hikyuu Ubuntu虚拟机环境, 百度网盘下载(提取码: ht8j): `下载地址 <https://pan.baidu.com/s/1CAiUWDdgV0c0VhPpe4AgVw?pwd=ht8j>`_

示例代码：

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

完整示例参见：`入门示例 Notebook <https://nbviewer.jupyter.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True>`_


🔥 为什么选择 Hikyuu？
-------------------------

**💹 组合灵活，分类构建策略资产库**

Hikyuu 对系统化交易方法进行了轻量化抽象，涵盖市场环境判断、系统有效条件、信号指示器、止损 / 止盈策略、资金管理、盈利目标、滑点算法、交易对象筛选、资金分配等核心组件。你可以基于这些组件自由搭建专属策略库，灵活组合、高效回测，并在策略探索时专注于单一模块的效果与影响，大幅提升研究效率。主要功能架构如下：

.. figure:: _static/10002-function-arc.png
        :width: 800px

**🚀 极致性能，轻松构建专属量化应用**

项目由三大部分构成：高性能 C++ 核心库、Python 接口层（hikyuu）、以及交互式探索工具。

* **性能实测：** AMD 7950x 实测：A 股全市场 1913 万日 K 线，首次加载 + 计算 20 日均线并求和，仅需 6 秒；数据预热后，同操作耗时仅需 166 毫秒。详见 `性能实测文章 <https://mp.weixin.qq.com/s?__biz=MzkwMzY1NzYxMA==&mid=2247483768&idx=1&sn=33e40aa9633857fa7b4c7ded51c95ae7&chksm=c093a09df7e4298b3f543121ba01334c0f8bf76e75c643afd6fc53aea1792ebb92de9a32c2be&mpshare=1&scene=23&srcid=05297ByHT6DEv6XAmyje1oOr&sharer_shareinfo=b38f5f91b4efd8fb60303a4ef4774748&sharer_shareinfo_first=b38f5f91b4efd8fb60303a4ef4774748#rd>`_

* **C++ 核心库：** 内置完整策略框架，原生支持多线程与多核加速，为超高算力场景预留扩展空间；核心库可独立剥离使用，帮助开发者快速构建自定义量化工具。

* **Python接口层（hikyuu）：** 对 C++ 核心进行轻量化封装，集成 TA-Lib，支持与 numpy、pandas 无缝互转，轻松对接主流 Python 数据分析生态。

* **hikyuu.interactive 交互式探索工具：** 内置 K 线、指标、信号可视化能力，适合快速策略验证与回测分析。

**🍳 语法简洁，策略探索更高效自由**

同时支持面向对象与命令行两种编程范式。尤其在策略探索阶段，命令行风格语法极简、表达直观，让你更快验证想法、迭代策略。

**🔐 自主可控，搭建专属云量化平台**

结合 Python + Jupyter 与云服务器，即可搭建完全自主可控的云量化平台。部署后随时随地访问（手机、平板、电脑均可使用），快速落地新想法。同时可无缝对接 numpy、scipy、pandas、TensorFlow 等成熟 AI 与数据分析工具，构建智能量化系统。也可按需自定义界面、实现服务化部署。

**🎁 模块化可扩展数据存储**

目前支持本地 HDF5、MySQL、ClickHouse 和 SQLite 四种存储方式，默认采用 HDF5（文件体积小、读写速度快、备份便捷）。截至 2017 年 4 月 21 日，沪市日线数据文件仅 149MB，深市 184MB，5 分钟线数据整体小于 2GB。通过插件可扩展 ClickHouse 存储，其读写速度优于 HDF5、空间占用远低于 MySQL，更适配分钟级及以下粒度的高频数据存储。


🍺 想要更多了解Hikyuu？请使用以下方式联系：
---------------------------------------------------------------

**因作者精力有限，对捐赠用户的问题会优先解答、确保响应；非捐赠用户的咨询会在时间允许的情况下尽力回复😁**

交流渠道以微信群为主、QQ 群为辅。**加入任一社群请备注 "加入 hikyuu"，未备注将无法通过验证哦～**

.. figure:: _static/qun.png


🎉 感谢捐赠，让 Hikyuu 走得更远
---------------------------------------------------------------

🎁 `捐赠计划与附赠参见 <https://hikyuu.readthedocs.io/zh-cn/latest/vip/donate-plan.html>`_

.. raw:: html

    <table>
    <thead>
    <tr>
    <th>说明</th>
    <th>扫描方式</th>
    <th>捐赠链接（与下方二维码同）</th>
    </tr>
    </thead>
    <tbody>
    <tr>
    <td>请作者喝杯☕️（30元）</td>
    <td>支付宝</td>
    <td><a href="https://pay.ldxp.cn/item/gflv3v" rel="nofollow">https://pay.ldxp.cn/item/gflv3v</a></td>
    </tr>
    <tr>
    <td>订阅180天（50元）</td>
    <td>支付宝</td>
    <td><a href="https://pay.ldxp.cn/item/du4h8s" rel="nofollow">https://pay.ldxp.cn/item/du4h8s</a></td>
    </tr>
    <tr>
    <td>订阅365天（100元）</td>
    <td>支付宝</td>
    <td><a href="https://pay.ldxp.cn/item/ehbz9b" rel="nofollow">https://pay.ldxp.cn/item/ehbz9b</a></td>
    </tr>
    <tr>
    <td>加入星球<br />(3台设备及其他，<br />首年300元，续费半价）<br />另有单独微信群及部件库</td>
    <td>微信或知识星球</td>
    <td><a href="https://t.zsxq.com/YSATD" rel="nofollow">https://t.zsxq.com/YSATD</a></td>
    </tr>
    </tbody>
    </table>

.. figure:: _static/dingyue.png


⭐️ 支持项目，点亮你的星
---------------------------------------------------------------

.. raw:: html

    <img src="https://api.star-history.com/svg?repos=fasiondog/hikyuu&type=Date" alt="Star History Chart" />


私域定制
---------------------------------------------------------------

.. figure:: _static/hikyuu_ads_640px.png


项目依赖说明
---------------------------------------------------------------

Hikyuu 的 C++ 核心模块直接依赖以下开源项目（间接依赖项及 Python 侧依赖未列出；Python 依赖可参考 requirements.txt 文件）。在此感谢所有开源作者的贡献。

.. raw:: html

    <table>
    <thead>
    <tr>
    <th>名称</th>
    <th>项目地址</th>
    <th>License</th>
    </tr>
    </thead>
    <tbody>
    <tr>
    <td>xmake</td>
    <td><a href="https://github.com/xmake-io/xmake">https://github.com/xmake-io/xmake</a></td>
    <td>Apache 2.0</td>
    </tr>
    <tr>
    <td>hdf5</td>
    <td><a href="https://github.com/HDFGroup/hdf5">https://github.com/HDFGroup/hdf5</a></td>
    <td><a href="https://github.com/HDFGroup/hdf5?tab=License-1-ov-file#License-1-ov-file">hdf5 license</a></td>
    </tr>
    <tr>
    <td>mysql(client)</td>
    <td><a href="https://github.com/mysql/mysql-server">https://github.com/mysql/mysql-server</a></td>
    <td><a href="https://github.com/mysql/mysql-server?tab=License-1-ov-file#readme">mysql license</a></td>
    </tr>
    <tr>
    <td>fmt</td>
    <td><a href="https://github.com/fmtlib/fmt">https://github.com/fmtlib/fmt</a></td>
    <td><a href="https://github.com/fmtlib/fmt?tab=License-1-ov-file#readme">fmt license</a></td>
    </tr>
    <tr>
    <td>spdlog</td>
    <td><a href="https://github.com/gabime/spdlog">https://github.com/gabime/spdlog</a></td>
    <td>MIT</td>
    </tr>
    <tr>
    <td>sqlite</td>
    <td><a href="https://www.sqlite.org/">https://www.sqlite.org/</a></td>
    <td><a href="https://www.sqlite.org/copyright.html">sqlite license</a></td>
    </tr>
    <tr>
    <td>flatbuffers</td>
    <td><a href="https://github.com/google/flatbuffers">https://github.com/google/flatbuffers</a></td>
    <td>Apache 2.0</td>
    </tr>
    <tr>
    <td>nng</td>
    <td><a href="https://github.com/nanomsg/nng">https://github.com/nanomsg/nng</a></td>
    <td>MIT</td>
    </tr>
    <tr>
    <td>nlohmann_json</td>
    <td><a href="https://github.com/nlohmann/json">https://github.com/nlohmann/json</a></td>
    <td>MIT</td>
    </tr>
    <tr>
    <td>boost</td>
    <td><a href="https://www.boost.org/">https://www.boost.org/</a></td>
    <td><a href="https://www.boost.org/users/license.html">Boost Software License</a></td>
    </tr>
    <tr>
    <td>python</td>
    <td><a href="https://www.python.org/">https://www.python.org/</a></td>
    <td><a href="https://docs.python.org/3/license.html">Python license</a></td>
    </tr>
    <tr>
    <td>pybind11</td>
    <td><a href="https://github.com/pybind/pybind11">https://github.com/pybind/pybind11</a></td>
    <td><a href="https://github.com/pybind/pybind11?tab=License-1-ov-file#readme">pybind11 license</a></td>
    </tr>
    <tr>
    <td>gzip-hpp</td>
    <td><a href="https://github.com/mapbox/gzip-hpp">https://github.com/mapbox/gzip-hpp</a></td>
    <td>BSD-2-Clause license</td>
    </tr>
    <tr>
    <td>doctest</td>
    <td><a href="https://github.com/doctest/doctest">https://github.com/doctest/doctest</a></td>
    <td>MIT</td>
    </tr>
    <tr>
    <td>ta-lib</td>
    <td><a href="https://github.com/TA-Lib/ta-lib.git">https://github.com/TA-Lib/ta-lib.git</a></td>
    <td>BSD-3-Clause license</td>
    </tr>
    <tr>
    <td>clickhouse</td>
    <td><a href="https://github.com/ClickHouse/ClickHouse">https://github.com/ClickHouse/ClickHouse</a></td>
    <td>Apache 2.0</td>
    </tr>
    <tr>
    <td>xxhash</td>
    <td><a href="https://github.com/Cyan4973/xxHash">https://github.com/Cyan4973/xxHash</a></td>
    <td>BSD 2-Clause License</td>
    </tr>
    <tr>
    <td>utf8proc</td>
    <td><a href="https://github.com/JuliaStrings/utf8proc">https://github.com/JuliaStrings/utf8proc</a></td>
    <td>MIT</td>
    </tr>
    <tr>
    <td>arrow</td>
    <td><a href="https://github.com/apache/arrow">https://github.com/apache/arrow</a></td>
    <td>Apache 2.0</td>
    </tr>
    <tr>
    <td>eigen</td>
    <td><a href="https://gitlab.com/libeigen/eigen">https://gitlab.com/libeigen/eigen</a></td>
    <td>Apache 2.0</td>
    </tr>
    <tr>
    <td>mimalloc</td>
    <td><a href="https://github.com/microsoft/mimalloc">https://github.com/microsoft/mimalloc</a></td>
    <td>MIT</td>
    </tr>
    </tbody>
    </table>    
