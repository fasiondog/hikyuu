.. _shm_server:

shm Data Server (single-machine shared memory)
==============================================

When multiple hikyuu processes run on the same machine at the same time (such as Jupyter research, strategy backtesting, data collection), and they use the **same data directory** (the ``datadir`` of the ``[hikyuu]`` section in ``hikyuu.ini``), one of the processes can act as the **server**, publishing the loaded data as the shared memory snapshots, and the other processes, as **clients**, read them with zero copy, avoiding each process repeatedly preloading and repeatedly occupying the memory.

Effect: only the server bears the time-consuming preloading and all the memory overhead; the startup time and the memory usage of the clients drop greatly, and the data seen by each process remains consistent.

.. important::

    Different from the old version's "automatically negotiating the master and the slave at the process startup", the server is **not created automatically**: you must **explicitly call** :func:`start_shm_server` in a process to start the service. Any hikyuu process (including the caller itself) will **never** automatically become the server because it cannot connect to the service — when the connection fails, it only degrades to the standalone mode, loading all the data by itself (the behavior is exactly the same as when this feature is not enabled).

    This feature (**both the server and the client sides**) is entirely provided by the standalone VIP plugin ``shmserver`` and requires a valid VIP license:

    - Server: when the plugin is not installed or the license is invalid, :func:`start_shm_server` returns ``False`` and prints a warning;
    - Client: the implementation of the shm (the protocol, the shared memory reading/writing, the proxy drivers) is also inside the plugin; when it is not installed or not licensed, the process **silently** degrades to the standalone mode — loading all the data by itself, with the functions fully working, only losing the shared memory acceleration,
      and the startup logs will not produce any error noise.

.. note::

    The negotiation is at the granularity of ``datadir`` (the service address is derived from the hash of ``datadir``); the processes using different ``datadir`` are independent of each other and do not interfere with each other.

    This feature is "sharing one copy of the data among multiple processes within a single machine", which is an independent concept from the cross-machine **market data collection service** (``vip/dataserver``, ``get_data_from_buffer_server``) and the market data collection address (``quotation_server``); do not confuse them.

Starting and Stopping the Server
--------------------------------

The server is controlled to start and stop within its own process with the Python API (the paradigm is the same as ``start_data_server`` / ``stop_data_server``):

.. py:function:: start_shm_server(datadir: str = '', publish_shm: bool = True, recv_spot: bool = True) -> bool

    Start the shm data server within the current process, for the other hikyuu processes to read with zero copy as the clients. It must be called after the hikyuu
    initialization (``import hikyuu`` completes the initialization by default); calling it before the initialization will return ``False`` because the data is not ready.

    :param str datadir: the data directory; when empty, the current StockManager data directory is used
    :param bool publish_shm: whether to publish the two kinds of the shared memory snapshots (the K-line hot data + the basic information)
    :param bool recv_spot: whether this process receives the real-time market data (internally subscribing to ``quotation_server`` and driving the real-time updates)
    :return: return ``True`` when started successfully; return ``False`` when this process is already in the client mode, the plugin is missing, or the license is invalid

.. py:function:: stop_shm_server() -> None

    Stop the shm data server within the current process, releasing the shared memory segments and unregistering the related hooks.

.. py:function:: is_shm_server_running() -> bool

    Query whether the shm data server within the current process is running.

Typical usage: start the service in a resident server process, and the other research / backtest processes, after explicitly enabling ``use_shm_server``, join as the clients:

::

    # Process A —— the server (resident)
    import hikyuu as hku
    if not hku.start_shm_server():
        ...  # the plugin is missing / the license is invalid; this process will run in the standalone mode

    # Process B / C / … —— the clients
    import hikyuu as hku
    hku.load_hikyuu(use_shm_server=True)  # explicitly join the service of process A; this process enters the client mode

.. note::

    When being the server and needing to keep the snapshot quasi-real-time, usually keep ``recv_spot=True``, so that the server subscribes to
    ``quotation_server`` and drives the real-time updates, mirroring the latest market data into the tail of the shared memory segment; at this time the clients do not need
    to receive the market data themselves, further saving the resources.

