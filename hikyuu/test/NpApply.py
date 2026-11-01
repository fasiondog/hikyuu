#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Tests for the M2 "numpy function indicator" wrapper (np_apply), implemented by the
# PyFuncIndicatorImp kernel layer in hikyuu_pywrap/indicator/pyfunc/.

import ctypes
import unittest

import numpy as np

from test_init import *

try:
    import numba
except ImportError:  # pragma: no cover - numba is optional
    numba = None

A = [1.0, 2.0, 4.0, 8.0, 16.0]


class NpApplyKernelTest(unittest.TestCase):
    """The kernel shapes: array, ufunc, njit, cfunc, pandas"""

    def test_array_callable(self):
        a = PRICELIST(A)
        ind = np_apply(a, lambda x: x - x.mean())
        self.assertEqual(len(ind), len(A))
        expected = np.asarray(A) - np.mean(A)
        self.assertTrue(np.allclose(ind.to_array(0), expected))
        self.assertEqual(ind.discard, 0)
        self.assertEqual(ind.get_result_num(), 1)

    def test_indicator_apply(self):
        a = PRICELIST(A)
        ind = a.apply(lambda x: x * 2)
        self.assertTrue(np.allclose(ind.to_array(0), np.asarray(A) * 2))

    def test_ufunc(self):
        a = PRICELIST(A)
        ind = np_apply(a, np.log)
        self.assertTrue(np.allclose(ind.to_array(0), np.log(np.asarray(A))))

    def test_raw_hint_false(self):
        a = PRICELIST(A)
        ind = np_apply(a, np.log, hints={"raw": False})
        self.assertTrue(np.allclose(ind.to_array(0), np.log(np.asarray(A))))

    def test_ufunc_multi_input(self):
        h = PRICELIST([1.0, 2.0, 3.0, 4.0, 5.0])
        l = PRICELIST([0.5, 1.0, 2.0, 3.0, 4.0])
        ind = np_apply((h, l), np.maximum)
        self.assertTrue(np.allclose(ind.to_array(0), [1.0, 2.0, 3.0, 4.0, 5.0]))

    def test_array_multi_input(self):
        h = PRICELIST([1.0, 2.0, 3.0, 4.0, 5.0])
        l = PRICELIST([0.5, 1.0, 2.0, 3.0, 4.0])
        ind = np_apply([h, l], lambda x, y: x + y)
        self.assertTrue(np.allclose(ind.to_array(0), [1.5, 3.0, 5.0, 7.0, 9.0]))

    @unittest.skipIf(numba is None, "numba is not installed")
    def test_njit(self):
        a = PRICELIST(A)

        @numba.njit(nogil=True, cache=False)
        def ewma(x, n):
            out = np.empty_like(x)
            alpha = 2.0 / (n + 1)
            out[0] = x[0]
            for i in range(1, x.size):
                out[i] = alpha * x[i] + (1 - alpha) * out[i - 1]
            return out

        ind = np_apply(a, lambda x: ewma(x, 3))
        # 1, 1.5, 2.75, 5.375, 10.6875
        self.assertTrue(np.allclose(ind.to_array(0), [1.0, 1.5, 2.75, 5.375, 10.6875]))

    @unittest.skipIf(numba is None, "numba is not installed")
    def test_cfunc(self):
        from numba import cfunc, types

        a = PRICELIST(A)

        @cfunc(types.void(types.CPointer(types.float64), types.CPointer(types.float64),
                          types.uint64), nopython=True)
        def double_kernel(src, dst, n):
            for i in range(n):
                dst[i] = src[i] * 2.0

        ind = np_apply(a, double_kernel.ctypes)
        self.assertTrue(np.allclose(ind.to_array(0), np.asarray(A) * 2))

        # the raw call mode takes the cfunc itself and calls it through its address
        raw = np_apply(a, double_kernel, hints={"raw": True})
        self.assertTrue(np.allclose(raw.to_array(0), np.asarray(A) * 2))

    def test_ctypes_cfunc(self):
        proto = ctypes.CFUNCTYPE(None, ctypes.POINTER(ctypes.c_double),
                                 ctypes.POINTER(ctypes.c_double), ctypes.c_size_t)

        def kernel(src, dst, n):
            for i in range(n):
                dst[i] = src[i] * 3.0

        a = PRICELIST(A)
        ind = np_apply(a, proto(kernel))
        self.assertEqual(len(ind), len(A))
        self.assertTrue(np.allclose(ind.to_array(0), np.asarray(A) * 3))

    def test_ctypes_cfunc_arg_num(self):
        proto = ctypes.CFUNCTYPE(None, ctypes.POINTER(ctypes.c_double))
        a = PRICELIST(A)
        with self.assertRaises(ValueError):
            np_apply(a, proto(lambda src: None))

    def test_cfunc_void_p_buffers(self):
        # c_void_p also receives the plain integer address of the buffer
        proto = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t)

        def kernel(src, dst, n):
            ctypes.memmove(dst, src, n * ctypes.sizeof(ctypes.c_double))

        a = PRICELIST(A)
        ind = np_apply(a, proto(kernel))
        self.assertTrue(np.allclose(ind.to_array(0), np.asarray(A)))

    def test_pandas_view(self):
        a = PRICELIST(A)
        ind = np_apply(a, lambda s: s.rolling(3).mean(), view="pandas")
        got = ind.to_array(0)
        self.assertTrue(np.isnan(got[0]) and np.isnan(got[1]))
        self.assertTrue(np.allclose(got[2:], [7.0 / 3.0, 14.0 / 3.0, 28.0 / 3.0]))

    def test_pandas_view_multi_input(self):
        h = PRICELIST([1.0, 2.0, 3.0, 4.0, 5.0])
        l = PRICELIST([0.5, 1.0, 2.0, 3.0, 4.0])
        ind = np_apply([h, l], lambda df: df.iloc[:, 0] + df.iloc[:, 1], view="pandas")
        self.assertTrue(np.allclose(ind.to_array(0), [1.5, 3.0, 5.0, 7.0, 9.0]))

    def test_with_dates(self):
        dates = [Datetime(2020, 1, i + 1) for i in range(len(A))]
        a = PRICELIST(A, 0, dates)
        ind = np_apply(a, lambda ds, x: np.array([d.day_of_week() for d in ds], dtype=float),
                       with_dates=True)
        self.assertEqual(len(ind), len(A))
        # 2020-01-01 is a Wednesday
        self.assertTrue(np.allclose(ind.to_array(0), [3.0, 4.0, 5.0, 6.0, 0.0]))

    def test_pandas_index_with_dates(self):
        dates = [Datetime(2020, 1, i + 1) for i in range(len(A))]
        a = PRICELIST(A, 0, dates)
        ind = np_apply(a, lambda s: np.array([d.year for d in s.index], dtype=float),
                       view="pandas")
        self.assertTrue(np.allclose(ind.to_array(0), [2020.0] * len(A)))

    def test_pandas_index_without_dates(self):
        a = PRICELIST(A)
        ind = np_apply(a, lambda s: np.array([i for i in s.index], dtype=float), view="pandas")
        self.assertTrue(np.allclose(ind.to_array(0), np.arange(len(A), dtype=float)))


