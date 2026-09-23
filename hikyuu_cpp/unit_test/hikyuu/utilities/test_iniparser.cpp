/*
 * test_iniparser.cpp
 *
 *  Created on: 2010-5-26
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cfloat>
#include <iostream>
#include <fstream>
#include <exception>
#include <hikyuu/utilities/os.h>
#include <hikyuu/utilities/ini_parser/IniParser.h>

using namespace hku;
using namespace std;

/**
 * @defgroup test_utilities_iniparser_suite test_utilities_iniparser_suite
 * Test IniParser
 * @{
 */

/**
 * Test the file reading operation of IniParser \n
 * @par Test points
 */
TEST_CASE("test_IniParser_read") {
    IniParser ini_parser;

    /** @arg The file to read does not exist */
    CHECK_THROWS_AS(ini_parser.read("sdjfljoiEI487sdbc.txt"), std::invalid_argument);

    /** @arg A wrong file format: the section is missing and a logic_error is thrown */
    std::string test_filename("test_iniparser_read.ini");
    std::ofstream testini;
    testini.open(test_filename, ofstream::trunc);
    testini << "Missing section Header\n";
    testini.close();
    CHECK_THROWS_AS(ini_parser.read(test_filename), std::logic_error);

    /** @arg A wrong file format: the section line is wrong, "]" is missing, e.g. "[section "*/
    testini.open(test_filename, ofstream::trunc);
    testini << "[section" << std::endl;
    testini.close();
    CHECK_THROWS_AS(ini_parser.read(test_filename), std::logic_error);

    /** @arg A wrong file format: the section line is wrong, "]" is missing, e.g. "[section;xx] "*/
    testini.open(test_filename, ofstream::trunc);
    testini << "[section;]" << std::endl;
    testini.close();
    CHECK_THROWS_AS(ini_parser.read(test_filename), std::logic_error);

    /** @arg A wrong file format: the key=value line is wrong, the key is missing, e.g. "= value "*/
    testini.open(test_filename, ofstream::trunc);
    testini << "[section]\n"
            << "= value";
    testini.close();
    CHECK_THROWS_AS(ini_parser.read(test_filename), std::logic_error);

    /** @arg A wrong file format: the key=value line is wrong, the value is missing, e.g. "key="*/
    testini.open(test_filename, ofstream::trunc);
    testini << "[section]\n"
            << "key=";
    testini.close();
    CHECK_THROWS_AS(ini_parser.read(test_filename), std::logic_error);

    /** @arg A wrong file format: a non-comment line with no section and no key=value, e.g. "key"*/
    testini.open(test_filename, ofstream::trunc);
    testini << "[section]\n"
            << "key";
    testini.close();
    CHECK_THROWS_AS(ini_parser.read(test_filename), std::logic_error);

    /** @arg Besides the comments and the empty lines, the only valid line is a section line */
    testini.open(test_filename, ofstream::trunc);
    testini << "[section1]";
    testini.close();
    ini_parser.read(test_filename);
    CHECK_UNARY(ini_parser.hasSection("section1"));

    /** @arg The other normal cases are tested in get, hasSection and hasOption */

    removeFile(test_filename);
}

/**
 * @par Test points
 */
TEST_CASE("test_IniParser_hasSection") {
    IniParser ini_parser;
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[test1]\n"
            << "key1=value1\n";
    testini.close();

    ini_parser.read(test_filename);

    /** @arg The given section exists */
    CHECK_UNARY(ini_parser.hasSection("test1"));

    /** @arg The given section does not exist */
    CHECK_UNARY(!ini_parser.hasSection("test2"));

    removeFile(test_filename);
}

/**
 * @par Test points
 */
