
.. note::

    为了顺利编译代码， 请勿使用从 github 直接下载源码包的方式编译。 原因是 git 上传时部分文件的换行符被置换为Linux式的换行符，将导致直接下载的部分代码在Windows下无法顺利编译。

C++ API参考, 使用下面的命令生成 Doxygen 文档:

.. code-block:: shell

    xmake doxygen -F hikyuu_cpp/Doxygen

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


6、Macosx 下安装 xcode 命令行工具
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

编译前请安装 xcode 及其命令行工具。
    

编译与安装
------------

1. 编译
^^^^^^^^^^

须先安装 python click包（pip install click)

进入源码目录下，执行 python setup.py build -j 10 , 其他支持的 command：

- python setup.py help        -- 查看帮助
- python setup.py build       -- 执行编译
- python setup.py install     -- 编译并执行安装（安装到 python 的 site-packages 目录下）
- python setup.py uninstall   -- 删除已安装的Hikyuu
- python setup.py test        -- 执行单元测试（可带参数 --compile=1，先执行编译）
- python setup.py clear       -- 清除本地编译结果
- python setup.py wheel       -- 生成wheel安装包


各命令参数，可以 执行 python setup.py commond --help 查看, 如: python setup.py build --help


2. 安装 Pyhton 包依赖
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

执行 python -m pip install -r requirements.txt 命令安装依赖的 python 包依赖


3. 设置 PYTHONPATH 环境变量
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Linux 下如修改 ~/.bashrc 文件，在末尾添加如下内容 （指向源码目录） ：

.. code-block:: shell

    export PYTHONPATH=/path/to/hikyuu:$PYTHONPATH


4、Windows 下转 Visual Studio 工程
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

请先使用 python setup.py build 直接编译过一次后，在转换。

Windows 下，习惯用 msvc 调试的，可以使用  xmake project -k vsxmake -m "debug,release" 命令生成 VS 工程。命令执行后，会在当前目录下生成如 vsxmake2022 的子目录，VS工程位于其内。

在 VS 内，可以将 demo 设为启动工程，进行调试。


5、Macosx 下编译注意事项
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

mac 下 conda 编译：

1. base 环境下 python setup.py build -j 10

2. 切换到需要的虚拟环境下，不要用 python setup.py build 去编译，直接执行 xmake -b core 编译

3. 切回 base 环境，重新执行 xmake -b core

4. 再切换至虚拟环境，python3 -m pip install -r requirements.txt

5. 导入环境变量，如：export PYTHONPATH="path/to/hikyuu:$PYTHONPATH"

就可以用了（不要再执行 xmake -b core, 只要在虚拟环境下 xmake -b core 了，都要回 base 下重新 xmake -b core)


6、IDE无法正常提示
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. 安装 pybind11-stubgen，使用命令 pip install pybind11-stubgen
2. 运行 pybind11-stubgen hikyuu -o . 命令，即可正常提示帮助信息。