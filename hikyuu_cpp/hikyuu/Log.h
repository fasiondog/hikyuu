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
#include "spdlog/spdlog.h"
#endif

#ifdef USE_STDOUT_FOR_LOGGING
#include "datetime/Datetime.h"
#endif

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

enum LOG_LEVEL {
    DEBUG    = 0,
    TRACE    = 1,
    INFO     = 2,
    WARN     = 3,
    ERROR    = 4,
    FATAL    = 5,
    NO_PRINT = 6,
};

/**
 * 初始化 LOGGER
 * @param configue_name 配置文件名称
 */
void HKU_API init_logger(const std::string& configue_name);

/**
 * 获取当前日志级别
 * @return
 */
LOG_LEVEL HKU_API get_log_level();

/**
 * 设置日志级别
 * @param level 指定的日志级别
 */
void HKU_API set_log_level(LOG_LEVEL level);



/**********************************************
 * Use Boost.log for logging
 *
 *********************************************/
#ifdef USE_BOOST_LOG_FOR_LOGGING
#define HKU_DEBUG(msg)  if (get_log_level() <= LOG_LEVEL::DEBUG) {\
                          BOOST_LOG_TRIVIAL(debug) << msg; }
#define HKU_TRACE(msg)  if (get_log_level() <= LOG_LEVEL::TRACE) {\
                          BOOST_LOG_TRIVIAL(trace) << msg; }
#define HKU_INFO(msg)   if (get_log_level() <= LOG_LEVEL::INFO) {\
                          BOOST_LOG_TRIVIAL(info) << msg; }
#define HKU_WARN(msg)   if (get_log_level() <= LOG_LEVEL::WARN) {\
                          BOOST_LOG_TRIVIAL(warning) << msg; }
#define HKU_ERROR(msg)  if (get_log_level() <= LOG_LEVEL::ERROR) {\
                          BOOST_LOG_TRIVIAL(error) << msg; }
#define HKU_FATAL(msg)  if (get_log_level() <= LOG_LEVEL::FATAL) {\
                          BOOST_LOG_TRIVIAL(fatal) << msg; }
#endif /* for USE_BOOST_LOG_FOR_LOGGING */


/**********************************************
 * Use SPDLOG for logging
 * note: SPDLOG TRACE < DEBUG < INFO < WARN
 *
 *********************************************/
#ifdef USE_SPDLOG_FOR_LOGGING
#define HKU_DEBUG(msg)  if (get_log_level() <= LOG_LEVEL::DEBUG) {\
                          std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->debug(buf.str().c_str());}
#define HKU_TRACE(msg)  if (get_log_level() <= LOG_LEVEL::TRACE){\
                          std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->trace(buf.str().c_str());}
#define HKU_INFO(msg)  if (get_log_level() <= LOG_LEVEL::INFO) {\
                          std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->info(buf.str().c_str());}
#define HKU_WARN(msg)  if (get_log_level() <= LOG_LEVEL::WARN) {\
                          std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->warn(buf.str().c_str());}
#define HKU_ERROR(msg)  if (get_log_level() <= LOG_LEVEL::ERROR) {\
                          std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->error(buf.str().c_str());}
#define HKU_FATAL(msg)  if (get_log_level() <= LOG_LEVEL::FATAL) {\
                          std::stringstream buf (std::stringstream::out); \
                          buf << msg;\
                          spdlog::get("hikyuu")->critical(buf.str().c_str());}
#endif


/**********************************************
 * Use STDOUT for logging
 *
 *********************************************/
#ifdef USE_STDOUT_FOR_LOGGING
#define HKU_DEBUG(msg)  if (get_log_level() <= LOG_LEVEL::DEBUG) {\
                          std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [DEBUG] " << msg;\
                          std::cout << buf.str() << std::endl}
#define HKU_TRACE(msg)  if (get_log_level() <= LOG_LEVEL::TRACE) {\
                          std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [TRACE] " << msg;\
                          std::cout << buf.str() << std::endl;}
#define HKU_INFO(msg)   if (get_log_level() <= LOG_LEVEL::INFO) {\
                          std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [INFO] " << msg;\
                          std::cout << buf.str() << std::endl;}
#define HKU_WARN(msg)   if (get_log_level() <= LOG_LEVEL::WARN) {\
                          std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [WARN] " << msg;\
                          std::cout << buf.str() << std::endl;}
#define HKU_ERROR(msg)  if (get_log_level() <= LOG_LEVEL::ERROR) {\
                          std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [ERROR] " << msg;\
                          std::cout << buf.str() << std::endl;}
#define HKU_FATAL(msg)  if (get_log_level() <= LOG_LEVEL::FATAL) {\
                          std::stringstream buf (std::stringstream::out);\
                          buf << Datetime::now() << " [FATAL] " << msg;\
                          std::cout << buf.str() << std::endl;}
#endif /* for USE_STDOUT_FOR_LOGGING */

} /* namespace hku */
#endif /* LOG_H_ */
