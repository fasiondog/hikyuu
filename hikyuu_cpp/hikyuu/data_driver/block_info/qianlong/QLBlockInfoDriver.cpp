/*
 * QLBlockInfoDriver.cpp
 *
 *  Created on: 2015年2月10日
 *      Author: fasiondog
 */

#include <fstream>
#include "hikyuu/utilities/arithmetic.h"
#include "QLBlockInfoDriver.h"

namespace hku {

QLBlockInfoDriver::~QLBlockInfoDriver() {}

bool QLBlockInfoDriver::_init() {
    return true;
}

Block QLBlockInfoDriver ::getBlock(const string& category, const string& name) {
    Block result(category, name);
    HKU_ERROR_IF_RETURN(!haveParam("dir"), result, "Missing 'dir' param!");
    HKU_INFO_IF_RETURN(!haveParam(category), result, "No such category ({})!", category);

    string filename;
    try {
        filename = getParam<string>("dir") + "/" + getParam<string>(category);
    } catch (...) {
        HKU_ERROR("Maybe parameters errors!");
        return result;
    }

    std::ifstream inifile(filename.c_str(), std::ifstream::in);
    HKU_ERROR_IF_RETURN(!inifile, result, "Can't open file({})!", filename);

    string line_str;
    string section, market, code;
    bool is_find = false;
    // string gb_name = utf8_to_gb(name);
    while (std::getline(inifile, line_str)) {
        trim(line_str);

        // 空行或注释行，跳过
        if (line_str.empty() || line_str.at(0) == ';') {
            continue;
        }

        // 检查第一个出现的注释符，并将其及其之后的字符清除
        size_t pos = line_str.find(';');
        if (pos != std::string::npos) {
            line_str.assign(line_str, 0, pos);
            trim(line_str);
        }

        // section行
        if (line_str.at(0) == '[') {
            if (is_find) {
                break;  // 跳出循环
            }
            size_t len = line_str.size();
            if (line_str[len - 1] != ']') {
                continue;
            }

            section.assign(line_str, 1, len - 2);
            trim(section);
            if (section.empty()) {
                continue;
            }

            if (section == name) {
                is_find = true;
            }
        }

        if (is_find) {
            pos = line_str.find(',');
            if (pos != std::string::npos) {
                market.assign(line_str, 0, pos);
                code.assign(line_str, pos + 1, line_str.size());
                trim(market);
                trim(code);
                if (market == "0") {
                    result.add("SH" + code);
                } else {
                    result.add("SZ" + code);
                }
            }
        }
    }

    inifile.close();
    return result;
}

BlockList QLBlockInfoDriver::getBlockList(const string& category) {
    BlockList result;
    HKU_ERROR_IF_RETURN(!haveParam("dir"), result, "Missing 'dir' param!");
    HKU_INFO_IF_RETURN(!haveParam(category), result, "No such category ({})!", category);

    string filename;
    try {
        filename = getParam<string>("dir") + "/" + getParam<string>(category);
    } catch (...) {
        HKU_ERROR("Maybe parameters errors!");
        return result;
    }

    std::ifstream inifile(filename.c_str(), std::ifstream::in);
    HKU_ERROR_IF_RETURN(!inifile, result, "Can't open file({})!", filename);

    std::string section;
    std::string key;
    std::string value;
    std::string line_str;
    Block block;
    while (std::getline(inifile, line_str)) {
        trim(line_str);

        // 空行或注释行，跳过
        if (line_str.empty() || line_str.at(0) == ';')
            continue;

        // 检查第一个出现的注释符，并将其及其之后的字符清除
        size_t pos = line_str.find(';');
        if (pos != std::string::npos) {
            line_str.assign(line_str, 0, pos);
            trim(line_str);
        }

        // section行
        if (line_str.at(0) == '[') {
            size_t len = line_str.size();
            if (line_str[len - 1] != ']')
                continue;

            section.assign(line_str, 1, len - 2);
            trim(section);
            if (section.empty())
                continue;

            block = Block(category, section);
            result.push_back(block);

        } else {
            if (section.empty())
                break;  // 缺少section定义，后续无须处理，直接跳出循环

            pos = line_str.find(',');
            if (pos == std::string::npos || pos == line_str.size() - 1)
                continue;

            key.assign(line_str, 0, pos);
            trim(key);
            if (key.empty())
                continue;

            value.assign(line_str, pos + 1, std::string::npos);
            trim(value);
            if (value.empty())
                continue;

            if (key == "0") {
                block.add("SH" + value);
            } else {
                block.add("SZ" + value);
            }
        }
    }

    inifile.close();
    return result;
}

BlockList QLBlockInfoDriver::getBlockList() {
    BlockList result;
    HKU_ERROR_IF_RETURN(!haveParam("dir"), result, "Missing 'dir' param!");
    StringList category_list = m_params.getNameList();
    for (auto iter = category_list.begin(); iter != category_list.end(); ++iter) {
        if (*iter == "dir" || *iter == "type")
            continue;

        BlockList tmp_blk_list = getBlockList(*iter);
        for (auto b_it = tmp_blk_list.begin(); b_it != tmp_blk_list.end(); ++b_it) {
            result.push_back(*b_it);
        }
    }

    return result;
}

} /* namespace hku */
