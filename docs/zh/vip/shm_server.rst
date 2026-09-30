.. _shm_server:

shm 数据服务（单机共享内存）
============================

使用前需先完成配置：客户端进程需在 ``hikyuu.ini`` 的 ``[hikyuu]`` 节开启 ``use_shm_server``，
具体配置项见下文 `配置项`_。

同一台机器上运行 hikyuu 时，可执行 ``shmserver`` （或安装目录 ``gui`` 下的 ``shmserver.py``）
单独拉起一个常驻的 **共享内存数据服务（shm server）** 进程，由其完成耗时的数据预加载，并将
**K 线热数据与基础信息** 发布为两类共享内存快照；其余 Jupyter 研究、策略回测、数据采集等进程
作为 **客户端**，经 IPC 接入后零拷贝读取快照，避免每个进程重复预加载、重复占用内存。
共享内存 K 线缓存 / 零拷贝 K 线视图在单进程场景下同样生效。

效果：只有服务端承担耗时的 K 线预加载与主要内存开销，客户端的启动时间与内存占用大幅下降，
且各进程看到的数据保持一致。

.. important::

    服务端 **不会自动产生**：必须在某个进程内 **显式调用**
    :func:`start_shm_server` 才会启动服务。任何 hikyuu 进程（包括调用者自身）都 **永远不会**
    因为连接不到服务而自动变成服务端——连接不上时只会降级为独立模式，自行加载全部数据（行为与
    未启用本特性时完全一致）。

    本特性（**服务端与客户端两侧**）全部由独立的 VIP 插件 ``shmserver`` 提供，需要有效的 VIP
    授权：

    - 服务端：未安装插件或授权无效时 :func:`start_shm_server` 返回 ``False`` 并打印告警；
    - 客户端：shm 的实现（协议、共享内存读写、代理驱动）同样在插件内，未安装或未授权时进程
      **静默** 降级为独立模式——自行加载全部数据、功能完全正常，只是失去共享内存加速，
      且启动日志不会产生报错噪音。

.. note::

    服务地址为 IPC 运行时目录下的固定名，与 ``datadir`` 解耦；客户端与服务端须使用同一
    ``datadir`` 以保证数据一致。IPC 运行时目录可通过 ``shm_ipc_dir`` 配置或环境变量
    ``HIKYUU_IPC_DIR`` 指定，默认 ``~/.hikyuu/ipc``\（见 `配置项`_）。

    本特性是"单机内多进程共享一份数据"，与跨机器的 **行情采集数据服务** （``vip/dataserver``、
    ``get_data_from_buffer_server``）以及行情采集地址（``quotation_server``）是相互独立的概念，
    请勿混淆。

启动与停止服务端
----------------

服务端在其自身进程内以 Python API 控制启停（范式同 ``start_data_server`` / ``stop_data_server``）：

.. py:function:: start_shm_server(datadir: str = '', publish_shm: bool = True, recv_spot: bool = True) -> bool

    在当前进程内启动 shm 数据服务，供其他 hikyuu 进程作为客户端零拷贝读取。须在 hikyuu
    初始化之后调用（``import hikyuu`` 默认完成初始化）；早于初始化调用会因数据未就绪而返回
    ``False``。

    :param str datadir: 数据目录，为空时使用当前 StockManager 数据目录（兼容保留；服务地址已与
        datadir 解耦，改由 IPC 运行时目录下的固定名构造）
    :param bool publish_shm: 是否发布两类共享内存快照（K 线热数据 + 基础信息）
    :param bool recv_spot: 是否由本进程接收实时行情（内部订阅 ``quotation_server`` 并驱动实时更新）
    :return: 启动成功返回 ``True``；本进程已处于客户端模式、插件缺失或授权无效时返回 ``False``

.. py:function:: stop_shm_server() -> None

    停止当前进程内的 shm 数据服务，释放共享内存段并注销相关挂钩。

.. py:function:: is_shm_server_running() -> bool

    查询当前进程内的 shm 数据服务是否在运行。

典型用法：在常驻的服务端进程里启动服务，其余研究 / 回测进程显式开启 ``use_shm_server`` 后以
客户端身份接入：

