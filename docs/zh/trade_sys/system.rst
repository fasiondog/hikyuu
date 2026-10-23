.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

系统策略|SYS
=============

系统是指针对单个交易对象的完整策略，包括环境判断、系统有效条件、资金管理、止损、止盈、盈利目标、移滑价差的完整策略，用于模拟回测。

对于多目标（多证券），可使用 :class:`MultiSystem` 将多个 System 实例（单证券或嵌套的 :class:`MultiSystem`）聚合为一个组合，统一在同一交易账户（TM）下回测。:class:`MultiSystem` 通过 ``add(subsystem)`` 添加子系统，并支持**任意嵌套**与循环引用检测；各子系统保留自身的 SG/MM/EV/CN/ST/TP/PG/SP 策略，仅由父系统统一记账与下单。

:class:`MultiSystem` 支持三种运行模式（``set_mode`` / 只读属性 ``mode``；模式由聚合系统自身持有，亦可经预置 PF 工厂设定）：

- **模式 A「信号汇总」（Signal Aggregation，默认）**：每个子系统持有一个信号「影子账户」（初始信号资金 ``set_sub_init_cash``，逐 bar 恒运行、不逐日重置；``run(reset=True)`` 仍会随整体复位），作为独立信号源；父在调仓日读取**被选中**子系统的**当前意图态**（净敞口持仓 + 未兑现买卖信号），经 AF L2 按「权重 × 净敞口比重 × 父净值」**合成每标的带符号净目标**并对父账户按差额（trade-the-delta）统一下单，多空净额相互抵消；未选中子系统贡献零信号（其标的从目标中消失而被平仓）。影子账本不触碰真实资金。采样时会把**嵌套子树叶子已排队未成交的延迟请求**计入净敞口比（嵌套 C 的待成交意图对采样可见）。
- **模式 B「资金配置」（Fund Allocation，配额划拨，FOF/MOM 风格）**：调仓日先由 AF L1 为每个选中子系统分配额度（目标总资产），并在驱动子系统**之前**将其账户校准到该额度（零配额清算 → 未选中清算/选中回收 → 减持超配 → 注资低配），子系统以精确额度自主交易；父**连续镜像**其真实指令（L2 透传）。配额现金是虚拟预算：``checkin/checkout`` 只记子账户账本、不从父账户划拨，同一调仓日的注资按父剩余现金累计封顶，因此镜像成交有资金支撑、两本账（持仓/成交）保持匹配。composite 子层的减持按叶子市值等比例执行（停牌叶子按最后可得收盘价估值），整手取整残差与停牌不可卖部分按剩余市值补卖（残余不足一手）。前一轮已排队的校准腿计为已卖出：其价值从残差基准扣除；若已覆盖整个缺口，则按缺口目标整手修剪（同标的多腿按队序贪心保留前腿），整手残差由既有补卖机制补足至缺口（残余不足一手）。
- **模式 B 的 SE 约定**：**未设置 SE 时视同全部子系统被选中**（与模式 A/C 一致），照常分配额度；SE 存在但调仓日选中为空则是「全不选」，由 ``sell_at_not_selected`` 决定是否清仓。
- **模式 C「共享账户兼容」（Shared Account Compatibility）**：不建影子账户，子系统直接在父真实账户（``shared_tm``）上按自身 MM 交易、持续运行、按信号当日退出；父仅准入（SE）+ 记录 + 可选风控，不做 L2 换算。用于向后兼容重构前的 ``Portfolio``。

.. note::

   模式 C 不是模式 B 的参数特例：无法通过模式 B 的任意参数档（``adjust_running_sys`` 取任意值、
   AF 不作分配、关闭父等比缩减等）等效得到。二者是**两种账户拓扑**——模式 B 的子系统运行在**隔离的
   影子账户**内、由父连续镜像其成交；模式 C 的子系统**直接共享父真实账户**（有意为之的例外）。
   共享账户下多个子系统**竞争同一现金池**（谁先驱动、先成交，谁先占住现金），这是模式 B 不具备的行为。

