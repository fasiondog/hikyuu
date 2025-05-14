/*
 * IExtdata.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#include "../Indicator.h"

namespace hku {

/**
 * 包装RPS成Indicator，用于其他指标计算    
 * @ingroup Indicator
 */

 
Indicator HKU_API EXTDATA(const string& targetfield ,const string&  file_path);
inline Indicator EXTDATA(const Indicator& ind,const string& targetfield,const string&  file_path) {
    return EXTDATA(targetfield,file_path)(ind);
}

}  // namespace hku
