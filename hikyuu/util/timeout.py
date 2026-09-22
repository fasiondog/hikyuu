#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import ctypes
import threading
import functools
import weakref
from typing import Optional, Type


class ThreadKiller(threading.Thread):
    """separate thread to kill TerminableThread"""

    def __init__(self, target_thread_ref: weakref.ref, exception_cls: Type[BaseException], repeat_sec: float = 2.0):
        threading.Thread.__init__(self)
        self.target_thread_ref = target_thread_ref
        self.exception_cls = exception_cls
        self.repeat_sec = repeat_sec
        self.daemon = True
        self._stop_event = threading.Event()

    def stop(self):
        """Stop the killer thread"""
        self._stop_event.set()

    def run(self):
        """loop raising exception incase it's caught hopefully this breaks us far out"""
        max_attempts = 5  # limit the maximum number of attempts
        attempts = 0

        while attempts < max_attempts and not self._stop_event.is_set():
            target_thread = self.target_thread_ref()
            if target_thread is None or not target_thread.is_alive():
                break

            try:
                # Use a weak reference to avoid a circular reference
                thread_ident = target_thread.ident
                if thread_ident is not None:
                    ctypes.pythonapi.PyThreadState_SetAsyncExc(
                        ctypes.c_long(thread_ident),
                        ctypes.py_object(self.exception_cls)
                    )
            except Exception:
                # Ignore the error when injecting the exception
                pass

            # Check whether the target thread is still alive
            target_thread = self.target_thread_ref()
            if target_thread is None or not target_thread.is_alive():
                break

            attempts += 1
            if attempts < max_attempts:
                # Wait for a while and retry
                self._stop_event.wait(min(self.repeat_sec, 0.1))

        # Clean up the resources
        self._stop_event.clear()


class TerminableThread(threading.Thread):
    """a thread that can be stopped by forcing an exception in the execution context"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._killer: Optional[ThreadKiller] = None

    def terminate(self, exception_cls: Type[BaseException], repeat_sec: float = 2.0):
        if not self.is_alive():
            return True

        # If a killer thread is already running, stop it first
        if self._killer is not None and self._killer.is_alive():
            self._killer.stop()

        # Use a weak reference to avoid a circular reference
        self._killer = ThreadKiller(weakref.ref(self), exception_cls, repeat_sec=repeat_sec)
        self._killer.start()
        return True


def timeout(sec: float, raise_sec: float = 1.0):
    """
    timeout decorator
    :param sec: function raise TimeoutError after ? seconds
    :param raise_sec: retry kill thread per ? seconds
        default: 1 second
    """
    if sec <= 0:
        raise ValueError("Timeout must be positive")

    def decorator(func):
        @functools.wraps(func)
        def wrapped_func(*args, **kwargs):
            err_msg = f'Function {func.__name__} timed out after {sec} seconds'

            class FuncTimeoutError(TimeoutError):
                def __init__(self):
                    super().__init__(err_msg)

            result = []
            exception = []
            completed = threading.Event()

            def run_func():
                try:
                    res = func(*args, **kwargs)
                    if not completed.is_set():  # save the result only when it has not timed out
                        result.append(res)
                except FuncTimeoutError:
                    # Timeout exception, the expected behavior in the normal case
                    pass
                except Exception as e:
                    if not completed.is_set():  # save the exception only when it has not timed out
                        exception.append(e)
                finally:
                    completed.set()

            # typically, a python thread cannot be terminated, use TerminableThread instead
            thread = TerminableThread(target=run_func, daemon=True)
            thread.start()

            # Wait for the thread to finish or time out
            thread.join(timeout=sec)

            if not completed.is_set():
                # Timeout: the thread is still running
                completed.set()  # mark as completed to prevent the subsequent result processing

                # Create a new exception class instance to terminate the thread
                exc = type('TimeoutError', FuncTimeoutError.__bases__, dict(FuncTimeoutError.__dict__))

                # Terminate the thread
                thread.terminate(exception_cls=exc, repeat_sec=raise_sec)

                # Wait for a short time to give the thread a chance to clean up
                thread.join(timeout=min(raise_sec, 1.0))

                raise TimeoutError(err_msg)
            elif exception:
                # If an exception occurred while the thread was running, raise it again
                raise exception[0]
            else:
                # The thread finished successfully, return the result
                return result[0] if result else None

        return wrapped_func

    return decorator
