.. figure:: _static/00000-title.png

Hikyuu 简介
============

基于 C++/Python 的开源量化研究框架 —— 金融与交易正确性优先，策略与组合同构、可任意深度嵌套，部件自由替换的高性能量化引擎。深度适配国内 A 股市场；核心库可独立剥离，Python 接口层与 ``hikyuu.interactive`` 探索工具构建在其上。

**Hikyuu 的内核是「交易理论与金融语义的完整与正确」**：一笔交易在金融上该怎样成立——何时成交、成本由哪几项构成、除权时持仓与现金如何变化、绩效用什么口径、空值与非有限值怎么传播——在框架里都是**完整且显式**的：环节不缺、规则被真正执行、可被逐项校验，而不是留给使用者自己保证。

**立意两句：不让一个「能跑通」却不成立的策略被当成结论；不让全市场验证变成等待。**

**快不是少算环节，而是让「逐项校验」在普通机器上也付得起**：一块 Orange Pi 5 Plus（ARM 开发板）上，3547 支证券 × 2015–2026 十年数据、每支跑完一套 ``SYS_Simple`` 回测（``SG_Flex(EMA(C, 5), slow_n=10)`` 的 EMA5 单线拐点 + ``MM_FixedCount(100)``，各自建账），约 13 秒。

两个手段服务于前一句：**按理论问题切分结构**——一个策略必须回答市场状态、条件、信号、仓位、退出与成交价，一个组合必须回答选谁、分多少钱、按什么评分，每一类都是理论上的一个必答问题，而不是工程意义上的模块；**组合即策略**——把组合做成一等公民（``SE`` / ``AF`` / ``MF``），内核才能从单策略延伸到多资产多策略。于是每一环可独立替换与评估，每一笔都记在可查询的账本里；框架的价值不在穷举，而在**每个位置都留对、替换后契约仍然成立**；速度则来自同一个内核直接跑在 C++ 上——**不为跑得快而少算一步**。

👉 **项目地址：**

* `GitHub <https://github.com/fasiondog/hikyuu>`_
* `Gitee <https://gitee.com/fasiondog/hikyuu>`_
* `GitCode <https://gitcode.com/hikyuu/hikyuu>`_

👉 **项目首页：** `https://hikyuu.org/ <https://hikyuu.org/>`_

👉 **帮助文档：** `https://hikyuu.readthedocs.io/zh-cn/latest/index.html <https://hikyuu.readthedocs.io/zh-cn/latest/index.html>`_

👉 **入门示例:** `https://nbviewer.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/zh/000-Index.ipynb?flush_cache=True <https://nbviewer.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/zh/000-Index.ipynb?flush_cache=True>`_

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

完整示例参见：`入门示例 Notebook <https://nbviewer.jupyter.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/zh/000-Index.ipynb?flush_cache=True>`_


入门之前：它是什么，不是什么
--------------------------------

.. note::

   **它不是自动交易工具，也不是订单执行系统**：不内置券商对接、不承诺自动下单，也不做 tick 级撮合或低延迟执行；``OrderBroker`` 只是一个可插拔的接口，是否对接、对接什么通道由使用者自己决定并承担合规责任。它按 K 线推进，把一笔交易的金融语义按真实规则完整执行，决策仍由使用者负责。撮合的微观结构（部分成交、撤单、订单队列）与现代组合优化（均值方差、风险平价）也不例外：框架不替你选定这类算法，但把它们的位置留好了——撮合经 ``OrderBroker`` 与成交模型接入，组合优化写一个 ``AF`` 子类即可，``SE`` / ``MF`` / ``MM`` / ``ST`` 同理。**框架的意义不是把算法写全，而是把位置留对：每个位置可替换，且替换后结算与校验仍然成立。**

**门槛是有意保留的**：框架不会用一条漂亮的曲线，替你补上那些你没想到的前提。这些环节必须自己懂——否则得到的只是一个能跑的错觉。

换个角度看，它更像一张地图：把做策略该考虑的环节逐项列出（环境 → 条件 → 信号 → 仓位 → 退出 → 滑点 → 成本；组合层：选样 → 资金分配 → 风控），照着这张图逐项填，路径本身就是入门路径。**它适合想把系统化交易认真学一遍的人**——包括自学者与学生——而不是想两天跑出一条收益曲线的人。

