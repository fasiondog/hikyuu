/*
 * config.h
 *
 *  Created on: 2013-4-28
 *      Author: fasiondog
 */

#ifndef CONFIG_H_
#define CONFIG_H_

//support serialization
#define HKU_SUPPORT_SERIALIZATION 1

#if HKU_SUPPORT_SERIALIZATION
#define HKU_SUPPORT_TEXT_ARCHIVE 0
#define HKU_SUPPORT_XML_ARCHIVE 1
#define HKU_SUPPORT_BINARY_ARCHIVE 1
#endif /* HKU_SUPPORT_SERIALIZATION*/

//检查下标越界
#define CHECK_ACCESS_BOUND 1

#endif /* CONFIG_H_ */
