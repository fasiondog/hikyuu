# Hikyuu 单机自动协商 KData 数据服务方案

## 一、设计原则（回应两点反馈 + 两点补充）

1. **不改配置文件、不影响实时更新**：不替换/修改 `hikyuu.ini` 中任何现有配置项（数据驱动、行情服务器地址、preload 等全部保持原样）。功能通过运行时**自动协商**生效；实时更新链路（`startSpotAgent` → `GlobalSpotAgent` → `Stock::realtimeUpdate`）完全不动，仍只在 Master 进程中运行。
2. **单机自动主从**：第一个启动的 hikyuu 进程自动成为数据服务进程（Master），后启动的进程自动成为客户端（Client），无需任何手工配置或独立部署服务进程。
3. **全量数据覆盖**：除 K 线外，所有可预加载数据——证券基本信息、市场信息、证券类型、节假日、权息、历史财务、中证10年国债、板块——同样经数据服务进程获取，客户端本地不落盘、不重复加载。
4. **纯二进制协议**：IPC 通信直接采用紧凑二进制编码（定长结构 + 长度前缀字符串），不使用 JSON。

## 二、自动协商机制

```
进程启动 → StockManager::init()
  ├─ 尝试对锁文件 {tmpdir}/hikyuu_kdata_server.lock 加独占锁 (flock / LockFileEx)
  │
  ├─ 加锁成功 → 本进程是 Master（加载与服务并行）
  │    ├─ 第一阶段（秒级）：加载元数据（股票列表/市场/类型/节假日/板块等）
  │    ├─ 元数据就绪后立即启动 HikyuuDataServer（nng REP, ipc://{tmpdir}/hikyuu_data.ipc）
  │    ├─ 第二阶段：后台继续预加载全部 K 线，进度实时更新到服务状态中（供客户端展示）
  │    └─ 实时行情照常：startSpotAgent() → realtimeUpdate 更新 Master 内存
  │
  └─ 加锁失败 → 本进程是 Client
       ├─ 若服务已存在但预加载未完成 → **阻塞等待其完成**（见下节"就绪状态协商"）
       ├─ 服务就绪后：关闭 K 线预加载（仅运行时覆盖 m_preloadParam，不写回文件）
       └─ 三类驱动全部换成 IPC 代理驱动（原驱动保留作降级兜底）：
            IpcBaseInfoDriver：股票列表/市场信息/类型/节假日/权息/国债/财务字段与数据/财务信息
            IpcBlockDriver   ：板块加载与查询（替代本地 block 文件读取）
            IpcKDataDriver   ：K 线 getCount/getIndexRangeByDate/getKRecordList/分时/分笔
```

关键点：
- **锁文件决定主从**，天然避免多进程竞态；Master 退出（含崩溃）后锁自动释放，下一个启动的进程自动成为新 Master。
- **tmpdir 取自现有 `hikyuuParam["tmpdir"]`**，不引入新配置；进程间约定同一台机器、同一数据目录。
- Client 侧 `Stock::isPreload()` 返回 false → 现有代码路径 `getCount/getKRecord/getIndexRange` 全部自动改走驱动（见 `Stock.cpp` 中 `isPreload(ktype) && !isBuffer(ktype)` 的判断），**Stock/KRecord/StockManager 数据结构零改动**。
- `loadData()` 流程本身不变：`loadAllStocks/loadAllStockWeights/loadAllHolidays/loadInnerBlocks/...` 全部经由被替换后的 IPC 驱动完成，服务端从自己的内存中应答；客户端启动耗时 ≈ 一次批量传输，无需读盘。
- **客户端缓存策略（运行时可选）**：默认纯远端查询（内存最省）；若回测中逐条 `getKRecord` 导致往返开销过大，可开启按证券惰性缓冲（首次访问经 IPC 批量拉取后沿用现有 `loadKDataToBuffer` 缓冲），代价是盘中实时段需等下一次重载才刷新——实时消费场景建议直接在 Master 进程中进行。

