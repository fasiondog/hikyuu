/*
 * pickle_template.h
 *
 *  Created on: 2013-4-28
 *      Author: fasiondog
 */

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
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/python.hpp>

namespace bp = boost::python;

/*
 * normal_pickle_suite 用于初始化函数__init__没有参数的情况
 */
template<class T>
struct normal_pickle_suite : bp::pickle_suite {
    static bp::object getstate(const T& param) {
        std::ostringstream os;
        OUTPUT_ARCHIVE oa(os);
        oa << param;
        return bp::str (os.str());
    }

    static void
      setstate(T& params, bp::object entries) {
        bp::str s = bp::extract<bp::str>(entries)();
        std::string st = bp::extract<std::string>(s)();
        std::istringstream is(st);

        INPUT_ARCHIVE ia(is);
        ia >> params;
      }
};


/*
 * name_init_pickle_suite，用于没有无参初始化函数，但有string做参数的初始化函数的情况
 */
template<class T>
struct name_init_pickle_suite : bp::pickle_suite {
    static
    boost::python::tuple
    getinitargs(T const& w) {
        return boost::python::make_tuple(w.name());
    }

    static bp::object getstate(const T& param) {
        std::ostringstream os;
        OUTPUT_ARCHIVE oa(os);
        oa << param;
        return bp::str (os.str());
    }

    static void
      setstate(T& params, bp::object entries) {
        bp::str s = bp::extract<bp::str>(entries)();
        std::string st = bp::extract<std::string>(s)();
        std::istringstream is(st);

        INPUT_ARCHIVE ia(is);
        ia >> params;
      }
};
#endif /* HKU_PYTHON_SUPPORT_PICKLE */

#endif /* PICKLE_SUPPORT_H_ */
