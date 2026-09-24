/*
 * AF_EqualWeight.h
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_EQUALWEIGHT_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_EQUALWEIGHT_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief Equal weight asset allocation, it allocates the selected assets in equal proportions
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_EqualWeight();

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_EQUALWEIGHT_H_ */
