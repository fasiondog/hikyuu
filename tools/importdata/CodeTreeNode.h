/*
 * CodeTreeNode.h
 *
 *  Created on: 2010-10-26
 *      Author: fasiondog
 */

#include <list>
#include <boost/shared_ptr.hpp>

class CodeTreeNode{
public:
    CodeTreeNode(void);
    CodeTreeNode(char code, unsigned int stocktype)
        : m_code(code), m_stocktype(stocktype) {}
    virtual ~CodeTreeNode(void);

    typedef boost::shared_ptr<CodeTreeNode> CodeTreeNodePtr;
    typedef std::list<CodeTreeNodePtr> ContainerType; 
    
    char code() const { return m_code; }
    unsigned int stocktype() const { return m_stocktype; }
    void stocktype(unsigned int stocktype) { m_stocktype = stocktype; }

    void addNode(const CodeTreeNodePtr& node);
    CodeTreeNodePtr getNode(char) const;

private:
    char m_code;
    unsigned int m_stocktype;
    ContainerType m_children;

};

typedef boost::shared_ptr<CodeTreeNode> CodeTreeNodePtr;