TEST_CASE("test_IniParser_hasOption") {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n"
            << "key1=value1\n\n"
            << "[section1]\n"
            << "key3=value1\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg The given section and option exist */
    CHECK_UNARY(ini_parser.hasOption("section1", "key1"));
    CHECK_UNARY(ini_parser.hasOption("section1", "key3"));

    /** @arg The given section exists but the corresponding option does not */
    CHECK_UNARY(!ini_parser.hasOption("section1", "key2"));

    /** @arg The given section does not exist */
    CHECK_UNARY(!ini_parser.hasOption("section2", "key1"));

    removeFile(test_filename);
}

/** @par Test points */
TEST_CASE("test_IniParser_getSectionList") {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n"
            << "key1=value1\n\n"
            << "[section1]\n"
            << "key3=value1\n"
            << "[section2]\n key1=value1\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg The section exists and can be read normally */
    IniParser::StringListPtr output = ini_parser.getSectionList();
    IniParser::StringList::iterator iter = output->begin();
    CHECK_EQ(output->size(), 2);
    CHECK_EQ((*iter++), "section1");
    CHECK_EQ((*iter), "section2");
    removeFile(test_filename);

    /** @arg There is no section at all */
    testini.open(test_filename, ofstream::trunc);
    testini.close();
    ini_parser.clear();
    ini_parser.read(test_filename);
    output = ini_parser.getSectionList();
    CHECK_UNARY(output->empty());
    removeFile(test_filename);
}

/** @par Test points */
TEST_CASE("test_IniParser_getOptionList") {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n"
            << "key1=value1\n\n"
            << "[section1]\n"
            << "key2=value1\n"
            << "[section2]\n key1=value1\n\n"
            << "[section3]";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);
    IniParser::StringListPtr output;

    /** @arg There is no option under the given section */
    output = ini_parser.getOptionList("section3");
    CHECK_UNARY(output->empty());

    /** @arg There is 1 option under the given section */
    output = ini_parser.getOptionList("section2");
    CHECK_EQ(output->size(), 1);
    CHECK_EQ(output->front(), "key1");

    /** @arg There are multiple options under the given section */
    output = ini_parser.getOptionList("section1");
    CHECK_EQ(output->size(), 2);
    IniParser::StringList::iterator iter = output->begin();
    CHECK_EQ((*iter++), "key1");
    CHECK_EQ((*iter), "key2");

    removeFile(test_filename);
}

/** @par Test points */
TEST_CASE("test_IniParser_get") {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n"
            << "key1 = value1\n"
            << "key3 = value3\n\n"
            << "[section2]\n"
            << "key1 = value1\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg The given section and option exist and there is no default value */
    CHECK_EQ(ini_parser.get("section1", "key1"), "value1");
    CHECK_EQ(ini_parser.get("section2", "key1"), "value1");

    /** @arg The given section and option exist and a default value is given */
    CHECK_EQ(ini_parser.get("section1", "key1", "value"), "value1");
    CHECK_NE(ini_parser.get("section2", "key1", "value"), "value");

    /** @arg The given section exists but the option does not, and there is no default value */
    CHECK_THROWS_AS(ini_parser.get("section1", "key2"), std::invalid_argument);

    /** @arg The section exists but the option does not, and a default value is given */
    CHECK_EQ(ini_parser.get("section1", "key2", "value2"), "value2");

    /** @arg The given section does not exist and there is no default value */
    CHECK_THROWS_AS(ini_parser.get("section3", "key1"), std::invalid_argument);

    /** @arg The given section does not exist but a default value is given */
    CHECK_THROWS_AS(ini_parser.get("section3", "key1", "value1"), std::invalid_argument);

    removeFile(test_filename);
}

