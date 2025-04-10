安装步骤
=========

安装前准备
----------

支持的操作系统：64位 Windows7及以上版本、Ubuntu、MacOSX （其中 Windows、Ubuntu 24.04及以上可以直接 pip 安装，其他建议使用源码编译安装）

1. Python环境：>= Python3.9 (macosx python >= 3.10)。 推荐使用安装时 python 的主流版本或低一个版本，比如截止 2025年4月1日, python主流版本为 3.12, 建议使用3.12或3.11, 避免因 python 版本低可能导致的其他依赖包互不兼容导致的错误。

.. note:: 

    - 推荐安装集成了常用数据科学软件包的Python发布版本： `Anaconda <https://www.anaconda.com/>`_ 。国内用户建议从 `清华镜像网站 <https://mirrors.tuna.tsinghua.edu.cn/help/anaconda/>`_ 下载，速度快。


2. 安装 git （如使用 hub，必须安装）

    git工具官方链接: `https://git-scm.com/downloads <https://git-scm.com/downloads>`_


pip 安装
----------

安装：python -m pip install hikyuu

版本升级：python -m pip install hikyuu -U

.. figure:: _static/20000-install.png

.. note::

    第一次使用，请详细阅读 :ref:`quickstart` 。
   

源码编译安装
----------------

Ubuntu24.04及以上支持 pip 安装，其他Linux及macsox上建议使用源码方式安装，参见 :ref:`developer` 。
