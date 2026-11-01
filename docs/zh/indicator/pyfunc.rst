.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

自定义函数指标
==============

用 numpy / pandas / ctypes 等 Python 生态的自定义函数对整段序列执行一次计算，并把结果包装成普通指标节点，
可以直接参与指标运算、信号生成与批量计算。入口有两个：

* :py:func:`np_apply` —— 输入为任意指标，或指标组成的 list / tuple（最多 6 个）
* :py:meth:`Indicator.apply` —— 以当前指标为输入，其余参数与 np_apply 相同

适用于 numpy 向量化计算、numba 编译的循环以及已有的 ctypes 原生函数。逐元素回调
（``np.vectorize`` / ``np.frompyfunc``）不受支持：每个元素都要回到 Python，性能不如直接使用指标公式。


.. py:function:: np_apply(ref, func, *, result_num=1, discard=None, view="numpy", with_dates=False, stateless=False, name="APPLY", hints=None)

    将自定义函数包装为指标，函数对整段序列执行一次，而不是逐元素执行

    :param ref: 输入指标，或由最多 6 个指标组成的 list / tuple；每个输入指标的每个结果集都会作为一个独立的输入数组传入
    :type ref: Indicator | Sequence[Indicator]
    :param func: 计算函数，形态见 `调用形态`_
    :param int result_num: 结果集个数（1~6），必须与 func 实际返回的结果个数一致
    :param discard: 需要抛弃的前导点数；None 表示继承输入（多输入时取各输入的最大值），显式指定时以指定值为准
    :type discard: int | None
    :param str view: ``"numpy"``（默认）或 ``"pandas"``
    :param bool with_dates: 是否把日期列表作为 func 的第一个参数（仅普通 Python 可调用）
    :param bool stateless: 声明 func 为纯函数（无内部状态）；否则包装同一 func 的两个节点永远不会被合并或复用
    :param str name: 指标名称，默认 ``"APPLY"``
    :param dict hints: 附加开关，见 `raw 直调模式`_；支持 ``contiguous``（接受但无效果，hikyuu 的缓冲区始终连续）与 ``raw``
    :rtype: Indicator
    :raises TypeError: func 形态不受支持（np.vectorize、仅提供 object 循环的 ufunc 等）
    :raises ValueError: result_num / discard / view / hints 非法，或 func 返回的结果个数、长度不符
    :note: 包装层自身的错误信息统一以 ``np_apply:`` 开头；func 内部抛出的异常保持原类型与信息


调用形态
--------

按 func 的形态选择内核，优先级自上而下：

1. **numpy ufunc**（``np.log`` / ``np.maximum`` / 带类型签名的 numba ``DUFunc``）：通过 ``out=`` 直接写入
   结果缓冲区，零拷贝；要求 ufunc 提供与构建精度匹配的循环（常规构建为 float64，低精度构建为 float32），
   且输入、输出个数与 ref 一致。numba ``DUFunc`` 若未暴露带 ``->`` 的类型签名，则退化为普通 Python 可调用。
2. **普通 Python 可调用**：以只读一维 ndarray 作为参数调用一次（dtype 为构建精度），返回值须为一维数组，
   长度与输入相同；``result_num > 1`` 时须返回等长的 tuple / list。
3. **pandas 视图**（``view="pandas"``）：单输入时传入 ``Series``（有日期时索引为 DatetimeIndex，否则为
   RangeIndex），多输入时传入列名为 ``value0`` / ``value1`` … 的 ``DataFrame``；可返回 ``Series``、
   ``DataFrame``（按列拆分为多个结果集）、数组或 tuple。
4. **ctypes 函数**（``func`` 为 ``ctypes._CFuncPtr``，或 numba cfunc 的 ``.ctypes``）：原型须为
   ``void(in..., out..., total)``，参数个数须等于 输入数 + 结果数 + 1（由 ``argtypes`` 校验），
   缓冲区参数须声明为指针或 ``c_void_p``，长度须为非指针；指针参数按 ``argtypes`` 转换，末尾的
   长度按整数传递。不支持 ``view="pandas"`` 与 ``with_dates``。
