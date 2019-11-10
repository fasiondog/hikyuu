/*
 * AmaInstance.h
 *
 *  Created on: 2015年3月12日
 *      Author: fasiondog
 */

#pragma once
#ifndef AMAINSTANCE_H
#define AMAINSTANCE_H

#include "../../indicator/Indicator.h"
#include "../../Block.h"

namespace hku {

Indicator HKU_API AmaSpecial(const Block& block, KQuery query, Indicator ind);

} /* namespace hku*/

#endif /* AMAINSTANCE_H */