### 就绪状态协商（解决"后启动进程需等待 Master 预加载完成"问题）

策略为**阻塞等待**：客户端必须等第一个进程预加载完成后再继续初始化，保证所有数据都从服务获取，不做未就绪穿透：
1. **服务先行**：Master 完成元数据加载（秒级）后立即启动服务端；K 线预加载在后台继续，期间维护实时进度状态（已加载证券数/总数，沿用现有加载循环中的计数）。
2. **状态探测**：新增 `status.ready` 命令，返回：`LOADING(进度)` / `ALL_READY`。客户端初始化时短轮询该命令（间隔 1～2 秒）：
   - 服务不存在（无锁/连接失败）→ 自己成为 Master 或降级本地驱动（见容错节）；
   - `LOADING` → 原地等待，日志输出进度（如"等待数据服务进程预加载完成：3215/5400"），期间不做任何数据加载，内存开销近乎为零；
   - `ALL_READY` → 继续正常初始化，数据全部来自服务端。
3. **超时保护**：等待总时长可配（新增可选项 `kdata_server_wait_timeout`，缺省如 10 分钟）；超时后记日志并降级为本地驱动模式启动（行为等同现状），避免永久挂死。
4. 效果：只有第一个进程承担耗时的预加载，后续进程启动耗时 ≈ 等待剩余预加载时间 + 一次批量传输；等待期间客户端无磁盘 IO、无内存积累，预加载完成即全量就绪。

## 三、二进制协议与新增组件（均为新增文件，不动现有类定义）

### 1. 二进制协议 `hikyuu_cpp/hikyuu/data_driver/ipc/IpcProtocol.h/.cpp`

消息帧（小端；单机同架构，跨字节序不在考虑范围）：
```
请求:  [magic u32 'HKDS'][version u16][cmd u16][body_len u32][body ...]
响应:  [magic u32][ret i32][body_len u32][body ...]
```

- 通用字段编码：定长数值直接平铺；字符串（market/code/name）用 `u16 len + utf8` 前缀编码；`Datetime` 一律编码为 `uint64`（`Datetime::number()`，`Datetime(ull)` 零开销还原）；`KQuery` 编码为 `(query_type u8, ktype u8, start i64, end i64)`。
- 记录紧凑布局：
  - **KRecord：56 字节** = `datetime u64 + 6×double`，数组直接 `memcpy` 平铺，无逐条封装开销；
  - **StockWeight**：`datetime u64` + 各 `price_t` 字段平铺（以 `StockWeight.h` 字段为准）；
  - **StockInfo / MarketInfo / StockTypeInfo / ZhBond10 / HistoryFinanceInfo**：数值字段定长平铺 + 名称类字符串前缀编码。
- 命令字（cmd）覆盖全部可预加载数据：
  - K 线：`kdata.count`、`kdata.index_range_by_date`、`kdata.get_krecord_list`、`kdata.get_timeline_list`、`kdata.get_trans_list`
  - 基础信息：`base.all_stock_info`、`base.stock_info`、`base.all_market_info`、`base.all_stock_type_info`、`base.all_holidays`、`base.all_zhbond10`、`base.stock_weight_list`、`base.all_stock_weight_list`、`base.history_finance_field`、`base.history_finance`、`base.finance_info`
  - 板块：`block.load`（一次性取回全部板块数据，客户端内存重建，之后不再往返）
  - 状态：`status.ready`（返回 LOADING+进度 或 ALL_READY，客户端轮询等待直至就绪）
- 编解码提供独立函数并带单测，服务端/客户端共用。
- 注：现有 `NodeServer` / `NodeClient`（`hikyuu/utilities/node/`）的消息处理硬编码为 JSON（`decodeMsg/encodeMsg`），无法满足二进制要求；故参考其 nng aio 工作模式**新写二进制收发组件**，不修改这两个现有类。

