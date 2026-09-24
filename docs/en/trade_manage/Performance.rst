.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Performance Statistics
======================

.. py:class:: Performance

    Lightweight performance metrics

    .. py:method:: reset(self)

        Reset and clear all computed results

    .. py:method:: report(self, tm[, datetime=Datetime.now()])

        A simple text statistics report for direct printing

        :param TradeManager tm: the specified trade manager instance
        :param Datetime datetime: the statistics end moment
        :rtype: str

    .. py:method:: statistics(self, tm[, datetime=Datetime.now()])

        Compute system performance metrics up to a given moment from the trade records; datetime must be greater than or equal to lastDatetime

        :param TradeManager tm: the specified trade manager instance
        :param Datetime datetime: the statistics end moment

    .. py:method:: get(self, name)

        Get a metric value by its name; it takes effect only after statistics or report has been run

        :param str name: the metric name
        :rtype: float

    .. py:method:: __getitem__(self, name)

        The same as the get method. Get a metric value by its name; it takes effect only after statistics or report has been run

        :param str name: the metric name
        :rtype: float
