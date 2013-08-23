#include <algorithm>
#include <map>
#include "util/util.h"
#include "branch_node.h"
#include "explorer_conf.h"

namespace explorer {

namespace model {

BranchNode::BranchNode(Node * p)
    : Node(p)
    , by_field_(NAME)
    , sort_order_(ASCENDING)
    , children_()
{
}

BranchNode::~BranchNode()
{
    util::DeletePtrContainer(&children_);
}

const NodePtrs& BranchNode::children(bool rescan)
{
    if (rescan)
    {
        updateChildren();
    }
    return children_;
}

NodePtrs& BranchNode::mutable_children(bool rescan)
{
    if (rescan)
    {
        updateChildren();
    }
    return children_;
}

void BranchNode::updateChildren()
{
}

/// Update children node but does not re-generate the child list.
NodePtrs& BranchNode::updateChildrenInfo()
{
    return children_;
}

void BranchNode::clearChildren()
{
    util::DeletePtrContainer(&children_);
}

bool BranchNode::sort(NodePtrs &nodes, Field by, SortOrder order)
{
    switch (by)
    {
    case NAME:
        if (order == ASCENDING)
        {
            std::sort(nodes.begin(), nodes.end(), LessByName());
        }
        else if (order == DESCENDING)
        {
            std::sort(nodes.begin(), nodes.end(), GreaterByName());
        }
        break;
    case LAST_ACCESS_TIME:
        if (order == ASCENDING)
        {
            std::sort(nodes.begin(), nodes.end(), LessByLastRead());
        }
        else if (order == DESCENDING)
        {
            std::sort(nodes.begin(), nodes.end(), GreaterByLastRead());
        }
        break;
    case NODE_TYPE:
        if (order == ASCENDING)
        {
            std::sort(nodes.begin(), nodes.end(), LessByNodetype());
        }
        else if (order == DESCENDING)
        {
            std::sort(nodes.begin(), nodes.end(), GreaterByNodeType());
        }
        break;
    default:
        return false;
    }
    return true;
}


size_t BranchNode::nodePosition(Node * node)
{
    // check
    if (node == 0)
    {
        return INVALID_ORDER;
    }

    const NodePtrs& nodes  = children(false);
    NodePtrs::const_iterator it = find(nodes.begin(), nodes.end(), node);
    if (it == nodes.end())
    {
        return INVALID_ORDER;
    }
    else
    {
        return it - nodes.begin();
    }
}

NodePtr BranchNode::node(const QString &name)
{
    NodePtrs& all = mutable_children(false);
    for(NodePtrs::iterator it = all.begin(); it != all.end(); ++it)
    {
        if ((*it)->name() == name)
        {
            return *it;
        }
    }
    return 0;
}

NodePtr BranchNode::node(NodeType type)
{
    NodePtrs& all = mutable_children(false);
    for(NodePtrs::iterator it = all.begin(); it != all.end(); ++it)
    {
        if ((*it)->type() == type)
        {
            return *it;
        }
    }
    return 0;
}

/// Return node position by name.
size_t BranchNode::nodePosition(const QString &name)
{
    const NodePtrs& all = children(false);
    for(NodePtrs::const_iterator it = all.begin(); it != all.end(); ++it)
    {
        if ((*it)->name() == name)
        {
            return it - all.begin();
        }
    }
    return INVALID_ORDER;
}

/// Only change the sort criteria does not really sort.
void BranchNode::changeSortCriteria(Field by, SortOrder order)
{
    by_field_ = by;
    sort_order_ = order;
}

/// Change sort criteria and sort.
bool BranchNode::sort(Field by, SortOrder order)
{
    if (!sort(children_, by, order))
    {
        return false;
    }

    changeSortCriteria(by, order);
    return true;
}

/// Search from current directory by using the specified name filters.
/// Recursively search if needed.
bool BranchNode::search(const QStringList &name_filters,
                        bool recursively,
                        bool & stop)
{
    return false;
}

void BranchNode::clearNameFilters()
{
    name_filters_.clear();
}


/// Return the thumbnail database for the given node.
ContentThumbnail & BranchNode::thumbDB(Node *node)
{
    if (node)
    {
        QFileInfo info(node->absolute_path());
        if (tdb_instances_.contains(info.path()))
        {
            return *tdb_instances_.value(info.path());
        }
        // It could happen when the folder does not contain any
        // image files or viewer does not store any thumbnail there.
        // qWarning("Thumbnail db not found for %s", qPrintable(info.path()));
    }
    static ContentThumbnail dummy(QDir::root().absolutePath(), false);
    return dummy;
}

}  // namespace model

}  // namespace explorer
