/*
 * KDataBufferImp.cpp
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
#include "KDataBufferImp.h"
#include "utilities/util.h"

namespace hku {

KDataBufferImp::KDataBufferImp(): KDataImp() {

}


KDataBufferImp::
KDataBufferImp(const Stock& stock, const KQuery& query)
: KDataImp(stock, query) {
    if (m_stock.isNull() || empty()) {
        return;
    }

    m_buffer = m_stock.getKRecordList(startPos(), endPos(), query.kType());

    //日线以上不支持复权
    if (query.recoverType() == KQuery::NO_RECOVER
            || query.kType() == KQuery::WEEK
            || query.kType() == KQuery::MONTH
            || query.kType() == KQuery::QUARTER
            || query.kType() == KQuery::HALFYEAR
            || query.kType() == KQuery::YEAR) {
        return;
    }

    switch(query.recoverType()) {
    case KQuery::NO_RECOVER:
        //do nothing
        break;

    case KQuery::FORWARD:
        _recoverForward();
        break;

    case KQuery::BACKWARD:
        _recoverBackward();
        break;

    case KQuery::EQUAL_FORWARD:
        _recoverEqualForward();
        break;

    case KQuery::EQUAL_BACKWARD:
        _recoverEqualBackward();
        break;

    default:
        HKU_ERROR("Invalid RecvoerType! [KDataBufferImp::KDataBufferImp]");
        return;
    }
}


KDataBufferImp::~KDataBufferImp() {

}


size_t KDataBufferImp::getPos(const Datetime& datetime) const {
    KRecordList::const_iterator iter;
    KRecord comp_record;
    comp_record.datetime = datetime;
    boost::function<bool(const KRecord&, const KRecord&)> f =
            boost::bind(&KRecord::datetime, _1) <
            boost::bind(&KRecord::datetime, _2);

    iter = lower_bound(m_buffer.begin(), m_buffer.end(), comp_record, f);
    if (iter == m_buffer.end() || iter->datetime != datetime) {
        return Null<size_t>();
    }

    return (iter - m_buffer.begin());
}


/******************************************************************************
 * 前复权公式:复权后价格＝[(复权前价格-现金红利)＋配(新)股价格×流通股份变动比例]÷(1＋流通股份变动比例)
 * 向前复权指以除权后的股价为基准（即除权后的股价不变），将除权前的股价降下来。
 * 复权计算时首先从上市日开始，逐日向后判断，遇到除权日，则将上市日到除权日之间（不包括除权日）的
 * 全部股价通过复权计算降下来；然后再继续向后判断，遇到下一个除权日，则再次将上市日到该除权日之间
 * （不包括除权日）的全部股价通过复权计算降下来。
 *****************************************************************************/
void KDataBufferImp::_recoverForward() {
    size_t total = m_buffer.size();
    if (total == 0) {
        return;
    }

    bd::date start_date = m_buffer.front().datetime.date();
    bd::date end_date = m_buffer.back().datetime.date() + bd::days(1);
    StockWeightList weightList = m_stock.getWeight(start_date, end_date);
    StockWeightList::const_iterator weightIter = weightList.begin();
    StockWeightList::const_iterator pre_weightIter = weightIter;

    size_t pre_pos = 0;
    for (; weightIter != weightList.end(); ++weightIter) {
        size_t i = pre_pos;
        while (i < total && m_buffer[i].datetime < weightIter->datetime()) {
            i++;
        }
        pre_pos = i;  //除权日

        //计算流通股份变动比例,但不处理仅仅只有流通股本改变的情况
        price_t change = 0.0;
        bool flag = false;
        if (weightIter != weightList.begin()
			&& !(weightIter->countAsGift() == 0.0
			&& weightIter->countForSell() == 0.0
			&& weightIter->priceForSell() == 0.0))
			//&& weightIter->bonus() == 0.0 
			//&& weightIter->increasement() == 0.0 )) 
		{
            pre_weightIter = weightIter - 1;
            if (pre_weightIter->freeCount() != 0.0) {
                change = (weightIter->freeCount()
                       - pre_weightIter->freeCount())
                       / pre_weightIter->freeCount();
                flag = true;
            }
        }
        if (!flag){
            change = 0.1 * (weightIter->countAsGift()
                           + weightIter->countForSell()
                           + weightIter->increasement());
        }

        price_t denominator = 1.0 + change; //分母 = (1+流通股份变动比例)
        price_t temp = weightIter->priceForSell() * change
                     - 0.1 * weightIter->bonus();

        for (i = 0; i < pre_pos; ++i) {
            m_buffer[i].openPrice = roundEx((m_buffer[i].openPrice + temp) / denominator, m_stock.precision());
            m_buffer[i].highPrice = roundEx((m_buffer[i].highPrice + temp) / denominator, m_stock.precision());
            m_buffer[i].lowPrice = roundEx((m_buffer[i].lowPrice + temp) / denominator, m_stock.precision());
            m_buffer[i].closePrice = roundEx((m_buffer[i].closePrice + temp) / denominator, m_stock.precision());
        }
    }
}