### 2. 服务端 `hikyuu_cpp/hikyuu/data_driver/ipc/HikyuuDataServer.h/.cpp`
- 自实现 nng REP 二进制派发（多 ctx 并发，参考 NodeServer 的 Work/aio 模式）+ 锁文件句柄，监听 `ipc://{tmpdir}/hikyuu_data_{datadir_hash}.ipc`。
- **启动时机：元数据加载完成后立即启动**；内部维护加载进度/就绪标志（随 `loadData` 阶段推进），`status.ready` 对外暴露；预加载完成前拒绝数据类查询（客户端此时仍在等待，不会发起）。
- handler 直接读 `StockManager::instance()` 内存中的预加载数据（含 `realtimeUpdate` 后的最新值）：Client 每次查询拿到的都是最新数据，拉模式天然一致，无需推送/失效通知。
- 板块数据由 `BlockInfoDriver` 内存结构序列化应答。
- 析构时 `stop()` 并释放锁。

### 3. 客户端代理驱动 `hikyuu_cpp/hikyuu/data_driver/ipc/`
三个代理驱动，均持有二进制 `IpcClient`（nng REQ）+ 本地原驱动作降级兜底：
- **IpcBaseInfoDriver**（继承 `BaseInfoDriver`）：实现全部虚函数（`getAllStockInfo/getAllHolidays/getAllStockWeightList/getHistoryFinance/...`），失败时降级原驱动。
- **IpcKDataDriver**（继承 `KDataDriver`）：`getCount/getIndexRangeByDate/getKRecordList/getTimeLineList/getTransList`；`isIndexFirst()` 返回 true（服务端内存二分更快）；`canParallelLoad()` 返回 false；`isColumnFirst()` 返回 false；`getAllKRecordList` 不实现（Client 不预加载）。
- **IpcBlockDriver**（继承 `BlockInfoDriver`）：`load()` 一次批量取回。

## 四、StockManager 接入点（唯一修改的现有文件）

仅修改 `StockManager.cpp` 的 `init()` 尾部（拿到 `driver` 之后、`loadData()` 之前）：

```
协商（文件锁 + 探测）：
  ├─ Master：先加载元数据 → 启动 HikyuuDataServer（成员持有）→ 后台继续预加载 K 线（就绪标志随阶段推进）
  └─ Client：短轮询 status.ready 阻塞等待至 ALL_READY（日志输出进度，超时后降级本地驱动）；
             就绪后 m_preloadParam 中所有 ktype 置 false；
             m_baseInfoDriver / m_blockDriver / K线驱动池 分别替换为三个 IPC 代理驱动，
             原驱动保留在代理内部作降级兜底；随后 loadData() 流程不变，数据全部来自 Master
```

- 新增私有成员：`HikyuuDataServerPtr m_data_server;`（Master 时非空，进程存活期间保活）。
- 可选开关：`hikyuuParam` 中读取 `"kdata_server"` 布尔值（缺省 = 自动协商；显式设为 false 可整体关闭该功能回退到现状）。这是唯一的新增配置项，默认不影响任何现有行为。

## 五、实时更新为何不受影响

- 实时更新配置（行情服务器地址等）与 K 线数据驱动配置是**两条独立链路**；本方案完全不触碰前者。
- Master 进程：`startSpotAgent()` → `realtimeUpdate()` 更新其预加载缓存 → Client 下次查询即见新值（拉模式，天然一致）。
- Client 进程：不运行 SpotAgent（其定位就是只读数据消费方）；因未预加载、无本地 buffer，不存在"客户端缓存过期"问题。
- 上一版方案的问题（替换配置中的驱动类型导致实时链路配置被破坏）不再存在：本方案不改任何配置项，驱动替换只发生在运行时内存对象上。

## 六、边界与容错

