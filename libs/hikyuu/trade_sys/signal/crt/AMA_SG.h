/*
 * AMA_SG.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#ifndef AMA_SG_H_
#define AMA_SG_H_

#include "../SignalBase.h"

namespace hku {

/**
 * 佩里.J 考夫曼（Perry J.Kaufman）自适应移动平均，参见《精明交易者》（2006年 广东经济出版社）
 * @param n 周期窗口
 * @param fast_n 快速损耗周期
 * @param slow_n 慢速损耗周期
 * @param filter_n 拐点判定样本标准差窗口
 * @param filter_p 拐点判定系数
 * @param kpart 取KDATA的哪个部分
 * @ingroup Signal
 */
SignalPtr HKU_API AMA_SG(int n = 10, int fast_n = 2, int slow_n = 30,
        int filter_n = 20, double filter_p = 0.1,
        const string& kpart = "CLOSE");

} /* namespace */

#endif /* AMA_SG_H_ */
