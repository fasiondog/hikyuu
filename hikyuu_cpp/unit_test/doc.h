/*
 * doc.h
 *
 *  Created on: 2010-6-3
 *      Author: fasiondog
 */

#ifndef DOC_H_
#define DOC_H_

/******************************************************************************
 * This file is used by doxygen to generate the documentation only
 *****************************************************************************/

/**
 * @mainpage
 * The test case documentation, used to organize and describe the test cases. \n
 *
 * @section test_rule The organization principles of the test project
 * Principle 1: the test project is physically isolated from the source code project, using a fully
 * independent parallel directory, but the internal directory structure should stay consistent with
 * the source code project.
 * \n
 * Principle 2: for a module (usually a class) a test suite (test_suite) should be created, its
 * naming rule is: test_module name (class name)_suite, for example, for the class IniParser a test
 * suite test_iniparser_suite is created. It is recommended to use lowercase letters only. \n
 * Principle 3: for every test suite a separate test file is used, its naming rule is:
 * test_module name (class name).cpp. \n
 * Principle 4: for every function or class member method an independent test case is created, its
 * naming rule is: test_function name (or test_class name_method name).
 *          In the case of a duplicate name, "_case" or another identifier can be appended for a
 *          simple distinction. \n
 * Principle 5: every test case should be defined and implemented in the corresponding test suite
 * file. \n
 *
 * @section maintenance_rule Maintenance principles
 * When the source code project changes (such as adding a new feature or fixing a BUG), the
 * corresponding test cases should be checked and corrected to guarantee the continuous usability of
 * the test project.
 */

/**
 * @defgroup test_hikyuu_base_suite test_hikyuu_base_suite
 * @defgroup test_hikyuu_serialize_suite test_hikyuu_serialize_suite
 * @defgroup test_hikyuu_utilities test_hikyuu_utilities
 * @defgroup test_hikyuu_datetime_suite test_hikyuu_datetime_suite
 * @defgroup test_hikyuu_indicator_suite test_hikyuu_indicator_suite
 * @defgroup test_hikyuu_trade_manage_suite test_hikyuu_trade_manage_suite
 * @defgroup test_hikyuu_trade_sys_suite test_hikyuu_trade_sys_suite
 * @defgroup test_hikyuu_analysis_suite test_hikyuu_analysis_suite
 *
 *
 */

#endif /* DOC_H_ */
