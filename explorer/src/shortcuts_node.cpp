#include <algorithm>
#include <map>
#include "util/util.h"
#include "shortcuts_node.h"
#include "file_node.h"

namespace explorer {

namespace model {


ShortcutsNode::ShortcutsNode(Node * p)
    : BranchNode(p)
{
}

ShortcutsNode::~ShortcutsNode()
{
}

NodePtrs& ShortcutsNode::updateChildrenInfo()
{
    updateChildren();
    return children_;
}

void ShortcutsNode::clearAll()
{
    ContentManager & db = mdb();

    // Retrieve the nodes from database.
    util::DeletePtrContainer(&children_);
    db.clearRecentDocuments();
}

void ShortcutsNode::updateChildren()
{
    ContentManager & db = mdb();

    QStringList srcs, targets;
    db.allShortcuts(srcs, targets);

    // Retrieve the nodes from database.
    util::DeletePtrContainer(&children_);
    NodePtr ptr = 0;

    children_.reserve(targets.size());
    for(int i = 0; i < targets.size(); ++i)
    {
        QFileInfo info(srcs.at(i));
        if (!info.exists())
        {
            continue;
        }
        if (info.isFile())
        {
            ptr = new FileNode(this, info);
            ptr->mutable_display_name() = targets.at(i);
            children_.push_back(ptr);
        }
        else if (info.isDir())
        {
            ptr = new DirNode(this, info);
            ptr->mutable_display_name() = targets.at(i);
            children_.push_back(ptr);
        }
    }
}

bool ShortcutsNode::removeShortcut(const QString &path)
{
    ContentManager & db = mdb();
    return db.unlinkBySource(path);
}

}  // namespace model

}  // namespace explorer
