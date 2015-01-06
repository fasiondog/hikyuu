
#include <iostream>
#include "CodeTreeNode.h"

CodeTreeNode::CodeTreeNode(void) : m_stocktype(0){
}

CodeTreeNode::~CodeTreeNode(void){
}

void CodeTreeNode::addNode(const CodeTreeNodePtr& node) {
    if (!node) {
        return;
    }

    if (m_children.size() == 0) {
        m_children.push_back(node);
        return;
    }

    ContainerType::iterator iter = m_children.begin();
    for(; iter != m_children.end(); ++iter) {
        if( (*iter)->code() == node->code() ) {
            break;
        }
    }

    if (iter == m_children.end()) {
        m_children.push_back(node);
    }
}

CodeTreeNodePtr CodeTreeNode::getNode(char code) const {
    CodeTreeNodePtr result;
    ContainerType::const_iterator iter = m_children.begin();
    for(; iter != m_children.end(); ++iter) {
        if( (*iter)->code() == code ){
            result = *iter;
            break;
        }
    }
    return result;
}
