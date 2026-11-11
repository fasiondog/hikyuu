.. highlight:: python

多市场数据
==========

hikyuu 最初面向 A 股单市场，当前版本已支持在同一 ``StockManager`` 内登记多个市场
（期货 / 美股 / 港股 / 外汇 / 数字货币等），并保证跨市场回测的记账与核算正确。

登记与市场信息
--------------

- ``Market`` 表登记市场基本信息，并增加三个时区字段：``tz_offset_min`` （相对 UTC 的
  分钟偏移，可负）、``dst_rule`` （0=无 / 1=美式 / 2=欧式，按日历规则计算夏令时，
  不依赖 tzdata）、``data_lag`` （相对基准市场的信息可见延迟天数）。
- 证券类型（``StockTypeInfo``）按市场习惯登记，``unit`` 为每手乘数（如期货合约乘数）。
- 每个市场须有代表指数（``market`` + 市场默认指数代码）的日 K 线，
  ``get_trading_calendar`` 基于它生成该市场交易日历；节假日按市场维度独立登记。
- 兼容性：``BaseInfoDriver::getAllHolidays()`` 由返回全部日期的列表改为按市场分组的映射，
  自定义驱动子类（含 Python 子类）须同步改写签名。
- Python 侧 ``MarketInfo`` 支持 pickle 往返，时区字段随对象保留。

统一时间表达
------------

- **存储**：各市场按其交易所本地墙钟落盘（naive ``YYYYMMDDHHMM``），A 股存量数据零迁移；
  日线及以上保持"交易日标签"语义，不参与时刻换算。
- **读取**：分钟及以下周期在 ``Stock`` 层双向归一——入口把统一表达的查询日期反归一到
  市场本地，出口把 bar 时间归一到统一表达；``MarketInfo.isUnifiedClock()``
  （时区 +480 且无夏令时）为恒等快速路径；``KQuery.isIntradayKType()``
  判定哪些周期参与换算。
- 日频跨市场按交易日标签对齐：``data_lag``（美股 / 外汇 = 1，A 股 / 期货 / 港股 = 0）在组合层由库
  处理——``MultiSystem`` 在日线及以上轴把成员可见 K 线截断到“≤ 轴日回退 ``data_lag`` 交易日”的
  最后一根，美股 / 外汇的 T 日收盘最早于基准轴 T+1 日可见并执行；成员市场休市的轴日组合照常
  推进，该成员冻结于最后可见收盘（模式 C 共享账户除外，其成员仍按轴日原样驱动）。运行起始
  ``data_lag`` 个轴日内轴历史不足，lag 成员不驱动（其影子账户腿改记首根可见 K 线，不会晚于首次
  驱动的成交时刻）。单证券
  ``System`` 直用时 ``data_lag`` 仍为声明性字段，策略层须自行回看，否则美股 / 外汇的 T 日收盘在
  基准轴 T 日可见，形成单向前视；分钟级不套 ``data_lag``（由时区归一承担）。
- **实盘调度**：``Strategy.run_daily`` / ``run_daily_at`` / ``StockManager.is_trading_hours``
  假定部署机时钟即统一口径（北京时间）——时段窗口自动按目标市场本地钟面换算后比较，
  假日判定按目标市场本地日期；``run_daily_at`` 的时刻钟点本身按统一口径解释。部署机时区
  不是北京时，上述判定将整体错位。

量纲口径
--------

一切以 hikyuu API 口径为准，导入或替换数据源后用三项比值自检：

- ``KRecord.amount``：**万元**
- ``KRecord.volume``：手（日线）/ 股（分钟及以下）
- ``unit``：每手乘数（期货=合约乘数、外汇=合约规模、股票 / 币=1），
  记账 = ``price × number × unit``

.. code-block:: python

    day = stk.get_kdata(Query(Datetime(202610270000), Datetime(202610300000)))
    m1 = stk.get_kdata(Query(Datetime(202610300930), Datetime(202610301500), Query.MIN))
    print('amount ratio', day[-1].amount / sum(r.amount for r in m1))   # ≈ 1
    print('count ratio', sum(r.volume for r in m1) / day[-1].volume)    # ≈ 100

比值不达标的数据不得导入：分钟成交量必须按股存（日线按手），分钟成交额必须与日线同单位。
仓库内 ``test_data`` 的 SH600004 / SH000001 / SZ000001 分钟 fixture 属历史遗留（金额约为应有值的
100 倍、成交量按手存），只用于驱动读取与聚合测试，勿据此做金额或量级断言；SZ399001 是按口径
重建的样本。

外汇与数字货币的记账口径：数量按基础货币量 / 币量理解，盈亏 = 价差 × 数量 × ``unit``
（默认 1，恰好给出报价货币金额）；标准手（10 万基础货币）、点值、最小名义额等交易约束
无内置建模，须通过运行期覆盖（如 ``min_trade_number``）或策略层自行落实。

品种参数与运行期覆盖
--------------------

类型级 ``StockTypeInfo`` 给出 tick / tickValue / precision / 手数的默认值；品种级差异
（期货合约乘数、港股每手、最小变动价位）在 ``stock`` 表中没有单独列，按运行期覆盖：

- ``tick`` / ``tick_value``：``unit = tick_value / tick`` 随覆盖即时生效；在 tick 保持 1 时
  把 ``tick_value`` 设为 10，即得螺纹钢的合约乘数 10 吨/手。
