#ifndef FOLDER_NODE_H_
#define FOLDER_NODE_H_

#include "branch_node.h"
#include "dir_node.h"
#include "file_node.h"
#include "onyx/cms/content_manager.h"
#include "onyx/cms/content_thumbnail.h"

using namespace cms;

namespace explorer {

namespace model {

/// Folder node represents a virtual container contains nodes.
/// It could be a directory or a category in cms library.
/// The branch node implements a directory based container.
class FolderNode :  public BranchNode
{
public:
    FolderNode(Node * parent, const QString &root);
    virtual ~FolderNode();

public:
    const NodePtrs& children(bool rescan = false);
    NodePtrs& mutable_children(bool rescan = false);
    NodePtrs& updateChildrenInfo();

    size_t nodePosition(Node * node);
    size_t nodePosition(const QString &name);

    QString leafNodeName();

    bool sort(Field by, SortOrder order = ASCENDING);

    bool setRoot(const QString &root_dir);
    const QString & root() const { return root_dir_; }

    bool search(const QStringList &filters, bool recursive, bool & stop);
    void clearNameFilters();
    const QStringList & name_filters() { return name_filters_; }

    bool cdRoot();
    bool cd(const QString & dir);
    bool canGoUp();
    bool cdUp();

    bool fileSystemChanged();
    QString currentPath();

    bool isVirtualFolder() { return virtual_folder_; }

protected:
    virtual void updateChildren();
    virtual bool sort(NodePtrs &result, Field by, SortOrder order = ASCENDING);

    void scan(QDir &dir, const QStringList &filters, NodePtrs &result, bool sort = true);
    void collectDirectories(const QString &dir, QStringList & result);

protected:
    bool dirty_;
    QString root_dir_;
    QDir dir_;
    bool virtual_folder_;
};

// Report current path.
inline QString FolderNode::currentPath()
{
    return dir_.absolutePath();
}

inline QString FolderNode::leafNodeName()
{
    return dir_.dirName();
}

/// Define simple sort type for file nodes. Use down_cast here.
/// All nodes are in the vector. Need to check the type
/// as caller may want to compare DirNode with FileNode.
struct LessBySize
{
    bool operator()( const Node * a, const Node *b ) const
    {
        if (a->type() == NODE_TYPE_FILE &&
            b->type() == NODE_TYPE_FILE)
        {
            return down_cast<const FileNode *>(a)->fileSize() <
                   down_cast<const FileNode *>(b)->fileSize();
        }
        else if (a->type() == NODE_TYPE_FILE &&
                 b->type() == NODE_TYPE_DIRECTORY)
        {
            return false;
        }
        else if (a->type() == NODE_TYPE_DIRECTORY &&
                 b->type() == NODE_TYPE_FILE)
        {
            return true;
        }
        return (a->display_name().compare(b->display_name(), Qt::CaseInsensitive)) < 0;
    }
};

struct GreaterBySize
{
    bool operator()( const Node * a, const Node *b ) const
    {
        if (a->type() == NODE_TYPE_FILE &&
            b->type() == NODE_TYPE_FILE)
        {
            return down_cast<const FileNode *>(a)->fileSize() >
                   down_cast<const FileNode *>(b)->fileSize();
        }
        else if (a->type() == NODE_TYPE_FILE &&
                 b->type() == NODE_TYPE_DIRECTORY)
        {
            return true;
        }
        else if (a->type() == NODE_TYPE_DIRECTORY &&
                 b->type() == NODE_TYPE_FILE)
        {
            return false;
        }
        return (a->display_name().compare(b->display_name(), Qt::CaseInsensitive) > 0);
    }
};

struct LessByRating
{
    bool operator()( Node * a,  Node *b ) const
    {
        if (a->type() == NODE_TYPE_FILE &&
            b->type() == NODE_TYPE_FILE)
        {
            return down_cast< FileNode *>(a)->metadata().rating() <
                   down_cast< FileNode *>(b)->metadata().rating();
        }
        if (a->type() == NODE_TYPE_FILE &&
            b->type() == NODE_TYPE_DIRECTORY)
        {
            return true;
        }
        else if (a->type() == NODE_TYPE_DIRECTORY &&
                 b->type() == NODE_TYPE_FILE)
        {
            return false;
        }
        return false;
    }
};

struct GreaterByRating
{
    bool operator()( Node * a, Node *b ) const
    {
        if (a->type() == NODE_TYPE_FILE &&
            b->type() == NODE_TYPE_FILE)
        {
            return down_cast< FileNode *>(a)->metadata().rating() >
                   down_cast< FileNode *>(b)->metadata().rating();
        }
        if (a->type() == NODE_TYPE_FILE &&
            b->type() == NODE_TYPE_DIRECTORY)
        {
            return true;
        }
        else if (a->type() == NODE_TYPE_DIRECTORY &&
                 b->type() == NODE_TYPE_FILE)
        {
            return false;
        }
        return (a->display_name().compare(b->display_name(), Qt::CaseInsensitive) > 0);
    }
};

}  // namespace model

}  // namespce explorer

#endif  // BRANCH_NODE_H_

