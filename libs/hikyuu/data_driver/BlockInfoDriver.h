/*
 * BlockInfoDriver.h
 *
 *  Created on: 2015年2月10日
 *      Author: fasiondog
 */

#ifndef DATA_DRIVER_BLOCKINFODRIVER_H_
#define DATA_DRIVER_BLOCKINFODRIVER_H_

#include "../utilities/Parameter.h"
#include "../Block.h"

namespace hku {

class HKU_API BlockInfoDriver {
    PARAMETER_SUPPORT

public:
    BlockInfoDriver(const string& name);
    virtual ~BlockInfoDriver() {};

    const string& name() const;

    /**
     * 驱动初始化
     * @param params
     * @return
     */
    bool init(const Parameter& params);

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

private:
    bool checkType();

protected:
    string m_name;
};

typedef shared_ptr<BlockInfoDriver> BlockInfoDriverPtr;


HKU_API std::ostream & operator<<(std::ostream&, const BlockInfoDriver&);
HKU_API std::ostream & operator<<(std::ostream&, const BlockInfoDriverPtr&);


inline const string& BlockInfoDriver::name() const {
    return m_name;
}

} /* namespace hku */
#endif /* DATA_DRIVER_BLOCKINFODRIVER_H_ */
