#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#==========================================================
# History
# 1. 20200822, Added by fasiondog
#==========================================================

import threading


class SingletonType(type):
    """基于 metalclass 实现单例

    示例：
    class MyClass(metaclass=SingletonType):
        def __init__(self,name):
            self.name = name
    """

    _instance_lock = threading.Lock()

    def __call__(cls, *args, **kwargs):
        if not hasattr(cls, "_instance"):
            with SingletonType._instance_lock:
                if not hasattr(cls, "_instance"):
                    cls._instance = super(SingletonType, cls).__call__(*args, **kwargs)
        return cls._instance