class NpApplyResultTest(unittest.TestCase):
    """result_num, discard and the multi result accessors"""

    def test_multi_result(self):
        a = PRICELIST(A)
        ind = np_apply(a, lambda x: (x * 1.02, x * 0.98), result_num=2)
        self.assertEqual(ind.get_result_num(), 2)
        self.assertTrue(np.allclose(ind.get_result(0).to_array(0), np.asarray(A) * 1.02))
        self.assertTrue(np.allclose(ind.get_result(1).to_array(0), np.asarray(A) * 0.98))

    def test_discard_inherited(self):
        a = PRICELIST(A, 2)
        ind = np_apply(a, np.log)
        self.assertEqual(ind.discard, 2)

    def test_discard_explicit(self):
        a = PRICELIST(A, 2)
        ind = np_apply(a, np.log, discard=1)
        self.assertEqual(ind.discard, 1)

    def test_discard_prefix_is_null(self):
        a = PRICELIST(A, 2)
        # the callable writes the whole series, the declared prefix is wiped anyway
        got = np_apply(a, lambda x: np.full_like(x, 7.0), discard=2).to_array(0)
        self.assertTrue(np.isnan(got[0]) and np.isnan(got[1]))
        self.assertTrue(np.allclose(got[2:], 7.0))

    def test_nan_is_null(self):
        a = PRICELIST([1.0, -1.0, 4.0])
        with np.errstate(invalid="ignore"):
            got = np_apply(a, np.sqrt).to_array(0)
        self.assertTrue(np.isnan(got[1]))
        self.assertTrue(np.allclose(got[[0, 2]], [1.0, 2.0]))


