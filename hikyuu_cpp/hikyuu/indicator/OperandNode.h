/*
 * OperandNode.h
 *
 *  Created on: 2015年3月30日
 *      Author: fasiondog
 */

#ifndef INDICATOR_OPERANDNODE_H_
#define INDICATOR_OPERANDNODE_H_

#include "Indicator.h"

namespace hku {

class HKU_API OperandNode {
public:
    enum OPType {
        LEAF, ///<叶子节点
        OP,  /// OP(OP1,OP2) OP1->calcalue(OP2->calculate(ind))
        ADD, ///<加
        SUB, ///<减
        MUL, ///<乘
        DIV, ///<除
        EQ,  ///<等于
        GT,  ///<大于
        LT,  ///<小于
        NE,  ///<不等于
        GE,  ///<大于等于
        LE,  ///<小于等于
        AND, ///<与
        OR,  ///<或
        INVALID
    };

    typedef shared_ptr<OperandNode> OperandNodePtr;

public:
    OperandNode();
    OperandNode(const Indicator&);
    virtual ~OperandNode();

    void add(OPType, OperandNodePtr left, OperandNodePtr right);

    Indicator calculate(const Indicator&);

    const string& name() const { return m_name; }
    void name(const string& name) { m_name = name; }

    static string getOPTypeName(OPType);

private:
    OPType m_optype;
    Indicator m_ind;
    string m_name;
    OperandNodePtr m_left;
    OperandNodePtr m_right;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & BOOST_SERIALIZATION_NVP(m_optype);
        ar & BOOST_SERIALIZATION_NVP(m_ind);
        ar & BOOST_SERIALIZATION_NVP(m_name);
        ar & BOOST_SERIALIZATION_NVP(m_left);
        ar & BOOST_SERIALIZATION_NVP(m_right);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

typedef shared_ptr<OperandNode> OperandNodePtr;

} /* namespace hku */

#endif /* INDICATOR_OPERANDNODE_H_ */
