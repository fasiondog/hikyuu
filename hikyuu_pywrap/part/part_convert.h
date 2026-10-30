/*
 * part_convert.h
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_PART_CONVERT_H_
#define HKU_PART_CONVERT_H_

#include <pybind11/pybind11.h>

#include <hikyuu/part/Part.h>

namespace hku {
namespace pywrap {

/** Convert the part handle into the concrete Python object of the held alternative */
pybind11::object partToPython(const Part& part);

/** Convert a Python object back into the part handle; throws when it is not a part */
Part partFromPython(const pybind11::object& obj);

}  // namespace pywrap
}  // namespace hku

#endif /* HKU_PART_CONVERT_H_ */