/******************************************************************************
 * 后复权公式:复权后价格＝复权前价格×(1＋流通股份变动比例)-配(新)股价格×流通股份变动比例＋现金红利
 * 向后复权指以除权前的股价为基准（即除权前的股价不变），将除权后的股价升上去。复权计算时首先从最新日开始，
 * 逐日向前判断，遇到除权日，则将除权日到最新日之间（包括除权日）的全部股价通过复权计算升上去；然后再继续
 * 向前判断，遇到下一个除权日，则再次将除权日到最新日之间（包括除权日）的全部股价通过复权计算升上去。
 *****************************************************************************/
void KDataBufferImp::_recoverBackward() {
    size_t total = m_buffer.size();
    if (0 == total) {
        return;
    }

    bd::date start_date = m_buffer.front().datetime.date();
    bd::date end_date = m_buffer.back().datetime.date() + bd::days(1);
    StockWeightList weightList = m_stock.getWeight(start_date, end_date);
    StockWeightList::const_reverse_iterator weightIter = weightList.rbegin();
    StockWeightList::const_reverse_iterator pre_weightIter;

    size_t pre_pos = total - 1;
    for (; weightIter != weightList.rend(); ++weightIter) {
        size_t i = pre_pos;
        while (i > 0 && m_buffer[i].datetime > weightIter->datetime()) {
            i--;
        }
        pre_pos = i;

        //流通股份变动比例
        price_t change = 0.0;
        bool flag = false;
        pre_weightIter = weightIter + 1;
        if (pre_weightIter != weightList.rend()
                && pre_weightIter->freeCount() != 0.0
				&& 	!(weightIter->countAsGift() == 0.0
				&&	weightIter->countForSell() == 0.0
				&&	weightIter->priceForSell() == 0.0))
				//&&	weightIter->bonus() == 0.0 
				//&&	weightIter->increasement() == 0.0 )) 
		{
            change = (weightIter->freeCount() - pre_weightIter->freeCount())
                    / pre_weightIter->freeCount();
            flag = true;
        }
        if (!flag){
            change = 0.1 * (weightIter->countAsGift()
                           + weightIter->countForSell()
                           + weightIter->increasement());
        }

        price_t denominator = 1.0 + change; //(1+流通股份变动比例)
        price_t temp = 0.1 * weightIter->bonus()
                     - weightIter->priceForSell() * change;;

        for (i = pre_pos; i < total; ++i) {
            m_buffer[i].openPrice = roundEx(m_buffer[i].openPrice * denominator + temp, m_stock.precision());
            m_buffer[i].highPrice = roundEx(m_buffer[i].highPrice * denominator + temp, m_stock.precision());
            m_buffer[i].lowPrice = roundEx(m_buffer[i].lowPrice * denominator + temp, m_stock.precision());
            m_buffer[i].closePrice = roundEx(m_buffer[i].closePrice * denominator + temp, m_stock.precision());
        }
    }
}

/******************************************************************************
 * 等比前复权公式:复权后价格＝复权前价格*复权率
 * 复权率＝｛[(股权登记日收盘价-现金红利)＋配(新)股价格×流通股份变动比例]÷(1＋流通股份变动比例)｝÷股权登记日收盘价
 * 向前复权指以除权后的股价为基准（即除权后的股价不变），将除权前的股价降下来。
 * 复权计算时首先从上市日开始，逐日向后判断，遇到除权日，则将上市日到除权日之间（不包括除权日）的
 * 全部股价通过复权计算降下来；然后再继续向后判断，遇到下一个除权日，则再次将上市日到该除权日之间
 * （不包括除权日）的全部股价通过复权计算降下来。
 *****************************************************************************/