.. epigraph::

   「不敢下注就赢不了；把筹码输光，就没法再下注。」—— 拉里·海特

给初入者与自学者的几条忠告：

* **先跑通流程，再谈收益**：回测收益率不是能力证明，能解释每一笔为什么成交才是。
* **一次只改一个部件**：其他环节固定不动做对照，否则无法知道收益变化来自哪里。
* **先有成本与权息，再调参数**：没有成本约束下的最优参数，往往只是拟合噪声。
* **关心「什么环境下失效」**：比关心「历史赚了多少」更有用；用样本外与走步验证（``SYS_WalkForward``）检验，而不是在同一个区间反复调参。
* **默认关掉的要慎用**：杠杆、做空默认关闭是刻意的，先弄清保证金、强平与借券成本，再显式打开。


交易理论与金融语义的完整与正确
--------------------------------

三层，各管一件事，缺一层就不成立：**交易理论**——环节不能缺，缺了组合层，多策略就只是并行回测；**交易规则**——规则不能错，缺了权息与分项成本，除权日与跨年收益就会失真，缺了成交时机约束，信号当根成交就是前视；**计算**——空值语义必须一致，若由各指标自行处置，一个 ``inf`` 就会污染下游所有窗口聚合。这三层在框架里都是显式对象与全局约定——不靠文档提醒，而靠框架执行与入口守卫。

.. list-table::
   :header-rows: 1

   * - 层面
     - 回答什么
   * - 交易理论
     - 一个策略该由哪些部分组成，一个组合该由哪些层组织
   * - 交易规则
     - 一笔交易在市场上该如何成交、扣费、除权与计价
   * - 计算
     - 空值与数值在整条计算链上如何保持一致

**交易理论**：按理论结构而非代码职责划分——策略必须回答市场状态（``EV``）、条件（``CN``）、信号（``SG``）、仓位（``MM``）、退出（``ST`` / ``TP`` / ``PG``）与成交价（``SP``）；组合必须回答选谁（``SE``）、分多少钱（``AF`` 的 L1 权重 / L2 换算 / L3 风控）、按什么评分（``MF``）。每一类都是理论上的一个必答问题，缺一环策略就是残缺的。

**交易规则**：成交默认延迟到下一根开盘（``buy_delay`` / ``sell_delay``），一价板或停牌时顺延、超过 ``max_delay_count`` 放弃，滑点价钳制在当日 ``[low, high]``；成本按 ``CostRecord`` 分项（佣金 / 印花税 / 过户费），买卖分别计算；``updateWithWeight`` 按权息调整持仓与现金；融资融券默认关闭；绩效按净资产与自然日年化（``duration / 365``）。

**计算**：未定义状态（除零 / 零方差 / 全缺失 / 零基准）产 ``Null``（NaN），而非 ``0.0`` 或 ``inf``——``0.0`` 会与合法零撞车，``inf`` 会污染下游聚合；聚合只累加有限值、分母只数有效项（均值 1 个、离差 2 个），全缺才产 ``Null``；序列化按 IEEE 位模式保存，NaN / ±Inf / 符号零无损往返。交易输入同样受约束：数量非有限或 ≤ 0、成交价非正即放弃，调仓周期为负直接抛异常。空值与有限值是**全局契约而非各部件自选**，新写部件自动继承同一口径。

这三层都不是文档承诺：核心代码 1300+ 处显式校验拦住规则与输入边界；doctest 用例 1600+ 个、断言 1.6 万条，按「一模块一套件、一边界一用例」组织，空输入、``Null<T>()``、非有限值与越界异常都在覆盖内；``small-test`` 不依赖真实行情即可在三平台 CI 跑通。

.. note::

   要绕过这些规则，只能显式改参数——**框架选择站在「交易上成立」这一边，而不是「跑起来最省事」这一边**。这也是同一策略在不同框架上跑出不同收益的常见原因：没被建模的环节，结果就取决于使用者是否记得补。

   这套取舍有代价：同一策略跑出的收益，通常低于理想化回测——费用扣了、成交等了、权息调整了。宁可让回测数字难看，也不让一个能跑却错误的结果变成可用的策略。


组合即策略：一个引擎覆盖系统化交易 / 资产配置 / 多因子
--------------------------------------------------------

