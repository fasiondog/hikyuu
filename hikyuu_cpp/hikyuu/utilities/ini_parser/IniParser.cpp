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

// Used to record the format error information during the file parsing (the line number and the
// wrong line content) For the internal use of IniParser only
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
 * Read and parse the given ini file
 * @details Multiple different ini files may be read and then the information is processed together
 * @throw std::invalid_argument this exception is thrown when the given file cannot be opened
 * @throw std::logic_error this exception is thrown when the file format is wrong
 * @param filename the given file name
 */
void IniParser::read(const std::string& filename) {
    std::ifstream inifile(HKU_PATH(filename), std::ifstream::in);
    if (!inifile) {
        throw(std::invalid_argument("Can't read file(" + filename + ")!"));
    }

    size_t line_no = 0;          // The current line number, used to record the error information
    ParsingError parsing_error;  // Records the format parsing error

    std::string section;
    std::string key;
    std::string value;
    std::string line_str;

    while (std::getline(inifile, line_str)) {
        line_no++;
        trim(line_str);

        // Skip the empty or the comment lines
        if (line_str.empty() || line_str.at(0) == ';') {
            continue;
        }

        // Check the first comment marker and clear it together with the following characters
        size_t pos = line_str.find(';');
        if (pos != std::string::npos) {
            line_str.assign(line_str, 0, pos);
            trim(line_str);
        }

        // A section line
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
                break;  // The section definition is missing, no further processing is needed
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
 * Clear the information already read
 */
void IniParser::clear() {
    m_sections.clear();
}

/**
 * Judge whether the given section exists
 */
bool IniParser::hasSection(const std::string& section) const {
    return m_sections.count(section) ? true : false;
}

/**
 * Judge whether the given option exists
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
 * Get all the sections
 * @return the list of all the sections
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
 * Get all the options under the given section
 * @throw std::invalid_argument is thrown when the given section does not exist
 * @param section the given section
 * @return the list of all the options under the given section
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
 * Get the value of the given option
 * @throw std::invalid_argument this exception is thrown when the given option has no value and no
 * default value is given
 * @param section the given section
 * @param option the given option
 * @param default_str
 * the default value, it is returned when there is no corresponding option value. When it is empty
 * there is no default value, and if there is no corresponding option value a std::invalid_argument
 * exception is thrown. It is empty by default.
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
 * Get the value of the given option and convert it into int
 * @throw std::invalid_argument this exception is thrown when the given option has no value and no
 * default value is given; it is also thrown when the given default value (non-empty) cannot be
 * converted into int,
 * @throw std::domain_error this exception is thrown when the given option value cannot be converted
 * into int
 * @param section the given section
 * @param option the given option
 * @param default_str the default value, returned when there is no corresponding option value; an
 * empty value means none It is empty by default, i.e. no default value is given.
 */
int IniParser::getInt(const std::string& section, const std::string& option,
                      const std::string& default_str) const {
    int result = 0;
    size_t remain = 0;

    // First check whether default_str can be converted into int
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
 * Get the value of the given option and convert it into float
 * @throw std::invalid_argument this exception is thrown when the given option has no value and no
 * default value is given; it is also thrown when the given default value (non-empty) cannot be
 * converted into float,
 * @throw std::domain_error this exception is thrown when the given option value cannot be converted
 * into float
 * @param section the given section
 * @param option the given option
 * @param default_str the default value, returned when there is no corresponding option value; an
 * empty value means none It is empty by default, i.e. no default value is given.
 */
float IniParser::getFloat(const std::string& section, const std::string& option,
                          const std::string& default_str) const {
    float result;
    size_t remain = 0;

    // First check whether default_str can be converted into float
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
 * Get the value of the given option and convert it into double
 * @throw std::invalid_argument this exception is thrown when the given option has no value and no
 * default value is given; it is also thrown when the given default value (non-empty) cannot be
 * converted into double,
 * @throw std::domain_error this exception is thrown when the given option value cannot be converted
 * into double
 * @param section the given section
 * @param option the given option
 * @param default_str the default value, returned when there is no corresponding option value; an
 * empty value means none It is empty by default, i.e. no default value is given.
 */
double IniParser::getDouble(const std::string& section, const std::string& option,
                            const std::string& default_str) const {
    double result;
    size_t remain = 0;

    // First check whether default_str can be converted into float
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
 * Get the value of the given option and convert it into bool
 * @details when the option means true, the acceptable values are: 1|true|yes|on (case insensitive)
 * \n when the option means false, the acceptable values are: 0|false|no|off (case insensitive)
 * @throw std::invalid_argument this exception is thrown when the given option has no value and no
 * default value is given; it is also thrown when the given default value (non-empty) cannot be
 * converted into bool,
 * @throw std::domain_error this exception is thrown when the given option value cannot be converted
 * into bool
 * @param section the given section
 * @param option the given option
 * @param default_str the default value, returned when there is no corresponding option value; an
 * empty value means none It is empty by default, i.e. no default value is given.
 */
bool IniParser::getBool(const std::string& section, const std::string& option,
                        const std::string& default_str) const {
    // First check whether default_str can be converted into bool
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
