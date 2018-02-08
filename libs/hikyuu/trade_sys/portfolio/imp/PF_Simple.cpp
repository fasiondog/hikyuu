/*
 * PF_Simple.cpp
 *
 *  Created on: 2018年1月13日
 *      Author: fasiondog
 */

#include "../crt/PF_Simple.h"

namespace hku {

PortfolioPtr HKU_API PF_Simple(
        const TMPtr& tm,
        const SYSPtr& sys,
        const SEPtr& st,
        const AFPtr& af) {
    return make_shared<Portfolio>(tm, sys, st, af);
}

} /* namespace hku */


