/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-10-29
 *      Author: fasiondog
 */

#include "convert_any.h"
#include <datetime.h>

Datetime pydatetime_to_Datetime(const pybind11::object& source) {
    Datetime value;
    if (source.is_none()) {
        return value;
    }

    if (!PyDateTimeAPI) {
        PyDateTime_IMPORT;
    }

    PyObject* src = source.ptr();

    long year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, microsecond = 0;
    if (PyDateTime_Check(src)) {
        second = PyDateTime_DATE_GET_SECOND(src);
        minute = PyDateTime_DATE_GET_MINUTE(src);
        hour = PyDateTime_DATE_GET_HOUR(src);
        day = PyDateTime_GET_DAY(src);
        month = PyDateTime_GET_MONTH(src);
        year = PyDateTime_GET_YEAR(src);
        microsecond = PyDateTime_DATE_GET_MICROSECOND(src);

    } else if (PyDate_Check(src)) {
        day = PyDateTime_GET_DAY(src);
        month = PyDateTime_GET_MONTH(src);
        year = PyDateTime_GET_YEAR(src);

    } else if (PyTime_Check(src)) {
        second = PyDateTime_TIME_GET_SECOND(src);
        minute = PyDateTime_TIME_GET_MINUTE(src);
        hour = PyDateTime_TIME_GET_HOUR(src);
        day = 1;      // This date (day, month, year) = (1, 0, 70)
        month = 1;    // represents 1-Jan-1940, which is the first
        year = 1400;  // earliest available date for Datetime, not Python datetime
        microsecond = PyDateTime_TIME_GET_MICROSECOND(src);

    } else if (pybind11::isinstance<Datetime>(source)) {
        value = source.cast<Datetime>();
    } else {
        throw std::invalid_argument("Can't convert this python object to Datetime!");
    }

    // Datetime 最小只到 1400年 1 月 1日，最大只到 9999 年 12月 31 日 0点
    if (year < 1400) {
        value = Datetime::min();
    } else if (Datetime(year, month, day) == Datetime::max()) {
        value = Datetime::max();
    } else {
        long millisecond = microsecond / 1000;
        microsecond = microsecond - millisecond * 1000;
        value = Datetime(year, month, day, hour, minute, second, millisecond, microsecond);
    }

    return value;
}
