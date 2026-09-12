.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

投资组合
========

多因子（MultiFactor）与选择器（Selector）对多个交易对象进行截面评分与筛选，可用于构建多证券策略；
投资组合与资金分配分别由 :doc:`portfolio` 与 :doc:`allocate_funds` 提供（兼容层工厂直通
:class:`MultiSystem` / :class:`AllocateFundsBase`）。

.. toctree::

   portfolio
   multifactor
   selector
   allocate_funds
   normalize.md
   scfilter.md
