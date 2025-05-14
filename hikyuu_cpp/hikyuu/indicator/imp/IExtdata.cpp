/*
 * IExtdata.cpp
 *
 *  Created on: 2013-2-11
 *      Author: fasiondog
 */

#include "IExtdata.h"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <H5Cpp.h>
using namespace H5;
#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IExtdata)
#endif

namespace hku {

IExtdata::IExtdata() : IndicatorImp("EXTDATA", 1) {
    setParam<string>("targetField", "");
    setParam<string>("filepath", "");
}

// 自定义数据结构
template<typename T>
T getFieldValue(const char* pBuf, const H5::CompType& compType, const std::string& fieldName, size_t index, size_t point_size) {
    int numFields = compType.getNmembers();
    for (int i = 0; i < numFields; ++i) {
        std::string currentFieldName = compType.getMemberName(i);
        if (currentFieldName == fieldName) {
            size_t fieldOffset = compType.getMemberOffset(i);
            H5::DataType fieldType = compType.getMemberDataType(i);
            size_t fieldSize = fieldType.getSize();

            // 计算数据在 pBuf 中的位置
            const char* dataPtr = pBuf + index * point_size + fieldOffset;
            return *reinterpret_cast<const T*>(dataPtr);
        }
    }
    std::cerr << "未找到字段: " << fieldName << std::endl;
    return T();
}
void IExtdata::_calculate(const Indicator& ind) {
    KData kdata = ind.getContext();
    size_t total = kdata.size();
    if (total == 0) {
        kdata = getContext();
        total = kdata.size();
        if (total == 0) {
            return;
        }
    }
    _readyBuffer(total, 1);
    Stock stock = kdata.getStock();

    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    string targetField = getParam<string>("targetField");

    string FILE_NAME = getParam<string>("filepath");
    if (FILE_NAME.empty()) {
        std::cerr << "获取文件路径参数失败" << std::endl;
        return;
    }
    string tablename = stock.market_code();
    string GROUP_NAME = "data";

    try {
        H5::H5File h5file(FILE_NAME, H5F_ACC_RDONLY);
        H5::Group group = h5file.openGroup(GROUP_NAME);

        if (!H5Lexists(group.getId(), tablename.c_str(), H5P_DEFAULT)) {
            std::cerr << "数据集 " << tablename << " 不存在于组 " << GROUP_NAME << " 中。" << std::endl;
            return;
        }

        H5::DataSet dataset = group.openDataSet(tablename);
        H5::DataSpace dataspace = dataset.getSpace();

        size_t all_total = dataspace.getSelectNpoints();
        H5::DataType datatype = dataset.getDataType();

        // 获取每个数据点的大小（以字节为单位）
        size_t point_size = datatype.getSize();

        hsize_t offsets[1] = {0};
        hsize_t count[1] = {all_total};
        H5::DataSpace memspace(1, count);
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offsets);

        // 分配内存给 pBuf
        char* pBuf = new char[all_total * point_size];
        dataset.read(pBuf, datatype, memspace, dataspace);

        size_t x_total = all_total;
        size_t x_start = 0;
        auto* dst = this->data();
        // if (x_total < total) {
        //     dst = dst + total + m_discard - x_total;
        // } else if (x_total > total) {
        //     x_start = x_total - total;
        //     dst = dst - x_start;
        // }
        H5::CompType compType=CompType(dataset);
         for (size_t i = x_start; i < x_total; ++i) {
             float value = getFieldValue<float>(pBuf, compType, targetField, i, point_size);
            //  dst[i] = value;
         }

        // 读取RPS数据targetValues
        PriceList targetValues;
        for (size_t i = 0; i < all_total; ++i) {
            float value = getFieldValue<float>(pBuf, compType, targetField, i, point_size);
            targetValues.push_back(value);
            // dst[i] = value;
        }

        // 读取时间列数据dates
        DatetimeList dates;
        string timeField="datetime";
        for (size_t i = 0; i < all_total; ++i) {
            // 假设时间列数据类型为 Datetime
            Datetime date = Datetime(getFieldValue<unsigned long long>(pBuf, compType, timeField, i, point_size));
            dates.push_back(date);
        }
        //kdata时间数据k
        DatetimeList k = kdata.getDatetimeList();

    if (!k.empty() && !dates.empty()) {
        if (k[0].ymd() < dates[0].ymd()) {
            size_t dates_pos = 0;
            for (size_t i = 0; i < total; ++i) {
                if (k[i].ymd() < dates[dates_pos].ymd()) {
                    dst[i] = 0; // 假设默认值为 0
                } else if (k[i].ymd() == dates[dates_pos].ymd()) {
                    dst[i] = targetValues[dates_pos];
                    dates_pos++;
                }
            }
        } else {
            size_t k_pos = 0;
            for (size_t i = 0; i < all_total; ++i) {
                if (k[k_pos].ymd() < dates[i].ymd()) { 
                    continue;
                } else if (k[k_pos].ymd() == dates[i].ymd()&&k_pos<total) {
                    dst[k_pos] = targetValues[i];
                    k_pos++;
                }
            }
        }
    }
    

         delete[] pBuf;

        memspace.close();
        dataspace.close();
        dataset.close();
        group.close();
        h5file.close();

    } catch (const H5::Exception& e) {
        std::cerr << "HDF5 操作出错: " << e.getDetailMsg() << std::endl;
    }
}


Indicator HKU_API EXTDATA(const string& targetfield,const string& filepath ) {
    auto p = make_shared<IExtdata>();
    p->setParam<string>("targetField", targetfield);
    p->setParam<string>("filepath",filepath);
    return Indicator(p);
}
// size_t x_start = 0;
//         auto* k = kdata.data();
//         auto* dst = this->data();
//         if (all_total < total) { \\RPS数据小于全局，遍历kdata，更新START
//             for (size_t i = 0; i < total; ++i) {
//                 auto value = getFieldValue<float>(pBuf, compType, 'datetime', 0, point_size);  
//                 if (value == k[i].datetime) break;
//                 x_start++
//             }
//             dst =dst-x_start;
//             H5::CompType compType=CompType(dataset);
//             for (size_t i = 0; i < total; ++i) {
//                 float value = getFieldValue<float>(pBuf, compType, targetField, i, point_size);
//                 dst[i] = value;
//             }
//         } else if (all_total > total) {\\RPS数据大于全局，遍历RPS，更新START
//             for (size_t i = 0; i < total; ++i) {
//                 auto value = getFieldValue<float>(pBuf, compType, 'datetime', i, point_size);  
//                 if (value == k[0].datetime) break;\\TODO  转换为DATATIME或数字格式
//                 x_start++
//             }
//             H5::CompType compType=CompType(dataset);
//             for (size_t i = x_start; i < total; ++i) {
//                 float value = getFieldValue<float>(pBuf, compType, targetField, i, point_size);
//                 dst[i] = value;
//             }
//         }
//          auto* k = kdata.data();
    
//         size_t cur_kix = 0;
//         size_t pos = 0;

//         H5::CompType compType=CompType(dataset);
//          for (size_t i = 0; i < total; ++i) {
//              float value = getFieldValue<float>(pBuf, compType, targetField, i, point_size);
//              dst[i] = value;
//          }
} /* namespace hku */
