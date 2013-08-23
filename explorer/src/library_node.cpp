#include <algorithm>
#include <map>
#include "util/util.h"
#include "library_node.h"
#include "file_node.h"
#include "explorer_conf.h"

namespace explorer {

namespace model {


LibraryNode::LibraryNode(Node * p, const cms_long category)
    : BranchNode(p)
    , category_(category)
{
    changeSortCriteria(LAST_ACCESS_TIME, DESCENDING);
}

LibraryNode::~LibraryNode()
{
}

NodePtrs& LibraryNode::updateChildrenInfo()
{
    updateChildren();
    return children_;
}

bool LibraryNode::removeRecentDocument(const QString &path)
{
    ContentManager & db = mdb();
    return db.removeRecentDocument(path);
}

void LibraryNode::clearAll()
{
    ContentManager & db = mdb();

    // Retrieve the nodes from database.
    util::DeletePtrContainer(&children_);
    db.clearRecentDocuments();
}

void LibraryNode::updateChildren()
{
    ContentManager & db = mdb();

    // Retrieve the nodes from database.
    util::DeletePtrContainer(&children_);
    NodePtr ptr = 0;

    cms_ids all;
    db.getRecentDocuments(all);

    ContentNode content;
    children_.reserve(all.size());
    for(cms_ids::iterator it = all.begin(); it != all.end(); ++it)
    {
        if (!db.getContentNode(*it, content))
        {
            continue;
        }

        QFileInfo info(content.location(), content.name());

        if (info.exists() && info.isFile())
        {
            ptr = new FileNode(this, info);
            children_.push_back(ptr);
        }
    }

    // Sort
    changeSortCriteria(LAST_ACCESS_TIME, DESCENDING);
    sort(sort_field(), sort_order());
}

}  // namespace model

}  // namespace explorer