- **Master 中途退出**：Client 查询失败 → 自动降级本地驱动（同一数据目录可读）；日志提示。
- **Client 早于 Master 就绪启动**：分两种情况——锁已被占但服务未 listen：短轮询重试（上限秒级）；服务已启动但预加载未完成：客户端阻塞等待 `ALL_READY`（带进度日志与超时保护），超时后降级本地驱动。
- **跨机器**：明确仅支持单机（ipc:// 传输）；锁文件与 ipc 路径均含数据目录哈希，避免同机不同数据目录的进程误连。
- **逐条访问开销**：纯远端模式下逐条 `getKRecord` 每次一个往返；指标/回测通常按 `getKData` 整段取数（单次往返），影响有限；确有逐条热点时启用第二节所述客户端惰性缓冲。

## 七、测试计划

- C++ 单测（`hikyuu_cpp/unit_test/hikyuu/data_driver/` 新增）：
  - `test_IpcProtocol.cpp`：二进制编解码往返一致性（KRecord/StockWeight/StockInfo 等，含空串、边界日期）。
  - `test_IpcDriver.cpp`：单进程内模拟：手工构造 Master（直接启动 HikyuuDataServer 绕过锁）+ 三个客户端代理驱动，对比 `getKRecordList/getCount/getIndexRangeByDate/getAllStockInfo/getAllHolidays/getStockWeightList/getAllZhBond10/板块加载` 与本地原驱动结果一致。
  - 就绪状态：服务端在预加载未完成时返回 `LOADING`+进度，客户端阻塞等待；完成后返回 `ALL_READY`，客户端正常获取数据；等待超时后降级本地驱动。
  - 降级路径：停掉服务端后查询仍返回正确数据。
- 集成验证（手工/示例）：两个进程先后 `import hikyuu` 初始化，确认第二个进程在 Master 预加载期间阻塞等待并输出进度，预加载完成后继续初始化且日志显示客户端模式，`sm.getStock(...).getKData()` 结果与单进程一致；Master 进程实时刷新后客户端可读到新 K 线。
- 回归：`xmake r small-test` + `python3 hikyuu/test/test.py`（功能开关关闭时行为与现状完全一致）。

## 八、方案对比：本方案（自动协商数据服务） vs 方案 A（mmap 扁平文件） vs 方案 B（boost::interprocess 共享内存）

### 可行性

| 维度 | 本方案（IPC 数据服务） | 方案 A（mmap 扁平文件） | 方案 B（bi::shared_memory） |
|---|---|---|---|
| 对现有代码侵入 | **极低**：仅新增文件 + `StockManager::init` 插入协商，驱动接口抽象现成 | 低：需新增导出/映射层，但读路径需适配"扁平布局→KRecord"转换 | **高**：共享内存中禁用了堆指针/`std::string`，`Stock::Data` 的 `pKData/pMutex` 等需 allocator 改造或整体绕开 |
| 新依赖 | 无（nng 已在用） | 无 | 需启用 boost::interprocess（当前依赖中未启用该模块） |
| 主要风险点 | 逐条查询往返开销、Master 崩溃降级 | 数据文件生成/一致性维护（实时更新后需增量回写或重建） | 进程崩溃后残留段清理、偏移量指针实现复杂度高、调试困难 |
| 实时更新支持 | **天然支持**：Master 照常接收，Client 拉取即得最新 | 差：需增量回写文件 + 版本戳/锁，复杂且易不一致 | 可行但需自行实现跨进程同步（原子序号/读写锁） |
| 自动主从协商 | **原生设计**（文件锁自动选举） | 需额外解决"谁生成/何时重建文件" | 需额外解决"谁创建/清理段" |
| 跨平台 | 好：nng ipc:// 在 Win/mac/Linux 均可用（命名管道/uds） | 好 | 一般：Win 与 POSIX 段管理行为差异需适配 |
| 覆盖数据范围 | **全部可预加载数据**（K线/元数据/板块） | 适合扁平数据（K线）；元数据/板块结构零散，需多套格式 | 同本方案可全覆盖，但实现成本最高 |
| 可回退性 | **好**：协商失败/开关关闭即完全回到现状 | 好（不映射文件即回到现状） | 差：一旦深度改造数据结构，回退代价大 |
| 开发工作量估计 | 中（协议+服务+3代理驱动+协商） | 中低（仅 K 线） | 高 |