class NpApplyContractTest(unittest.TestCase):
    """The calculation contract of the wrapper"""

    def test_empty_input_never_calls_func(self):
        called = []

        def func(x):
            called.append(len(x))
            return x

        ind = np_apply(MA(CLOSE(), 5), func)
        self.assertEqual(len(ind), 0)
        self.assertEqual(called, [])

    def test_result_length_mismatch(self):
        a = PRICELIST(A)
        with self.assertRaises(ValueError) as ctx:
            np_apply(a, lambda x: x[:-1])
        self.assertTrue(str(ctx.exception).startswith("np_apply:"))

    def test_result_count_mismatch(self):
        a = PRICELIST(A)
        with self.assertRaises(ValueError):
            np_apply(a, lambda x: (x, x))
        with self.assertRaises(ValueError):
            np_apply(a, lambda x: x, result_num=2)

    def test_exception_propagated(self):
        a = PRICELIST(A)

        def func(x):
            raise KeyError("boom")

        # a Python exception of the callable is not wrapped by the wrapper
        with self.assertRaises(KeyError):
            np_apply(a, func)


class NpApplyTreeTest(unittest.TestCase):
    """The node must stay a real indicator inside an indicator tree"""

    def test_node_keeps_cpp_implementation(self):
        a = PRICELIST(A)
        x = np_apply(a, np.log)
        imp = x.get_imp()
        # the node is the C++ PyFuncIndicatorImp, not a Python implementation
        self.assertEqual(x.name, "APPLY")
        self.assertIsInstance(imp, IndicatorImp)
        self.assertEqual(type(imp).__name__, "IndicatorImp")
        self.assertIs(imp, x.get_imp())

    def test_clone(self):
        a = PRICELIST(A)
        x = np_apply(a, np.log)
        cloned = x.clone()
        self.assertEqual(cloned.name, "APPLY")
        self.assertIsInstance(cloned.get_imp(), IndicatorImp)
        self.assertTrue(np.allclose(cloned.to_array(0), x.to_array(0)))

    def test_support_batch_reuse(self):
        a = PRICELIST(A)

        def func(x):
            return x * 2

        # a callable may keep state, it is only reusable when declared stateless
        self.assertFalse(np_apply(a, func).get_imp().get_param("_support_batch_reuse"))
        self.assertTrue(
            np_apply(a, func, stateless=True).get_imp().get_param("_support_batch_reuse"))
        self.assertTrue(np.allclose(np_apply(a, func, stateless=True).to_array(0),
                                    np.asarray(A) * 2))

    def test_tree_not_merged(self):
        a = PRICELIST(A)
        lg = np_apply(a, np.log)
        sq = np_apply(a, np.sqrt)
        tree = lg + sq
        expected = np.log(np.asarray(A)) + np.sqrt(np.asarray(A))
        self.assertTrue(np.allclose(tree.to_array(0), expected))
        self.assertNotEqual(tree.formula(), "APPLY(PRICELIST)")

    def test_tree_with_price_list(self):
        a = PRICELIST(A)
        x = np_apply(a, np.log)
        self.assertTrue(np.allclose((a + x).to_array(0), np.asarray(A) + np.log(np.asarray(A))))

    def test_nested_same_stateless(self):
        values = [2.0, 4.0, 8.0, 16.0, 32.0]
        b = PRICELIST(values)
        x = np_apply(b, np.log, stateless=True)
        # a new node wraps the node, it is not pruned as an equal node
        y = np_apply(x, np.log, stateless=True)
        self.assertTrue(np.allclose(y.to_array(0), np.log(np.log(np.asarray(values)))))

    def test_set_context(self):
        ind = np_apply(CLOSE(), np.log)
        ind.set_context(sm['sh000001'], Query(-100))
        self.assertEqual(len(ind), 100)
        self.assertTrue(np.allclose(ind.to_array(0), np.log(CLOSE(sm['sh000001'].get_kdata(
            Query(-100))).to_array(0))))


