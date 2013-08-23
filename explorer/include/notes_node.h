#ifndef NOTES_NODE_H_
#define NOTES_NODE_H_

#include "node.h"
#include "branch_node.h"
#include "onyx/cms/content_manager.h"

using namespace cms;

namespace explorer {

namespace model {

class NoteNode :  public Node
{
public:
    NoteNode(Node * parent, const NoteInfo & info);
    ~NoteNode();

public:
    const NoteInfo & info() const { return info_; }

private:
    NoteInfo info_;
};



/// Notes container.
class NotesNode :  public BranchNode
{
public:
    NotesNode(Node * parent);
    ~NotesNode();

public:
    NoteNode & actionNode() { return action_node_; }

    NodePtrs& updateChildrenInfo();
    bool sort(Field by, SortOrder order = ASCENDING);
    void clearAll();

protected:
    virtual void updateChildren();

private:
    NoteNode action_node_;
};


}  // namespace model

}  // namespce explorer

#endif  // NOTES_NODE_H_

