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
 * It supports the reading of a simple ini format file \n
 * @details
 * A typical ini file format is as follows, where ";" is the line comment symbol: \n
 *     [section1] \n
 *     ;The first section \n
 *     key1 = value1 \n
 *     key2 = value2 \n
 *     \n
 *     [section2] \n
 *     ;The second section \n
 *     key1 = value1 ;comment 1 \n
 *     key2 = value2 ;comment 2 \n
 *
 * @note The same section can be defined in different positions, but if the sections at different
 *       positions contain an option (key) with the same name,
 *       the value of that option (key) is the last read value. This may cause a potential error. It
 * is recommended not to define the same section in different positions. \n When the configuration
 * information is scattered in multiple files, all of them can be read in by calling the read member
 * method multiple times and then processed uniformly. \n This class does not support the copy
 * operation at present, there is no such requirement for now \n
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

    // The reason why the following default value type uses string is that the int/float/double/bool
    // types have no empty object
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