### 性能

| 维度 | 本方案（二进制 IPC） | 方案 A（mmap） | 方案 B（共享内存） |
|---|---|---|---|
| 单次读取方式 | 内核拷贝 + nng 消息开销，约几十µs/次 + 数据拷贝 | 直接内存访问（缺页中断，首次读盘） | 直接内存访问，零拷贝零往返 |
| 批量吞吐（如单股全史 ≈8000×56B≈450KB） | 单机 ipc 数百 MB/s，毫秒级，**与读盘同量级或更快** | 首次读盘，后续命中 page cache 接近内存速度 | 内存速度，最快 |
| 逐条随机访问 | 每次一个往返，**最弱环节**（可用客户端惰性缓冲缓解） | 优（直接寻址） | 优（直接寻址） |
| 内存占用 | **最优（纯远端模式）**：全机仅 Master 一份；客户端只存元数据+按需缓冲；但缓存模式下每进程仍会积累 | 物理页由 page cache 共享，接近一份，但需预先生成扁平文件（额外磁盘占用 ≈ 原始数据量） | 仅一份共享段，最优 |
| 启动速度 | 客户端秒级（批量传输，免读盘解析） | 客户端最快（直接映射，无传输）；但首次需导出文件 | 客户端快（直接挂载）；首次需构建共享段 |
| 多客户端并发 | REP 多 ctx 并发，吞吐随查询粒度下降；大批量查询下不是瓶颈 | 无服务端，天然无竞争 | 无服务端，天然无竞争；但写入者更新时需与读者同步 |

### 结论与权衡
- **改造侵入性/风险**：本方案 ≈ A < B；**读延迟**：B ≈ A > 本方案；**内存效率**：B ≈ 本方案(纯远端) > A；**实时更新支持**：本方案 原生 > B > A。
- 本方案与共享内存方案的本质差异：读路径多一次"拷贝+往返"，换来的是对现有数据结构**零改动**、驱动接口自然复用、故障自动降级。对 hikyuu 的典型负载（指标/回测按 `getKData` 整段取数），差距很小；真正敏感的是逐条随机访问，已预留客户端缓冲缓解手段。
- 建议仍以本方案为第一阶段；若实测确认单查询延迟不可接受，可在不改驱动接口的前提下，将客户端热数据升级为 bi 只读共享内存缓存（A/B 作为后续演进路径而非推倒重来）。

## 九、实施步骤

1. `IpcProtocol.h/.cpp`：二进制消息帧 + 全部记录类型编解码（含单测）。
2. 二进制 `IpcServer/IpcClient` 收发组件（nng REP/REQ，参考 NodeServer/NodeClient 的 aio 模式）。
3. `HikyuuDataServer`：数据服务端 + 文件锁 + 全部命令 handler（含单测）。
4. 三个客户端代理驱动（IpcBaseInfoDriver / IpcBlockDriver / IpcKDataDriver）+ 降级逻辑（含单测）。
5. `StockManager::init()` 接入自动协商 + `kdata_server` 可选开关 + `kdata_server_wait_timeout` 等待超时；（可选）客户端惰性缓冲模式。
6. 双进程集成验证、吞吐/延迟粗测；（后续可选）热点场景优化。

## 十、明确不做的事

- 不修改 `KRecord`、`Stock`、`StockManager` 的数据成员定义。
- 不修改 `hikyuu.ini` 任何现有配置项。
- 不做跨机器、不做实时数据推送/订阅（拉模式即可满足）。
- 不在 Client 侧运行行情接收。
