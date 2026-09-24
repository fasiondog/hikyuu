#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import unittest
from hikyuu.util.timeout import timeout, TerminableThread


class TestTimeout(unittest.TestCase):
    """Test various cases of the timeout decorator"""

    def test_normal_function(self):
        """Test a normally executed function"""
        @timeout(2)
        def normal_func():
            return "success"
        
        result = normal_func()
        self.assertEqual(result, "success")

    def test_timeout_function(self):
        """Test a timed-out function"""
        @timeout(0.1)
        def slow_func():
            time.sleep(0.5)
            return "should not reach here"
        
        with self.assertRaises(TimeoutError):
            slow_func()

    def test_exception_in_function(self):
        """Test an exception raised inside the function"""
        @timeout(1)
        def error_func():
            raise ValueError("test error")
        
        with self.assertRaises(ValueError):
            error_func()

    def test_zero_timeout(self):
        """Test a zero timeout value"""
        with self.assertRaises(ValueError):
            @timeout(0)
            def test_func():
                pass

    def test_negative_timeout(self):
        """Test a negative timeout value"""
        with self.assertRaises(ValueError):
            @timeout(-1)
            def test_func():
                pass

    def test_nested_calls(self):
        """Test nested calls"""
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
        """Test multi-thread concurrency"""
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
        """Test the cleanup function of TerminableThread"""
        def quick_func():
            return "quick"

        thread = TerminableThread(target=quick_func)
        thread.start()
        thread.join()
        
        # The thread should have finished
        self.assertFalse(thread.is_alive())
        
        # Test the behavior of terminate on a finished thread
        result = thread.terminate(Exception)
        self.assertTrue(result)  # Should return True, meaning no termination is needed


if __name__ == '__main__':
    unittest.main()