/*
 * Context.h
 *
 *  Created on: 2019-03-09
 *      Author: fasiondog
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "Stock.h"
#include "utilities/Parameter.h"

namespace hku {


void HKU_API setGlobalContext(const Stock&, const KQuery&);

void HKU_API setGlobalContext(const KData&);

KData HKU_API getGlobalContextKData();

} /* namespace */

#endif /* CONTEXT_H_ */
