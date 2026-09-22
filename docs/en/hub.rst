.. py:currentmodule:: hikyuu
.. highlight:: python

Using the HUB
=============

The HUB refers to the platform of the quantitative trading strategy parts, used to:

1. Store the strategies: the strategy repository provides users with a centralized place to store and manage the quantitative trading strategies. These strategies may include various investment logics, trading rules, risk control measures, etc.
2. Share and learn: everyone can share the strategies in the strategy repository for other users to learn and draw on, and make progress together.
3. Accumulate and manage the personal strategies: besides the public hikyuu_hub, you can also build your own strategy repository. By adding a local directory as a repository, individuals can manage and control their own strategy library more flexibly. For example, manage different factor libraries such as alpha_101, alpha_36 through different repositories.
4. Repository version management: the local strategy library is only a local directory; you can choose to use git for the version management. For example, the public hikyuu_hub repository is managed with git.
5. The strategy parts in the repository support both python and C++, which can avoid directly modifying the hikyuu source code to add new features.

For the detailed usage of the HUB, see: `Hikyuu | 量化交易策略部件仓库使用说明 <https://mp.weixin.qq.com/s/4oh9DEEUkv1gk4KoCcGhtQ>`_


.. py:function:: add_remote_hub(name, url, branch='main')
    
    Add a remote strategy repository

    :param str name: the local repository name (name it yourself)
    :param str url: the git repository address
    :param str branch: the git repository branch


.. py:function:: add_local_hub(name, path)

    Add a local strategy repository

    :param str name: the local repository name (name it yourself)
    :param str path: the local repository path


.. py:function:: update_hub(name)

    Update the strategy repository

    :param str name: the repository name


.. py:function:: remove_hub(name)

    Remove the strategy repository

    :param str name: the repository name


.. py:function:: get_hub_name_list()

    Get the list of the strategy repository names

    :return: the repository list
    :rtype: list


.. py:function:: get_hub_path(name)

    Get the strategy repository path

    :param str name: the repository name


.. py:function:: get_current_hub(filename)

    Used to get the name of the current repository in the part.py of the repository.
    
    Example: get_current_hub(__file__)

    :return: the repository name
    :rtype: str


.. py:function:: get_part(name, *args, **kwargs)

    Get the strategy part

    :param str name: the strategy part name
    :param args: the strategy part parameters
    :param kwargs: the strategy part parameters
    :return: the strategy part instance
    :rtype: object


.. py:function:: get_part_name_list(self, hub=None, part_type=None)

    Get the list of the strategy part names

    :param str name: the repository name
    :param str part_type: the part type
    :return: the list of the strategy part names
    :rtype: list


.. py:function:: help_part(name)

    Get the help information of the strategy part

    :param str name: the part name


.. py:function:: search_part(name: str=None, hub: str = None, part_type: str = None, label=None)

    Search the strategy parts

    Example: search_part('趋势')

    :param str name: the part name
    :param str hub: the repository name
    :param str part_type: the part type
    :param str label: the label
    :return: the list of the strategy part names
