/*
 * Log.h
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#ifndef LOG_H_
#define LOG_H_

#include <string>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

void HKU_API init_logger(const std::string& configue_name);
log4cplus::Logger HKU_API getLogger();

typedef log4cplus::Logger Logger;

#define HKU_DEBUG(msg)  LOG4CPLUS_DEBUG(getLogger(), msg)
#define HKU_TRACE(msg)  LOG4CPLUS_TRACE(getLogger(), msg)
#define HKU_INFO(msg)   LOG4CPLUS_INFO(getLogger(), msg)
#define HKU_WARN(msg)   LOG4CPLUS_WARN(getLogger(), msg)
#define HKU_ERROR(msg)  LOG4CPLUS_ERROR(getLogger(), msg)
#define HKU_FATAL(msg)  LOG4CPLUS_FATAL(getLogger(), msg)

#define HKU_DEBUG_LOG LOG4CPLUS_DEBUG
#define HKU_TRACE_LOG  LOG4CPLUS_TRACE
#define HKU_INFO_LOG  LOG4CPLUS_INFO
#define HKU_WARN_LOG  LOG4CPLUS_WARN
#define HKU_ERROR_LOG LOG4CPLUS_ERROR
#define HKU_FATAL_LOG LOG4CPLUS_FATAL

#define HKU_CLASS_LOG(classname) log4cplus::Logger getLogger() const { \
           return log4cplus::Logger::getInstance(classname); }

} /* namespace hku */
#endif /* LOG_H_ */
