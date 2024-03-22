#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2024-03-11
#    Author: fasiondog

# 来源于 pandas 中对 ipython，notebook 环境的检测代码

def in_interactive_session() -> bool:
    """
    Check if we're running in an interactive shell.

    Returns
    -------
    bool
        True if running under python/ipython interactive shell.
    """
    def check_main():
        try:
            import __main__ as main
        except ModuleNotFoundError:
            return False
        return not hasattr(main, "__file__")

    try:
        # error: Name '__IPYTHON__' is not defined
        return __IPYTHON__ or check_main()  # type: ignore[name-defined]
    except NameError:
        return check_main()


def in_ipython_frontend() -> bool:
    """
    Check if we're inside an IPython zmq frontend. 检测是否在 jupyter 环境中

    Returns
    -------
    bool
    """
    try:
        # error: Name 'get_ipython' is not defined
        ip = get_ipython()  # type: ignore[name-defined]
        return "zmq" in str(type(ip)).lower()
    except NameError:
        pass

    return False
