/*
 * IniFile.h
 *
 *  Created on: 2010-5-19
 *      Author: fasiondog
 */

#pragma once
#ifndef INIPARSER_H_
#define INIPARSER_H_

#include "hikyuu/utilities/config.h"
#if !HKU_ENABLE_INI_PARSER
#error "Don't enable ini_parser, please config with --ini_parser=y"
#endif

#include <stdexcept>
#include <string>
#include <list>
#include <map>
#include <memory>

#if defined(_MSC_VER)
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)
#pragma warning(disable : 4290)
#endif

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

/**
 * 支持简单的ini格式文件的读取 \n
 * @details
 * 典型的ini文件格式如下, 其中";"为行注释符号：\n
 *     [section1] \n
 *     ;第一段 \n
 *     key1 = value1 \n
 *     key2 = value2 \n
 *     \n
 *     [section2] \n
 *     ;第二段 \n
 *     key1 = value1 ;注释1 \n
 *     key2 = value2 ;注释2 \n
 *
 * @note 同一个section可以在不同的位置定义，但如果不同位置的section中包含同名的option（key），
 *       则该option（key）的值为最后读入的值。这可能造成潜在的错误。建议不要将同一个section
 *       在不同的位置定义。\n
 *       对于配置信息分散在多个文件中的情况，可以通过多次调用read成员方法全部读入后，再统一处理。\n
 *       该类目前不支持复制操作，暂时没有此需求 \n
 *
 * @author fasiondog
 * @date 20100519
 * @ingroup Utilities
 */

class HKU_UTILS_API IniParser {
public:
    typedef std::list<std::string> StringList;
    typedef std::shared_ptr<std::list<std::string> > StringListPtr;

    IniParser(const IniParser&) = delete;
    IniParser& operator=(const IniParser&) = delete;

    IniParser();
    virtual ~IniParser();

    void read(const std::string& filename);
    void clear();

    bool hasSection(const std::string& section) const;
    bool hasOption(const std::string& section, const std::string& option) const;

    StringListPtr getSectionList() const;
    StringListPtr getOptionList(const std::string& section) const;

    std::string get(const std::string& section, const std::string& option,
                    const std::string& default_str = std::string()) const;

    // 以下默认值类型使用string的原因是因为int/float/double/bool类型没有空对象
    int getInt(const std::string& section, const std::string& option,
               const std::string& default_str = std::string()) const;

    float getFloat(const std::string& section, const std::string& option,
                   const std::string& default_str = std::string()) const;

    double getDouble(const std::string& section, const std::string& option,
                     const std::string& default_str = std::string()) const;

    bool getBool(const std::string& section, const std::string& option,
                 const std::string& default_str = std::string()) const;

private:
    typedef std::map<std::string, std::string> item_map_type;
    typedef std::map<std::string, item_map_type> section_map_type;
    section_map_type m_sections;
};

}  // namespace hku

#endif /* INIFILE_H_ */