::

    # 进程 A —— 服务端（常驻）
    import hikyuu as hku
    if not hku.start_shm_server():
        ...  # 插件缺失 / 授权无效，本进程将以独立模式运行

    # 进程 B / C / … —— 客户端
    import hikyuu as hku
    hku.load_hikyuu(use_shm_server=True)  # 显式接入进程 A 的服务，本进程进入客户端模式

.. note::

    作为服务端且需要维持快照准实时时，通常保持 ``recv_spot=True``，由服务端订阅
    ``quotation_server`` 并驱动实时更新，把最新行情镜像写入共享内存段尾部；此时客户端无需
    各自再接收行情，进一步节省资源。

.. note::

    服务端进程只作快照发布者，并不需要接入其他 shm 服务。``use_shm_server`` 默认关闭，服务端
    进程按独立模式完成初始化与发布，无需额外处理；仅当配置文件中显式置 ``use_shm_server=True``
    时，才需在服务端进程内以 ``load_hikyuu(use_shm_server=False)`` 跳过客户端探测——否则该进程
    会先当作客户端去连接既有服务，在等待 ``shm_server_wait_timeout`` （默认 600 秒）超时后打印
    ``Failed connect to hikyuu shm server, fallback to standalone mode!`` 告警再降级为独立模式
    （数据仍能正常加载与发布，仅多出启动等待与一条无害告警）。命令行工具 ``shmserver`` 已显式
    关闭客户端探测。

命令行常驻启动
~~~~~~~~~~~~~~

也可用随包安装的命令行工具 ``shmserver`` 单独拉起一个常驻服务端进程，等价于在 Python 中调用
:func:`start_shm_server` 后阻塞等待，按 ``Ctrl-C`` 优雅停止：

::

    # pip 安装后可直接执行；尚未安装为命令时等价：python -m hikyuu.gui.shmserver
    shmserver [选项]

    Options:
    --datadir TEXT         数据目录，为空时使用 hikyuu.ini 中 [hikyuu] datadir
    --publish_shm BOOLEAN  是否发布共享内存快照（K 线热数据 + 基础信息），默认 True
    --recv_spot BOOLEAN    本进程是否接收实时行情并镜像写入快照尾部，默认 True
    --config TEXT          指定 hikyuu 配置文件路径，为空则使用默认 ~/.hikyuu/hikyuu.ini

服务端进程启动后，其余研究 / 回测进程若要以客户端身份接入（而非独立加载全部数据），需在配置
或 ``load_hikyuu`` 参数中显式开启 ``use_shm_server=True``，并与服务端使用相同 ``datadir``。

工作原理
--------

1. **显式启动服务端**：服务端进程调用 :func:`start_shm_server` 后，加载数据并在服务地址上
   监听，供客户端连接。核心库不做任何主从选举、不竞争文件锁。
2. **客户端协商**：普通进程初始化时，若 ``use_shm_server`` 为真，则经 shmserver 插件的客户端
   接口尝试连接既有服务并轮询其就绪状态（约 1 秒/次，覆盖服务端"正在启动"的窗口）；等待时长
   受 ``shm_server_wait_timeout`` 总预算约束，超时则 **降级为独立模式**，自行加载全部数据
   ——**绝不** 在本进程拉起服务。连接成功后，插件创建的三类代理驱动（K 线 / 基础信息 / 板块）
   被装配进 StockManager。
3. **关闭客户端预加载**：客户端会把自身全部 K 线类型的预加载关闭（仅内存中生效，不修改配置
   文件），改由服务端提供。
4. **共享内存快照**：服务端完成预加载后，把已加载数据发布为只读共享内存段，客户端零拷贝直接
   读取（微秒级）；服务端收到实时行情更新时会同步写入快照尾部，因此客户端读到的是准实时数据。
5. **等待就绪**：客户端连接后按约 1 秒间隔轮询服务端的加载进度。服务端在基础数据（证券列表、
   市场、证券类型、节假日、板块等）加载完毕即对外提供服务，**不等待 K 线预加载完成**；客户端
   在 ``shm_server_wait_timeout`` 秒（协商总预算，含探测与就绪等待，``0`` 为无限）内等到就绪
   即进入客户端模式，超时则降级为独立模式。
