使用HUB
==========

HUB 指的是量化交易策略部件的平台，用于：

1. 存储策略：策略仓库为用户提供了一个集中的地方来存储和管理的量化交易策略。这些策略可能包括各种投资逻辑、交易规则、风险控制措施等。
2. 共享与学习：大家可以在策略仓库中分享策略，供他人用户学习和借鉴，共同进步。
3. 个人策略积累和管理：处理公共的 hikyuu_hub 外，可以自行搭建自己的策略仓库。通过加入本地目录做仓库，个人可以更加灵活地管理和控制自己的策略库。比如，通过不同的仓库来管理 alpha_101, alpha_36 等不同的因子库。
4. 仓库版本管理：本地策略库仅仅为本地的目录，可以选择使用 git 进行版本管理。比如，公共的 hikyuu_hub 库就是使用 git 进行管理。
5. 仓库中的策略部件同时支持 python 和 C++ 编写，可以避免直接修改 hikyuu 源码来添加新功能。

有关 HUB 的详细使用, 参见: `Hikyuu | 量化交易策略部件仓库使用说明 <https://mp.weixin.qq.com/s/4oh9DEEUkv1gk4KoCcGhtQ>`_


.. py:function:: add_remote_hub(name, url, branch='main')
    
    增加远程策略仓库

    :param str name: 本地仓库名称（自行起名）
    :param str url: git 仓库地址
    :param str branch: git 仓库分支


.. py:function:: add_local_hub(name, path)

    增加本地策略仓库

    :param str name: 本地仓库名称（自行起名）
    :param str path: 本地仓库路径


.. py:function:: update_hub(name)

    更新策略仓库

    :param str name: 仓库名称


.. py:function:: remove_hub(name)

    删除策略仓库

    :param str name: 仓库名称


.. py:function:: get_hub_name_list()

    获取策略仓库名称列表

    :return: 仓库列表
    :rtype: list


.. py:function:: get_hub_path(name)

    获取策略仓库路径

    :param str name: 仓库名称


.. py:function:: get_current_hub(filename)

    用于在仓库part.py中获取当前所在的仓库名。
    
    示例： get_current_hub(__file__)

    :return: 仓库名称
    :rtype: str


.. py:function:: get_part(name, *args, **kwargs)

    获取策略部件

    :param str name: 策略部件名称
    :param args: 策略部件参数
    :param kwargs: 策略部件参数
    :return: 策略部件实例
    :rtype: object


.. py:function:: get_part_name_list(self, hub=None, part_type=None)

    获取策略部件名称列表

    :param str name: 仓库名称
    :param str part_type: 部件类型
    :return: 策略部件名称列表
    :rtype: list


.. py:function:: help_part(name)

    获取策略部件帮助信息

    :param str name: 部件名称


.. py:function:: search_part(name: str=None, hub: str = None, part_type: str = None, label=None)

    搜索策略部件

    示例: search_part('趋势')

    :param str name: 部件名称
    :param str hub: 仓库名称
    :param str part_type: 部件类型
    :param str label: 标签
    :return: 策略部件名称列表