void KDataBufferImp::_recoverEqualForward() {
    size_t total = m_buffer.size();
    if (0 == total) {
        return;
    }

    bd::date start_date = m_buffer.front().datetime.date();
    bd::date end_date = m_buffer.back().datetime.date() + bd::days(1);
    StockWeightList weightList = m_stock.getWeight(start_date, end_date);
    if (weightList.empty()) {
        return;
    }

    KRecordList kdata = m_buffer; //防止同一天两条权息记录
    StockWeightList::const_iterator weightIter = weightList.begin();
    StockWeightList::const_iterator pre_weightIter;
    size_t pre_pos = 0;
    for (; weightIter != weightList.end(); ++weightIter) {
        size_t i = pre_pos;
        while (i < total && m_buffer[i].datetime < weightIter->datetime()) {
            i++;
        }
        pre_pos = i; //除权日

        //股权登记日（即除权日的前一天数据）收盘价
        if (pre_pos == 0) {
            continue;
        }
        price_t closePrice = kdata[pre_pos - 1].closePrice;
        if (closePrice == 0.0) {
            continue;  //除零保护
        }

        //流通股份变动比例
        price_t change = 0.0;
        bool flag = false;
        if (weightIter != weightList.begin()
			&& 	!(weightIter->countAsGift() == 0.0
			&&	weightIter->countForSell() == 0.0
			&&	weightIter->priceForSell() == 0.0))
			//&&	weightIter->bonus() == 0.0 
			//&&	weightIter->increasement() == 0.0 )) 
		{
            pre_weightIter = weightIter - 1;
            if (pre_weightIter->freeCount() != 0.0) {
                change = (weightIter->freeCount() - pre_weightIter->freeCount())
                        / pre_weightIter->freeCount();
                flag = true;
            }
        }
        if (!flag) {
            change = 0.1 * (weightIter->countAsGift()
                            + weightIter->countForSell()
                            + weightIter->increasement());
        }

        price_t denominator = 1.0 + change; //(1+流通股份变动比例)
        price_t temp = weightIter->priceForSell() * change
                     -  0.1 * weightIter->bonus();

        price_t k = (closePrice + temp) / (denominator * closePrice);

        for (i = 0; i < pre_pos; ++i) {
            m_buffer[i].openPrice = roundEx(k * m_buffer[i].openPrice, m_stock.precision());
            m_buffer[i].highPrice = roundEx(k * m_buffer[i].highPrice, m_stock.precision());
            m_buffer[i].lowPrice = roundEx(k * m_buffer[i].lowPrice, m_stock.precision());
            m_buffer[i].closePrice = roundEx(k * m_buffer[i].closePrice, m_stock.precision());
        }
    }
}

/******************************************************************************
 * 等比后复权公式:复权后价格＝复权前价格÷复权率
 * 复权率＝｛[(股权登记日收盘价-现金红利)＋配(新)股价格×流通股份变动比例]÷(1＋流通股份变动比例)｝÷股权登记日收盘价
 * 向后复权指以除权前的股价为基准（即除权前的股价不变），将除权后的股价升上去。复权计算时首先从最新日开始，
 * 逐日向前判断，遇到除权日，则将除权日到最新日之间（包括除权日）的全部股价通过复权计算升上去；然后再继续
 * 向前判断，遇到下一个除权日，则再次将除权日到最新日之间（包括除权日）的全部股价通过复权计算升上去。
 *****************************************************************************/
void KDataBufferImp::_recoverEqualBackward() {
    size_t total = m_buffer.size();
    if (0 == total) {
        return;
    }

    bd::date start_date = m_buffer.front().datetime.date();
    bd::date end_date = m_buffer.back().datetime.date() + bd::days(1);
    StockWeightList weightList = m_stock.getWeight(start_date, end_date);
    StockWeightList::const_reverse_iterator weightIter = weightList.rbegin();
    StockWeightList::const_reverse_iterator pre_weightIter;

    size_t pre_pos = total - 1;
    for (; weightIter != weightList.rend(); ++weightIter) {
        size_t i = pre_pos;
        while (i > 0 && m_buffer[i].datetime > weightIter->datetime()) {
            i--;
        }
        pre_pos = i; //除权日

        //股权登记日（即除权日的前一天数据）收盘价
        if (pre_pos == 0){
            continue;
        }
        price_t closePrice = m_buffer[pre_pos - 1].closePrice;

        //流通股份变动比例
        price_t change = 0.0;
        bool flag = false;
        pre_weightIter = weightIter + 1;
        if (pre_weightIter != weightList.rend()
                && pre_weightIter->freeCount() != 0.0
				&& 	!(weightIter->countAsGift() == 0.0
				&&	weightIter->countForSell() == 0.0
				&&	weightIter->priceForSell() == 0.0)) 
				//&&	weightIter->bonus() == 0.0 
				//&&	weightIter->increasement() == 0.0 )) 
		{
            change = (weightIter->freeCount() - pre_weightIter->freeCount())
                    / pre_weightIter->freeCount();
            flag = true;
        }
        if (!flag) {
            change = 0.1 * (weightIter->countAsGift()
                           + weightIter->countForSell()
                           + weightIter->increasement());
        }

        price_t denominator = 1.0 + change; //(1+流通股份变动比例)
        price_t temp = closePrice + weightIter->priceForSell() * change
                     - 0.1 * weightIter->bonus();
        if (temp == 0.0) {
            continue;
        }
        price_t k =  (denominator * closePrice) / temp;

        for (i = pre_pos; i < total; ++i) {
            m_buffer[i].openPrice = roundEx(k * m_buffer[i].openPrice, m_stock.precision());
            m_buffer[i].highPrice = roundEx(k * m_buffer[i].highPrice, m_stock.precision());
            m_buffer[i].lowPrice =  roundEx(k * m_buffer[i].lowPrice, m_stock.precision());
            m_buffer[i].closePrice = roundEx(k * m_buffer[i].closePrice, m_stock.precision());
        }
    }
}


} /* namespace hku */