6. **容错降级**：快照未覆盖的证券或 K 线类型自动改走 IPC；IPC 请求失败或响应无法解析时，
   客户端回退到自己的本地数据驱动（日志中出现 ``Fallback to local driver``），因此服务端退出后
   客户端仍可继续工作。该本地驱动连接在客户端启动时即建立并全程持有（兜底用，非按需懒创建）：
   对 sqlite 开销可忽略，对 MySQL / ClickHouse 则会建立真实连接池。不过占大头的 K 线预加载已
   免除，故前述启动时间与内存占用大幅下降的效果仍然成立。

配置项
------

均位于 ``hikyuu.ini`` 的 ``[hikyuu]`` 节。``use_shm_server`` / ``shm_server_wait_timeout`` 作用于
**客户端** （服务端由 :func:`start_shm_server` 的参数控制）；``shm_ipc_dir`` 对服务端与客户端均生效。
``use_shm_server`` 默认为 ``False``，仅当需要接入既有服务时才需显式开启：

::

    [hikyuu]
    tmpdir = /home/user/stock/tmp
    datadir = /home/user/stock
    ; 本进程是否作为客户端连接既有 shm 服务；默认为 False（始终以独立模式运行），需要接入时置 True
    use_shm_server = True
    ; 客户端接入协商的总时长预算（秒，含连接探测与就绪等待）；0 表示无限等待，超时后降级为独立模式
    shm_server_wait_timeout = 600
    ; shm 数据服务的 IPC 运行时目录（socket / 锁 / 段名记录落点），默认配置生成时写入用户家目录
    ; docker 共享场景改为宿主与容器挂载的相同绝对路径
    shm_ipc_dir = /home/user/.hikyuu/ipc

.. list-table::
   :header-rows: 1
   :widths: 30 10 60

   * - 配置项
     - 默认值
     - 说明
   * - ``use_shm_server``
     - ``False``
     - 本进程是否作为客户端连接既有服务。置 ``True`` 时本进程尝试接入同一 ``datadir`` 的既有
       服务；置 ``False`` （默认）时既不探测、也不映射、也不转发，始终以独立模式运行，行为与
       未启用本特性时一致。
   * - ``shm_server_wait_timeout``
     - ``600``
     - 客户端接入协商的总时长预算（秒，含连接探测与就绪等待）；``0`` 表示无限等待；超时后客户端
       降级为独立模式启动，避免永久挂起。
   * - ``shm_ipc_dir``
     - ``~/.hikyuu/ipc``
     - shm 数据服务的 IPC 运行时目录（socket、单实例锁、段名记录文件的落点），服务端与客户端均
       生效；亦可经环境变量 ``HIKYUU_IPC_DIR`` 覆盖（优先级高于本配置项）。默认位于用户家目录下，
       无需配置；docker 等容器共享场景建议配置为绝对路径（容器内 ``HOME`` 可能与宿主不同）。

docker 容器共享
---------------

跨容器共享服务时，客户端容器需能访问两类资源：**IPC 运行目录中的 socket 文件** 与 **/dev/shm 中的
共享内存段**。配置要点：

- 服务端与客户端使用 **同一 datadir**\（连同数据目录一并挂载进容器），且插件版本一致；
- 在 ``hikyuu.ini`` 中将 ``shm_ipc_dir`` 配置为 **挂载范围内的绝对路径**\（不依赖容器内 ``HOME``）；
- 客户端容器以 ``--ipc=host`` 运行（或 ``--ipc=container:<服务端容器名>``），与服务端共享 /dev/shm；
- 若服务端运行于容器内且未使用 ``--ipc=host``，须调大 ``--shm-size``\（默认仅 64MB，K 线段可达
  GB 级）；
- Windows 宿主下的容器均为 Linux 容器（Docker Desktop / WSL2），而 Windows 原生进程使用命名管道
  与 Windows 共享内存，无法与 Linux 容器互通，故服务端也应放入容器（或 WSL）内运行。

**Linux 宿主示例（服务端运行于宿主，客户端运行于容器）**

服务端在宿主上直接运行（``shmserver`` 常驻亦可），``hikyuu.ini`` 中配置：

::

    shm_ipc_dir = /data/hikyuu-ipc

客户端容器将配置、数据与 IPC 运行目录按 **相同路径** 挂载：

::

    docker run -it --rm \
      --ipc=host \
      -v /data/hikyuu-ipc:/data/hikyuu-ipc \
      -v /home/user/.hikyuu:/home/user/.hikyuu \
      -v /home/user/stock:/home/user/stock \
      my-hikyuu-image

