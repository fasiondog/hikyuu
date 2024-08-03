/*
 * IniFile.cpp
 *
 *  Created on: 2010-5-19
 *      Author: fasiondog
 */

#include <iostream>
#include <sstream>
#include <fstream>

#include "../arithmetic.h"
#include "IniParser.h"

namespace hku {

// 用于记录分析文件时的格式错误信息(行号、错误的行内容）
// 仅限IniParser内部使用
class ParsingError {
public:
    ParsingError() {
        m_haveError = false;
    }

    void append(size_t lineno, const std::string& line);

    bool haveError() {
        return m_haveError;
    }

    std::string str() {
        return m_info.str();
    }

private:
    std::stringstream m_info;
    bool m_haveError;
};

void ParsingError::append(size_t lineno, const std::string& line) {
    m_info << "\n\t[line " << lineno << "] " << line;
    m_haveError = true;
}

IniParser::IniParser() {}

IniParser::~IniParser() {}

/**
 * 读取并分析指定的ini文件
 * @details 可多次读取不同的ini文件后，再统一处理信息
 * @throw std::invalid_argument 当指定的文件无法打开时，抛出该异常
 * @throw std::logic_error 当文件格式错误时，抛出该异常
 * @param filename 指定文件名
 */
void IniParser::read(const std::string& filename) {
    std::ifstream inifile(HKU_PATH(filename), std::ifstream::in);
    if (!inifile) {
        throw(std::invalid_argument("Can't read file(" + filename + ")!"));
    }

    size_t line_no = 0;          // 当前行号，用于记录错误信息
    ParsingError parsing_error;  // 记录格式分析错误

    std::string section;
    std::string key;
    std::string value;
    std::string line_str;

    while (std::getline(inifile, line_str)) {
        line_no++;
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
            size_t len = line_str.size();
            if (line_str[len - 1] != ']') {
                parsing_error.append(line_no, line_str);
                continue;
            }

            section.assign(line_str, 1, len - 2);
            trim(section);
            if (section.empty()) {
                parsing_error.append(line_no, line_str);
                continue;
            }

            m_sections[section];

        } else {
            if (section.empty()) {
                parsing_error.append(line_no, "Missing section header!");
                break;  // 缺少section定义，后续无须处理，直接跳出循环
            }

            pos = line_str.find('=');
            if (pos == std::string::npos || pos == line_str.size() - 1) {
                parsing_error.append(line_no, line_str);
                continue;
            }

            key.assign(line_str, 0, pos);
            trim(key);
            if (key.empty()) {
                parsing_error.append(line_no, line_str);
                continue;
            }

            value.assign(line_str, pos + 1, std::string::npos);
            trim(value);
            if (value.empty()) {
                parsing_error.append(line_no, line_str);
                continue;
            }

            m_sections[section][key] = value;
        }
    }

    if (parsing_error.haveError()) {
        inifile.close();
        throw(std::logic_error(parsing_error.str()));
    }

    inifile.close();
}

/**
 * 清除现有已读入的信息
 */
void IniParser::clear() {
    m_sections.clear();
}

/**
 * 判断指定的section是否存在
 */
bool IniParser::hasSection(const std::string& section) const {
    return m_sections.count(section) ? true : false;
}

/**
 * 判断指定option是否存在
 */
bool IniParser::hasOption(const std::string& section, const std::string& option) const {
    if (m_sections.count(section) == 0) {
        return false;
    }

    // if(m_sections[section].count(option) == 0) {
    if (m_sections.find(section)->second.count(option) == 0) {
        return false;
    }

    return true;
}

/**
 * 获取所有的secton
 * @return 所有的section列表
 */
IniParser::StringListPtr IniParser::getSectionList() const {
    StringListPtr result = std::make_shared<std::list<std::string>>();
    section_map_type::const_iterator iter = m_sections.begin();
    for (; iter != m_sections.end(); ++iter) {
        result->push_back(iter->first);
    }
    return result;
}

/**
 * 获取指定Section下的所有option
 * @throw 若指定的section不存在，将抛出std::invalid_argument异常
 * @param section 指定的section
 * @return 指定的section下所有option列表
 */
IniParser::StringListPtr IniParser::getOptionList(const std::string& section) const {
    if (m_sections.count(section) == 0) {
        throw(std::invalid_argument("No section: " + section));
    }

    StringListPtr result = std::make_shared<std::list<std::string>>();
    item_map_type option_map = m_sections.find(section)->second;
    item_map_type::const_iterator iter = option_map.begin();
    for (; iter != option_map.end(); ++iter) {
        result->push_back(iter->first);
    }

    return result;
}

/**
 * 获取指定的option值
 * @throw std::invalid_argument 当指定option不存在对应值，并且没有指定缺省值时，抛出该异常
 * @param section 指定的section
 * @param option 指定的option
 * @param default_str
 * 缺省值，在不存在对应的option值时，返回该值。当该值为空时，表示没有缺省值，此时如果不存在对应option值，
 *                    将抛出std::invalid_argument异常。默认为空，没有指定缺省值。
 */
std::string IniParser::get(const std::string& section, const std::string& option,
                           const std::string& default_str) const {
    std::string result;
    if (m_sections.count(section) == 0) {
        throw(std::invalid_argument("No section: " + section));
    }

    if (m_sections.find(section)->second.count(option) == 0) {
        if (default_str.empty()) {
            throw(std::invalid_argument("No option(" + option + ") in section(" + section + ")"));
        } else {
            result.assign(default_str);
            trim(result);
        }
    } else {
        result.assign(m_sections.find(section)->second.find(option)->second);
    }

    return result;
}

