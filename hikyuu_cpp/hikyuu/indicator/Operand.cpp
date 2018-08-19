/*
 * Operand.cpp
 *
 *  Created on: 2015年3月30日
 *      Author: fasiondog
 */

#include "Operand.h"
#include "crt/CVAL.h"

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

Operand Operand::operator==(const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::EQ, m_root, op.m_root);
    return result;
}

Operand Operand::operator!=(const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::NE, m_root, op.m_root);
    return result;
}

Operand Operand::operator>(const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::GT, m_root, op.m_root);
    return result;
}

Operand Operand::operator<(const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::LT, m_root, op.m_root);
    return result;
}

Operand Operand::operator>=(const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::GE, m_root, op.m_root);
    return result;
}

Operand Operand::operator<=(const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::LE, m_root, op.m_root);
    return result;
}

HKU_API Operand operator+(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::ADD, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand operator+(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::ADD, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand operator-(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::SUB, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand operator-(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::SUB, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand operator*(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::MUL, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand operator*(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::MUL, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand operator/(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::DIV, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand operator/(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::DIV, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand operator>(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::GT, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand operator>(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::GT, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand operator<(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::LT, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand operator<(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::LT, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand operator>=(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::GE, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand operator>=(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::GE, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand operator<=(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::LE, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand operator<=(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::LE, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand operator==(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::EQ, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand operator==(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::EQ, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand operator!=(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::NE, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand operator!=(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::NE, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand OP_AND(const Operand& op1, const Operand& op2) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::AND, op1.m_root, op2.m_root);
    return result;
}

HKU_API Operand OP_AND(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::AND, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand OP_AND(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::AND, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

HKU_API Operand OP_OR(const Operand& op1, const Operand& op2) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::OR, op1.m_root, op2.m_root);
    return result;
}

HKU_API Operand OP_OR(const Operand& op, price_t val) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::OR, op.m_root, Operand(CVAL(val)).m_root);
    return result;
}

HKU_API Operand OP_OR(price_t val, const Operand& op) {
    Operand result;
    result.m_root = make_shared<OperandNode>();
    result.m_root->add(OperandNode::OR, Operand(CVAL(val)).m_root, op.m_root);
    return result;
}

} /* namespace hku */