.. note::

    The server process only acts as the snapshot publisher and does not need to join the other shm services. ``use_shm_server`` is disabled by default; the server
    process completes the initialization and the publishing in the standalone mode without any extra handling; only when ``use_shm_server=True`` is explicitly set in the configuration file,
    the server process needs to skip the client probing with ``load_hikyuu(use_shm_server=False)`` — otherwise this process
    will first act as a client to connect to the existing service, and after retrying for about 10 seconds, print
    the ``Failed connect to hikyuu shm server, fallback to standalone mode!`` warning and then degrade to the standalone mode
    (the data can still be loaded and published normally, with only an extra startup wait and a harmless warning). The command line tool ``shmserver`` has explicitly
    disabled the client probing.

Resident Start from the Command Line
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can also use the command line tool ``shmserver`` installed with the package to start a resident server process separately, which is equivalent to calling
:func:`start_shm_server` in Python and blocking to wait, stopping gracefully with ``Ctrl-C``:

::

    # It can be executed directly after the pip installation; when it is not yet installed as a command, the equivalent is: python -m hikyuu.gui.shmserver
    shmserver [options]

    Options:
    --datadir TEXT         the data directory; when empty, the [hikyuu] datadir in hikyuu.ini is used
    --publish_shm BOOLEAN  whether to publish the shared memory snapshots (the K-line hot data + the basic information), defaulting to True
    --recv_spot BOOLEAN    whether this process receives the real-time market data and mirrors it into the tail of the snapshot, defaulting to True
    --config TEXT          specify the hikyuu configuration file path; when empty, the default ~/.hikyuu/hikyuu.ini is used

After the server process is started, if the other research / backtest processes want to join as the clients (instead of independently loading all the data), they need to explicitly enable ``use_shm_server=True`` in the configuration
or the ``load_hikyuu`` parameters, and use the same ``datadir`` as the server.

How It Works
------------

1. **Explicitly start the server**: after the server process calls :func:`start_shm_server`, it loads the data and listens on the service address, for the clients to connect. The core library does not perform any master-slave election and does not compete for the file locks.
2. **Client negotiation**: during the initialization of an ordinary process, if ``use_shm_server`` is true, the client interface of the shmserver plugin is used to try to connect to the existing service and poll its ready state (about once per second, covering the window when the server is "starting up"); the waiting duration
   is constrained by the total budget of ``shm_server_wait_timeout``; on timeout, it **degrades to the standalone mode**, loading all the data by itself
   — it **never** starts the service in this process. After the connection succeeds, the three kinds of the proxy drivers created by the plugin (K-line / basic information / block)
   are assembled into the StockManager.
3. **Disable the client preloading**: the client disables the preloading of all its own K-line types (only taking effect in the memory, without modifying the configuration file), and the server provides them instead.
4. **Shared memory snapshot**: after the server completes the preloading, it publishes the loaded data as the read-only shared memory segments, and the clients read them directly with zero copy (at the microsecond level); when the server receives the real-time market data updates, it synchronously writes them into the tail of the snapshot, so what the clients read is the quasi-real-time data.
5. **Waiting for readiness**: after connecting, the client polls the loading progress of the server at an interval of about 1 second. The server starts to provide the service as soon as the basic data (the security list, the markets, the security types, the holidays, the blocks, etc.) is loaded, **without waiting for the K-line preloading to complete**; if the client
   waits for readiness within ``shm_server_wait_timeout`` seconds (the total negotiation budget, including the probing and the readiness waiting; ``0`` means unlimited), it enters the client mode; on timeout, it degrades to the standalone mode.
6. **Fault-tolerant degradation**: the securities or the K-line types not covered by the snapshot automatically switch to IPC; when the IPC request fails or the response cannot be parsed,
   the client falls back to its own local data driver (``Fallback to local driver`` appears in the logs), so the clients can still continue to work after the server exits. This local driver connection is established when the client starts and is held throughout (as a fallback, not lazily created on demand):
   for sqlite the overhead is negligible; for MySQL / ClickHouse a real connection pool will be established. However, the major part of the K-line preloading has been
   exempted, so the effect that the aforementioned startup time and memory usage drop greatly still holds.

Configuration Items
-------------------