class NpApplyRejectTest(unittest.TestCase):
    """The unsupported shapes are rejected when the wrapper is built"""

    def test_vectorize_rejected(self):
        a = PRICELIST(A)
        with self.assertRaises(TypeError):
            np_apply(a, np.vectorize(lambda v: v))

    def test_object_loop_ufunc_rejected(self):
        a = PRICELIST(A)
        with self.assertRaises(TypeError):
            np_apply(a, np.frompyfunc(lambda v: v, 1, 1))

    def test_view(self):
        a = PRICELIST(A)
        with self.assertRaises(ValueError):
            np_apply(a, lambda x: x, view="list")

    def test_result_num_range(self):
        a = PRICELIST(A)
        for num in (0, 7):
            with self.assertRaises(ValueError):
                np_apply(a, lambda x: x, result_num=num)

    def test_discard_negative(self):
        a = PRICELIST(A)
        with self.assertRaises(ValueError):
            np_apply(a, lambda x: x, discard=-1)

    def test_ref_type(self):
        for ref in ([], [PRICELIST(A)] * 7, [PRICELIST(A), 1.0], 1.0):
            with self.assertRaises((TypeError, ValueError)):
                np_apply(ref, lambda *x: x[0])

    def test_unknown_hint(self):
        a = PRICELIST(A)
        with self.assertRaises(ValueError):
            np_apply(a, lambda x: x, hints={"copy": True})

    def test_ufunc_with_dates(self):
        a = PRICELIST(A)
        with self.assertRaises(TypeError):
            np_apply(a, np.log, with_dates=True)

    def test_pandas_with_cfunc(self):
        a = PRICELIST(A)
        with self.assertRaises(TypeError):
            np_apply(a, ctypes.CFUNCTYPE(None)(lambda *args: None), view="pandas")

    def test_cfunc_buffer_arg_type(self):
        a = PRICELIST(A)
        # a buffer slot declared as a value type would silently convert the address
        proto = ctypes.CFUNCTYPE(None, ctypes.c_double, ctypes.POINTER(ctypes.c_double),
                                 ctypes.c_size_t)
        with self.assertRaises(TypeError):
            np_apply(a, proto(lambda src, dst, n: None))
        # the length slot must not be a pointer
        proto = ctypes.CFUNCTYPE(None, ctypes.POINTER(ctypes.c_double),
                                 ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double))
        with self.assertRaises(TypeError):
            np_apply(a, proto(lambda *args: None))

    def test_non_callable(self):
        a = PRICELIST(A)
        with self.assertRaises(TypeError):
            np_apply(a, 1.0)


