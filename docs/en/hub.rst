.. py:currentmodule:: hikyuu
.. highlight:: python

Using the HUB
=============

The HUB is a platform for quantitative trading strategy parts. It is used to:

1. Store strategies: the strategy repository provides a centralized place to store and manage quantitative trading strategies, covering all kinds of investment logic, trading rules, risk-control measures, and so on.
2. Share and learn: anyone can publish strategies to the repository so that other users can study and reuse them.
3. Build and manage a personal strategy library: in addition to the public hikyuu_hub, you can create your own strategy repositories. Registering a local directory as a repository lets you organize and maintain your own strategy library more flexibly — for example, keeping different factor libraries such as alpha_101 and alpha_36 in separate repositories.
4. Version repositories: a local strategy library is just a directory on disk, and you can version it with git if you wish. The public hikyuu_hub repository, for example, is managed with git.
5. Write strategy parts in either Python or C++, so new features can be added without modifying the Hikyuu source code.

For details on how to use the HUB, see the article `Hikyuu Hub: user guide for the quantitative trading strategy parts repository <https://mp.weixin.qq.com/s/4oh9DEEUkv1gk4KoCcGhtQ>`_ (in Chinese).


.. py:function:: add_remote_hub(name, url, branch='main')
    
    Add a remote strategy repository

    :param str name: local repository name (choose your own)
    :param str url: URL of the git repository
    :param str branch: branch of the git repository


.. py:function:: add_local_hub(name, path)

    Add a local strategy repository

    :param str name: local repository name (choose your own)
    :param str path: path of the local repository


.. py:function:: update_hub(name)

    Update a strategy repository

    :param str name: repository name


.. py:function:: remove_hub(name)

    Remove a strategy repository

    :param str name: repository name


.. py:function:: get_hub_name_list()

    Get the list of registered strategy repository names

    :return: list of repository names
    :rtype: list


.. py:function:: get_hub_path(name)

    Get the path of a strategy repository

    :param str name: repository name


.. py:function:: get_current_hub(filename)

    Get the name of the current repository from within the repository's part.py.
    
    Example: get_current_hub(__file__)

    :return: repository name
    :rtype: str


.. py:function:: get_part(name, *args, **kwargs)

    Get a strategy part

    :param str name: strategy part name
    :param args: positional arguments passed to the strategy part
    :param kwargs: keyword arguments passed to the strategy part
    :return: strategy part instance
    :rtype: object


.. py:function:: get_part_name_list(self, hub=None, part_type=None)

    Get the list of available strategy part names

    :param str name: repository name
    :param str part_type: part type
    :return: list of strategy part names
    :rtype: list


.. py:function:: help_part(name)

    Get the help information for a strategy part

    :param str name: part name


.. py:function:: search_part(name: str=None, hub: str = None, part_type: str = None, label=None)

    Search for strategy parts

    Example: search_part('MA')  # part names in the public hikyuu_hub repository are in Chinese, so Chinese keywords such as '趋势' (trend) also work

    :param str name: part name
    :param str hub: repository name
    :param str part_type: part type
    :param str label: label
    :return: list of matching strategy part names