/** @par Test points */
TEST_CASE("test_IniParser_getInt") {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n"
            << "key1 = 1\n"
            << "key3 = -3\n\n"
            << "[section2]\n"
            << "key1 = 10\n\n"
            << "key2 = A\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg The given section and option exist and there is no default value */
    CHECK_EQ(ini_parser.getInt("section1", "key1"), 1);
    CHECK_EQ(ini_parser.getInt("section1", "key3"), -3);
    CHECK_EQ(ini_parser.getInt("section2", "key1"), 10);

    /** @arg The given section and option exist and a valid default value is given */
    CHECK_EQ(ini_parser.getInt("section1", "key1", "2"), 1);
    CHECK_NE(ini_parser.getInt("section2", "key1", "20"), 20);

    /** @arg The given section and option exist but the value cannot be converted into int */
    CHECK_THROWS_AS(ini_parser.getInt("section2", "key2"), std::invalid_argument);

    /** @arg The section and option exist but an invalid default value is given (it cannot be
     * converted into int), an exception should be thrown */
    CHECK_THROWS_AS(ini_parser.getInt("section1", "key1", "tow"), std::invalid_argument);
    CHECK_THROWS_AS(ini_parser.getInt("section2", "key1", "twenty"), std::invalid_argument);

    /** @arg The given section exists but the option does not, and there is no default value */
    CHECK_THROWS_AS(ini_parser.getInt("section1", "key2"), std::invalid_argument);

    /** @arg The section exists but the option does not, and a valid default value is given */
    CHECK_EQ(ini_parser.getInt("section1", "key2", "10"), 10);

    /** @arg The section exists but the option does not, and an invalid default value is given (it
     * cannot be converted into int) */
    CHECK_THROWS_AS(ini_parser.getInt("section1", "key2", "10.0"), std::invalid_argument);

    /** @arg The given section does not exist and there is no default value */
    CHECK_THROWS_AS(ini_parser.getInt("section3", "key1"), std::invalid_argument);

    /** @arg The given section does not exist but a valid default value is given */
    CHECK_THROWS_AS(ini_parser.getInt("section3", "key1", "1"), std::invalid_argument);

    removeFile(test_filename);
}

/** @par Test points */
TEST_CASE("test_IniParser_getFloat") {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n"
            << "key1 = 1.123456\n"
            << "key3 = -3.145789\n\n"
            << "[section2]\n"
            << "key1 = 10\n\n"
            << "key2 = A\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg The given section and option exist and there is no default value */
    CHECK_EQ(ini_parser.getFloat("section1", "key1"), doctest::Approx(1.123456).epsilon(0.00001));
    CHECK_EQ(ini_parser.getFloat("section1", "key3"), doctest::Approx(-3.145789).epsilon(0.00001));
    CHECK_EQ(ini_parser.getFloat("section2", "key1"), doctest::Approx(10).epsilon(0.00001));

    /** @arg The given section and option exist and a valid default value is given */
    CHECK_EQ(ini_parser.getFloat("section1", "key1", "2.01"),
             doctest::Approx(1.123456).epsilon(0.00001));
    CHECK_EQ(ini_parser.getFloat("section2", "key1", "20.20"),
             doctest::Approx(10).epsilon(0.00001));

    /** @arg The given section and option exist but the value cannot be converted into int */
    CHECK_THROWS_AS(ini_parser.getFloat("section2", "key2"), std::invalid_argument);

    /** @arg The section and option exist but an invalid default value is given (it cannot be
     * converted into int), an exception should be thrown */
    CHECK_THROWS_AS(ini_parser.getFloat("section1", "key1", "tow"), std::invalid_argument);
    CHECK_THROWS_AS(ini_parser.getFloat("section2", "key1", "twenty"), std::invalid_argument);

    /** @arg The given section exists but the option does not, and there is no default value */
    CHECK_THROWS_AS(ini_parser.getFloat("section1", "key2"), std::invalid_argument);

    /** @arg The section exists but the option does not, and a valid default value is given */
    CHECK_EQ(ini_parser.getFloat("section1", "key2", "10"), doctest::Approx(10.0));

    /** @arg The section exists but the option does not, and an invalid default value is given (it
     * cannot be converted into int) */
    CHECK_THROWS_AS(ini_parser.getFloat("section1", "key2", "1A0"), std::invalid_argument);

    /** @arg The given section does not exist and there is no default value */
    CHECK_THROWS_AS(ini_parser.getFloat("section3", "key1"), std::invalid_argument);

    /** @arg The given section does not exist but a valid default value is given */
    CHECK_THROWS_AS(ini_parser.getFloat("section3", "key1", "1"), std::invalid_argument);

    /** @arg Test the values -3.4e-38 and 3.4e+38 */
    CHECK_EQ(ini_parser.getFloat("section1", "key2", "-3.4e-38"),
             doctest::Approx(-3.4e-38F).epsilon(0.1e-30));
    CHECK_EQ(ini_parser.getFloat("section1", "key2", "3.402823466e+38"),
             doctest::Approx(3.402823466e+38F).epsilon(0.1e-38));

    /** @arg
     * Test the data beyond the float range: 3.41+38. Note: with MSVC such data does not throw but
     * becomes INF
     */
#ifdef __MSVC__
    CHECK_THROWS_AS(ini_parser.getFloat("section1", "key2", "3.41e+38"), std::invalid_argument);
#endif

    removeFile(test_filename);
}

