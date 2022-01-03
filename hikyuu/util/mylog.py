#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import logging
import traceback
import time
import functools


# 统计函数运行时间
def spend_time(func):
    @functools.wraps(func)
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
logging.basicConfig(format=FORMAT, level=logging.INFO)

hku_logger_name = 'hikyuu'
hku_logger = logging.getLogger(hku_logger_name)


def get_default_logger():
    return logging.getLogger(hku_logger_name)


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


def hku_debug(msg, logger=None):
    st = traceback.extract_stack()[-2]
    if logger:
        logger.debug("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
    else:
        hku_logger.debug("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))


hku_trace = hku_debug


def hku_info(msg, logger=None):
    st = traceback.extract_stack()[-2]
    if logger:
        logger.info("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
    else:
        hku_logger.info("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))


def hku_warn(msg, logger=None):
    st = traceback.extract_stack()[-2]
    if logger:
        logger.warning("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
    else:
        hku_logger.warning("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))


def hku_error(msg, logger=None):
    st = traceback.extract_stack()[-2]
    if logger:
        logger.error("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
    else:
        hku_logger.error("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))


def hku_fatal(msg, logger=None):
    st = traceback.extract_stack()[-2]
    if logger:
        logger.critical("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
    else:
        hku_logger.critical("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))


def hku_debug_if(exp, msg, logger=None, callback=None):
    if exp:
        st = traceback.extract_stack()[-2]
        if logger:
            logger.info("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
        else:
            hku_logger.info("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
        if callback:
            callback()


hku_trace_if = hku_debug_if


def hku_info_if(exp, msg, logger=None, callback=None):
    if exp:
        st = traceback.extract_stack()[-2]
        if logger:
            logger.info("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
        else:
            hku_logger.info("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
        if callback:
            callback()


def hku_warn_if(exp, msg, logger=None, callback=None):
    if exp:
        st = traceback.extract_stack()[-2]
        if logger:
            logger.warning("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
        else:
            hku_logger.warning("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
        if callback:
            callback()


def hku_error_if(exp, msg, logger=None, callback=None):
    if exp:
        st = traceback.extract_stack()[-2]
        if logger:
            logger.error("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
        else:
            hku_logger.error("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
        if callback:
            callback()


def hku_fatal_if(exp, msg, logger=None, callback=None):
    if exp:
        st = traceback.extract_stack()[-2]
        if logger:
            logger.critical("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
        else:
            hku_logger.critical("{} [{}] ({}:{})".format(msg, st.name, st.filename, st.lineno))
        if callback:
            callback()


# 跟踪函数运行
def with_trace(level=logging.INFO):
    def with_trace_wrap(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            hku_info('start run: %s' % func.__name__)
            result = func(*args, **kwargs)
            hku_info('completed: %s' % func.__name__)
            return result

        return wrapper

    return with_trace_wrap