They are all in the ``[hikyuu]`` section of ``hikyuu.ini``, and take effect on the **client** (the server is controlled by the parameters of :func:`start_shm_server`).
``use_shm_server`` defaults to ``False``; it only needs to be explicitly enabled when joining an existing service:

::

    [hikyuu]
    tmpdir = /home/user/stock/tmp
    datadir = /home/user/stock
    ; whether this process acts as a client to connect to the existing shm service; defaults to False (always running in the standalone mode), set it to True when joining is needed
    use_shm_server = True
    ; the total duration budget of the client negotiation (seconds, including the connection probing and the readiness waiting); 0 means waiting infinitely, degrading to the standalone mode after the timeout
    shm_server_wait_timeout = 600

.. list-table::
   :header-rows: 1
   :widths: 30 10 60

   * - Configuration item
     - Default value
     - Description
   * - ``use_shm_server``
     - ``False``
     - Whether this process acts as a client to connect to the existing service. When set to ``True``, this process tries to join the existing
       service with the same ``datadir``; when set to ``False`` (default), it neither probes, nor maps, nor forwards, always running in the standalone mode, with the behavior the same as
       when this feature is not enabled.
   * - ``shm_server_wait_timeout``
     - ``600``
     - The total duration budget of the client negotiation (seconds, including the connection probing and the readiness waiting); ``0`` means waiting infinitely; after the timeout, the client
       degrades to the standalone mode to start, avoiding hanging forever.

Data Access Paths
-----------------

Every K-line query of the client chooses the path according to the following priorities:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Path
     - Applicable scenario
   * - Shared memory snapshot
     - The K-line types that the server has preloaded and that are not truncated by ``xxx_max``; read with zero copy, the fastest
   * - Local data driver
     - The K-line types that the server has not preloaded, and the time-line (``TIMELINE``) and the tick (``TRANS``) data
   * - IPC request
     - The securities not covered by the snapshot (e.g. truncated by ``xxx_max``), and the fallback when the shared memory is unavailable
   * - Local data driver
     - The final degradation when the IPC request fails or the response is abnormal

.. note::

    The ``[preload]`` configuration of the server determines the content of the snapshot: only when a K-line type is ``True`` will it be preloaded by the server and
    enter the shared memory snapshot; ``xxx_max`` limits the number of the preloaded records; the truncated securities do not enter the snapshot, and the client switches to the IPC query for them (the results of the two paths are consistent).

    The ``[preload]`` configuration of the client process itself is automatically ignored (all set to ``False``).

    The recovery processing is always done locally on the client; what the server transmits is the original data without recovery.

Block Reading and Writing in the Client Mode
--------------------------------------------

The block data of the client processes is also provided with a consistent view by the server: when entering the client mode, all the blocks are pulled once from the server through IPC (``BLOCK_LOAD``) and cached in this process (once per session, see below).

- **Reading** (``get_block`` / ``get_block_list``, etc.): hit the local cache of this process above; when the cache is unavailable or misses, fall back
  to the local block driver (the client and the server share the same ``datadir``, and the fallback results are consistent).
- **Saving and removing** (``save_block`` / ``remove_block``): in the client mode, the local block library is **not written directly**; instead, they are forwarded through
  IPC (``BLOCK_SAVE`` / ``BLOCK_REMOVE``), and the **server process persists them at a single point**. Reason: when multiple clients share
  the same server, if each of them writes the local shared library directly, multiple writers will be formed, overwriting each other and bypassing the server cache; so the write operations are always converged to the
  server.

The block write operations take the server's response as the criterion: when the connection is unavailable or the server reports an error when persisting, the initiating process will raise an exception, and it will **not** quietly switch to the
local driver fallback write (the write does not apply the "three-layer fallback" semantics of the read path, to avoid the user mistakenly thinking the modification has taken effect).