**组合与策略是同一件事**：一个组合节点不是「一组权重」，而是同样持有一套完整的交易语义——子系统各自的 ``EV`` / ``CN`` / ``SG`` / ``MM`` / ``ST`` / ``TP`` / ``PG`` / ``SP`` 决定「怎么打」，账本（``TM``）逐笔记录成交与持仓，按分项计算成本、按权息调整，执行层负责延迟到下一根成交、失败重试与停牌处理——这三层语义在组合节点上依然成立，只在最外层合并为同一账户。

组合层因此只回答投资组合该回答的两个问题：**谁参与**（``SE`` 选样、``MF`` 多因子打分）与**分多少钱**（``AF`` 的 L1 权重 / L2 换算 / L3 组合风控）。

这不是命名上的讲究，而是三个可以直接用的结果：

* **节点 = 完整交易系统，不是资金分配口** —— 单系统、走步优化系统、自定义 Python 系统，乃至组合本身，都能直接挂成父组合的节点，不必为它们写适配层；层级也不设上限（重复添加与循环引用会被拒绝）。
* **金融语义下沉到每个节点** —— 子系统里的止损、分项成本、权息与延迟成交照旧成立，不需要因为进了组合就重写一遍。
* **``AF`` 的三层可分别替换（模式 A）** —— 换权重来源（``AF_EqualWeight`` / ``AF_FixedWeightList`` / ``AF_MultiFactor``）不必动 L3 风控，改 L3 也不必动 L1 权重；模式 B 按额度自治，不做 L3 裁剪。

.. code-block:: python

    from hikyuu.interactive import *

    # 每个子系统都是一套完整的系统化交易：各自的 SG / MM / ST 与账本
    def make_leaf(code, sg):
        sys = SYS_Simple(tm=crtTM(init_cash=100000), sg=sg,
                         mm=MM_FixedPercent(0.5), st=ST_FixedPercent(0.05))
        sys.to = sm[code].get_kdata(Query(-500))    # 绑定自身 K 线
        return sys

    trend = make_leaf("sh600000", SG_Cross(EMA(CLOSE(), n=5), MA(CLOSE(), n=60)))
    grid = make_leaf("sh600004", SG_Cross(EMA(CLOSE(), n=3), MA(CLOSE(), n=10)))

    # 组合工厂只做投资组合该做的事：谁参与（SE）+ 分多少钱（AF）
    group = PF_SignalAggregate(tm=crtTM(Datetime(202001010000), 300000), adjust_cycle=5)
    group.add(trend)
    group.add(grid)

    # 这个组合还能作为节点参与更大的组合（多资产多策略的逐层聚合）
    top = PF_Simple(tm=crtTM(Datetime(202001010000), 1000000),
                    af=AF_FixedWeightList([0.6, 0.4]), adjust_cycle=5)
    top.add(group)
    top.add(make_leaf("sh600005", SG_Cross(EMA(CLOSE(), n=10), MA(CLOSE(), n=30))))

    # 一次运行，整个组合在同一账户内统一结算
    top.run(Query(-500))
    print(top.tm.get_funds().total_assets)

同一套引擎同时支持三种组合运行模式，各有对应工厂（也可用 ``set_mode`` 在已建组合上显式切换）：

.. list-table::
   :header-rows: 1

   * - 工厂
     - 账户形态与资金流转
     - 对应的方法论
     - 典型场景
   * - ``PF_SignalAggregate``（模式 A）
     - 子系统用固定影子账户只作信号源；父按 ``AF`` 的 L2 换算把子系统的意图折算为执行指令，经 L3 组合风控后统一下单
     - 信号 / 观点融合与组合构建；多因子打分可作选样或权重来源
     - 多策略观点叠加到同一标的池、因子加权选股
   * - ``PF_Simple``（模式 B）
     - 调仓日由 ``AF`` 给每个子系统划拨额度，子系统在额度内自主决策，父透传其指令
     - 资产配置与再平衡：权重 / 风险预算 / 调仓周期
     - FOF / MOM、核心-卫星、定期再平衡
   * - ``PF_WithoutAF``（模式 C）
     - 不建影子账户，子系统直接在父的真实账户上按自身 MM 交易
     - 系统化交易的并行执行：每个系统保留完整决策链
     - 复现旧 ``Portfolio`` 回测口径、多策略共享资金池

