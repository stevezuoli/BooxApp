#ifndef LIBRARY_NODE_H_
#define LIBRARY_NODE_H_

#include "node.h"
#include "branch_node.h"
#include "onyx/cms/content_manager.h"
#include "onyx/cms/content_thumbnail.h"

using namespace cms;

namespace explorer {

namespace model {

/// Library node. The library is constructed from a category in database.
class LibraryNode :  public BranchNode
{
public:
    LibraryNode(Node * parent, const cms_long category);
    ~LibraryNode();

public:
    NodePtrs& updateChildrenInfo();
    bool removeRecentDocument(const QString &path);
    void clearAll();

protected:
    virtual void updateChildren();

private:
    cms_long category_;
};


}  // namespace model

}  // namespce explorer

#endif  // LIBRARY_NODE_H_

