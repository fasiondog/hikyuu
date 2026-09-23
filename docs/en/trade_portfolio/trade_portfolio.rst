.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Portfolio
=========

Multi-factor (MultiFactor) and the Selector perform cross-sectional scoring and filtering on multiple trading objects, usable for building multi-security strategies;
the portfolio and the fund allocation are provided by :doc:`portfolio` and :doc:`allocate_funds` respectively (the compatibility layer factories pass through to
:class:`MultiSystem` / :class:`AllocateFundsBase`).

.. toctree::

   portfolio
   multifactor
   selector
   allocate_funds
   normalize.md
   scfilter.md
