#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import unittest
from hikyuu.util.timeout import timeout, TerminableThread


class TestTimeout(unittest.TestCase):
    """测试timeout装饰器的各种情况"""

    def test_normal_function(self):
        """测试正常执行的函数"""
        @timeout(2)
        def normal_func():
            return "success"
        
        result = normal_func()
        self.assertEqual(result, "success")

    def test_timeout_function(self):
        """测试超时的函数"""
        @timeout(0.1)
        def slow_func():
            time.sleep(0.5)
            return "should not reach here"
        
        with self.assertRaises(TimeoutError):
            slow_func()

    def test_exception_in_function(self):
        """测试函数内部抛出异常"""
        @timeout(1)
        def error_func():
            raise ValueError("test error")
        
        with self.assertRaises(ValueError):
            error_func()

    def test_zero_timeout(self):
        """测试零超时值"""
        with self.assertRaises(ValueError):
            @timeout(0)
            def test_func():
                pass

    def test_negative_timeout(self):
        """测试负数超时值"""
        with self.assertRaises(ValueError):
            @timeout(-1)
            def test_func():
                pass

    def test_nested_calls(self):
        """测试嵌套调用"""
        @timeout(0.2)
        def inner_func():
            time.sleep(0.1)
            return "inner"

        @timeout(0.3)
        def outer_func():
            result = inner_func()
            time.sleep(0.1)
            return f"outer-{result}"

        result = outer_func()
        self.assertEqual(result, "outer-inner")

    def test_multiple_threads(self):
        """测试多线程并发"""
        import concurrent.futures
        
        @timeout(0.2)
        def test_func(x):
            time.sleep(0.1)
            return x * 2

        with concurrent.futures.ThreadPoolExecutor(max_workers=3) as executor:
            futures = [executor.submit(test_func, i) for i in range(5)]
            results = [f.result() for f in futures]
            
        expected = [0, 2, 4, 6, 8]
        self.assertEqual(results, expected)

    def test_terminable_thread_cleanup(self):
        """测试TerminableThread的清理功能"""
        def quick_func():
            return "quick"

        thread = TerminableThread(target=quick_func)
        thread.start()
        thread.join()
        
        # 线程应该已经结束
        self.assertFalse(thread.is_alive())
        
        # 测试terminate方法在已结束线程上的行为
        result = thread.terminate(Exception)
        self.assertTrue(result)  # 应该返回True表示无需终止


if __name__ == '__main__':
    unittest.main()