- ``min_trade_number`` / ``max_trade_number``：``TradeManager.buy/sell`` 的手数校验直接读取。
- ``precision``：交易成本与价格的舍入位数。
- ``last_datetime``：到期日，此后 ``buy`` / ``sell_short`` 一律拒开新仓（平仓不受限）。

.. code-block:: python

    rb = sm["SHFRB0000"]
    rb.tick_value = 10.0                         # unit = tick_value / tick = 10
    rb.min_trade_number = 1
    rb.last_datetime = Datetime(202612310000)    # 到期后不再开新仓

覆盖只作用于当前进程、不落库：换进程或在 ``load_hikyuu`` 之后需重新设置（也可把类型级默认值
写进 ``StockTypeInfo``，覆盖同市场同类型的全部品种）。

kdata 配置
----------

``[kdata]`` 段以 ``{market}_{ktype}`` 为键配置各市场的数据文件：键按第一个 ``_`` 切分
（市场代码不得含下划线，大小写不敏感），``time`` 键映射分时 TIMELINE。未配置的市场
或周期返回空数据（静默零输出）。HDF5 价格固定 ×1000 存储（3 位小数上限），
高精度品种（外汇 / 币）应改走 MySQL / ClickHouse / SQLite。

SQLite 后端的 K 线表名只取 code（忽略 market），因此 ``type = sqlite3`` 时必须为每个市场各配
一个库文件（``sh_day`` / ``sz_day`` 分别指向各自的 ``.db``），同码品种（如 SH000001 与
SZ000001）才不会互读对方的数据；库内表名按 code 小写建。

加载范围
--------

- ``load_hikyuu(stock_list=[...])`` 或环境变量 ``HKU_STOCK_LIST``（``;`` 分隔）显式限定
  加载范围：未列出的证券不登记，随之不加载 K 线、不参与预加载；识别不了的代码告警后跳过。
- 证券代码在 BaseInfo 与 K 线存储中统一大写（由导入端保证）；内部匹配与查询同样按大写归一，
  因此清单里的代码大小写不影响匹配，币对可带 ``/``；SQLite / MySQL 的 K 线表名由驱动内部转小写。
- ``SH000001`` / ``SH000300`` 属默认必加载项，无论清单如何都会被登记。
- ``load_hikyuu(market_list=[...])`` 在 ``stock_list=["all"]`` 时按市场限定加载范围，
  ``stock_type_list=[...]`` 同理按证券类型限定，两者同时给出时取交集；列表为空表示不过滤，
  市场清单中含 ``ALL`` 也视为不过滤。范围外的证券不登记、不参与预加载，列式批量 K 线加载
  自动退化为逐只预加载。范围内各市场的代表指数（``MarketInfo`` 记录的指数）总是保留，因此
  ``stock_type_list`` 不含指数类型时交易日历仍可用；其余板块指数仍按类型过滤。
- ``HKU_STOCK_LIST`` 仅被 Python 层 ``load_hikyuu`` 消费（C++ ``hikyuu_init``
  绑定层固定为全量加载，C++ 侧可用 hikyuu 参数 ``load_market_list`` /
  ``load_stock_type_list`` 收窄）。
- 使用 ``stock_list`` 限定时，须把所用市场的代表指数列入清单，否则其交易日历为空。
- 预加载（preload）仍按周期全局生效（无按市场粒度的预加载开关），但只有已登记的证券参与。

组合日历
--------

``MultiSystem.run(query)`` 的驱动日历默认为 **A 股（SH）**：跨市场组合不显式指定时，
美股独有交易日会被跳过、A 股独有交易日空跑（NYSE 与 SH 在 2005-01-04~2011-12-06 区间
相差 115 / 54 天）。跨市场组合应显式注入目标市场日历：

.. code-block:: python

    q = Query(Datetime(20050104), Datetime(20111206), Query.DAY)
    ms.set_axis_mode("calendar")
    ms.set_date_axis(sm.get_trading_calendar(q, "NYSE"))  # 默认参数为 "SH"
    ms.run(q)

能力边界
--------

- **MySQL 升级脚本含 DDL，中断后不可直接重跑**：DDL 隐式提交，``0032`` / ``0033`` 中断后重跑
  会在 ADD COLUMN 处报 Duplicate column，须按脚本头注释手动恢复（或开发库直接删库重建）；
  SQLite / ClickHouse 的升级脚本幂等可重跑。
- **SQLite 后端仅支持 K 线**：无分时 / 逐笔（需 HDF5 / MySQL / ClickHouse）；表名只取
  code（忽略 market），同一库文件内跨市场同码会串号，**必须每市场每周期独立库文件**；
  ``convert`` 聚合的分钟桶边界为硬编码 A 股时段，仅适用于 A 股。
- **夜盘 / 多时段**：``isTransactionTime`` 仅支持两时段且跨午夜判 false，只影响实时
  行情过滤，回测不受影响。
- **期货回测无盯市与强平**：无结算价、无保证金占用快照、无强平流程；期货空头无借券
  记录（``borrow_asset == 0``）。资金口径统一用 ``net_assets() / profit()``，股票做空
  场景下 ``total_assets()`` 会因 ``borrow_asset`` 虚增。
- **多币种记账**：``cash / market_value`` 等字段无币种直接相加，跨币种市场应在数据侧
  预换算为单一本位币。
