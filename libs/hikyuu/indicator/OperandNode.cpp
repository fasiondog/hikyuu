/*
 * OperandNode.cpp
 *
 *  Created on: 2015年3月30日
 *      Author: fasiondog
 */

#include "OperandNode.h"
#include "crt/IND_LOGIC.h"

namespace hku {

string OperandNode::getOPTypeName(OPType op) {
    if (LEAF == op) {
        return "LEAF";
    } else if (OP == op) {
        return "OP";
    } else if (ADD == op) {
        return "ADD";
    } else if (SUB == op) {
        return "SUB";
    } else if (MUL == op) {
        return "MUL";
    } else if (DIV == op) {
        return "DIV";
    } else if (EQ == op) {
        return "EQ";
    } else if (GT == op) {
        return "GT";
    } else if (LT == op) {
        return "LT";
    } else if (NE == op) {
        return "NE";
    } else if (GE == op) {
        return "GE";
    } else if (LE == op) {
        return "LE";
    } else if (AND == op) {
        return "AND";
    } else if (OR == op) {
        return "OR";
    } else {
        return "INVALID";
    }
}

OperandNode::OperandNode()
: m_optype(LEAF) {

}

OperandNode::OperandNode(const Indicator& ind)
: m_optype(LEAF),
  m_ind(ind) {

}

OperandNode::~OperandNode() {

}

void OperandNode::add(OPType op, OperandNodePtr left, OperandNodePtr right) {
    if (op == LEAF || op >= INVALID || !left || !right) {
        HKU_ERROR("Wrong used [OperandNode::add]");
        return;
    }

    m_optype = op;
    m_left = left;
    m_right = right;
}


Indicator OperandNode::calculate(const Indicator& ind) {
    Indicator result;

    switch (m_optype) {
    case LEAF:
        result = m_ind(ind);
        break;

    case OP:
        result = m_left->calculate(m_right->calculate(ind));
        break;

    case ADD:
        result = m_left->calculate(ind) + m_right->calculate(ind);
        break;

    case SUB:
        result = m_left->calculate(ind) - m_right->calculate(ind);
        break;

    case MUL:
        result = m_left->calculate(ind) * m_right->calculate(ind);
        break;

    case DIV:
        result = m_left->calculate(ind) / m_right->calculate(ind);
        break;

    case EQ:
        result = m_left->calculate(ind) == m_right->calculate(ind);
        break;

    case NE:
        result = m_left->calculate(ind) != m_right->calculate(ind);
        break;

    case GT:
        result = m_left->calculate(ind) > m_right->calculate(ind);
        break;

    case LT:
        result = m_left->calculate(ind) < m_right->calculate(ind);
        break;

    case GE:
        result = m_left->calculate(ind) >= m_right->calculate(ind);
        break;

    case LE:
        result = m_left->calculate(ind) <= m_right->calculate(ind);
        break;

    case AND:
        result = IND_AND(m_left->calculate(ind), m_right->calculate(ind));
        break;

    case OR:
        result = IND_OR(m_left->calculate(ind), m_right->calculate(ind));
        break;
    }

    if (m_name != "")
        result.name(m_name);

    return result;
}

} /* namespace hku */
