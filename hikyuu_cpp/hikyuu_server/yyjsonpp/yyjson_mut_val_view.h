/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-15
 *     Author: fasiondog
 */

#pragma once

#include <type_traits>
#include <string>
#include <vector>
#include <iterator>
#include <yyjson.h>
#include "exception.h"

namespace yyjson {

#define YY_MUT_VAL_IS(typ)                 \
    bool is_##typ() const {                \
        return yyjson_mut_is_##typ(m_val); \
    }

class mut_val_view {
public:
    mut_val_view(yyjson_mut_val *val) : m_val(val) {}

    yyjson_mut_val *ptr() {
        return m_val;
    }

    operator bool() const {
        return m_val != nullptr;
    }

    YY_MUT_VAL_IS(null)
    YY_MUT_VAL_IS(true)
    YY_MUT_VAL_IS(false)
    YY_MUT_VAL_IS(bool)
    YY_MUT_VAL_IS(uint)
    YY_MUT_VAL_IS(sint)
    YY_MUT_VAL_IS(int)
    YY_MUT_VAL_IS(real)  // double number
    YY_MUT_VAL_IS(num)   // integer or double number
    YY_MUT_VAL_IS(str)   // c字符串
    YY_MUT_VAL_IS(arr)   // array
    YY_MUT_VAL_IS(obj)   // object
    YY_MUT_VAL_IS(ctn)   // array or object

    bool arr_insert(mut_val_view val, size_t idx) {
        return yyjson_mut_arr_insert(m_val, val.ptr(), idx);
    }

private:
    yyjson_mut_val *m_val;
};

}  // namespace yyjson