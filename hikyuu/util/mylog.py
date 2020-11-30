#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import logging
import sys
import time
from functools import wraps


def spend_time(func):
    @wraps(func)
    def wrappedFunc(*args, **kargs):
        starttime = time.time()
        try:
            print("\nCalling: %s" % func.__name__)
            return func(*args, **kargs)
        finally:
            endtime = time.time()
            print("spend time: %.4fs, %.2fm" % (endtime - starttime, (endtime - starttime) / 60))

    return wrappedFunc


FORMAT = '%(asctime)-15s [%(levelname)s] %(message)s [%(name)s::%(funcName)s]'
logging.basicConfig(format=FORMAT)

hku_logger_name = 'hikyuu'
hku_logger = logging.getLogger(hku_logger_name)


def class_logger(cls, enable=False):
    #logger = logging.getLogger("{}.{}".format(cls.__module__, cls.__name__))
    logger = logging.getLogger("{}".format(cls.__name__))
    if enable == 'debug':
        logger.setLevel(logging.DEBUG)
    elif enable == 'info':
        logger.setLevel(logging.INFO)
    cls._should_log_debug = logger.isEnabledFor(logging.DEBUG)
    cls._should_log_info = logger.isEnabledFor(logging.INFO)
    cls.logger = logger


def add_class_logger_handler(handler, class_list, level=logging.INFO):
    """为指定的类增加日志 handler，并设定级别

    :param handler: logging handler
    :param class_list: 类列表
    :param level: 日志级别
    """
    for cls in class_list:
        #logger = logging.getLogger("{}.{}".format(cls.__module__, cls.__name__))
        logger = logging.getLogger("{}".format(cls.__name__))
        logger.addHandler(handler)
        logger.setLevel(level)
