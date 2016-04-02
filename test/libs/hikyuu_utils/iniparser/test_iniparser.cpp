/*
 * test_iniparser.cpp
 *
 *  Created on: 2010-5-26
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_iniparser
    #include <boost/test/unit_test.hpp>
#endif

#include <cfloat>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <hikyuu_utils/iniparser/IniParser.h>

using namespace boost::unit_test;
using namespace boost::filesystem;
using namespace hku;

/**
 * @defgroup test_utilities_iniparser_suite test_utilities_iniparser_suite
 * 测试IniParser
 * @{
 */

/**
 * 测试IniParser读取文件操作 \n
 * @par 检测点
 */
BOOST_AUTO_TEST_CASE( test_IniParser_read ) {
    IniParser ini_parser;

	/** @arg 读取文件不存在 */
	BOOST_CHECK_THROW(ini_parser.read("sdjfljoiEI487sdbc.txt"), std::invalid_argument);

	/** @arg 错误的文件格式：缺少section，抛出logic_error异常 */
	std::string test_filename("test_iniparser_read.ini");
	ofstream testini(test_filename, ofstream::trunc);
	testini << "Missing section Header\n";
	testini.close();
	BOOST_CHECK_THROW(ini_parser.read(test_filename), std::logic_error);

	/** @arg 错误的文件格式：section行格式错误，缺少"]"，如："[section "*/
	testini.open(test_filename, ofstream::trunc);
	testini << "[section" << std::endl;
	testini.close();
    BOOST_CHECK_THROW(ini_parser.read(test_filename), std::logic_error);

    /** @arg 错误的文件格式：section行格式错误，缺少"]"，如："[section;xx] "*/
    testini.open(test_filename, ofstream::trunc);
    testini << "[section;]" << std::endl;
    testini.close();
    BOOST_CHECK_THROW(ini_parser.read(test_filename), std::logic_error);

    /** @arg 错误的文件格式：key=value行格式错误，缺少key，如："= value "*/
    testini.open(test_filename, ofstream::trunc);
    testini << "[section]\n" << "= value";
    testini.close();
    BOOST_CHECK_THROW(ini_parser.read(test_filename), std::logic_error);

    /** @arg 错误的文件格式：key=value行格式错误，缺少value，如："key="*/
    testini.open(test_filename, ofstream::trunc);
    testini << "[section]\n" << "key=";
    testini.close();
    BOOST_CHECK_THROW(ini_parser.read(test_filename), std::logic_error);

    /** @arg 错误的文件格式：非注释行，即不包含section，也不包含key=value，如："key"*/
    testini.open(test_filename, ofstream::trunc);
    testini << "[section]\n" << "key";
    testini.close();
    BOOST_CHECK_THROW(ini_parser.read(test_filename), std::logic_error);

    /** @arg 除注释和空行外，仅存在唯一有效行，并且是section行*/
    testini.open(test_filename, ofstream::trunc);
    testini << "[section1]";
    testini.close();
    ini_parser.read(test_filename);
    BOOST_CHECK(ini_parser.hasSection("section1"));

    /** @arg 其他正常情况，在其他成员函数如：get、hasSection、hasOption中测试 */

    remove(test_filename);
}

/**
 * @par 检测点
 */
BOOST_AUTO_TEST_CASE( test_IniParser_hasSection ) {
    IniParser ini_parser;
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[test1]\n" << "key1=value1\n";
    testini.close();

    ini_parser.read(test_filename);

    /** @arg 存在指定的section */
    BOOST_CHECK(ini_parser.hasSection("test1"));

    /** @arg 不存在指定section */
    BOOST_CHECK(!ini_parser.hasSection("test2"));

    remove(test_filename);
}

/**
 * @par 检测点
 */
