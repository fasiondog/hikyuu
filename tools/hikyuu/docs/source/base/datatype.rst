.. py:currentmodule:: hikyuu
.. highlightlang:: python

基础数据类型
============


.. py:class:: PriceList

    价格序列，其中价格使用double表示，对应C++中的std::vector<double>。

    to_np()

        仅在安装了numpy模块时生效，转换为numpy.array

    to_df()

        仅在安装了pandas模块时生效，转换为pandas.DataFrame