模式嵌套与混搭
---------------

子系统可以是另一个 :class:`MultiSystem`（支持**任意层级**）。嵌套时遵循两条规则：**外层模式决定账户拓扑**（给子层什么账户），**内层模式决定其自身子系统的调度与取舍**。

.. list-table:: 外层模式 × 内层模式
   :header-rows: 1
   :widths: 14 30 30 26

   * - 外层 \\ 内层
     - A（信号汇总）
     - B（资金配置）
     - C（共享账户）
   * - A
     - 子层得到**专用影子账户**（初始 ``set_sub_init_cash``）；内层以它为父账户，按自身 L2 把净目标执行到该账户，内层叶子另建影子
     - 子层得到**专用影子账户**；内层以它为父账户做 L1 配额（quota = 权重 × 该账户净值），内层叶子再建配额账户
     - 内层 C 的叶子直接共享该**专用影子账户**（不是外层真实账户）
   * - B
     - 子层得到**专用配额账户**（0 起）；外层先按配额注资，内层再按自身 L2 把净目标执行到该账户
     - 子层得到**专用配额账户**（0 起）；外层注资后，内层再对自身叶子做一次配额分配（嵌套配额）
     - 内层 C 的叶子直接共享该**专用配额账户**
   * - C
     - **无专用账户**：内层以共享账户为父账户，把“权重 × 影子净敞口比 × 共享净值”的净目标直接执行到共享账户；孤儿/补零只限自身子树标的
     - quota = 权重 × 共享净值；注资为虚拟记账（落内层叶子配额账户），减持的父侧卖出落共享账户；多子层竞争同一现金池，持仓按标的不可区分
     - 全层共享同一账户（等价于更深的 legacy Portfolio）

**关键事项**

