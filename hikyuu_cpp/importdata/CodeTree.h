
#ifndef CODETREE_H_
#define CODETREE_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include "CodeTreeNode.h"

class CodeTree
{
public:
    CodeTree(void);
    virtual ~CodeTree(void);

    void addCode(const std::string& code, unsigned int stocktype);
    unsigned int getStockType(const std::string& code) const;

private:
    CodeTreeNodePtr m_head;
};

typedef boost::shared_ptr<CodeTree> CodeTreePtr;

#endif /* CODETREE_H_ */
