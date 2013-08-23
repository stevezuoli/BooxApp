#ifndef WEBSITES_NODE_H_
#define WEBSITES_NODE_H_

#include "node.h"
#include "branch_node.h"

namespace explorer {

namespace model {

/// Websites node. 
class WebSitesNode :  public BranchNode
{
public:
    WebSitesNode(Node * parent);
    ~WebSitesNode();

public:
    NodePtrs& updateChildrenInfo();
    bool removeWebSite(const QString &path);
    bool addWebSite(const QString & str_url);
    void clearAll();

protected:
    virtual void updateChildren();

};


}  // namespace model

}  // namespce explorer

#endif  // WEBSITES_NODE_H_

