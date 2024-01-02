/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240102 added by fasiondog
 */

#include "MySQLBlockInfoDriver.h"

namespace hku {

MySQLBlockInfoDriver::~MySQLBlockInfoDriver() {}

bool MySQLBlockInfoDriver::_init() {
    return true;
}

Block MySQLBlockInfoDriver::getBlock(const string&, const string&) {
    Block ret;
    return ret;
}

BlockList MySQLBlockInfoDriver::getBlockList(const string& category) {
    BlockList ret;
    return ret;
}

BlockList MySQLBlockInfoDriver::getBlockList() {
    BlockList ret;
    return ret;
}

}  // namespace hku