容器内使用的 ``hikyuu.ini`` 即挂载进来的同一份（``shm_ipc_dir`` 指向挂载路径），开启
``use_shm_server = True`` 即可接入宿主服务端。

服务端也可运行于容器内（与客户端同为 ``--ipc=host``，共享内存段落在宿主 /dev/shm）：

::

    docker run -d --name shmserver \
      --ipc=host \
      -v /home/user/.hikyuu:/home/user/.hikyuu \
      -v /home/user/stock:/home/user/stock \
      my-hikyuu-image shmserver

**Windows 宿主示例（Docker Desktop / WSL2，服务端与客户端均为 Linux 容器）**

宿主目录 ``D:/hikyuu`` 下放置 ``.hikyuu``\（配置）与 ``stock``\（数据），``hikyuu.ini`` 中配置
``shm_ipc_dir = /home/user/.hikyuu/ipc``\（容器内路径，两个容器挂载点一致）：

.. code-block:: powershell

    # 服务端容器
    docker run -d --name shmserver --ipc=host `
      -v D:/hikyuu/.hikyuu:/home/user/.hikyuu `
      -v D:/hikyuu/stock:/home/user/stock `
      my-hikyuu-image shmserver

    # 客户端容器
    docker run -it --rm --ipc=host `
      -v D:/hikyuu/.hikyuu:/home/user/.hikyuu `
      -v D:/hikyuu/stock:/home/user/stock `
      my-hikyuu-image

数据获取路径
------------

客户端的每一次 K 线查询按以下优先级选择路径：

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - 路径
     - 适用场景
   * - 共享内存快照
     - 服务端已预加载、且该证券未被 ``xxx_max`` 截断的 K 线类型，零拷贝读取，最快
   * - 本地数据驱动
     - 服务端未预加载的 K 线类型，以及分时（``TIMELINE``）、分笔（``TRANS``）数据
   * - IPC 请求
     - 快照未覆盖的证券（如被 ``xxx_max`` 截断），以及共享内存不可用时的兜底
   * - 本地数据驱动
     - IPC 请求失败或响应异常时的最终降级

.. note::

    服务端的 ``[preload]`` 配置决定了快照的内容：某个 K 线类型为 ``True`` 才会被服务端预加载、
    才会进入共享内存快照；``xxx_max`` 限制预加载条数，被截断的证券不进快照，由客户端改走 IPC
    查询（两条路径的结果一致）。

    客户端进程自身的 ``[preload]`` 配置会被自动忽略（全部置为 ``False``）。

    复权处理始终在客户端本地完成，服务端传输的都是不复权的原始数据。

    客户端模式下的 **权息与历史财务** 改为按需懒加载并在本地缓存：首次访问某证券的权息 /
    财务数据时才加载（服务端基础信息快照在初始发布时不含财务数据，待财务预加载完成后再刷新
    快照），而非随预加载一次性载入，后续访问命中本地缓存，避免客户端为每个证券重复拉取。

客户端模式下的板块读写
----------------------

客户端进程的板块数据同样由服务端提供一致视图：进入客户端模式时经 IPC（``BLOCK_LOAD``）从服务端
一次性拉取全量板块并缓存在本进程（会话期一次性，见下）。

- **读取** （``get_block`` / ``get_block_list`` 等）：命中上述本进程缓存；缓存不可用或未命中时回退
  到本地板块驱动（客户端与服务端共用同一 ``datadir``，兜底结果一致）。
- **保存与删除** （``save_block`` / ``remove_block``）：客户端模式下 **不直接写本地板块库**，而是经
  IPC（``BLOCK_SAVE`` / ``BLOCK_REMOVE``）转发，由 **服务端进程单点落库**。原因：多个客户端共享
  同一服务端时，若各自直写本地共享库会形成多写者、互相覆盖并绕过服务端缓存，故写操作一律收敛到
  服务端。

板块写操作以服务端应答为准：连接不可用或服务端落库报错时，发起进程会抛出异常，且 **不会** 悄悄改走
本地驱动兜底写（写不适用读路径的"三层回退"语义，避免用户误以为修改已生效）。