class NpApplyRawTest(unittest.TestCase):
    """The raw mode calls a native function through its address"""

    @unittest.skipIf(numba is None, "numba is not installed")
    def test_raw_call(self):
        from numba import cfunc, types

        @cfunc(types.void(types.CPointer(types.float64), types.CPointer(types.float64),
                          types.uint64), nopython=True)
        def double_kernel(src, dst, n):
            for i in range(n):
                dst[i] = src[i] * 2.0

        a = PRICELIST(A, 1)
        ind = np_apply(a, double_kernel, hints={"raw": True})
        self.assertEqual(ind.name, "APPLY")
        self.assertEqual(ind.discard, 1)
        got = ind.to_array(0)
        # the input keeps its invalid prefix, the kernel only sees the values as they are
        self.assertTrue(np.isnan(got[0]))
        self.assertTrue(np.allclose(got[1:], np.asarray(A)[1:] * 2))

    @unittest.skipIf(numba is None, "numba is not installed")
    def test_raw_multi_input(self):
        from numba import cfunc, types

        @cfunc(types.void(types.CPointer(types.float64), types.CPointer(types.float64),
                          types.CPointer(types.float64), types.uint64), nopython=True)
        def sum_kernel(x, y, dst, n):
            for i in range(n):
                dst[i] = x[i] + y[i]

        h = PRICELIST([1.0, 2.0, 3.0, 4.0, 5.0])
        l = PRICELIST([0.5, 1.0, 2.0, 3.0, 4.0])
        ind = np_apply((h, l), sum_kernel, hints={"raw": True})
        self.assertTrue(np.allclose(ind.to_array(0), [1.5, 3.0, 5.0, 7.0, 9.0]))

    @unittest.skipIf(numba is None, "numba is not installed")
    def test_raw_multi_result(self):
        from numba import cfunc, types

        @cfunc(types.void(types.CPointer(types.float64), types.CPointer(types.float64),
                          types.CPointer(types.float64), types.uint64), nopython=True)
        def band_kernel(src, up, down, n):
            for i in range(n):
                up[i] = src[i] * 1.02
                down[i] = src[i] * 0.98

        a = PRICELIST(A)
        ind = np_apply(a, band_kernel, result_num=2, hints={"raw": True})
        self.assertEqual(ind.get_result_num(), 2)
        self.assertTrue(np.allclose(ind.get_result(0).to_array(0), np.asarray(A) * 1.02))
        self.assertTrue(np.allclose(ind.get_result(1).to_array(0), np.asarray(A) * 0.98))

    @unittest.skipIf(numba is None, "numba is not installed")
    def test_raw_fills_discard_prefix(self):
        from numba import cfunc, types

        @cfunc(types.void(types.CPointer(types.float64), types.CPointer(types.float64),
                          types.uint64), nopython=True)
        def const_kernel(src, dst, n):
            for i in range(n):
                dst[i] = 7.0

        a = PRICELIST(A, 2)
        got = np_apply(a, const_kernel, hints={"raw": True}).to_array(0)
        # the native kernel writes the whole series, the inherited prefix is wiped anyway
        self.assertTrue(np.isnan(got[0]) and np.isnan(got[1]))
        self.assertTrue(np.allclose(got[2:], 7.0))

    def test_raw_needs_address(self):
        proto = ctypes.CFUNCTYPE(None, ctypes.POINTER(ctypes.c_double),
                                 ctypes.POINTER(ctypes.c_double), ctypes.c_size_t)
        a = PRICELIST(A)
        # a ctypes function has no address, and a plain callable is not native at all
        with self.assertRaises(TypeError):
            np_apply(a, proto(lambda src, dst, n: None), hints={"raw": True})
        with self.assertRaises(TypeError):
            np_apply(a, lambda x: x, hints={"raw": True})

    def test_raw_rejects_ufunc(self):
        a = PRICELIST(A)
        with self.assertRaises(TypeError):
            np_apply(a, np.log, hints={"raw": True})

    def test_raw_rejects_views(self):
        a = PRICELIST(A)
        with self.assertRaises(TypeError):
            np_apply(a, lambda x: x, view="pandas", hints={"raw": True})
        with self.assertRaises(TypeError):
            np_apply(a, lambda x: x, with_dates=True, hints={"raw": True})

    @unittest.skipIf(numba is None, "numba is not installed")
    def test_raw_arg_num(self):
        from numba import cfunc, types

        @cfunc(types.void(types.CPointer(types.float64), types.uint64), nopython=True)
        def kernel(src, n):
            for i in range(n):
                src[i] = src[i] * 2.0

        a = PRICELIST(A)
        with self.assertRaises(ValueError):
            np_apply(a, kernel, hints={"raw": True})


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(NpApplyKernelTest)


def suiteNpApplyResult():
    return unittest.TestLoader().loadTestsFromTestCase(NpApplyResultTest)


def suiteNpApplyContract():
    return unittest.TestLoader().loadTestsFromTestCase(NpApplyContractTest)


def suiteNpApplyTree():
    return unittest.TestLoader().loadTestsFromTestCase(NpApplyTreeTest)


def suiteNpApplyReject():
    return unittest.TestLoader().loadTestsFromTestCase(NpApplyRejectTest)


def suiteNpApplyRaw():
    return unittest.TestLoader().loadTestsFromTestCase(NpApplyRawTest)


if __name__ == '__main__':
    unittest.main()
