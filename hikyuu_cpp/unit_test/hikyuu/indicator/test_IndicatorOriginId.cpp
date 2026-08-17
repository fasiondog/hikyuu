/*
 * test_IndicatorOriginId.cpp
 *
 *  Created on: 2026-08-12
 *      Author: woleigegg
 *
 *  origin_id（构造出身标识）专项测试：
 *  构造即唯一、clone 传承、序列化 load 持新号。
 *  注：CompiledFactorPlan::cloneNode 的传承（同一行拷贝语义）为 private static 路径，
 *  无公共探针，由代码审查覆盖；此处验证公共可断言的不变量。
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/REF.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>

#if HKU_SUPPORT_SERIALIZATION
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_indicator_OriginId test_indicator_OriginId
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

static uint64_t origin_id_of(const Indicator& ind) {
    return ind.getImp()->originId();
}

/** @par 检测点：独立构造必然不同号（公式碰撞类 bug 的根治基础） */
TEST_CASE("test_IndicatorOriginId_construct_unique") {
    Indicator c = CLOSE(), o = OPEN(), h = HIGH();

    // 同构不同参：B1 复现 bug 的全部代表族
    CHECK_NE(origin_id_of(MA(c, 20)), origin_id_of(MA(c, 60)));
    CHECK_NE(origin_id_of(REF(c, 1)), origin_id_of(REF(c, 5)));
    CHECK_NE(origin_id_of(CVAL(1.0)), origin_id_of(CVAL(2.0)));
    CHECK_NE(origin_id_of(CVAL(1.0)), origin_id_of(CVAL(1.0)));  // 同参独立构造也不同号

    // 结构不同：括号歧义与同形异算符
    CHECK_NE(origin_id_of((c + o) * h), origin_id_of(c + o * h));
    CHECK_NE(origin_id_of(c + o), origin_id_of(c - o));

    // 同一叶子对象被两棵树共享（菱形）时各自根节点仍独立
    Indicator shared = c + o;
    CHECK_NE(origin_id_of(shared * h), origin_id_of(shared + h));
}

/** @par 检测点：clone 传承（跨股票/批量路径缓存命中的前提） */
TEST_CASE("test_IndicatorOriginId_clone_inherit") {
    Indicator c = CLOSE(), o = OPEN(), h = HIGH();

    Indicator f = MA(c, 20) + CVAL(1.0);
    Indicator g = f.clone();
    CHECK_EQ(origin_id_of(f), origin_id_of(g));

    // 整树逐节点传承：根、左右子树、OP 节点的数据叶（MA(n)(ind) 经 operator() 挂为 m_right）
    auto fp = f.getImp(), gp = g.getImp();
    CHECK_EQ(fp->getLeftNode()->originId(), gp->getLeftNode()->originId());    // MA 子树
    CHECK_EQ(fp->getRightNode()->originId(), gp->getRightNode()->originId());  // CVAL 子树
    auto ma_l = fp->getLeftNode(), ma_g = gp->getLeftNode();
    CHECK_EQ(ma_l->getRightNode()->originId(), ma_g->getRightNode()->originId());  // CLOSE 叶

    // clone 的 clone 同号（克隆链）
    Indicator e = g.clone();
    CHECK_EQ(origin_id_of(f), origin_id_of(e));

    // clone 后与独立构造仍不同号
    CHECK_NE(origin_id_of(g), origin_id_of(MA(c, 20) + CVAL(1.0)));
}

#if HKU_SUPPORT_SERIALIZATION
/** @par 检测点：反序列化持新号（origin_id 不入档，load 走默认构造发新号） */
TEST_CASE("test_IndicatorOriginId_serialize_fresh") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ORIGIN_ID.xml";

    Indicator x1 = MA(CLOSE(), 20);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    Indicator x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    // load 出的节点持本进程新号：与原对象、与现场新构的同公式对象均不同
    CHECK_NE(origin_id_of(x2), origin_id_of(x1));
    CHECK_NE(origin_id_of(x2), origin_id_of(MA(CLOSE(), 20)));

    // 同一存档再次 load：又发新号，互不共享（安全方向）
    Indicator x3;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x3);
    }
    CHECK_NE(origin_id_of(x3), origin_id_of(x2));
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
