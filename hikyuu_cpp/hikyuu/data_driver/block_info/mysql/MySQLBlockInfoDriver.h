/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240102 added by fasiondog
 */

#pragma once

#include "../../BlockInfoDriver.h"

namespace hku {

class MySQLBlockInfoDriver : public BlockInfoDriver {
public:
    MySQLBlockInfoDriver() : BlockInfoDriver("mysql"){};
    virtual ~MySQLBlockInfoDriver();

    virtual bool _init() override;
    virtual Block getBlock(const string&, const string&) override;
    virtual BlockList getBlockList(const string& category) override;
    virtual BlockList getBlockList() override;
};

}  // namespace hku