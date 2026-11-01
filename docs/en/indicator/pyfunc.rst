.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

Custom Function Indicators
==========================

Run a custom function from the Python ecosystem (numpy / pandas / ctypes) once over the whole series
and wrap its result as an ordinary indicator node, so that it takes part in indicator arithmetic,
signal generation and batch calculation like any other node. There are two entry points:

* :py:func:`np_apply` — the input is any indicator, or a list / tuple of indicators (at most 6)
* :py:meth:`Indicator.apply` — uses the current indicator as the input, with the same remaining
  parameters as np_apply

It suits numpy vectorized math, numba compiled loops and existing native ctypes functions. A per
element callback (``np.vectorize`` / ``np.frompyfunc``) is not supported: every element would have to
come back to Python, which is slower than an indicator formula.


.. py:function:: np_apply(ref, func, *, result_num=1, discard=None, view="numpy", with_dates=False, stateless=False, name="APPLY", hints=None)

    Wrap a custom function as an indicator; the function runs once over the whole series, not per
    element

    :param ref: an input indicator, or a list / tuple of at most 6 indicators; every result set of
        every input is passed as a separate input array
    :type ref: Indicator | Sequence[Indicator]
    :param func: the calculation function, see `Callable Shapes`_
    :param int result_num: the number of result sets (1 to 6), it must match the number of results
        the function actually returns
    :param discard: the number of leading points to discard; None inherits the input (the maximum of
        the inputs when there are several), an explicit value takes precedence
    :type discard: int | None
    :param str view: ``"numpy"`` (the default) or ``"pandas"``
    :param bool with_dates: whether the date list is passed as the first argument (a plain Python
        callable only)
    :param bool stateless: declares the function as pure (no internal state); otherwise two nodes
        wrapping the same function are never merged or reused
    :param str name: the indicator name, ``"APPLY"`` by default
    :param dict hints: further switches, see `Raw Call Mode`_; ``contiguous`` is accepted but has no
        effect (the hikyuu buffers are always contiguous), ``raw`` is the opt-in native call
    :rtype: Indicator
    :raises TypeError: the shape of func is not supported (np.vectorize, a ufunc with object loops
        only, ...)
    :raises ValueError: result_num / discard / view / hints is invalid, or func returns a wrong
        number of results or a wrong length
    :note: An error of the wrapper itself starts with ``np_apply:``; an exception raised inside func
        keeps its own type and message


Callable Shapes
---------------

The kernel is chosen from the shape of func, in this order:

1. **numpy ufunc** (``np.log`` / ``np.maximum`` / a numba ``DUFunc`` carrying a type signature):
   written straight into the result buffer through ``out=``, no copy; the ufunc must provide a loop
   matching the build precision (float64 in a normal build, float32 in a low-precision build) and the
   input / output counts must match ref. A numba ``DUFunc`` without an ``->`` type signature falls
   back to a plain Python callable.
2. **plain Python callable**: called once with read-only one-dimensional ndarrays (dtype of the
   build precision); it must return a one-dimensional array of the same length, and with
   ``result_num > 1`` a tuple / list of exactly that many arrays.
3. **pandas view** (``view="pandas"``): a single input arrives as a ``Series`` (a DatetimeIndex when
   dates are available, a RangeIndex otherwise), several inputs as a ``DataFrame`` with the columns
   ``value0`` / ``value1`` ...; the function may return a ``Series``, a ``DataFrame`` (split into one
   result per column), an array or a tuple.
4. **ctypes function** (``func`` is a ``ctypes._CFuncPtr``, or a numba cfunc ``.ctypes``): the
   prototype must be ``void(in..., out..., total)`` and the argument count must equal
   inputs + results + 1 (validated through ``argtypes``); the buffer arguments must be declared as
   ctypes pointers or ``c_void_p`` and the length as a non pointer; the pointer arguments are cast
   according to ``argtypes`` and the trailing length is passed as an integer. ``view="pandas"``
   and ``with_dates`` are not supported.
5. **raw call** (``hints={"raw": True}``): see the next section.