/** @par Test points */
TEST_CASE("test_IniParser_getDouble") {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n"
            << "key1 = 1.123456\n"
            << "key3 = -3.145789\n\n"
            << "[section2]\n"
            << "key1 = 10\n\n"
            << "key2 = A\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg The given section and option exist and there is no default value */
    CHECK_EQ(ini_parser.getDouble("section1", "key1"), doctest::Approx(1.123456).epsilon(0.00001));
    CHECK_EQ(ini_parser.getDouble("section1", "key3"), doctest::Approx(-3.145789).epsilon(0.00001));
    CHECK_EQ(ini_parser.getDouble("section2", "key1"), doctest::Approx(10).epsilon(0.00001));

    /** @arg The given section and option exist and a valid default value is given */
    CHECK_EQ(ini_parser.getDouble("section1", "key1", "2.01"), doctest::Approx(1.123456));
    CHECK_EQ(ini_parser.getDouble("section2", "key1", "20.20"), doctest::Approx(10));

    /** @arg The given section and option exist but the value cannot be converted into int */
    CHECK_THROWS_AS(ini_parser.getDouble("section2", "key2"), std::invalid_argument);

    /** @arg The section and option exist but an invalid default value is given (it cannot be
     * converted into int), an exception should be thrown */
    CHECK_THROWS_AS(ini_parser.getDouble("section1", "key1", "tow"), std::invalid_argument);
    CHECK_THROWS_AS(ini_parser.getDouble("section2", "key1", "twenty"), std::invalid_argument);

    /** @arg The given section exists but the option does not, and there is no default value */
    CHECK_THROWS_AS(ini_parser.getDouble("section1", "key2"), std::invalid_argument);

    /** @arg The section exists but the option does not, and a valid default value is given */
    CHECK_EQ(ini_parser.getDouble("section1", "key2", "10"),
             doctest::Approx(10.0).epsilon(0.00001));

    /** @arg The section exists but the option does not, and an invalid default value is given (it
     * cannot be converted into int) */
    CHECK_THROWS_AS(ini_parser.getDouble("section1", "key2", "1A0"), std::invalid_argument);

    /** @arg The given section does not exist and there is no default value */
    CHECK_THROWS_AS(ini_parser.getDouble("section3", "key1"), std::invalid_argument);

    /** @arg The given section does not exist but a valid default value is given */
    CHECK_THROWS_AS(ini_parser.getDouble("section3", "key1", "1"), std::invalid_argument);

    removeFile(test_filename);
}

