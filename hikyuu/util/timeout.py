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
        """停止killer线程"""
        self._stop_event.set()

    def run(self):
        """loop raising exception incase it's caught hopefully this breaks us far out"""
        max_attempts = 5  # 限制最大尝试次数
        attempts = 0

        while attempts < max_attempts and not self._stop_event.is_set():
            target_thread = self.target_thread_ref()
            if target_thread is None or not target_thread.is_alive():
                break

            try:
                # 使用弱引用避免循环引用
                thread_ident = target_thread.ident
                if thread_ident is not None:
                    ctypes.pythonapi.PyThreadState_SetAsyncExc(
                        ctypes.c_long(thread_ident),
                        ctypes.py_object(self.exception_cls)
                    )
            except Exception:
                # 忽略注入异常时的错误
                pass

            # 检查目标线程是否还活着
            target_thread = self.target_thread_ref()
            if target_thread is None or not target_thread.is_alive():
                break

            attempts += 1
            if attempts < max_attempts:
                # 等待一段时间再重试
                self._stop_event.wait(min(self.repeat_sec, 0.1))

        # 清理资源
        self._stop_event.clear()


class TerminableThread(threading.Thread):
    """a thread that can be stopped by forcing an exception in the execution context"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._killer: Optional[ThreadKiller] = None

    def terminate(self, exception_cls: Type[BaseException], repeat_sec: float = 2.0):
        if not self.is_alive():
            return True

        # 如果已经有killer线程在运行，先停止它
        if self._killer is not None and self._killer.is_alive():
            self._killer.stop()

        # 使用弱引用避免循环引用
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
                    if not completed.is_set():  # 只有在未超时的情况下才保存结果
                        result.append(res)
                except FuncTimeoutError:
                    # 超时异常，正常情况下的预期行为
                    pass
                except Exception as e:
                    if not completed.is_set():  # 只有在未超时的情况下才保存异常
                        exception.append(e)
                finally:
                    completed.set()

            # typically, a python thread cannot be terminated, use TerminableThread instead
            thread = TerminableThread(target=run_func, daemon=True)
            thread.start()

            # 等待线程完成或超时
            thread.join(timeout=sec)

            if not completed.is_set():
                # 超时情况：线程仍在运行
                completed.set()  # 标记为已完成以防止后续结果处理

                # 创建新的异常类实例用于终止线程
                exc = type('TimeoutError', FuncTimeoutError.__bases__, dict(FuncTimeoutError.__dict__))

                # 终止线程
                thread.terminate(exception_cls=exc, repeat_sec=raise_sec)

                # 等待一小段时间让线程有机会清理
                thread.join(timeout=min(raise_sec, 1.0))

                raise TimeoutError(err_msg)
            elif exception:
                # 如果在线程运行期间发生异常，重新抛出
                raise exception[0]
            else:
                # 线程成功完成，返回结果
                return result[0] if result else None

        return wrapped_func

    return decorator
