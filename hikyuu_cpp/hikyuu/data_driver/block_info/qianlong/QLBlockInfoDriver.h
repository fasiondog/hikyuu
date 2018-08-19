/*
 * QLBlockInfoDriver.h
 *
 *  Created on: 2015年2月10日
 *      Author: fasiondog
 */

#ifndef DATA_DRIVER_BLOCK_INFO_QIANLONG_QLBLOCKINFODRIVER_H_
#define DATA_DRIVER_BLOCK_INFO_QIANLONG_QLBLOCKINFODRIVER_H_

#include "../../BlockInfoDriver.h"

namespace hku {

class QLBlockInfoDriver: public BlockInfoDriver {
public:
    QLBlockInfoDriver(): BlockInfoDriver("qianlong") {};
    virtual ~QLBlockInfoDriver();

    virtual bool _init();
    virtual Block getBlock(const string&, const string&);
    virtual BlockList getBlockList(const string& category);
    virtual BlockList getBlockList();
};

} /* namespace hku */

#endif /* DATA_DRIVER_BLOCK_INFO_QIANLONG_QLBLOCKINFODRIVER_H_ */
