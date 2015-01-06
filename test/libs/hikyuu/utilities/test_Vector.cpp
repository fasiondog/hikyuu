/*
 * test_Vector.cpp
 *
 *  Created on: 2012-9-4
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_utilities
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/utilities/Vector.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_Vector
 * @ingroup test_hikyuu_utilities
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Vector ) {
    /** @arg 默认构造，size为0, empty返回true */
    Vector<int> a;
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.empty());

    /** @arg 添加一个元素后，size为1，empty返回false,
     * []、at、front、back取值正确 */
    a.push_back(0);
    BOOST_CHECK(a.size() == 1);
    BOOST_CHECK(!a.empty());
    BOOST_CHECK(a[0] == 0);
    BOOST_CHECK(a.at(0) == 0);
    BOOST_CHECK(a.front() == 0);
    BOOST_CHECK(a.back() == 0);

    /** @arg 添加第二个元素后，size为2，empty返回false,
     * []、at、front、back取值正确 */
    a.push_back(1);
    BOOST_CHECK(a.size() == 2);
    BOOST_CHECK(!a.empty());
    BOOST_CHECK(a[1] == 1);
    BOOST_CHECK(a.at(1) == 1);
    BOOST_CHECK(a.front() == 0);
    BOOST_CHECK(a.back() == 1);

    /** @arg []赋值 */
    a[0] = 10;
    BOOST_CHECK(a[0] == 10);
    BOOST_CHECK(a.at(0) == 10);

    /** @arg at赋值 */
    a.at(0) = 100;
    BOOST_CHECK(a[0] == 100);
    BOOST_CHECK(a.at(0) == 100);

    /** @arg clear后，size为0，empty为true */
    a.clear();
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.empty());

    /** @arg clone生成的新对象和原对象不互相影响 */
    a.push_back(0);
    a.push_back(1);
    Vector<int> b = a.clone();
    b[0] = 1;
    BOOST_CHECK(a[0] == 0);
    BOOST_CHECK(b[0] == 1);

    /** @arg 默认赋值=，修改其中一个对象的值，另一个同步变更 */
    b = a;
    b[0] = 1;
    BOOST_CHECK(b[0] == 1);
    BOOST_CHECK(b[0] == a[0]);
    a.clear();
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.empty());
    BOOST_CHECK(b.size() == 0);
    BOOST_CHECK(b.empty());

    /** @arg 从标准vector赋值 */
    a.clear();
    vector<int> std_a;
    std_a.push_back(0);
    std_a.push_back(1);
    a = std_a;
    BOOST_CHECK(a.size() == 2);
    BOOST_CHECK(!a.empty());
    BOOST_CHECK(a[0] == 0);
    BOOST_CHECK(a[1] == 1);

    /** @arg 从标准vector构造 */
    Vector<int> x(std_a);
    BOOST_CHECK(x[0] == 0);
    BOOST_CHECK(x[1] == 1);

    /** @arg 从拷贝构造函数构造，原始数据变更，同步影响 */
    Vector<int> y(a);
    BOOST_CHECK(y[0] == 0);
    BOOST_CHECK(y[1] == 1);
    a[1] = 100;
    BOOST_CHECK(y[1] == 100);

    /** @arg 测试const_iterator */
    a.clear();
    for(int i = 0; i < 10; ++i) {
        a.push_back(i);
    }
    Vector<int>::const_iterator const_iter = a.begin();
    for(int i = 0; const_iter != a.end(); ++const_iter, ++i) {
        BOOST_CHECK((*const_iter) == i);
    }

    /** @arg 测试iterator */
    a.clear();
    for(int i = 9; i >= 0; --i) {
        a.push_back(i);
    }
    Vector<int>::iterator iter = a.begin();
    for(int i = 9; iter != a.end(); ++iter, --i) {
        BOOST_CHECK((*iter) == i);
    }
    iter = a.begin();
    for(int i = 0; iter != a.end(); ++iter, ++i) {
        (*iter) = i;
    }
    iter = a.begin();
    for(int i = 0; i < 10; ++i) {
        BOOST_CHECK(a[i] == i);
    }


}


/** @} */
