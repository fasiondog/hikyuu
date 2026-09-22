.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Performance Statistics
======================

.. py:class:: Performance

    Simple performance statistics

    .. py:method:: reset(self)

        Reset, clearing the calculated results

    .. py:method:: report(self, tm[, datetime=Datetime.now()])

        A simple text statistics report, used for direct printing

        :param TradeManager tm: the specified trade manager instance
        :param Datetime datetime: the statistics end moment
        :rtype: str

    .. py:method:: statistics(self, tm[, datetime=Datetime.now()])

        Count the system performance up to a certain moment according to the trade records; datetime must be greater than or equal to lastDatetime

        :param TradeManager tm: the specified trade manager instance
        :param Datetime datetime: the statistics end moment

    .. py:method:: get(self, name)

        Get the indicator value by the indicator name; it takes effect only after statistics or report has been run

        :param str name: the indicator name
        :rtype: float

    .. py:method:: __getitem__(self, name)

        The same as the get method. Get the indicator value by the indicator name; it takes effect only after statistics or report has been run

        :param str name: the indicator name
        :rtype: float
