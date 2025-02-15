
.. note::

    为了顺利编译代码， 请勿使用从 github 直接下载源码包的方式编译。 原因是 git 上传时部分文件的换行符被置换为Linux式的换行符，将导致直接下载的部分代码在Windows下无法顺利编译。

C++ API参考：`<http://fasiondog.cn/hikyuu/cpp_ref/index.html>`_。

C++测试工程参考：`<http://fasiondog.cn/hikyuu/test_doc/index.html>`_


.. _developer:


编译前准备
----------------

1、安装C++编译器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Windows 平台：Visual C++ 2017 (或以上）
- Linux 平台: g++ > = 5.4.0 、 clang++ >= 3.8.0


2、安装构建工具 xmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^

xmake >= 2.8.2，网址：`<https://github.com/xmake-io/xmake>`_

参见：https://xmake.io/#/zh-cn/guide/installation `<https://xmake.io/#/zh-cn/guide/installation>`


3、克隆 Hikyuu 源码
^^^^^^^^^^^^^^^^^^^^^^^^

执行以下命令克隆 hikyuu 源码：（请勿在中文目录下克隆代码）

.. code-block:: shell

    git clone https://github.com/fasiondog/hikyuu.git


4、Linux下安装依赖软件包
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Linux下需安装依赖的开发软件包。如 Ubuntu 下，执行以下命令：

.. code-block:: shell
    
    sudo apt-get install -y libsqlite3-dev   

MacOsx 下需要使用 brew 安装 sqlite3, hdf5, mysqlclient 的开发包（具体包名请自行查询）。 

5、转 Visual Studio 工程
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

请先试用 python setup.py build 直接编译过一次后，在转换。

Windows 下，习惯用 msvc 调试的，可以使用  xmake project -k vsxmake -m "debug,release" 命令生成 VS 工程。命令执行后，会在当前目录下生成如 vsxmake2022 的子目录，VS工程位于其内。

在 VS 内，可以将 demo 设为启动工程，进行调试。

6、Macosx 下编译注意事项
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. 编译前请安装 xcode 及其命令行工具。
2. Macosx 下使用 Anaconda 中的虚拟环境，请先在虚拟环境下编译后，切换回 base 环境，然后执行 xmake -b core 重新编译 python 包装部分，之后再切换回虚拟环境下，方可正常只用。只要在虚拟环境执行过编译，都需要切回 base 重新 xmake -b core 然后再切回去。
    

编译与安装
------------

须先安装 python click包（pip install click)

直接在克隆的 hikyuu 目录下执行 python setup.py command , 支持的 command：

- python setup.py help        -- 查看帮助
- python setup.py build       -- 执行编译
- python setup.py install     -- 编译并执行安装（安装到 python 的 site-packages 目录下）
- python setup.py uninstall   -- 删除已安装的Hikyuu
- python setup.py test        -- 执行单元测试（可带参数 --compile=1，先执行编译）
- python setup.py clear       -- 清除本地编译结果
- python setup.py wheel       -- 生成wheel安装包


各命令参数，可以 执行 python setup.py commond --help 查看, 如: python setup.py build --help


