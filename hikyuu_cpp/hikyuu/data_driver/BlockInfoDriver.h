/*
 * BlockInfoDriver.h
 *
 *  Created on: 2015年2月10日
 *      Author: fasiondog
 */

#pragma once
#ifndef DATA_DRIVER_BLOCKINFODRIVER_H_
#define DATA_DRIVER_BLOCKINFODRIVER_H_

#include "../utilities/Parameter.h"
#include "../Block.h"

namespace hku {

/**
 * 板块数据驱动
 * @ingroup DataDriver
 */
class HKU_API BlockInfoDriver {
    PARAMETER_SUPPORT

public:
    BlockInfoDriver(const string& name);
    virtual ~BlockInfoDriver() {};

    /** 获取驱动名称 */
    const string& name() const;

    /**
     * 驱动初始化
     * @param params
     * @return
     */
    bool init(const Parameter& params);

    /**
     * 获取指定证券所属的板块列表
     * @param stk 指定证券
     * @param category 板块分类，如果为空字符串，返回所有板块分类下的所属板块
     * @return BlockList
     */
    BlockList getStockBelongs(const Stock& stk, const string& category);

    /**
     * 子类如果需要缓存，可实现该方法将数据加载至自身的缓存
     */
    virtual void load() {}

    /**
     * 驱动初始化，具体实现时应注意将之前打开的相关资源关闭。
     */
    virtual bool _init() = 0;

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

    /**
     * 保存指定的板块
     * @note 如果已存在同名板块，则覆盖；如果板块分类或名称存在修改，需要手工在修改前删除原板块
     * @param block
     */
    virtual void save(const Block& block) = 0;

    /**
     * 删除指定的板块
     * @param category 板块分类
     * @param name 板块名称
     */
    virtual void remove(const string& category, const string& name) = 0;

private:
    bool checkType();

protected:
    string m_name;
};

typedef shared_ptr<BlockInfoDriver> BlockInfoDriverPtr;

HKU_API std::ostream& operator<<(std::ostream&, const BlockInfoDriver&);
HKU_API std::ostream& operator<<(std::ostream&, const BlockInfoDriverPtr&);

inline const string& BlockInfoDriver::name() const {
    return m_name;
}

} /* namespace hku */
#endif /* DATA_DRIVER_BLOCKINFODRIVER_H_ */
