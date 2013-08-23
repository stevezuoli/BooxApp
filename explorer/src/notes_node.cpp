#include <algorithm>
#include <map>
#include "util/util.h"
#include "notes_node.h"
#include "file_node.h"

namespace explorer {

namespace model {


NoteNode::NoteNode(Node * parent, const NoteInfo & info)
    : Node(parent)
    , info_(info)
{
    mutable_type() = NODE_TYPE_NOTE;
    mutable_name() = info.name();
    mutable_display_name() = info.name();
}

NoteNode::~NoteNode()
{
}

NotesNode::NotesNode(Node * p)
    : BranchNode(p)
    , action_node_(this, NoteInfo())
{
    mutable_type() = NODE_TYPE_NOTE_CONTAINER;
    actionNode().mutable_type() = NODE_TYPE_NEW_NOTE;
}

NotesNode::~NotesNode()
{
    if (children_.size() > 0)
    {
        children_.erase(children_.begin());
    }
}

NodePtrs& NotesNode::updateChildrenInfo()
{
    updateChildren();
    NotesNode::sort(by_field_, sort_order_);
    return children_;
}

bool NotesNode::sort(Field by, SortOrder order)
{
    children_.erase(children_.begin());
    BranchNode::sort(children_, by, order);
    children_.insert(children_.begin(), &action_node_);
    changeSortCriteria(by, order);
    return true;
}

void NotesNode::clearAll()
{
    ContentManager & db = mdb();
    ScopedDB<ContentManager> lock(db);

    // Retrieve the nodes from database.
    children_.erase(children_.begin());
    util::DeletePtrContainer(&children_);
    db.removeAllNotes();
}

void NotesNode::updateChildren()
{
    if (children_.size() > 0)
    {
        children_.erase(children_.begin());
    }
    util::DeletePtrContainer(&children_);
    NodePtr ptr = 0;

    ContentManager & db = mdb();
    ScopedDB<ContentManager> lock(db);
    cms::Notes notes;
    db.allNotes(notes);

    children_.reserve(notes.size() + 1);
    for(int i = 0; i < notes.size(); ++i)
    {
        ptr = new NoteNode(this, notes.at(i));
        ptr->mutable_type() = NODE_TYPE_NOTE;
        children_.push_back(ptr);
    }

    children_.insert(children_.begin(), &action_node_);
}

}  // namespace model

}  // namespace explorer
