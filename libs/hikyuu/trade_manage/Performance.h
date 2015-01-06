/*
 * Performance.h
 *
 *  Created on: 2013-4-23
 *      Author: fasiondog
 */

#ifndef PERFORMANCE_H_
#define PERFORMANCE_H_

#include "TradeManager.h"

namespace hku {

/*
 * 简单绩效统计
 */
class HKU_API Performance {
public:
    Performance();
    virtual ~Performance();

    void reset();

    void report(const TradeManagerPtr& tm,
            const Datetime& datetime = Datetime::now());

    /*
     * 根据交易记录，统计截至某一日期的系统绩效, datetime必须大于等于lastDatetime，
     * 以便用于计算当前市值
     */
    void statistics(const TradeManagerPtr& tm, const Datetime& datetime);


    typedef map<string, double> map_type;

private:
    list<string> m_name_list; //保存指标顺序
    map_type m_result;

};

} /* namespace hku */
#endif /* PERFORMANCE_H_ */