5. **raw 直调**（``hints={"raw": True}``）：见下节。


计算契约
--------

* 输出长度恒等于输入长度，且与所有结果集一致；长度不符时报 ValueError，并给出实际 shape。
* 结果集中的 NaN 即 Null，不会被替换成 0.0 或其它值。
* 返回值统一按构建精度（常规 float64 / 低精度 float32）转换后写入缓冲区。
* 空输入不会调用 func，直接得到空结果。
* ``discard`` 声明的前导区段在计算结束后统一写成 Null：func 即便写满了整段，前导点也不会泄漏出去。


示例
----

整段一次计算，输入与结果各自只跨越 GIL 边界一次::

    import numpy as np

    a = PRICELIST([1.0, 2.0, 4.0, 8.0])
    ind = np_apply(a, np.log)
    ind.to_array(0)

    # 等价写法
    ind = a.apply(np.log)

ufunc 与多输入::

    ind = np_apply((HIGH(), LOW()), np.maximum)
    ind = np_apply([HIGH(), LOW()], lambda h, l: (h + l) / 2.0)

pandas 视图（func 收到 Series 或 DataFrame）::

    ind = np_apply(CLOSE(), lambda s: s.ewm(span=12).mean(), view="pandas")

多结果集::

    ind = np_apply(CLOSE(),
                   lambda x: (np.minimum.accumulate(x), np.maximum.accumulate(x)),
                   result_num=2)

日期作为首个参数::

    dates = [Datetime(2020, 1, i + 1) for i in range(5)]
    a = PRICELIST([1.0, 2.0, 4.0, 8.0, 16.0], 0, dates)
    ind = np_apply(a, lambda ds, x: np.array([d.day_of_week() for d in ds], dtype=float),
                   with_dates=True)

numba 编译的循环::

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

numba cfunc（原生原型 ``void(in..., out..., total)``）::

    from numba import cfunc, types

    @cfunc(types.void(types.CPointer(types.float64), types.CPointer(types.float64),
                      types.uint64), nopython=True)
    def scale(src, dst, n):
        for i in range(n):
            dst[i] = src[i] * 2.0

    ind = np_apply(CLOSE(), scale.ctypes)                # 经 ctypes 调用
    ind = np_apply(CLOSE(), scale, hints={"raw": True})  # 直调，整段调用期间释放 GIL


raw 直调模式
------------

``hints={"raw": True}`` 跳过 ctypes 包装，按 ``address`` 直接调用 numba cfunc，并在整段调用期间释放 GIL，
批量计算时不会因为这一个节点把整批线程串行化：

* 只对暴露 ``address`` 的原生函数有意义（numba cfunc）；不接受 ctypes 函数、ufunc、``view="pandas"``
  与 ``with_dates``。
* 只在能读到 ``.ctypes.argtypes`` 时校验参数个数，指针类型与顺序不做校验：签名不匹配会直接崩溃解释器，
  而不是抛出异常，因此该模式必须显式开启。
* 参数顺序固定为 ``in..., out..., total``；输入数组与结果集各自上限为 6。


在指标树中的行为
----------------

* 节点是真正的 C++ 指标实现（PyFuncIndicatorImp），不是 Python 实现，因此可直接参与运算、``formula()``、
  ``clone()``；克隆共享同一份不可变内核。
* 默认 ``stateless=False``：即使两个节点包装同一个函数，也不会被视为同一计算，不会被合并或批量复用。
* ``stateless=True`` 且其余参数完全一致时，同一函数对象构造的节点在 ``alike()`` 判定中被视为等价，
  从而允许批量复用与“相同子表达式只算一次”的常规剪枝。
* 不支持 pickle；不支持增量计算，也没有动态指标参数：每次计算都按完整区间重新执行。