个人资金有限、策略又多时，模式 A 更实用：子系统只作信号源、不占资金，父账户把观点合成净头寸后统一下单，避免「每条策略分一小份、连一手都买不满」的碎片化；模式 B 则适合资金可分割、需要独立核算的场景。

三种模式并不互斥：同一组合里可以替换 ``SE`` / ``AF`` / ``MF``，也可以把一种模式的组合作为另一种模式的节点（例如外层 B 给内层 A 划拨额度），逐层组合。多因子不是单独的一层开关，而是通过 ``SE_MultiFactor``（打分选样）与 ``AF_MultiFactor``（按 ``SE`` 的评分分配权重）接入 A / B 两条路径。

调仓日由 ``set_adjust_cycle(n)`` 或 ``set_adjust_dates([...])`` 指定；子系统的现金在调仓日被回收并由 ``AF`` 重新划拨，所以某个子系统止损 / 止盈释放的资金会在下一次调仓日回流到组合层（``set_adjust_cycle(1)`` 即每交易日结算）。

.. warning::

   两个前提：每个子系统持有各自独立的 SG / MM（共享会被互相覆盖）；``run(kdata)`` 需各子系统已绑定自身 K 线，否则直接用 ``run(Query(...))`` 由交易日历驱动。架构上支持多市场，内置导入目前覆盖 A 股，其他资产经自定义数据驱动或插件接入。

**团队 / 个人的策略池，可以整体回测，也可以拆开算账**：把每条策略各自建成独立节点——不论标的、不论作者，它们保留各自的信号、仓位与止损；组合层统一决定谁参与、分多少钱，父账户给出整体绩效。模式 A / B 下每个子系统都有自己的账本，因此既能看整体，也能拆开看单条策略的贡献；层级可以按「团队 → 小组 → 成员」组织，和策略嵌套用的是同一套机制。


与常见做法的差异
~~~~~~~~~~~~~~~~~~

两类常见做法各自更适合不同的工作流，差别在于这些环节由谁承担：一类是事件驱动框架里把多个策略并列跑在同一个账户上，组合层并没有被建模——选谁、分多少钱、什么时候再平衡，都要用户自己写一层包装；另一类是把信号 / 因子做成矩阵交给优化器或模型，得到一组权重，而权重背后的下单数量、止损止盈、成交时机与成本由另设的执行层承担，不与策略抽象合一。

.. list-table::
   :header-rows: 1

   * - 维度
     - 事件驱动多策略并列
     - 向量化因子 / 信号研究
     - Hikyuu
   * - 层级模型
     - 多策略并列共享一个账户，无组合层
     - 信号矩阵 → 组合权重 → 独立执行器
     - 组合与策略同构，可任意深度嵌套
   * - 组合层职责
     - 用户手写包装层做资金分配与再平衡
     - 权重在优化器 / 模型层给出
     - ``SE`` 选样 + ``AF``（L1 权重 / L2 换算 / L3 风控）+ ``MF`` 打分内建
   * - 交易语义
     - 由每个策略自行实现
     - 由独立执行层承担，不与策略抽象合一
     - ``EV`` / ``CN`` / ``SG`` / ``MM`` / ``ST`` / ``TP`` / ``PG`` / ``SP`` + 分项成本、权息、延迟成交与重试
   * - 状态同步
     - 共享现金池，子系统间只能间接影响
     - 无逐日状态机
     - 同一根 K 线的开盘 / 收盘阶段，父账户统一记账
   * - 计算内核
     - 多为解释型实现
     - 向量化加速
     - C++ 内核 + 原生多线程，可选本地向量化编译（``local`` 选项）
   * - AI 接管粒度
     - 生成整个策略或参数集
     - 生成因子 / 组合权重
     - 可下沉到单个部件，其余由框架托管

如果你只想在向量化环境里做因子研究，另两条路线的分工未必是问题；Hikyuu 的选择是把组合层与交易语义都纳入同一套抽象，代价是要多理解一层结构。

**差别不在「能不能组合」，而在犯错面积**：商业工具把组合层交给你的脚本——资金分配、现金约束、再平衡与对账写在同一段代码里，一处写错就是整本账错；Hikyuu 把组合层拆成选样 ``SE``、资金分配 ``AF``（L1 权重 / L2 换算 / L3 风控）与打分 ``MF`` 三个对象，并给出默认实现。自定义时你只回答「分多少」，结算、对账、现金上限与费用仍由框架负责。