- **账户归属**：A/B 聚合要求“独占账本”——由外层为它创建专用影子/配额账户；C 则把父账户直接交给子层（共享语义）。因此 C 下挂 A/B 时，子层的“父账户”就是共享账户：A 的净值基数、B 的配额基数都是共享净值，多个子层会竞争同一现金池，请用 SE 权重显式控制各自规模。
- **父账户只记录镜像**：A/B 外层只把内层的**成交**镜像到自己的真实账户（B 另加配额校准）；C 外层的子层在共享账户上直接成交，父层不再二次下单。
- **同批净额**：同一批建议中同标的的反向腿先净额再执行（如开盘延迟成交买入 + 当日收盘反向卖出），不会在父账户留下“幽灵持仓”；CLEAR 只清**可归属部分**（各清除腿的数量之和，且不超过父持仓 + 同批同侧买入），不会吞掉兄弟子层的同批买入或持仓。
- **`trade_on_close=False`**：模式 B 的配额校准腿（减持/清仓）与配额注资、以及信号建议一起**延后到次日开盘**执行（先卖出后注资），统一价格基准；换手率在开盘执行处补记。无报价腿（停牌）**保留重试**：延后校准腿与排队建议都保留到下一个有报价的开盘执行（排队建议重试批与新的调仓批次合并），不会在停牌日按陈旧价格成交。延后注资与同批延后卖出一起等待报价；连续镜像（非调仓日）的成交不计入换手率。若对应持仓已清零（如停牌期间退市强平），该死腿直接丢弃，不再阻塞注资。父侧已无可卖数量的残量直接丢弃，不阻塞后续注资。``trade_on_close=True`` 时若清算/减持的收盘遇停牌，同样回落为次日开盘的延后腿（执行端重试），不会在无报价日成交。
- **两本账匹配**：模式 B 的“影子账本 = 父账本”仅在专用账户下成立；共享账户（C）下持仓按标的不可区分，不存在逐笔归属。
- **清仓语义**：``sell_at_not_selected`` 只作用于本层。B 未选中时：``True`` 父子两本账同步清仓；``False`` 两本账保持运行（仅跳过本轮配额校准；**调仓日收盘不驱动该子系统**，只镜像其开盘已成交的延迟单）。选中但 L1 配额为 0 时同样按上述规则清算（受该开关控制；其中 ``adjust_running_sys=False`` 的运行中子豁免——不清算、继续运行但无预算注入）。C 出池时：``True`` 递归清空子树内部账本并丢弃排队镜像批；``False`` 保持子账本冻结（对应仍留在共享账户上的未跟踪持仓）。
- **cycle 与调仓节拍**：外层预注入只覆盖**直接子系统**的自身 SG（composite 无 SG）；内层 A/B 在自身调仓日运行期声明周期日。内层按**被驱动日**计数：外层 B 未选中（``False``）或外层 C 未入选时内层不推进，因此内层 ``adjust_cycle=N`` 表示“每 N 个被驱动日”。日历型 ``adjust_mode``（week/month/quarter/year）一律按**运行期周期规则**判定（周期第 N 个自然日；``delay_to_trading_day`` 时顺延到该周期首个驱动日；每周期一次），批量 ``run`` 与 live 单 bar 同源；显式 ``set_adjust_dates`` 优先。嵌套 A 子层在自身采样日已把叶子的延迟意图折入目标，外层不会重复折叠（避免 2×）。
- **B→B 超配额**：外层模式 B 把目标配额下发给内层模式 B，由内层按自身 AF 权重减持其叶子（预算制），两层账本与外层父账保持匹配。
- **未入选的嵌套聚合**：不驱动、不声明周期日、不交易（预期行为）。
- **共享边界不沿链传递**：C 的“持仓不可归属”只作用于其**直接子层**；``C → A → B`` 的第三层 B 由中层 A 创建专用影子账本并以 A 层 ``sub_init_cash`` 注资（重新可归属），只有 ``C → C → *`` 才是全链共享。级联下的延后是**每层一跳**（toc=false/停牌回落逐层次日开盘执行），三层链路最外层最多滞后 2 个交易日。
- **同标的多子层**：B 下未选中清仓时，父侧卖出量同时受**父账本持仓**与**子层自身账本持仓**约束；延后减持腿在收盘时按**父账本持仓**封顶。C 共享账户下按标的不区分归属——A 的孤儿/补零目标虽然只限自身子树标的，但**数量按共享账户净持仓执行**，同标的多子层请避免交叉重叠。
- **SE 与告警**：每层有自己的 SE；模式 C 无 SE 时仅**直接** cycle SG 静默并一次性告警（嵌套聚合由内层自身模式处理）。模式 C 下 cycle SG 也**不可与 ``SE_Signal`` 组合**：SE 在 run 前快照已算好的买入信号，而周期日要到入选才声明，因此永远选不中；且模式 C 仅在入选时声明周期日，同一周期内被止损平仓不会重开。
- **级联示例**：``C → B → A`` 表示共享账户下挂配额型子组合 B，B 内部再挂信号汇总子组合 A；A 的影子只服务 B 的采样，B 的镜像直接落共享账户，外层 C 表面化、不二次下单。

各层 SE、``sell_at_not_selected``、调仓节拍与周期日声明互不继承、各自独立。

聚合回测还支持：

