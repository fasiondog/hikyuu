/*
 * doc.h
 *
 *  Created on: 2010-6-3
 *      Author: fasiondog
 */

#ifndef DOC_H_
#define DOC_H_

/******************************************************************************
 * 该文件仅用于生成doxygen文档
 *****************************************************************************/

/**
 * @mainpage
 * 测试用例文档，用于组织和描述测试用例。 \n
 *
 * @section test_rule 测试工程的组织原则
 * 组织原则1：测试工程与源代码工程在物理上隔离，使用完全独立的并行目录，但内部目录结构应于源代码工程保持一致。
 * \n
 * 组织原则2：针对一个模块（通常为一个类），应建立一个测试套件（test_suite），其命名规则为：test_模块名（类名)_suite，
 *          例如，针对类IniParser，建立一个测试套件：test_iniparser_suite。建议全部使用小写字母。 \n
 * 组织原则3：针对每一测试套件，使用一个单独的测试文件，文件命名规则：test_模块名（类名）.cpp。 \n
 * 组织原则4：针对每一个函数或类成员方法，建立一个独立的测试用例，命名规则为：test_函数名(或test_类名_方法名)。
 *          对于重名的情况，可以在其后，加上“_case”或其他标识进行简单区分。\n
 * 组织原则5：对于每一测试用例，应在对应的测试套件文件中予以定义和实现。\n
 *
 * @section maintenance_rule 维护原则
 * 在源代码工程发生变更时（如增加新功能、修改BUG），应对相应的测试用例进行检查和修正，以保证测试工程的持续可用。
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
