/*
 * test_CVAL.cpp
 *
 *  Created on: 2017年6月25日
 *      Author: Administrator
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_CVAL test_indicator_CVAL
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_CVAL") {
    Indicator result;

    PriceList d;
    d.push_back(6063);
    d.push_back(6041);
    d.push_back(6065);
    d.push_back(6078);
    d.push_back(6114);
    d.push_back(6121);
    d.push_back(6106);
    d.push_back(6101);
    d.push_back(6166);
    d.push_back(6169);
    d.push_back(6195);
    d.push_back(6222);
    d.push_back(6186);
    d.push_back(6214);
    d.push_back(6185);
    d.push_back(6209);
    d.push_back(6221);
    d.push_back(6278);
    d.push_back(6326);
    d.push_back(6347);

    /** @arg operator(ind) */
    Indicator ind = PRICELIST(d);
    CHECK_EQ(ind.size(), 20);
    result = CVAL(ind, 100);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.getResultNumber(), 1);
    for (size_t i = 0; i < ind.size(); ++i) {
        CHECK_EQ(result[i], 100);
    }

    /** @arg operator() */
    result = CVAL(100);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result[0], 100);

    /** @arg 测试discard, 未指定ind discard=2 */
    result = CVAL(100, 2);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.getParam<int>("discard"), 2);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_UNARY(std::isnan(result[0]));

    /** @arg 测试discard, ind discard=2 */
    ind = PRICELIST(d);
    CHECK_EQ(ind.size(), 20);
    result = CVAL(ind, 100, 2);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.getParam<int>("discard"), 2);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 2);
    CHECK_EQ(result.getResultNumber(), 1);
    for (size_t i = result.discard(); i < ind.size(); ++i) {
        CHECK_EQ(result[i], 100);
    }
}

/** @par 检测点 */
TEST_CASE("test_CVAL_nested_size_propagation") {
    // 修复前: CVAL(one, 10) 中 one=CVAL(10) → 走 Indicator::operator()
    //   alike(新空壳 ICval, 已计算 CVAL10) 为 true → 短路返回 m_imp 的克隆空壳
    //   (m_imp 未 calculate, size==0) → 违反 CVAL.h "其长度和输入的ind相同" 契约
    // 修复后: 复用已计算的 ind, size 正确传播

    Indicator one = CVAL(10);
    CHECK_EQ(one.size(), 1);
    CHECK_EQ(one[0], 10);
    CHECK_EQ(one.discard(), 0);

    // alike==true 嵌套(值相同) → 触发修复后的 return ind
    Indicator two = CVAL(one, 10);
    CHECK_EQ(two.size(), one.size());  // 修复前 0, 修复后 1
    CHECK_EQ(two.discard(), one.discard());
    CHECK_EQ(two[0], 10);
    // 白盒断言: 验证走的是 return ind 路径(指针复用), 而非克隆空壳
    CHECK_EQ(two.getImp().get(), one.getImp().get());

    // alike==false 嵌套(值不同 5!=10) → 走正常分支 B(clone + calculate)
    Indicator three = CVAL(two, 5);
    CHECK_EQ(three.size(), 1);
    CHECK_EQ(three.discard(), 0);
    CHECK_EQ(three[0], 5);
    // 白盒断言: 验证未走短路分支(独立克隆)
    CHECK_NE(three.getImp().get(), two.getImp().get());
}

/** @par 检测点 */
TEST_CASE("test_CVAL_nested_state_sharing") {
    // 修复使 alike==true 时返回 ind(共享底层节点), 而非 m_imp 的独立克隆.
    // 这是修复引入的语义变更: 返回值与 ind 共享同一 IndicatorImp.
    // hikyuu 不可变参数语义下, alike 已校验 m_params 相同,
    // setParam 触发原地重算(不立即改 buffer), 共享安全.
    // 本用例固化该共享行为为预期不变式.

    Indicator base = CVAL(100);
    CHECK_EQ(base.size(), 1);
    CHECK_EQ(base[0], 100);

    // 自嵌套: base.operator()(base) → alike(this==other) true → return ind(即 base)
    Indicator result = base(base);
    CHECK_EQ(result.size(), base.size());
    // 指针完全相同: AST 裁剪直接复用入参
    CHECK_EQ(result.getImp().get(), base.getImp().get());

    // 状态共享立案: 修改 result 的参数同步影响 base(共享同一 imp)
    result.setParam<double>("value", 999.0);
    CHECK_EQ(base.getParam<double>("value"), 999.0);
    CHECK_EQ(result.getParam<double>("value"), 999.0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_CVAL_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/CVAL.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = CVAL(CLOSE(kdata), 100.0);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ma1);
    }

    Indicator ma2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ma2);
    }

    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = 0; i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