/**
 * 获取指定的option值，并将其转换为int类型
 * @throw std::invalid_argument 当指定option不存在对应值，并且没有指定缺省值时，抛出该异常;
 *        或者当指定的缺省值（非空）无法转换为int类型时，无论指定的option值是否存在都将抛出该异常。
 * @throw std::domain_error 当指定的option值，无法转换为int类型时，抛出该异常
 * @param section 指定的section
 * @param option 指定的option
 * @param default_str 缺省值。在不存在对应的option值时，返回该值。当该值为空时，表示没有缺省值。
 *                    默认为空，没有指定缺省值。
 */
int IniParser::getInt(const std::string& section, const std::string& option,
                      const std::string& default_str) const {
    int result = 0;
    size_t remain = 0;

    // 先检查default_str是否可以转换为int
    if (!default_str.empty()) {
        result = std::stoi(default_str, &remain);
        if (remain != default_str.size()) {
            throw(std::invalid_argument("Invalid default value: " + default_str));
        }
    }

    std::string value_str = get(section, option, default_str);
    remain = 0;
    result = std::stoi(value_str, &remain);  // cppcheck-suppress redundantAssignment
    if (remain != value_str.size()) {
        throw(std::invalid_argument("This option cannot be converted to an integer! " + value_str));
    }

    return result;
}

/**
 * 获取指定的option值，并将其转换为float类型
 * @throw std::invalid_argument 当指定option不存在对应值，并且没有指定缺省值时，抛出该异常;
 *        或者当指定的缺省值（非空）无法转换为float类型时，无论指定的option值是否存在都将抛出该异常。
 * @throw std::domain_error 当指定的option值，无法转换为float类型时，抛出该异常
 * @param section 指定的section
 * @param option 指定的option
 * @param default_str 缺省值。在不存在对应的option值时，返回该值。当该值为空时，表示没有缺省值。
 *                    默认为空，没有指定缺省值。
 */
float IniParser::getFloat(const std::string& section, const std::string& option,
                          const std::string& default_str) const {
    float result;
    size_t remain = 0;

    // 先检查default_str是否可以转换为float
    if (!default_str.empty()) {
        result = std::stof(default_str, &remain);
        if (remain != default_str.size()) {
            throw(std::invalid_argument("Invalid default value: " + default_str));
        }
    }

    std::string value_str = get(section, option, default_str);
    remain = 0;
    result = std::stof(value_str, &remain);  // cppcheck-suppress redundantAssignment
    if (remain != value_str.size()) {
        throw(std::invalid_argument("This option cannot be converted to an float! " + value_str));
    }

    return result;
}

/**
 * 获取指定的option值，并将其转换为double类型
 * @throw std::invalid_argument 当指定option不存在对应值，并且没有指定缺省值时，抛出该异常;
 *        或者当指定的缺省值（非空）无法转换为double类型时，无论指定的option值是否存在都将抛出该异常。
 * @throw std::domain_error 当指定的option值，无法转换为double类型时，抛出该异常
 * @param section 指定的section
 * @param option 指定的option
 * @param default_str 缺省值。在不存在对应的option值时，返回该值。当该值为空时，表示没有缺省值。
 *                    默认为空，没有指定缺省值。
 */
double IniParser::getDouble(const std::string& section, const std::string& option,
                            const std::string& default_str) const {
    double result;
    size_t remain = 0;

    // 先检查default_str是否可以转换为float
    if (!default_str.empty()) {
        result = std::stod(default_str, &remain);
        if (remain != default_str.size()) {
            throw(std::invalid_argument("Invalid default value: " + default_str));
        }
    }

    std::string value_str = get(section, option, default_str);
    remain = 0;
    result = std::stod(value_str, &remain);  // cppcheck-suppress redundantAssignment
    if (remain != value_str.size()) {
        throw(std::invalid_argument("This option cannot be converted to an double! " + value_str));
    }

    return result;
}

/**
 * 获取指定的option值，并将其转换为bool类型
 * @details option表示为true时，可接受的值为：1|true|yes|on （不区分大小写） \n
 *          option表示为false时，可接受的值为：0|false|no|off （不区分大小写）
 * @throw std::invalid_argument 当指定option不存在对应值，并且没有指定缺省值时，抛出该异常;
 *        或者当指定的缺省值（非空）无法转换为bool类型时，无论指定的option值是否存在都将抛出该异常。
 * @throw std::domain_error 当指定的option值，无法转换为bool类型时，抛出该异常
 * @param section 指定的section
 * @param option 指定的option
 * @param default_str 缺省值。在不存在对应的option值时，返回该值。当该值为空时，表示没有缺省值。
 *                    默认为空，没有指定缺省值。
 */
bool IniParser::getBool(const std::string& section, const std::string& option,
                        const std::string& default_str) const {
    // 先检查default_str是否可以转换为bool
    std::string new_default_str(default_str);
    if (!default_str.empty()) {
        if (new_default_str != "1" && new_default_str != "0") {
            to_upper(new_default_str);
            if (new_default_str == "TRUE" || new_default_str == "YES" || new_default_str == "ON") {
                new_default_str.assign("1");
            } else if (new_default_str == "FALSE" || new_default_str == "NO" ||
                       new_default_str == "OFF") {
                new_default_str.assign("0");
            } else {
                throw(std::invalid_argument("Invalid default value: " + default_str));
            }
        }
    }

    std::string value_str = get(section, option, new_default_str);
    if (value_str == "1") {
        return true;
    }

    if (value_str == "0") {
        return false;
    }

    to_upper(value_str);
    if (value_str == "TRUE" || value_str == "YES" || value_str == "ON") {
        return true;
    }

    if (value_str == "FALSE" || value_str == "NO" || value_str == "OFF") {
        return false;
    }

    throw(std::domain_error(value_str + " can not be translated to bool!"));
}

}  // namespace hku