- **AF L1/L2/L3 分层**（详见 :doc:`../trade_portfolio/allocate_funds`）：L1 系统级分配（模式 A 名义权重 / 模式 B 真实额度，例：等权 1/N、固定权重、按 SE 得分加权）、L2 行为级换算（A 读子当前意图态、按「权重 × 净敞口比重 × 父净值」合成每标的**带符号净目标**并按差额再平衡，多空净额相互抵消 / B 透传）、L3 组合风控（集中度上限 ``max_single_position``，≤0 不限制；默认 1.0 即单标的不超过总资产；模式 B/C 跳过）。
- **调仓周期**（``set_adjust_cycle``）：默认每个收盘日都再平衡；设置大于 1 的天数后仅在周期日调仓。周期模式（week/month/quarter/year）下该值是"周期内第 n 个日历日"（周自周一起算，故 1~5 才可能落在交易日内）：``0`` 在 ``readyForRun`` 时告警并按 1 处理（负值在设置时抛异常）；永远无法落在该周期任一被驱动日的值（周 > 5 / 月 > 31 / 季度 > 92 / 年 > 366）只告警、**不做钳制**，此时该模式不会触发调仓。目标日按**当前周期的首日**计算，非交易日延后也不会跨入下一个周期，因此靠近周期末尾的值（如 31 日的月份、季末、年末）可能整周期不调仓。
- **参数别名**：``adjust_cycle``（int，默认 1）、``trade_on_close``（bool，默认 True）、``sell_at_not_selected``（bool，默认 False）同时以参数形式暴露，``set_param`` 与对应 setter 等价（完全非法值在设置时抛异常，如 ``adjust_cycle < 0``、非法 ``adjust_mode`` / ``axis_mode``；``adjust_cycle == 0`` 归一为 1（负值在设置时抛异常）、``adjust_mode`` 大小写归一、周期日缓存清理与越界告警均在 ``readyForRun`` 时处理）。
- **交易对象选择（SE）**（``set_se``）：可选设置，调仓日仅采样选中子系统；模式 B/C 中未选中者可强制清仓/回收额度（``set_sell_at_not_selected``），模式 A 中影子账户恒运行，未选中的标的从父目标中消失而被平仓（与 ``sell_at_not_selected`` 无关）。
- **换手率**（``get_adjust_turnover``）：记录各调仓日的成交金额与调仓前总资产之比。
- **退市/停牌**：父在开盘阶段统一强制卖出已退市标的持仓（模式 C 的共享账户也由父层执行，子层不重复卖出）；停牌日子系统自动跳过。收盘清算/减持遇停牌的腿回落为次日开盘的延后腿（任意 `trade_on_close` 模式），持仓保持不变直至有报价。

- **归档与克隆**（`save`/`load`、`clone`）：影子/配额账户注册表按子层槽位随系统一并保存并在载入后重建，调度计数（close-day 计数、最后驱动日）、固定时间轴、外部调仓日表与未成交的校准腿也会恢复，载入后可用 ``run(reset=False)`` 续跑；模式 C 的运行池在下一个调仓日重建。`goalPrice` 等可能为 NaN 的价格字段以字符串写入归档（文本/XML 归档无法回读 NaN）。

MultiSystem 公共参数：

    * **axis_mode="kdata"** *(str)* : 驱动时间轴模式。"kdata" 使用 run(kdata) 输入 KData 的日期序列；"calendar" 使用 set_date_axis 注入的固定日期表（输入 KData 仅提供 query/ktype 与价格上下文）。非法值在设置时抛异常
    * **adjust_cycle=1** *(int)* : 调仓周期。query/day 模式下为收盘日间隔；week/month/quarter/year 模式下为周期内第 n 个日历日。必须 >= 0（负值在设置时抛异常）；0 在 readyForRun 时告警并按 1 处理；永远无法落在该周期任一被驱动日的值只告警、不钳制，此时该模式不会触发调仓
    * **adjust_mode="query"** *(str)* : 调仓模式 query / day / week / month / quarter / year。完全非法值在设置时抛异常，大小写归一在 readyForRun 时完成
    * **delay_to_trading_day=True** *(bool)* : 调仓日为非交易日时，是否顺延到当前周期内的首个交易日（仅 week/month/quarter/year 模式生效）
    * **trade_on_close=True** *(bool)* : 调仓指令是否在收盘阶段执行；False 时整批延后到次日开盘（按开盘价重估、先卖后买、停牌腿保留重试）
    * **sell_at_not_selected=False** *(bool)* : 模式 B/C 下是否对未选中（或 L1 配额为 0）的子系统强制清仓并回收其账户

    运行期开关（adjust_cycle / adjust_mode / delay_to_trading_day / trade_on_close / sell_at_not_selected）在 readyForRun 时快照生效（批量 run 每次运行刷新；直接调用 runMoment* 需先 readyForRun）。:class:`System` 的单证券交易参数（buy_delay / sell_delay / delay_use_current_price / max_delay_count / tp_* / ignore_sell_sg 等）属于各子系统，聚合父层不使用；父层自身的 trace、shared_*、support_borrow_* 仍然生效。模式 C 例外——父层按 trade_on_close 向子系统下发 buy_delay / sell_delay。