.. warning::

    板块修改的传播存在 **可见性延迟** （板块缓存在客户端会话内一次性拉取、不重拉所致），各方的
    可见时点如下：

    - **发起修改的进程**：应答成功后立即就地同步本进程缓存，**立即生效**；
    - **服务端 / 库**：修改即时持久化；服务端进程重启或每日 ``reload`` 重载数据后修改依然保留；
    - **其它已在运行的客户端**：其板块缓存于会话建立时拉取、会话内不重拉，**本次修改在当前会话内
      不可见**；需重启 / 重连客户端（新会话重新拉取），或启用了每日重载者等到下一次 ``reload`` 后
      才可见；
    - **之后新启动的客户端**：连接时拉取到的即是最新板块，**直接可见**。

    另外，板块写依赖 **服务端配置的板块数据源可写**：若服务端使用的是只读板块数据源（如钱龙板块目录
    驱动的实现），保存 / 删除会失败并在发起进程抛出异常、板块保持不变，客户端不会绕过服务端去写
    本地库。

    若服务端不可用、客户端降级为独立模式运行，则板块读写与未启用本特性时完全一致——直接读写本地
    板块库，不存在上述传播边界。

常见问题
--------

**多个进程之间的数据会不一致吗？**

不会。客户端与服务端使用同一 ``datadir``，快照与 IPC 返回的都是服务端已加载的数据，实时行情的
镜像写入保证了准实时一致。（例外：板块的 **写** 操作，见上文「客户端模式下的板块读写」——某客户端
保存 / 删除的板块，其它已在线客户端在当前会话内不会立即看到，需重启 / 重连或下一次 ``reload``
后才可见。）

**在服务端进程里直接保存板块，客户端能看到吗？**

不能立即看到。若在 **服务端进程自身** 内直接调用 ``save_block`` / ``remove_block`` （不经 IPC
转发，直接操作服务端本地板块库），服务端用于应答客户端 ``BLOCK_LOAD`` 的板块缓存不会同步更新；
需对服务端进程执行 ``reload()`` 或重启服务端，之后新连接 / 重连的客户端才可见。因此板块写操作
应统一经由客户端经服务端转发进行，并保证同一时刻只有一个进程在写板块。

**没有启动服务端会怎样？**

各进程连不到服务，会各自以独立模式加载全部数据，功能与结果不受影响，只是无法享受启动时间与
内存的优化。

**服务端运行在 Jupyter 中，客户端变慢？**

服务端的 Python 主线程若长时间持有 GIL（例如一段未释放 GIL 的密集计算），服务端处理请求的线程
在写日志时会被阻塞，客户端表现为请求延迟被放大。建议把服务端放在独立的 Python 进程中运行。

**如何完全关闭该功能？**

``use_shm_server`` 默认为 ``False``：不启动服务端、也不在任何进程的配置或 ``load_hikyuu``
参数中开启该选项，所有进程即完全按独立模式运行，行为与未启用本特性时一致。

**IPC 运行目录下会残留哪些文件？**

IPC 运行目录默认为 ``~/.hikyuu/ipc``\（可经 ``[hikyuu] shm_ipc_dir`` 配置或环境变量
``HIKYUU_IPC_DIR`` 覆盖）。socket / 命名管道文件名形如 ``hikyuu_shm_server.ipc``
（Windows 下为同名命名管道），另有配套的 ``.lock`` 文件；服务端还会用 ``hikyuu_ks.last`` /
``hikyuu_bi.last`` 记录当前共享内存段名，用于清理上一个异常退出的服务端残留段，并以
``hikyuu_shmserver.lock`` 作为单实例文件锁（见下）。服务端正常退出时会删除共享内存段；上述
文件残留后均可安全手工删除（单实例锁在进程终止时由操作系统自动释放）。

.. note::

    服务端启动时会对 ``hikyuu_shmserver.lock`` 加非阻塞排他文件锁，**本机同一时刻只允许一个
    shmserver 实例运行**：已有实例存活时，新启动的实例将记录 FATAL 日志后直接退出；进程终止
    （含 Ctrl-C、崩溃）时由操作系统自动释放锁，不影响下一次正常启动。共享内存段名与其记录文件
    （``hikyuu_ks.last`` / ``hikyuu_bi.last``）为全局固定、未按 ``datadir`` 隔离，在单实例约束下
    后启动者清理的必为上一个已退出服务端的残留段，不存在两个运行中服务端互删段的问题；如需切换
    ``datadir``，请先关闭当前服务端进程，否则新实例会因单实例锁拒绝启动并退出。
