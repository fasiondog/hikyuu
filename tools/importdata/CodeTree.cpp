
#include <iostream>
#include "CodeTree.h"

CodeTree::CodeTree(void){
    m_head = CodeTreeNodePtr(new CodeTreeNode('H', 0));
}

CodeTree::~CodeTree(void){

}

void CodeTree::addCode(const std::string& code, unsigned int stocktype) {
    size_t len = code.length();
    if (len == 0 ) {
        return;
    }

    CodeTreeNodePtr preNode = m_head;
    CodeTreeNodePtr node;
    for(size_t i = 0; i < len - 1; ++i){
        node = preNode->getNode(code.at(i));
        if( !node ) {
            CodeTreeNodePtr temp(new CodeTreeNode(code.at(i), 0));
            preNode->addNode(temp);
            preNode = temp;
        } else {
            preNode = node;
        }
    }

    node = preNode->getNode(code.at(len-1));
    if (node) {
        node->stocktype(stocktype);
    } else {
        preNode->addNode(CodeTreeNodePtr(new CodeTreeNode(code.at(len-1), stocktype)));
    }
}

unsigned int CodeTree::getStockType(const std::string& code) const {
    size_t len = code.length();
    CodeTreeNodePtr preNode = m_head;
    CodeTreeNodePtr matchNode = m_head;
    for (size_t i = 0; i < len; ++i) {
        CodeTreeNodePtr node = preNode->getNode(code.at(i));
        if (!node) {
            break;
        }

        if (node->stocktype() != 0) {
            matchNode = node;
        }
        preNode = node;        
    }
    return matchNode->stocktype();
}