System 公共参数：

    * **buy_delay=True** *(bool)* : 买入操作是否延迟到下一个bar开盘时进行交易；市场环境（ev）或系统有效条件（cn）失效触发的空头强制平仓（买回）同样遵守该参数
    * **sell_delay=True** *(bool)* : 卖出操作是否延迟到下一个bar开盘时进行交易；市场环境（ev）或系统有效条件（cn）失效触发的多头强制平仓同样遵守该参数（True 时在下一根开盘平仓，而非失效当日收盘）
    * **delay_use_current_price=True** *(bool)* : 延迟操作的情况下，是使用当前交易时bar的价格计算新的止损价/止赢价/目标价还是使用上次计算的结果
    * **max_delay_count=3** *(int)* : 连续延迟交易请求的限制次数，应大于等于0，0表示只允许延迟1次
    * **tp_monotonic=True** *(bool)* : 止赢单调递增
    * **tp_delay_n=3** *(int)* : 止盈延迟开始的天数，即止盈策略判断从实际交易几天后开始生效
    * **ignore_sell_sg=False** *(bool)* : 忽略卖出信号，只使用止损/止赢等其他方式卖出
    * **can_trade_when_high_eq_low=False** *(bool)* : 当最高价等于最低价时，是否允许交易（一字涨停不可买入、一字跌停不可卖出）

    * **ev_open_position=False** *(bool)*: 是否使用市场环境判定进行初始建仓
    * **cn_open_position=False** *(bool)*: 是否使用系统有效性条件进行初始建仓

    * **shared_tm=False** *(bool)*: tm 部件是否为共享部件
    * **shared_ev=True** *(bool)*: ev 部件是否为共享部件
    * **shared_cn=False** *(bool)*: cv 部件是否为共享部件
    * **shared_mm=False** *(bool)*: mm 部件是否为共享部件
    * **shared_sg=False** *(bool)*: sg 部件是否为共享部件
    * **shared_st=False** *(bool)*: st 部件是否为共享部件
    * **shared_tp=False** *(bool)*: tp 部件是否为共享部件
    * **shared_pg=False** *(bool)*: pg 部件是否为共享部件
    * **shared_sp=False** *(bool)*: sp 部件是否为共享部件

    .. note::
        当前除 ev（市场环境）部件外，其余共享部件（tm、cn、mm、sg、st、tp、pg、sp）内部均无锁，在多个
        系统并发运行时共享会产生数据竞争。因此并发运行的多个系统不应共享这些部件，应为每个系统各自
        克隆（即保持对应的 ``shared_*=False``）。该限制为当前实现，后续可能扩展为支持线程安全的共享。

        ev 部件内部自带读写锁（详见 :doc:`environment` 的"共享与并发"）：``is_valid`` 每根 bar 取一次
        共享锁、设置 ``query`` 与 ``reset`` 取排他锁。共享同一 ev 的各系统必须使用相同的查询条件，否则
        每次不同的 ``setQuery`` 都会在排他锁下完整重算；多线程高频调用场景可为每个系统克隆独立的 ev
        （``shared_ev=False``）。

    .. note::
       资金管理策略的 ``auto_checkin`` 参数在系统成交环节生效：账户现金不足以买入资金管理策略指示的
       数量时，由系统在实际买入成交前按实际成交价与数量一次性补足差额。延迟买入会在信号日与次日开盘
       各计算一次数量，但注资仅发生一次；交易被取消时不产生注资流水。数量查询本身不改变账户。