/** @par Test points */
TEST_CASE("test_IniParser_getBool") {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n"
            << "key1 = 1\n"
            << "key2 = 0\n"
            << "key3 = TRUE\n"
            << "key4 = true\n"
            << "key5 = True\n"
            << "key6 = FALSE\n"
            << "key7 = false\n"
            << "key8 = False\n"
            << "key9 = YES\n"
            << "key10 = Yes\n"
            << "key11 = yes\n"
            << "key12 = NO\n"
            << "key13 = No\n"
            << "key14 = no\n"
            << "key15 = ON\n"
            << "key16 = On\n"
            << "key17 = on\n"
            << "key18 = OFF\n"
            << "key19 = Off\n"
            << "key20 = off\n"
            << "\n"
            << "[section2]\n"
            << "key1 = false\n\n"
            << "key2 = A\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg The given section and option exist,
     * no default value is given and the values are 1|0|true|yes|on|false|no|off (mixed cases) */
    CHECK_UNARY(ini_parser.getBool("section1", "key1"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key2"));
    CHECK_UNARY(ini_parser.getBool("section1", "key3"));
    CHECK_UNARY(ini_parser.getBool("section1", "key4"));
    CHECK_UNARY(ini_parser.getBool("section1", "key5"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key6"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key7"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key8"));
    CHECK_UNARY(ini_parser.getBool("section1", "key9"));
    CHECK_UNARY(ini_parser.getBool("section1", "key10"));
    CHECK_UNARY(ini_parser.getBool("section1", "key11"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key12"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key13"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key14"));
    CHECK_UNARY(ini_parser.getBool("section1", "key15"));
    CHECK_UNARY(ini_parser.getBool("section1", "key16"));
    CHECK_UNARY(ini_parser.getBool("section1", "key17"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key18"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key19"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key20"));

    /** @arg The given section and option exist and a valid default value is given */
    CHECK_UNARY(ini_parser.getBool("section1", "key1", "0"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key2", "1"));

    /** @arg The given section and option exist but the value cannot be converted into bool */
    CHECK_THROWS_AS(ini_parser.getBool("section2", "key2"), std::domain_error);

    /** @arg The section and option exist but an invalid default value is given (not convertible to
     * bool), an exception should be thrown */
    CHECK_THROWS_AS(ini_parser.getBool("section1", "key1", "tow"), std::invalid_argument);
    CHECK_THROWS_AS(ini_parser.getBool("section1", "key1", "twenty"), std::invalid_argument);

    /** @arg The given section exists but the option does not, and there is no default value */
    CHECK_THROWS_AS(ini_parser.getBool("section1", "key22"), std::invalid_argument);

    /** @arg
     * The section exists but the option does not, and a valid default value is given
     * (0|1|true|false|yes|no|on|off) (mixed cases)
     */
    CHECK_UNARY(!ini_parser.getBool("section1", "key22", "0"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key22", "false"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key22", "False"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key22", "FALSE"));
    CHECK_UNARY(ini_parser.getBool("section1", "key22", "1"));
    CHECK_UNARY(ini_parser.getBool("section1", "key22", "true"));
    CHECK_UNARY(ini_parser.getBool("section1", "key22", "TRUE"));
    CHECK_UNARY(ini_parser.getBool("section1", "key22", "True"));
    CHECK_UNARY(ini_parser.getBool("section1", "key22", "YES"));
    CHECK_UNARY(ini_parser.getBool("section1", "key22", "Yes"));
    CHECK_UNARY(ini_parser.getBool("section1", "key22", "yes"));
    CHECK_UNARY(ini_parser.getBool("section1", "key22", "ON"));
    CHECK_UNARY(ini_parser.getBool("section1", "key22", "On"));
    CHECK_UNARY(ini_parser.getBool("section1", "key22", "on"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key22", "NO"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key22", "No"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key22", "no"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key22", "OFF"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key22", "Off"));
    CHECK_UNARY(!ini_parser.getBool("section1", "key22", "off"));

    /** @arg The section exists but the option does not, and an invalid default value is given (not
     * convertible to bool) */
    CHECK_THROWS_AS(ini_parser.getBool("section1", "key22", "10"), std::invalid_argument);

    /** @arg The given section does not exist and there is no default value */
    CHECK_THROWS_AS(ini_parser.getBool("section3", "key1"), std::invalid_argument);

    /** @arg The given section does not exist but a valid default value is given */
    CHECK_THROWS_AS(ini_parser.getBool("section3", "key1", "1"), std::invalid_argument);

    removeFile(test_filename);
}

/** @}*/