.. warning::

    The propagation of the block modifications has a **visibility delay** (caused by the block cache being pulled once within the client session and not re-pulled); the visible moments of each party are as follows:

    - **The process initiating the modification**: after the response succeeds, it synchronizes its own cache in place immediately, **taking effect immediately**;
    - **The server / the library**: the modification is persisted immediately; the modification is still retained after the server process restarts or the daily ``reload`` reloads the data;
    - **The other already running clients**: their block cache is pulled when the session is established and is not re-pulled within the session, **so this modification is not visible within the current session**; they need to restart / reconnect the client (a new session re-pulls), or those with the daily reload enabled wait until the next ``reload`` to be visible;
    - **The clients started afterwards**: what they pull when connecting is the latest blocks, **directly visible**.

    In addition, the block writes depend on **the block data source configured by the server being writable**: if the server uses a read-only block data source (such as the implementation of the Qianlong block directory driver), the save / remove will fail and raise an exception in the initiating process, and the blocks remain unchanged; the client will not bypass the server to write
    the local library.

    If the server is unavailable and the client degrades to run in the standalone mode, the block reading and writing are exactly the same as when this feature is not enabled — directly reading and writing the local
    block library, without the above propagation boundary.

FAQ
---

**Will the data be inconsistent among the multiple processes?**

No. The client and the server use the same ``datadir``; what the snapshot and the IPC return are both the data loaded by the server, and the mirrored writing of the real-time market data guarantees the quasi-real-time consistency. (Exception: the **write** operations of the blocks, see "Block Reading and Writing in the Client Mode" above — the blocks saved / removed by one client will not be immediately seen by the other online clients within the current session, and they are visible only after restarting / reconnecting or the next ``reload``.)

**If a block is saved directly in the server process, can the clients see it?**

Not immediately. If ``save_block`` / ``remove_block`` is called directly in the **server process itself** (not forwarded through IPC,
directly operating the server's local block library), the block cache used by the server to respond to the clients' ``BLOCK_LOAD`` will not be updated synchronously;
you need to execute ``reload()`` on the server process or restart the server, and then the newly connected / reconnected clients can see it. Therefore, the block write operations
should be uniformly performed by the clients forwarded through the server, and it should be ensured that only one process is writing the blocks at the same time.

**What happens if the server is not started?**

Each process cannot connect to the service, and will load all the data in the standalone mode by itself; the functions and the results are not affected, only the startup time and
the memory optimizations cannot be enjoyed.

**The server runs in Jupyter, and the clients become slow?**

If the Python main thread of the server holds the GIL for a long time (e.g. a piece of intensive computation that does not release the GIL), the thread of the server processing the requests will be blocked when writing the logs, and the client shows the request latency being amplified. It is recommended to run the server in a separate Python process.

**How to turn this feature off completely?**

``use_shm_server`` defaults to ``False``: do not start the server, and do not enable this option in any process's configuration or ``load_hikyuu``
parameters; all the processes will run completely in the standalone mode, with the behavior the same as when this feature is not enabled.

**Which files will be left in the temporary directory?**

The service address is in the system temporary directory (on unix, take the environment variable ``TMPDIR``, defaulting to ``/tmp``; on Windows, take the system temporary directory);
the socket / named pipe file name is like ``hikyuu_shm_server_{hash}.ipc`` (on Windows, the named pipe with the same name),
and there are also the accompanying ``.lock`` files; the server also uses ``hikyuu_ks.last`` / ``hikyuu_bi.last`` to record the current shared memory
segment names, used to clean up the residual segments of the last abnormally exited server. When the server exits normally, it deletes the shared memory segments; the lock files and the record files
are empty files themselves, and the residuals can be safely deleted manually.

.. warning::

    The shared memory segment names and their record files (``hikyuu_ks.last`` / ``hikyuu_bi.last``) are globally fixed and are not isolated by
    ``datadir``. Therefore, after modifying the ``datadir`` in the configuration, if the server started with the old ``datadir`` is
    **still running**, the old and the new servers will share the same segment record file: when the later starter publishes the snapshot, it will clean up the shared memory segment currently used by the previous
    server according to the record, causing the clients newly connected to it afterwards to fail to hit the snapshot and degrade to the IPC requests (the data is still
    correct, the existing clients are not affected, and only the new clients' latency increases). In actual use, ``datadir`` is generally fixed, so the impact is
    limited; if you need to switch ``datadir``, it is recommended to first close all the hikyuu processes running with the old ``datadir``, and then start the processes with the new configuration.