在「按交易理论切分结构、每个位置可替换、替换后契约仍然成立」这个标准上，公开可见的开源框架里尚未见到第二个实现。


对 AI 的适用性：接管粒度与接入成本
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

设想一个具体任务：**只让 AI 优化止盈，入场与仓位全部沿用现成规则**。多数框架里，这件事的成本不在训练，而在改造：退出逻辑写在策略类内部，要把它拆出来、把账户状态暴露给外部、还要让奖励反映真实成交与成本——一层厚封装。Hikyuu 里，止盈本来就是一个独立对象（``TP``），写一个 ``TP`` 子类即可，其余部件完全不动。

.. list-table::
   :header-rows: 1

   * - AI 接入要解决的三件事
     - 多数框架
     - Hikyuu
   * - 把决策点拆出来
     - 改策略代码，或生成整个策略
     - 部件本身就是对象，换一个即可
   * - 托管账户与持仓状态
     - 自己维护，或暴露内部状态
     - ``TM`` 托管，``get_funds(dt)`` 直接取
   * - 让奖励反映真实摩擦
     - 自己重建成本 / 权息 / 成交口径
     - 净值已扣成本、已调权息、按真实时机成交

于是两件事同时变小：**搜索空间**从「策略 × 参数」缩到「一个部件的输出」；**实验成本**从「造一层适配」降到「写一个部件」。再叠加 bar 驱动的确定性（episode 可重放）与入口守卫（非法配置提前失败），可以先把 AI 限制在一个部件上做窄实验，再逐步扩大接管范围。

.. note::

   现状：``runMoment`` / ``get_funds(dt)`` / ``get_performance(dt)`` 提供 ``step`` / ``state`` / ``reward`` 原语，部件契约（``clone``、序列化、参数枚举）支持搜索与进化；走步优化已内置（``SYS_WalkForward`` 配合 ``SE_MaxFundsOptimal`` / ``SE_PerformanceOptimal`` / ``SE_EvaluateOptimal``）。计划补一层 Gym 风格薄封装与示例级特征、奖励参考；训练器留在外部。


交易系统化架构核心部件
--------------------------------

项目由 **C++ 核心库**、**Python 接口层**与**交互式探索工具**三部分构成，每个部件可独立替换、自由组合：

* **C++ 核心库（``hikyuu_cpp/hikyuu``）**：内置完整策略框架，原生支持多线程与多核加速，为超高算力场景预留扩展空间；核心库可独立剥离使用，帮助开发者快速构建自定义量化工具。
* **Python 接口层（``hikyuu``）**：对 C++ 核心进行轻量化封装，集成 TA-Lib，支持与 numpy、pandas 无缝互转，轻松对接主流 Python 数据分析生态。
* **hikyuu.interactive**：内置 K 线、指标、信号可视化，适合快速策略验证与回测分析；同时支持面向对象与命令行两种编程范式，命令行风格语法极简、表达直观，便于快速验证想法。

数据层支持 **HDF5 / MySQL / ClickHouse / SQLite** 四种存储（默认 HDF5；ClickHouse 经插件扩展，分钟级数据的读写快于 HDF5、空间占用远低于 MySQL，更适配分钟级及以下高频数据）。

.. figure:: _static/10002-function-arc.png
        :width: 800px

这张表就是**领域模型的位置清单**：每一行是一个可替换的卡口（``EV`` / ``CN`` / ``SG`` / ``MM`` / ``ST`` / ``TP`` / ``PG`` / ``SP``；``SE`` / ``AF`` / ``MF``），换掉任何一个，结算、成本与校验仍由框架承担。

