#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import ctypes
import threading
import functools


class ThreadKiller(threading.Thread):
    """separate thread to kill TerminableThread"""
    def __init__(self, target_thread, exception_cls, repeat_sec=2.0):
        threading.Thread.__init__(self)
        self.target_thread = target_thread
        self.exception_cls = exception_cls
        self.repeat_sec = repeat_sec
        self.daemon = True

    def run(self):
        """loop raising exception incase it's caught hopefully this breaks us far out"""
        while self.target_thread.is_alive():
            ctypes.pythonapi.PyThreadState_SetAsyncExc(
                ctypes.c_long(self.target_thread.ident), ctypes.py_object(self.exception_cls)
            )
            self.target_thread.join(self.repeat_sec)


class TerminableThread(threading.Thread):
    """a thread that can be stopped by forcing an exception in the execution context"""
    def terminate(self, exception_cls, repeat_sec=2.0):
        if self.is_alive() is False:
            return True
        killer = ThreadKiller(self, exception_cls, repeat_sec=repeat_sec)
        killer.start()


def timeout(sec, raise_sec=1):
    """
    timeout decorator
    :param sec: function raise TimeoutError after ? seconds
    :param raise_sec: retry kill thread per ? seconds
        default: 1 second
    """
    def decorator(func):
        @functools.wraps(func)
        def wrapped_func(*args, **kwargs):
            err_msg = f'Function {func.__name__} timed out after {sec} seconds'

            class FuncTimeoutError(TimeoutError):
                def __init__(self):
                    TimeoutError.__init__(self, err_msg)

            result, exception = [], []

            def run_func():
                try:
                    res = func(*args, **kwargs)
                except FuncTimeoutError:
                    pass
                except Exception as e:
                    exception.append(e)
                else:
                    result.append(res)

            # typically, a python thread cannot be terminated, use TerminableThread instead
            thread = TerminableThread(target=run_func, daemon=True)
            thread.start()
            thread.join(timeout=sec)
            if thread.is_alive():
                # a timeout thread keeps alive after join method, terminate and raise TimeoutError
                exc = type('TimeoutError', FuncTimeoutError.__bases__, dict(FuncTimeoutError.__dict__))
                thread.terminate(exception_cls=FuncTimeoutError, repeat_sec=raise_sec)
                raise TimeoutError(err_msg)
            elif exception:
                # if exception occurs during the thread running, raise it
                raise exception[0]
            else:
                # if the thread successfully finished, return its results
                return result[0]

        return wrapped_func

    return decorator