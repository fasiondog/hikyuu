/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2020-5-24
 *      Author: fasiondog
 */

#pragma once
#ifndef PICKLE_SUPPORT_H_
#define PICKLE_SUPPORT_H_

#include <hikyuu/config.h>

#if HKU_SUPPORT_BINARY_ARCHIVE || HKU_SUPPORT_XML_ARCHIVE || HKU_SUPPORT_TEXT_ARCHIVE
#define HKU_PYTHON_SUPPORT_PICKLE 1
#endif

#if HKU_PYTHON_SUPPORT_PICKLE
#if HKU_SUPPORT_BINARY_ARCHIVE
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#define OUTPUT_ARCHIVE boost::archive::binary_oarchive
#define INPUT_ARCHIVE boost::archive::binary_iarchive

#else
#if HKU_SUPPORT_XML_ARCHIVE
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#define OUTPUT_ARCHIVE boost::archive::xml_oarchive
#define INPUT_ARCHIVE boost::archive::xml_iarchive

#else
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#define OUTPUT_ARCHIVE boost::archive::text_oarchive
#define INPUT_ARCHIVE boost::archive::text_iarchive
#endif /* HKU_SUPPORT_XML_ARCHIVE */

#endif /* HKU_SUPPORT_BINARY_ARCHIVE */
#endif /* HKU_PYTHON_SUPPORT_PICKLE */

#if HKU_PYTHON_SUPPORT_PICKLE
#include <string>
#include <sstream>
#include <pybind11/pybind11.h>

namespace py = pybind11;

#define DEF_PICKLE(classname)                                                                      \
    .def(py::pickle(                                                                               \
      [](const classname& p) {                                                                     \
          std::ostringstream os;                                                                   \
          OUTPUT_ARCHIVE oa(os);                                                                   \
          oa << p;                                                                                 \
          std::string tmp(os.str());                                                               \
          return py::make_tuple(py::handle(PyBytes_FromStringAndSize(tmp.data(), tmp.size())));    \
      },                                                                                           \
      [](py::tuple t) {                                                                            \
          classname result;                                                                        \
          if (len(t) != 1) {                                                                       \
              PyErr_SetObject(                                                                     \
                PyExc_ValueError,                                                                  \
                py::str("expected 1-item tuple in call to __setstate__; got {}").format(t).ptr()); \
              throw py::error_already_set();                                                       \
          }                                                                                        \
          py::object obj = t[0];                                                                   \
          if (py::isinstance<py::str>(obj)) {                                                      \
              std::string st = obj.cast<py::str>();                                                \
              std::istringstream is(st);                                                           \
              INPUT_ARCHIVE ia(is);                                                                \
              ia >> result;                                                                        \
          } else if (PyBytes_Check(py::object(t[0]).ptr())) {                                      \
              py::object obj = t[0];                                                               \
              char* data = PyBytes_AsString(obj.ptr());                                            \
              auto num = PyBytes_Size(obj.ptr());                                                  \
              std::istringstream sin(std::string(data, num));                                      \
              INPUT_ARCHIVE ia(sin);                                                               \
              ia >> result;                                                                        \
              return result;                                                                       \
          } else {                                                                                 \
              throw std::runtime_error("Unable to unpickle, error in input file.");                \
          }                                                                                        \
          return result;                                                                           \
      }))

#else
#define DEF_PICKLE(classname)
#endif /* HKU_PYTHON_SUPPORT_PICKLE */

#endif /* PICKLE_SUPPORT_H_ */