.. list-table::
   :header-rows: 1

   * - 层级
     - 部件
     - 说明
   * - 投资组合层
     - ``PF_*``（组合工厂，``Portfolio`` 为兼容别名）
     - 多资产多策略组合：选样、资金分配与统一记账
   * - 投资组合层
     - ``Selector / SE``
     - 系统对象选择：系统策略筛选
   * - 投资组合层
     - ``AllocateFunds / AF``
     - 资金分配：组合级 L1 权重 / L2 换算 / L3 风控
   * - 投资组合层
     - ``MultiFactor / MF``
     - 多因子模型：因子评分与排序
   * - 交易系统 SYS
     - ``Environment / EV``
     - 市场环境：大盘环境有效性判断
   * - 交易系统 SYS
     - ``Condition / CN``
     - 系统有效条件：系统适用条件
   * - 交易系统 SYS
     - ``Signal / SG``
     - 信号指示器：产生买卖信号
   * - 交易系统 SYS
     - ``MoneyManager / MM``
     - 资金管理：买卖数量控制
   * - 交易系统 SYS
     - ``Stoploss / ST``
     - 止损：风险控制退出
   * - 交易系统 SYS
     - ``Takeprofit / TP``
     - 止盈：达到目标价位退出
   * - 交易系统 SYS
     - ``ProfitGoal / PG``
     - 盈利目标：目标达成退出
   * - 交易系统 SYS
     - ``Slippage / SP``
     - 滑点：回测价格模拟
   * - 交易管理
     - ``TradeManager / TM``
     - 交易管理：账户资金与持仓记录
   * - 交易管理
     - ``OrderBroker / OB``
     - 订单执行：实盘下单 broker 对接
   * - 数据层
     - ``StockManager``
     - 证券统一管理
   * - 数据层
     - ``KData``
     - K 线量价序列
   * - 数据层
     - ``Query``
     - 时间范围查询筛选


工程与架构
--------------------------------

对想二次开发或长期维护的人，以下几项是可直接查证的：

* **C++20 + xmake**：Windows / Ubuntu / macOS 三平台 CI；第三方依赖及其许可汇总于 ``THIRD_PARTY_LICENSES.md``
* **三档测试**：``small-test``（CI 默认，不依赖真实行情）/ ``unit-test`` / ``real-test``（需真实行情数据），另有 ``-m coverage`` 生成 lcov 覆盖率报告
* **C++ 核心可独立剥离**：``hikyuu_cpp/hikyuu`` 是独立的构建目标，不套 Python 也可基于它自建工具
* **双语文档**：``docs/zh`` 与 ``docs/en`` 结构一致，核心 C++ / Python 注释与 docstring 已英文化
* **性能实测**：AMD 7950x 上，A 股全市场 1913 万日 K 线首次加载 + 计算 20 日均线并求和 6 秒，数据预热后同操作 166 毫秒，详见 `性能实测文章 <https://mp.weixin.qq.com/s?__biz=MzkwMzY1NzYxMA==&mid=2247483768&idx=1&sn=33e40aa9633857fa7b4c7ded51c95ae7>`_

.. warning::

   **免责声明**：本项目为开源金融技术研究工具，仅供个人学习、学术研究与数据分析使用，不构成任何投资建议与交易指导，不提供、不内置证券交易服务。用户自主新增、对接各类交易接口、开发拓展功能以及对应的实操行为，均由用户自行承担全部风险与法律责任，严禁对接非法交易通道、用于违规交易场景。


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
    <td>请作者喝杯☕️（30元）（赠历史日线数据）</td>
    <td>支付宝</td>
    <td><a href="https://wzyp.cn/item/gflv3v" rel="nofollow">https://wzyp.cn/item/gflv3v</a></td>
    </tr>
    <tr>
    <td>订阅180天（50元）（赠历史日线数据）</td>
    <td>支付宝</td>
    <td><a href="https://wzyp.cn/item/du4h8s" rel="nofollow">https://wzyp.cn/item/du4h8s</a></td>
    </tr>
    <tr>
    <td>订阅365天（100元）（赠历史日/分/时/笔数据）</td>
    <td>支付宝</td>
    <td><a href="https://wzyp.cn/item/ehbz9b" rel="nofollow">https://wzyp.cn/item/ehbz9b</a></td>
    </tr>
    <tr>
    <td>加入星球<br />(3台设备及其他，<br />首年300元，续费半价）<br />另有单独微信群及部件库（赠历史日/分/时/笔数据）</td>
    <td>微信或知识星球</td>
    <td><a href="https://t.zsxq.com/YSATD" rel="nofollow">https://t.zsxq.com/YSATD</a></td>
    </tr>
    </tbody>
    </table>

.. figure:: _static/dingyue.png

捐赠用户支持群（仅接受捐赠用户，入群请注明： Hikyuu 订阅）

.. figure:: _static/support.jpg



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
