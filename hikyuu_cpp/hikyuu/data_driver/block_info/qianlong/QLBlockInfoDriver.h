/*
 * QLBlockInfoDriver.h
 *
 *  Created on: 2015年2月10日
 *      Author: fasiondog
 */

#pragma once
#ifndef DATA_DRIVER_BLOCK_INFO_QIANLONG_QLBLOCKINFODRIVER_H_
#define DATA_DRIVER_BLOCK_INFO_QIANLONG_QLBLOCKINFODRIVER_H_

#include "../../BlockInfoDriver.h"

namespace hku {

class QLBlockInfoDriver : public BlockInfoDriver {
public:
    QLBlockInfoDriver() : BlockInfoDriver("qianlong") {};
    virtual ~QLBlockInfoDriver();

    virtual bool _init() override;
    virtual Block getBlock(const string&, const string&) override;
    virtual BlockList getBlockList(const string& category) override;
    virtual BlockList getBlockList() override;
    virtual void save(const Block& block) override;
    virtual void remove(const string& category, const string& name) override;
};

} /* namespace hku */

#endif /* DATA_DRIVER_BLOCK_INFO_QIANLONG_QLBLOCKINFODRIVER_H_ */
