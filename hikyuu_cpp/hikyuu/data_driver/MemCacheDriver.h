/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-11
 *     Author: fasiondog
 */

#pragma once

#include "../utilities/Parameter.h"

namespace hku {

/**
 * 内存缓存驱动
 * @ingroup DataDriver
 */
class HKU_API MemCacheDriver {
    PARAMETER_SUPPORT

public:
    /**
     * 构造函数
     * @param name 驱动名称
     */
    MemCacheDriver(const string& name);

    /** 析构函数 */
    virtual ~MemCacheDriver();

    /**
     * 获取驱动名称
     */
    const string& name() const {
        return m_name;
    }

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

private:
    bool checkType();

private:
    string m_name;
};

typedef shared_ptr<MemCacheDriver> MemCacheDriverPtr;

}  // namespace hku