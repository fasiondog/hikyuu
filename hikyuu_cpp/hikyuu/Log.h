/*
 * Log.h
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#ifndef LOG_H_
#define LOG_H_

#include <string>

#define USE_SPDLOG_FOR_LOGGING 1
//#define USE_BOOST_LOG_FOR_LOGGING 1
//#define USE_STDOUT_FOR_LOGGING 1

#ifdef USE_BOOST_LOG_FOR_LOGGING
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#endif

#ifdef USE_SPDLOG_FOR_LOGGING
#include <spdlog/spdlog.h>
#endif

#ifdef USE_STDOUT_FOR_LOGGING
#include "datetime/Datetime.h"
#endif

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

void HKU_API init_logger(const std::string& configue_name);


/**********************************************
 * Use Boost.log for logging
 *
 *********************************************/
#ifdef USE_BOOST_LOG_FOR_LOGGING
#define HKU_DEBUG(msg)  BOOST_LOG_TRIVIAL(debug) << msg
#define HKU_TRACE(msg)  BOOST_LOG_TRIVIAL(trace) << msg
#define HKU_INFO(msg)   BOOST_LOG_TRIVIAL(info) << msg
#define HKU_WARN(msg)   BOOST_LOG_TRIVIAL(warning) << msg
#define HKU_ERROR(msg)  BOOST_LOG_TRIVIAL(error) << msg
#define HKU_FATAL(msg)  BOOST_LOG_TRIVIAL(fatal) << msg
#endif /* for USE_BOOST_LOG_FOR_LOGGING */


/**********************************************
 * Use SPDLOG for logging
 * note: SPDLOG TRACE < DEBUG < INFO < WARN
 *
 *********************************************/
#ifdef USE_SPDLOG_FOR_LOGGING
#define HKU_DEBUG(msg)  { std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->debug(buf.str().c_str());}
#define HKU_TRACE(msg)  { std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->trace(buf.str().c_str());}
#define HKU_INFO(msg)  { std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->info(buf.str().c_str());}
#define HKU_WARN(msg)  { std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->warn(buf.str().c_str());}
#define HKU_ERROR(msg)  { std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->error(buf.str().c_str());}
#define HKU_FATAL(msg)  { std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->critical(buf.str().c_str());}
#endif


/**********************************************
 * Use STDOUT for logging
 *
 *********************************************/
#ifdef USE_STDOUT_FOR_LOGGING
#define HKU_DEBUG(msg)  { std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [DEBUG] " << msg;\
                          std::cout << buf.str() << std::endl}
#define HKU_TRACE(msg)  { std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [TRACE] " << msg;\
                          std::cout << buf.str() << std::endl;}
#define HKU_INFO(msg)   { std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [INFO] " << msg;\
                          std::cout << buf.str() << std::endl;}
#define HKU_WARN(msg)   { std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [WARN] " << msg;\
                          std::cout << buf.str() << std::endl;}
#define HKU_ERROR(msg)  { std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [ERROR] " << msg;\
                          std::cout << buf.str() << std::endl;}
#define HKU_FATAL(msg)  { std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [FATAL] " << msg;\
                          std::cout << buf.str() << std::endl;}
#endif /* for USE_STDOUT_FOR_LOGGING */

} /* namespace hku */
#endif /* LOG_H_ */