.. raw:: html

    <table border="1">
        <thead>
            <tr>
                <th>部件命名规范</th>
                <th>部件说明</th>
                <th>部件用途</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td>EV_Xxx</td>
                <td>市场环境判定策略</td>
                <td>用于判断市场环境，只有当市场有效时，才会实际发生交易。该策略通常可在不同的系统策略实例中共享，以减少计算量。</td>
            </tr>
            <tr>
                <td>CN_Xxx</td>
                <td>系统有效条件</td>
                <td>用于判断系统本身适用条件，只有当条件有效时，才会实际发生交易。</td>
            </tr>
            <tr>
                <td>SG_Xxx</td>
                <td>信号指示器</td>
                <td>负责产生买入、卖出信号。</td>
            </tr>
            <tr>
                <td>ST_Xxx</td>
                <td>止损/止盈策略</td>
                <td>止损：仅在交易发生亏损时生效，用于终止交易<br>止盈：仅在交易已盈利时生效，通过系统公共参数 tp_monotonic 控制是否保证递增</td>
            </tr>
            <tr>
                <td>MM_Xxx</td>
                <td>资金管理策略</td>
                <td>用于控制交易风险，决定交易每次的买卖数量</td>
            </tr>
            <tr>
                <td>PG_Xxx</td>
                <td>盈利目标策略</td>
                <td>在交易达到盈利目标时退出交易，本质是一种特殊的止盈策略</td>
            </tr>
            <tr>
                <td>SP_Xxx</td>
                <td>移滑价差算法</td>
                <td>仅用于回测，在回测时模拟实际交易时发生的计划价格和实际价格差异</td>
            </tr>
        </tbody>
    </table>
    <p></p>


创建系统并执行回测
-----------------------

