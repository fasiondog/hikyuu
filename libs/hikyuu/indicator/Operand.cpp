/*
 * Operand.cpp
 *
 *  Created on: 2015年3月30日
 *      Author: Administrator
 */

#include <hikyuu/indicator/Operand.h>

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const Operand& op) {
    os << "OP(" << op.name() << ")";
    return os;
}

Operand::~Operand() {

}

Operand::Operand(){

}

Operand::Operand(const Indicator& ind) {
    m_root = make_shared<OperandNode>(ind);
}

Operand::Operand(const Operand& op) {
    m_root = op.m_root;
}

Operand& Operand::operator=(const Operand& op) {
    if (this == &op)
        return *this;

    if (m_root != op.m_root)
        m_root = op.m_root;

    return *this;
}

Operand::Operand(const Operand& left, const Operand& right) {
    m_root = make_shared<OperandNode>();
    m_root->add(OperandNode::OP, left.m_root, right.m_root);
}

Indicator Operand::calculate(const Indicator& ind) {
    return m_root ? m_root->calculate(ind) : Indicator();
}

string Operand::name() const {
    return m_root ? m_root->name(): "";
}

void Operand::name(const string& name) {
    if (m_root)
        m_root->name(name);
}

Operand Operand::operator()(const Operand& op) {
    return Operand(*this, op);
}

Operand Operand::operator+(const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::ADD, m_root, op.m_root);
    return result;
}

Operand Operand::operator-(const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::SUB, m_root, op.m_root);
    return result;
}

Operand Operand::operator*(const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::MUL, m_root, op.m_root);
    return result;
}

Operand Operand::operator/(const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::DIV, m_root, op.m_root);
    return result;
}

} /* namespace hku */
