#ifndef WRITE_PAD_NODE_H_
#define WRITE_PAD_NODE_H_

#include "node.h"
#include "branch_node.h"
#include "onyx/cms/content_manager.h"
#include "file_node.h"

using namespace cms;

namespace explorer {

namespace model {

/// WritePad container.
class WritePadContainer :  public BranchNode
{
public:
    WritePadContainer(Node * parent);
    ~WritePadContainer();

public:
    FileNode & actionNode() { return action_node_; }

    NodePtrs& updateChildrenInfo();
    bool sort(Field by, SortOrder order = ASCENDING);
    void clearAll();

protected:
    virtual void updateChildren();

private:
    FileNode action_node_;
};


}  // namespace model

}  // namespce explorer

#endif  // WRITE_PAD_NODE_H_