Calculation Contract
--------------------

* The output length always equals the input length, for every result set; a mismatch raises a
  ValueError carrying the actual shape.
* A NaN in a result set is Null and is never replaced with 0.0 or another value.
* Results are converted to the build precision (float64 normally, float32 in a low-precision build)
  before being written into the buffer.
* An empty input never calls func and yields an empty result.
* The leading region declared by ``discard`` is written as Null once the calculation finishes: even
  when the function writes the whole series, the discarded prefix never leaks out.


Examples
--------

One call per series, so the input and the results each cross the GIL boundary once::

    import numpy as np

    a = PRICELIST([1.0, 2.0, 4.0, 8.0])
    ind = np_apply(a, np.log)
    ind.to_array(0)

    # the equivalent form
    ind = a.apply(np.log)

A ufunc and several inputs::

    ind = np_apply((HIGH(), LOW()), np.maximum)
    ind = np_apply([HIGH(), LOW()], lambda h, l: (h + l) / 2.0)

The pandas view (the function receives a Series or a DataFrame)::

    ind = np_apply(CLOSE(), lambda s: s.ewm(span=12).mean(), view="pandas")

Several result sets::

    ind = np_apply(CLOSE(),
                   lambda x: (np.minimum.accumulate(x), np.maximum.accumulate(x)),
                   result_num=2)

Dates as the first argument::

    dates = [Datetime(2020, 1, i + 1) for i in range(5)]
    a = PRICELIST([1.0, 2.0, 4.0, 8.0, 16.0], 0, dates)
    ind = np_apply(a, lambda ds, x: np.array([d.day_of_week() for d in ds], dtype=float),
                   with_dates=True)

A numba compiled loop::

    import numba

    @numba.njit(nogil=True, cache=False)
    def ewma(x, n):
        out = np.empty_like(x)
        alpha = 2.0 / (n + 1)
        out[0] = x[0]
        for i in range(1, x.size):
            out[i] = alpha * x[i] + (1 - alpha) * out[i - 1]
        return out

    ind = np_apply(CLOSE(), lambda x: ewma(x, 10))

A numba cfunc (native prototype ``void(in..., out..., total)``)::

    from numba import cfunc, types

    @cfunc(types.void(types.CPointer(types.float64), types.CPointer(types.float64),
                      types.uint64), nopython=True)
    def scale(src, dst, n):
        for i in range(n):
            dst[i] = src[i] * 2.0

    ind = np_apply(CLOSE(), scale.ctypes)                # through ctypes
    ind = np_apply(CLOSE(), scale, hints={"raw": True})  # direct, the GIL is dropped for the call


Raw Call Mode
-------------

``hints={"raw": True}`` skips the ctypes wrapper and calls the numba cfunc through its ``address``,
dropping the GIL for the whole call so that one such node does not serialize a whole batch:

* It only applies to a native function exposing ``address`` (a numba cfunc); a ctypes function, a
  ufunc, ``view="pandas"`` and ``with_dates`` are refused.
* The argument count is checked only when ``.ctypes.argtypes`` is readable; the pointer types and
  their order are not, so a mismatched signature crashes the interpreter instead of raising. That is
  why the mode is an explicit opt-in.
* The argument order is fixed at ``in..., out..., total``; the input arrays and the result sets are
  each limited to 6.


Inside an Indicator Tree
------------------------

* The node is a real C++ indicator implementation (PyFuncIndicatorImp), not a Python one, so it
  takes part in arithmetic, ``formula()`` and ``clone()``; clones share the same immutable kernel.
* ``stateless=False`` is the default: even two nodes wrapping the same function are not treated as
  the same calculation, so they are never merged or reused.
* With ``stateless=True`` and otherwise identical parameters, nodes built from the same function
  object count as equivalent in the ``alike()`` test, which allows batch reuse and the usual
  "evaluate a repeated sub-expression once" pruning.
* Pickling is not supported; there is no incremental calculation and no dynamic indicator parameter:
  every calculation re-runs the function over the full range.
