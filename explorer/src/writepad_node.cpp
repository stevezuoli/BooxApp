#include <algorithm>
#include <map>
#include "util/util.h"
#include "writepad_node.h"
#include "onyx/sys/sys_utils.h"

namespace explorer {

namespace model {

QStringList writePadFolders()
{
    QStringList list;
#ifndef _WINDOWS
    list << "/media/flash/text editor";
    list << "/media/sd/text editor";
#else
    list << "c://text editor";
    list << "d://text editor";
#endif
    return list;
}


WritePadContainer::WritePadContainer(Node * p)
    : BranchNode(p)
    , action_node_(this, QFileInfo())
{
    mutable_type() = NODE_TYPE_WRITEPAD_CONTAINER;

    actionNode().mutable_type() = NODE_TYPE_NEW_WRITEPAD;
    actionNode().mutable_name() = nodeName(NODE_TYPE_NEW_WRITEPAD);
    actionNode().mutable_display_name() = nodeDisplayName(NODE_TYPE_NEW_WRITEPAD);
}

WritePadContainer::~WritePadContainer()
{
    if (children_.size() > 0)
    {
        children_.erase(children_.begin());
    }
}

NodePtrs& WritePadContainer::updateChildrenInfo()
{
    updateChildren();
    WritePadContainer::sort(by_field_, sort_order_);
    return children_;
}

bool WritePadContainer::sort(Field by, SortOrder order)
{
    children_.erase(children_.begin());
    BranchNode::sort(children_, by, order);
    children_.insert(children_.begin(), &action_node_);
    changeSortCriteria(by, order);
    return true;
}

void WritePadContainer::clearAll()
{
    QStringList folders = writePadFolders();
    foreach(QString f, folders)
    {
        QStringList args;
        args << "-rf";
        args << f;
        sys::runScriptBlock("rm", args);
    }
}

void WritePadContainer::updateChildren()
{
    if (children_.size() > 0)
    {
        children_.erase(children_.begin());
    }
    util::DeletePtrContainer(&children_);
    NodePtr ptr = 0;

    ContentManager & db = mdb();
    ScopedDB<ContentManager> lock(db);

    QStringList folders = writePadFolders();
    QStringList filters;
    filters << "*.txt";
    foreach(QString folder, folders)
    {
        QDir dir(folder);
        QFileInfoList infos = dir.entryInfoList(filters, QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot);

        for(int i = 0; i < infos.size(); ++i)
        {
            ptr = new FileNode(this, infos.at(i));
            ptr->mutable_type() = NODE_TYPE_WRITEPAD;
            children_.push_back(ptr);
        }
    }

    children_.insert(children_.begin(), &action_node_);
}

}  // namespace model

}  // namespace explorer
