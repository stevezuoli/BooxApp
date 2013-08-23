#ifndef SHORTCUTS_NODE_H_
#define SHORTCUTS_NODE_H_

#include "node.h"
#include "branch_node.h"
#include "onyx/cms/content_manager.h"

using namespace cms;

namespace explorer {

namespace model {

/// Shortcuts node.
class ShortcutsNode :  public BranchNode
{
public:
    ShortcutsNode(Node * parent);
    ~ShortcutsNode();

public:
    NodePtrs& updateChildrenInfo();
    void clearAll();

    bool removeShortcut(const QString &path);

protected:
    virtual void updateChildren();

};


}  // namespace model

}  // namespce explorer

#endif  // SHORTCUTS_NODE_H_