.. py:function:: SYS_Simple([tm=None, mm=None, ev=None, cn=None, sg=None, st=None, tp=None, pg=None, sp=None])

    创建简单系统实例（每次交易不进行多次加仓或减仓，即每次买入后在卖出时全部卖出），  系统实例在运行时(调用run方法），至少需要一个配套的交易管理实例、一个资金管理策略
    和一个信号指示器），可以在创建系统实例后进行指定。如果出现调用run时没有任何输出，
    且没有正确结果的时候，可能是未设置tm、sg、mm。进行回测时，使用 run 方法，如::

        #创建模拟交易账户进行回测，初始资金30万
        my_tm = crtTM(init_cash = 300000)

        #创建信号指示器（以5日EMA为快线，5日EMA自身的10日EMA作为慢线，快线向上穿越慢线时买入，反之卖出）
        my_sg = SG_Flex(EMA(CLOSE(), n=5), slow_n=10)

        #固定每次买入1000股
        my_mm = MM_FixedCount(1000)

        #创建交易系统并运行
        sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
        sys.run(sm['sz000001'], Query(-150))

    :param TradeManager tm: 交易管理实例
    :param MoneyManager mm: 资金管理策略
    :param EnvironmentBase ev: 市场环境判断策略
    :param ConditionBase cn: 系统有效条件
    :param SignalBase sg: 信号指示器
    :param StoplossBase st: 止损策略
    :param StoplossBase tp: 止盈策略
    :param ProfitGoalBase pg: 盈利目标策略
    :param SlippageBase sp: 移滑价差算法
    :return: system实例



系统部件枚举定义
------------------

.. py:class:: System.Part

    系统部件枚举值，系统的买入/卖出等操作可由这些部件触发，用于标识实际交易指令的来源，参见：:py:class:`TradeRecord`。

    实际使用中，可使用 System.ENVIRONMENT 的简化方式 代替 System.Part.ENVIRONMENT，其他与此类似。

    - System.Part.ENVIRONMENT  - 市场环境判断策略
    - System.Part.CONDITION    - 系统有效条件
    - System.Part.SIGNAL       - 信号指示器
    - System.Part.STOPLOSS     - 止损策略
    - System.Part.TAKEPROFIT   - 止盈策略
    - System.Part.MONEYMANAGER - 资金管理策略
    - System.Part.PROFITGOAL   - 盈利目标策略
    - System.Part.SLIPPAGE     - 移滑价差算法
    - System.Part.INVALID      - 无效值边界，大于等于该值时为无效部件


.. py:function:: get_system_part_name(part)

    获取部件的字符串名称

        - System.Part.ENVIRONMENT  - "EV"
        - System.Part.CONDITION    - "CN"
        - System.Part.SIGNAL       - "SG"
        - System.Part.STOPLOSS     - "ST"
        - System.Part.TAKEPROFIT   - "TP"
        - System.Part.MONEYMANAGER - "MM"
        - System.Part.PROFITGOAL   - "PG"
        - System.Part.SLIPPAGE     - "SP"
        - System.Part.INVALID      - "--"

    :param int part: System.Part 枚举值
    :rtype: str


.. py:function:: get_system_part_enum(part_name)

     根据系统部件的字符串名称获取相应的枚举值

    :param str part_name: 系统部件的字符串名称，参见：:py:func:`getSystemPartName`
    :rtype: System.Part



止损、止盈与盈利目标的语义
--------------------------

三者都在持仓期内参与"是否了结"的判定，但状态归属与触发口径不同：

* **止损 ST**（``setST``）—— **静态**止损价。开仓（含加仓）时计算一次并写入持仓记录，持仓期内每根 K 线的判定沿用该值、不随行情重算；卖出时会再计算一次并写回持仓，因此部分卖出后剩余仓位沿用卖出时的止损价。触发口径：多头 ``当前价 <= 止损价``，空头 ``当前价 >= 止损价``。
* **止盈 TP**（``setTP``）—— **移动**退出位。与 ST 同族（同为 ``StoplossBase``），但棘轮由 System 维护：多头只升不降、空头只降不升（``tp_monotonic``），加仓不会把棘轮拉回；且必须持仓已盈利（扣除手续费后净盈利为正）才会了结；``tp_delay_n`` 规定开仓后至少间隔多少根 K 线才允许触发；部分了结后棘轮重置为当前价。触发口径：多头 ``当前价 <= 止盈位``，空头 ``当前价 >= 止盈位``。
* **盈利目标 PG**（``setPG``）—— 锚定**持仓成本**的纯目标价，每根 K 线现算、不持有跨 bar 状态。成本口径为多头 ``buyMoney / totalNumber``、空头 ``sellMoney / totalNumber``，而非入场价快照。触发口径：多头 ``当前价 >= 目标价``，空头 ``当前价 <= 目标价``。

同一根 K 线内按固定优先级判定，且**只执行第一个命中的动作**（命中即返回，其余判定不再评估，下一根 K 线重新从头判定）：

.. code-block:: text

    环境/条件失效强平 → 买入信号 → 卖出信号 → 止损 ST → 盈利目标 PG → 止盈 TP

无效值的处理各不相同：

* ST 未设置或返回无效值：仍照常开仓，但持仓止损价为 0，即不设止损；风险预算按满额计划价计算（视为最坏情况全损）。
* PG 未设置：目标价为 ``constant.null_price``，比较恒不成立，该根 K 线没有 PG 动作。
* PG 返回 ``0``：多头表示立即卖出（强平哨兵）；空头无效果（``当前价 <= 0`` 恒不成立）——空头要无条件平空应返回当前价。
* PG 返回负值或非有限值：由基类钳制为 ``constant.null_price``，视为未设置。
* TP 返回 ``0``：视为该根 K 线没有止盈位，既不平仓也不更新棘轮。

止损价的返回值契约详见 :doc:`stoploss` 的"止损价返回值契约与系统行为"，盈利目标的多空口径详见 :doc:`profitgoal`。



系统基类定义
-------------

.. py:class:: System

    系统基类。需要扩展或实现更复杂的系统交易行为，可从此类继承。

    .. py:attribute:: name

        系统名称

    .. py:attribute:: tm

        关联的交易管理实例

    .. py:attribute:: mm

        资金管理策略

    .. py:attribute:: ev

        市场环境判断策略

    .. py:attribute:: cn

        系统有效条件

    .. py:attribute:: sg

        信号指示器

    .. py:attribute:: st

        止损策略

    .. py:attribute:: tp

        止盈策略

    .. py:attribute:: pg

        盈利目标策略

    .. py:attribute:: sp

        移滑价差算法

    .. py:method:: get_param(self, name)

        获取指定的参数

        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数

    .. py:method:: set_param(self, name, value)

        设置参数

        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型

    .. py:method:: get_stock(self)

        获取关联的证券

        :rtype: Stock

    .. py:method:: get_trade_record_list(self)

        获取实际执行的交易记录，和 TM 的区别是不包含权息调整带来的交易记录

        :rtype: TradeRecordList

    .. py:method:: get_buy_trade_request_list(self)

        获取买入请求列表，“delay”模式下查看下一时刻是否存在买入操作

        :rtype: list[TradeRequest]

    .. py:method:: get_sell_trade_request_list(self)

        获取卖出请求列表，“delay”模式下查看下一时刻是否存在卖出操作

        :rtype: list[TradeRequest]

    .. py:function:: run(self, stock, query[, reset=True])

        运行系统，执行回测

        :param Stock stock: 交易的证券
        :param Query query: K线数据查询条件
        :param bool reset: 执行前是否依据系统部件共享属性复位
        :param bool reset_all: 强制复位所有部件

    .. py:method:: reset(self)

        复位，但不包括已有的交易对象，以及共享的部件

    .. py:method:: force_reset_all(self)

        强制复位所有组件以及清空已有的交易对象，忽略组件的共享属性

    .. py:method:: clone(self)

        克隆操作，会依据部件的共享特性进行克隆，共享部件不进行实际的克隆操作，保持共享



聚合系统类定义
--------------

.. py:class:: MultiSystem([name="MultiSystem"])

    聚合交易系统（组合回测）。将多个 :class:`System`（单证券或嵌套的 :class:`MultiSystem`）聚合为一个组合，统一在同一交易账户下回测；各子系统保留自身的 SG/MM/EV/CN/ST/TP/PG/SP，仅由父系统统一记账与下单。支持任意嵌套与循环引用检测；三种运行模式、嵌套规则与关键事项见上文。

    继承 :class:`System` 的 ``get_param`` / ``set_param``，参数见“MultiSystem 公共参数”。重载 ``MultiSystem(sys_list, name="MultiSystem")``：直接以子系统列表构造。

    .. py:attribute:: mode

        运行模式（只读）："A" 信号汇总 / "B" 资金配置 / "C" 共享账户兼容

    .. py:method:: add(self, sys)

        添加子系统（含循环引用检测；同一实例重复添加被忽略）

    .. py:method:: set_mode(self, mode)

        设置运行模式（"A" / "B" / "C"，不区分大小写）

    .. py:method:: run(self, kdata[, reset=True, reset_all=False])

        组合回测入口：以给定 KData 为驱动轴，逐日驱动所有子系统（开盘/收盘阶段），并在父账户上统一记账与下单。

        重载 ``run(self, query[, reset=True, reset_all=False])``：以交易日历为驱动轴（等价于旧 ``Portfolio.run(query)``），详见上文。

交易请求记录
--------------

.. py:class:: TradeRequest

    交易请求记录。系统内部在实现延迟操作时登记的交易请求信息。暴露该结构的主要目的是用于在“delay”模式（延迟到下一个bar开盘时进行交易）的情况下，系统实际已知下一个Bar将要进行交易，此时可通过 :py:meth:`System.getBuyTradeRequestList` 、 :py:meth:`System.getSellTradeRequestList` 来获知下一个BAR是否需要买入/卖出。主要用于提醒或打印下一个Bar需要进行操作。对于系统本身的运行没有影响。

    .. py:attribute:: valid

        该交易请求记录是否有效（True | False）

    .. py:attribute:: business

        交易业务类型，参见：:py:class:`hikyuu.trade_manage.BUSINESS`

    .. py:attribute:: datetime

        发出交易请求的时刻

    .. py:attribute:: stoploss

        发出交易请求时刻的止损价

    .. py:attribute:: part

        发出交易请求的来源，参见：:py:class:`System.Part`

    .. py:attribute:: count

        因操作失败，连续延迟的次数
