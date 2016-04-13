/*
 * BlockInfoDriver.h
 *
 *  Created on: 2015年2月10日
 *      Author: fasiondog
 */

#ifndef DATA_DRIVER_BLOCKINFODRIVER_H_
#define DATA_DRIVER_BLOCKINFODRIVER_H_

#include <hikyuu_utils/iniparser/IniParser.h>
#include "../Block.h"

namespace hku {

class BlockInfoDriver {
public:
    BlockInfoDriver(const shared_ptr<IniParser>& config) : m_config(config) {};
    virtual ~BlockInfoDriver() {};

    /**
     * 获取指定的板块
     * @param category 指定的板块分类
     * @param name 板块名称
     * @return 指定的板块
     */
    virtual Block getBlock(const string& category, const string& name) = 0;

    /**
     * 获取指定分类的板块列表
     * @param category 板块分类
     * @return 板块列表
     */
    virtual BlockList getBlockList(const string& category) = 0;

    /**
     * 获取所有板块
     * @return 所有板块列表
     */
    virtual BlockList getBlockList() = 0;

protected:
    shared_ptr<IniParser> m_config;
};

typedef shared_ptr<BlockInfoDriver> BlockInfoDriverPtr;

} /* namespace hku */
#endif /* DATA_DRIVER_BLOCKINFODRIVER_H_ */
