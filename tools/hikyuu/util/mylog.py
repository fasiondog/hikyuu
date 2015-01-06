#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

"""
该模块除escapetime函数外，均来自SQLAlchemy，做了少量修改

Logging control and utilities.

Control of logging for SA can be performed from the regular python logging
module.  The regular dotted module namespace is used, starting at
'sqlalchemy'.  For class-level logging, the class name is appended.

The "echo" keyword parameter which is available on SQLA ``Engine``
and ``Pool`` objects corresponds to a logger specific to that 
instance only.

E.g.::

    engine.echo = True

is equivalent to::

    import logging
    logger = logging.getLogger('sqlalchemy.engine.Engine.%s' % hex(id(engine)))
    logger.setLevel(logging.DEBUG)
    
"""

import logging
import sys
import time

def escapetime( func ):
    def wrappedFunc( *args, **kargs ):
        starttime = time.time()
        try:
            print("\nCalling: %s" % func.__name__)
            return func(*args, **kargs)
        finally:
            endtime = time.time()
            #print( "Called: %r" % func)
            print( "Escaped time: %.4fs, %.2fm" % (endtime-starttime, (endtime-starttime)/60) )
    return wrappedFunc


mylogger = logging.getLogger('my')
handler = logging.StreamHandler(sys.stdout)
handler.setFormatter(logging.Formatter(
            '%(asctime)s %(levelname)s %(name)s %(message)s'))
mylogger.addHandler(handler)

def set_debug_mode(logtype='debug'):
    global mylogger
    if logtype == 'debug':
        mylogger.setLevel(logging.DEBUG)
    elif logtype == 'info':
        mylogger.setLevel(logging.INFO)
    elif logtype == 'warning':
        mylogger.setLevel(logging.WARNING)
    elif logtype == 'error':
        mylogger.setLevel(logging.ERROR)
    elif logtype == 'critical':
        mylogger.setLevel(logging.CRITICAL)
    else:
        mylogger.setLevel(logging.ERROR)
     
rootlogger = logging.getLogger('stock')
if rootlogger.level == logging.NOTSET:
    rootlogger.setLevel(logging.WARN)

default_enabled = False
def default_logging(name):
    global default_enabled
    if logging.getLogger(name).getEffectiveLevel() < logging.WARN:
        default_enabled = True

    if not default_enabled:
        default_enabled = True
        handler = logging.StreamHandler(sys.stdout)
        handler.setFormatter(logging.Formatter(
            '%(asctime)s %(levelname)s %(name)s %(message)s'))
        rootlogger.addHandler(handler)

def class_logger(cls, enable=False):
    logger = logging.getLogger(cls.__module__ + "." + cls.__name__)
    if enable == 'debug':
        logger.setLevel(logging.DEBUG)
    elif enable == 'info':
        logger.setLevel(logging.INFO)
    cls._should_log_debug = logger.isEnabledFor(logging.DEBUG)
    cls._should_log_info = logger.isEnabledFor(logging.INFO)
    cls.logger = logger

def instance_logger(instance, echoflag=None):
    """create a logger for an instance.
    
    Warning: this is an expensive call which also results in a permanent
    increase in memory overhead for each call.  Use only for 
    low-volume, long-time-spanning objects.
    
    """
    
    # limit the number of loggers by chopping off the hex(id).
    # many novice users unfortunately create an unlimited number 
    # of Engines in their applications which would otherwise
    # cause the app to run out of memory.
    name = "%s.%s.0x...%s" % (instance.__class__.__module__,
                             instance.__class__.__name__,
                             hex(id(instance))[-4:])
    
    if echoflag is not None:
        l = logging.getLogger(name)
        if echoflag == 'debug':
            default_logging(name)
            l.setLevel(logging.DEBUG)
        elif echoflag is True:
            default_logging(name)
            l.setLevel(logging.INFO)
        elif echoflag is False:
            l.setLevel(logging.NOTSET)
    else:
        l = logging.getLogger(name)
    
    if not l.handlers:
        handler = logging.StreamHandler(sys.stdout)
        handler.setFormatter(logging.Formatter(
            '%(asctime)s %(levelname)s %(name)s %(message)s'))
        l.addHandler(handler)
    
    instance._should_log_debug = l.isEnabledFor(logging.DEBUG)
    instance._should_log_info = l.isEnabledFor(logging.INFO)
    return l

class echo_property(object):
    __doc__ = """\
    When ``True``, enable log output for this element.

    This has the effect of setting the Python logging level for the namespace
    of this element's class and object reference.  A value of boolean ``True``
    indicates that the loglevel ``logging.INFO`` will be set for the logger,
    whereas the string value ``debug`` will set the loglevel to
    ``logging.DEBUG``.
    """

    def __get__(self, instance, owner):
        if instance is None:
            return self
        else:
            return instance._should_log_debug and 'debug' or (instance._should_log_info and True or False)

    def __set__(self, instance, value):
        instance_logger(instance, echoflag=value)
