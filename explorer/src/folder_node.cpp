#include <algorithm>
#include <map>
#include "util/util.h"
#include "folder_node.h"
#include "explorer_conf.h"
#include "file_node.h"

namespace explorer {

namespace model {

FolderNode::FolderNode(Node * p, const QString &root)
    : BranchNode(p)
    , dirty_(true)
    , root_dir_(root)
    , dir_()
    , virtual_folder_(false)
{
}

FolderNode::~FolderNode()
{
    util::DeletePtrContainer(&children_);
    closeMdb();
}

const NodePtrs& FolderNode::children(bool rescan)
{
    if (dirty_ || rescan)
    {
        updateChildren();
    }
    return children_;
}

NodePtrs& FolderNode::mutable_children(bool rescan)
{
    if (dirty_ || rescan)
    {
        updateChildren();
    }
    return children_;
}

NodePtrs& FolderNode::updateChildrenInfo()
{
    for(NodePtrsIter iter = children_.begin(); iter != children_.end(); ++iter)
    {
        // Update information if it's file node.
        if ((*iter)->type() == NODE_TYPE_FILE)
        {
            down_cast<FileNode *>(*iter)->update();
        }
    }
    sort(children_, by_field_, sort_order_);
    return children_;
}

void FolderNode::updateChildren()
{
    util::DeletePtrContainer(&children_);
    name_filters_.clear();
    scan(dir_, name_filters_, children_, true);
    dirty_ = false;
}

void FolderNode::scan(QDir &dir,
                      const QStringList &name_filters,
                      NodePtrs &result,
                      bool sort_list)
{
    bool create_db = false;
    NodePtr ptr = 0;

    QDir::Filters filters = QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot;
    QFileInfoList all = dir.entryInfoList(name_filters, filters);
    for(QFileInfoList::iterator iter = all.begin(); iter != all.end(); ++iter)
    {
        if (iter->isFile())
        {
            ptr = new FileNode(this, *iter);
            result.push_back(ptr);
        }
        else if (iter->isDir())
        {
            ptr = new DirNode(this, *iter);
            result.push_back(ptr);
        }

        if (!create_db || cms::isImage(iter->completeSuffix()))
        {
            create_db = true;
        }
    }

    // Update the thumbnail db.
    // Always reset the thumbnail database when it's dirty.
    // Otherwise the old database connection is removed incorrectly.
    if (!tdb_instances_.contains(dir.absolutePath()) && create_db)
    {
        // We don't open the database automatically.
        // The caller will open it. The db is opened only when needed.
        // It's necessary especially when the database is located in
        // removable storage like SD card or USB stick.
        // qDebug("create thumbnail for %s", qPrintable(dir.absolutePath()));
        ThumbnailPtr ptr(new ContentThumbnail(dir.absolutePath(), false));
        tdb_instances_.insert(dir.absolutePath(), ptr);
    }

    // Sort.
    if (sort_list)
    {
        sort(result, by_field_, sort_order_);
    }
}

bool FolderNode::sort(NodePtrs &nodes, Field by, SortOrder order)
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
    case SIZE:
        if (order == ASCENDING)
        {
            std::sort(nodes.begin(), nodes.end(), LessBySize());
        }
        else if (order == DESCENDING)
        {
            std::sort(nodes.begin(), nodes.end(), GreaterBySize());
        }
        break;
    case RATING:
        if (order == ASCENDING)
        {
            std::sort(nodes.begin(), nodes.end(), LessByRating());
        }
        else if (order == DESCENDING)
        {
            std::sort(nodes.begin(), nodes.end(), GreaterByRating());
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


size_t FolderNode::nodePosition(Node * node)
{
    // check
    if (node == 0)
    {
        return INVALID_ORDER;
    }

    const NodePtrs& nodes  = children();
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

/// Exactly match.
size_t FolderNode::nodePosition(const QString &name)
{
    const NodePtrs& all = children();
    for(NodePtrs::const_iterator it = all.begin(); it != all.end(); ++it)
    {
        if ((*it)->name() == name)
        {
            return it - all.begin();
        }
    }
    return INVALID_ORDER;
}

/// Change sort criteria and sort.
bool FolderNode::sort(Field by,
                      SortOrder order)
{
    if (!sort(children_, by, order))
    {
        return false;
    }

    by_field_ = by;
    sort_order_ = order;
    return true;
}

bool FolderNode::setRoot(const QString &root_dir)
{
    root_dir_ = root_dir;
    return true;
}

void FolderNode::collectDirectories(const QString &path,
                                    QStringList & result)
{
    QDir dir(path);
    QDir::Filters filters = QDir::Dirs|QDir::NoDotAndDotDot;
    QFileInfoList all = dir.entryInfoList(filters);
    foreach(QFileInfo info, all)
    {
        result.push_back(info.absoluteFilePath());
    }
}

/// Search from current directory by using the specified name filters.
/// Recursively search if needed.
bool FolderNode::search(const QStringList &name_filters,
                        bool recursive,
                        bool & stop)
{
    name_filters_ = name_filters;
    util::DeletePtrContainer(&children_);

    // Search from current directory.
    if (!recursive)
    {
        scan(dir_, name_filters_, children_, true);
        return true;
    }

    scan(dir_, name_filters_, children_, false);

    // Collect all directories.
    QStringList targets;
    collectDirectories(dir_.absolutePath(), targets);

    while (!targets.isEmpty())
    {
        QStringList sub;
        foreach(QString t, targets)
        {
            // Search the directory.
            NodePtrs tmp;
            QDir dir(t);
            scan(dir, name_filters_, tmp, false);
            children_.insert(children_.end(), tmp.begin(), tmp.end());
            collectDirectories(t, sub);

            // Check if caller wants to stop.
            QApplication::processEvents();
            if (stop)
            {
                sub.clear();
                targets.clear();
                break;
            }
        }
        targets = sub;
    }

    if (stop)
    {
        return false;
    }

    // Sort.
    sort(children_, by_field_, sort_order_);
    dirty_ = false;
    return true;
}

void FolderNode::clearNameFilters()
{
    name_filters_.clear();
}

bool FolderNode::cdRoot()
{
    if (dir_.cd(root()))
    {
        mutable_absolute_path() = dir_.absolutePath();
        clearNameFilters();
        dirty_ = true;
        return true;
    }
    return false;
}

/// Changes the QDir's directory to dirName.
/// Returns true if the new directory exists and is readable;
/// otherwise returns false. Note that the logical cd() operation
/// is not performed if the new directory does not exist.
bool FolderNode::cd(const QString & new_dir)
{
    if (dir_.cd(new_dir))
    {
        mutable_absolute_path() = dir_.absolutePath();
        clearNameFilters();
        dirty_ = true;
        return true;
    }
    return cdRoot();
}

bool FolderNode::canGoUp()
{
    // Not a nice way.
    if (dir_.absolutePath().size() <= root_dir_.size())
    {
        return false;
    }
    return true;
}

bool FolderNode::cdUp()
{
    if (!canGoUp())
    {
        return false;
    }

    if (dir_.cdUp())
    {
        mutable_absolute_path() = dir_.absolutePath();
        clearNameFilters();
        dirty_ = true;
        return true;
    }
    return false;
}

// This function is called when mount point is changed.
bool FolderNode::fileSystemChanged()
{
    // Make sure we close the thumbnail db. Actually it's
    // not really necesary, as the connection is always closed
    // when caller get a thumbnail.
    tdb_instances_.clear();
    clearNameFilters();

    return cdRoot();
}



}  // namespace model

}  // namespace explorer