BOOST_AUTO_TEST_CASE( test_IniParser_hasOption ) {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n" << "key1=value1\n\n"
            << "[section1]\n" << "key3=value1\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg 存在指定section和option*/
    BOOST_CHECK(ini_parser.hasOption("section1", "key1"));
    BOOST_CHECK(ini_parser.hasOption("section1", "key3"));

    /** @arg 存在指定section，但不存在相应的option */
    BOOST_CHECK(!ini_parser.hasOption("section1", "key2"));

    /** @arg 不存在指定的section */
    BOOST_CHECK(!ini_parser.hasOption("section2", "key1"));

    remove(test_filename);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE(test_IniParser_getSectionList) {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n" << "key1=value1\n\n"
           << "[section1]\n" << "key3=value1\n"
           << "[section2]\n key1=value1\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg 存在section，可正常读取 */
    IniParser::StringListPtr output = ini_parser.getSectionList();
    IniParser::StringList::iterator iter = output->begin();
    BOOST_CHECK(output->size() == 2);
    BOOST_CHECK((*iter++) == "section1");
    BOOST_CHECK((*iter) == "section2");
    remove(test_filename);

    /** @arg 不存在任何sction的情况 */
    testini.open(test_filename, ofstream::trunc);
    testini.close();
    ini_parser.clear();
    ini_parser.read(test_filename);
    output = ini_parser.getSectionList();
    BOOST_CHECK(output->empty());
    remove(test_filename);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE(test_IniParser_getOptionList) {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n" << "key1=value1\n\n"
           << "[section1]\n" << "key2=value1\n"
           << "[section2]\n key1=value1\n\n"
           << "[section3]";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);
    IniParser::StringListPtr output;

    /** @arg 指定的section下不存在任何option */
    output = ini_parser.getOptionList("section3");
    BOOST_CHECK(output->empty());

    /** @arg 指定的section下，存在1个option */
    output = ini_parser.getOptionList("section2");
    BOOST_CHECK(output->size() == 1);
    BOOST_CHECK(output->front() == "key1");

    /** @arg 指定的section下，存在多个option */
    output = ini_parser.getOptionList("section1");
    BOOST_CHECK(output->size() == 2);
    IniParser::StringList::iterator iter = output->begin();
    BOOST_CHECK((*iter++) == "key1");
    BOOST_CHECK((*iter) == "key2");

    remove(test_filename);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE(test_IniParser_get) {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n" << "key1 = value1\n"
            << "key3 = value3\n\n"
            << "[section2]\n" << "key1 = value1\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg 存在指定section和option, 并且无缺省值*/
    BOOST_CHECK(ini_parser.get("section1", "key1") == "value1");
    BOOST_CHECK(ini_parser.get("section2", "key1") == "value1");

    /** @arg 存在指定section和option, 并且指定了缺省值*/
    BOOST_CHECK(ini_parser.get("section1", "key1", "value") == "value1");
    BOOST_CHECK(ini_parser.get("section2", "key1", "value") != "value");

    /** @arg 存在指定section，但不存在相应的option，并且无缺省值 */
    BOOST_CHECK_THROW(ini_parser.get("section1", "key2"), std::invalid_argument);

    /** @arg 存在指定section，但不存在相应的option，但指定了缺省值 */
    BOOST_CHECK(ini_parser.get("section1", "key2", "value2") == "value2");

    /** @arg 不存在指定的section，并且无缺省值 */
    BOOST_CHECK_THROW(ini_parser.get("section3", "key1"), std::invalid_argument);

    /** @arg 不存在指定的section，但指定了缺省值 */
    BOOST_CHECK_THROW(ini_parser.get("section3", "key1", "value1"), std::invalid_argument);

    remove(test_filename);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_IniParser_getInt ) {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n" << "key1 = 1\n"
            << "key3 = -3\n\n"
            << "[section2]\n" << "key1 = 10\n\n"
            << "key2 = A\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg 存在指定section和option, 并且无缺省值*/
    BOOST_CHECK(ini_parser.getInt("section1", "key1") == 1);
    BOOST_CHECK(ini_parser.getInt("section1", "key3") == -3);
    BOOST_CHECK(ini_parser.getInt("section2", "key1") == 10);

    /** @arg 存在指定section和option, 并且指定了有效缺省值*/
    BOOST_CHECK(ini_parser.getInt("section1", "key1", "2") == 1);
    BOOST_CHECK(ini_parser.getInt("section2", "key1", "20") != 20);

    /** @arg 存在指定section和option，但对应的值无法转换为int类型*/
    BOOST_CHECK_THROW(ini_parser.getInt("section2", "key2"), std::domain_error);

    /** @arg 存在指定section和option, 但指定了无效缺省值（无法转换到int类型），应抛出异常*/
    BOOST_CHECK_THROW(ini_parser.getInt("section1", "key1", "tow"), std::invalid_argument);
    BOOST_CHECK_THROW(ini_parser.getInt("section2", "key1", "twenty"), std::invalid_argument);

    /** @arg 存在指定section，但不存在相应的option，并且无缺省值 */
    BOOST_CHECK_THROW(ini_parser.getInt("section1", "key2"), std::invalid_argument);

    /** @arg 存在指定section，但不存在相应的option，但指定了有效缺省值 */
    BOOST_CHECK(ini_parser.getInt("section1", "key2", "10") == 10);

    /** @arg 存在指定section，但不存在相应的option，但指定了无效缺省值（无法转换到int类型） */
    BOOST_CHECK_THROW(ini_parser.getInt("section1", "key2", "10.0"), std::invalid_argument);

    /** @arg 不存在指定的section，并且无缺省值 */
    BOOST_CHECK_THROW(ini_parser.getInt("section3", "key1"), std::invalid_argument);

    /** @arg 不存在指定的section，但指定了有效缺省值 */
    BOOST_CHECK_THROW(ini_parser.getInt("section3", "key1", "1"), std::invalid_argument);

    remove(test_filename);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_IniParser_getFloat ) {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n" << "key1 = 1.123456\n"
            << "key3 = -3.145789\n\n"
            << "[section2]\n" << "key1 = 10\n\n"
            << "key2 = A\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg 存在指定section和option, 并且无缺省值*/
    BOOST_CHECK_CLOSE(ini_parser.getFloat("section1", "key1"), 1.123456, 0.00001);
    BOOST_CHECK_CLOSE(ini_parser.getFloat("section1", "key3"), -3.145789, 0.00001);
    BOOST_CHECK_CLOSE(ini_parser.getFloat("section2", "key1"), 10, 0.00001);

    /** @arg 存在指定section和option, 并且指定了有效缺省值*/
    BOOST_CHECK_CLOSE(ini_parser.getFloat("section1", "key1", "2.01"), 1.123456, 0.00001);
    BOOST_CHECK_CLOSE(ini_parser.getFloat("section2", "key1", "20.20"), 10, 0.00001);

    /** @arg 存在指定section和option，但对应的值无法转换为int类型*/
    BOOST_CHECK_THROW(ini_parser.getFloat("section2", "key2"), std::domain_error);

    /** @arg 存在指定section和option, 但指定了无效缺省值（无法转换到int类型），应抛出异常*/
    BOOST_CHECK_THROW(ini_parser.getFloat("section1", "key1", "tow"), std::invalid_argument);
    BOOST_CHECK_THROW(ini_parser.getFloat("section2", "key1", "twenty"), std::invalid_argument);

    /** @arg 存在指定section，但不存在相应的option，并且无缺省值 */
    BOOST_CHECK_THROW(ini_parser.getFloat("section1", "key2"), std::invalid_argument);

    /** @arg 存在指定section，但不存在相应的option，但指定了有效缺省值 */
    BOOST_CHECK_CLOSE(ini_parser.getFloat("section1", "key2", "10"), 10.0, 0.00001);

    /** @arg 存在指定section，但不存在相应的option，但指定了无效缺省值（无法转换到int类型） */
    BOOST_CHECK_THROW(ini_parser.getFloat("section1", "key2", "1A0"), std::invalid_argument);

    /** @arg 不存在指定的section，并且无缺省值 */
    BOOST_CHECK_THROW(ini_parser.getFloat("section3", "key1"), std::invalid_argument);

    /** @arg 不存在指定的section，但指定了有效缺省值 */
    BOOST_CHECK_THROW(ini_parser.getFloat("section3", "key1", "1"), std::invalid_argument);

    /** @arg 测试值为-3.4e-38和3.4e+38的情况 */
    BOOST_CHECK_CLOSE(ini_parser.getFloat("section1", "key2", "-3.4e-38"), -3.4e-38F, 0.1e-30);
    BOOST_CHECK_CLOSE(ini_parser.getFloat("section1", "key2", "3.402823466e+38"), 3.402823466e+38F, 0.1e-38);

    /** @arg 测试超出float范围的数据：3.41+38。注：MSVC使用中，超出float范围的数据不会抛出异常，而是转换为INF */
#ifdef __MSVC__
    BOOST_CHECK_THROW(ini_parser.getFloat("section1", "key2", "3.41e+38"), std::invalid_argument);
#endif

    remove(test_filename);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_IniParser_getDouble ) {
    std::string test_filename("test_iniparser.ini");
    ofstream testini(test_filename, ofstream::trunc);
    testini << "[section1]\n" << "key1 = 1.123456\n"
            << "key3 = -3.145789\n\n"
            << "[section2]\n" << "key1 = 10\n\n"
            << "key2 = A\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg 存在指定section和option, 并且无缺省值*/
    BOOST_CHECK_CLOSE(ini_parser.getDouble("section1", "key1"), 1.123456, 0.00001);
    BOOST_CHECK_CLOSE(ini_parser.getDouble("section1", "key3"), -3.145789, 0.00001);
    BOOST_CHECK_CLOSE(ini_parser.getDouble("section2", "key1"), 10, 0.00001);

    /** @arg 存在指定section和option, 并且指定了有效缺省值*/
    BOOST_CHECK_CLOSE(ini_parser.getDouble("section1", "key1", "2.01"), 1.123456, 0.00001);
    BOOST_CHECK_CLOSE(ini_parser.getDouble("section2", "key1", "20.20"), 10, 0.00001);

    /** @arg 存在指定section和option，但对应的值无法转换为int类型*/
    BOOST_CHECK_THROW(ini_parser.getDouble("section2", "key2"), std::domain_error);

    /** @arg 存在指定section和option, 但指定了无效缺省值（无法转换到int类型），应抛出异常*/
    BOOST_CHECK_THROW(ini_parser.getDouble("section1", "key1", "tow"), std::invalid_argument);
    BOOST_CHECK_THROW(ini_parser.getDouble("section2", "key1", "twenty"), std::invalid_argument);

    /** @arg 存在指定section，但不存在相应的option，并且无缺省值 */
    BOOST_CHECK_THROW(ini_parser.getDouble("section1", "key2"), std::invalid_argument);

    /** @arg 存在指定section，但不存在相应的option，但指定了有效缺省值 */
    BOOST_CHECK_CLOSE(ini_parser.getDouble("section1", "key2", "10"), 10.0, 0.00001);

    /** @arg 存在指定section，但不存在相应的option，但指定了无效缺省值（无法转换到int类型） */
    BOOST_CHECK_THROW(ini_parser.getDouble("section1", "key2", "1A0"), std::invalid_argument);

    /** @arg 不存在指定的section，并且无缺省值 */
    BOOST_CHECK_THROW(ini_parser.getDouble("section3", "key1"), std::invalid_argument);

    /** @arg 不存在指定的section，但指定了有效缺省值 */
    BOOST_CHECK_THROW(ini_parser.getDouble("section3", "key1", "1"), std::invalid_argument);

    remove(test_filename);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_IniParser_getBool ) {
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
            << "[section2]\n" << "key1 = false\n\n"
            << "key2 = A\n";
    testini.close();

    IniParser ini_parser;
    ini_parser.read(test_filename);

    /** @arg 存在指定section和option, 并且无缺省值，对应值分别为1|0|true|yes|on|false|no|off（含不同大小写）*/
    BOOST_CHECK(ini_parser.getBool("section1", "key1"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key2"));
    BOOST_CHECK(ini_parser.getBool("section1", "key3"));
    BOOST_CHECK(ini_parser.getBool("section1", "key4"));
    BOOST_CHECK(ini_parser.getBool("section1", "key5"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key6"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key7"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key8"));
    BOOST_CHECK(ini_parser.getBool("section1", "key9"));
    BOOST_CHECK(ini_parser.getBool("section1", "key10"));
    BOOST_CHECK(ini_parser.getBool("section1", "key11"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key12"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key13"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key14"));
    BOOST_CHECK(ini_parser.getBool("section1", "key15"));
    BOOST_CHECK(ini_parser.getBool("section1", "key16"));
    BOOST_CHECK(ini_parser.getBool("section1", "key17"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key18"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key19"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key20"));

    /** @arg 存在指定section和option, 并且指定了有效缺省值*/
    BOOST_CHECK(ini_parser.getBool("section1", "key1", "0"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key2", "1"));

    /** @arg 存在指定section和option，但对应的值无法转换为bool类型*/
    BOOST_CHECK_THROW(ini_parser.getBool("section2", "key2"), std::domain_error);

    /** @arg 存在指定section和option, 但指定了无效缺省值（无法转换到bool类型），应抛出异常*/
    BOOST_CHECK_THROW(ini_parser.getBool("section1", "key1", "tow"), std::invalid_argument);
    BOOST_CHECK_THROW(ini_parser.getBool("section1", "key1", "twenty"), std::invalid_argument);

    /** @arg 存在指定section，但不存在相应的option，并且无缺省值 */
    BOOST_CHECK_THROW(ini_parser.getBool("section1", "key22"), std::invalid_argument);

    /** @arg 存在指定section，但不存在相应的option，但指定了有效缺省值（0|1|true|false|yes|no|on|off）（含不同大小写） */
    BOOST_CHECK(!ini_parser.getBool("section1", "key22", "0"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key22", "false"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key22", "False"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key22", "FALSE"));
    BOOST_CHECK(ini_parser.getBool("section1", "key22", "1"));
    BOOST_CHECK(ini_parser.getBool("section1", "key22", "true"));
    BOOST_CHECK(ini_parser.getBool("section1", "key22", "TRUE"));
    BOOST_CHECK(ini_parser.getBool("section1", "key22", "True"));
    BOOST_CHECK(ini_parser.getBool("section1", "key22", "YES"));
    BOOST_CHECK(ini_parser.getBool("section1", "key22", "Yes"));
    BOOST_CHECK(ini_parser.getBool("section1", "key22", "yes"));
    BOOST_CHECK(ini_parser.getBool("section1", "key22", "ON"));
    BOOST_CHECK(ini_parser.getBool("section1", "key22", "On"));
    BOOST_CHECK(ini_parser.getBool("section1", "key22", "on"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key22", "NO"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key22", "No"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key22", "no"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key22", "OFF"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key22", "Off"));
    BOOST_CHECK(!ini_parser.getBool("section1", "key22", "off"));

    /** @arg 存在指定section，但不存在相应的option，但指定了无效缺省值（无法转换到bool类型） */
    BOOST_CHECK_THROW(ini_parser.getBool("section1", "key22", "10"), std::invalid_argument);

    /** @arg 不存在指定的section，并且无缺省值 */
    BOOST_CHECK_THROW(ini_parser.getBool("section3", "key1"), std::invalid_argument);

    /** @arg 不存在指定的section，但指定了有效缺省值 */
    BOOST_CHECK_THROW(ini_parser.getBool("section3", "key1", "1"), std::invalid_argument);

    remove(test_filename);
}
/** @}*/
