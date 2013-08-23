#include <limits.h>
#include <algorithm>
#include <map>
#include "file_node.h"
#include "folder_node.h"
#include "onyx/cms/cms_tags.h"

namespace explorer {

namespace model {

FileNode::FileNode(Node * parent, const QFileInfo & info)
        : Node(parent)
        , data_state_(MD_TOSCAN)
        , cms_node_()
        , size_(info.size())
        , suffix_(info.suffix())
{
    mutable_absolute_path() = info.absoluteFilePath();
    mutable_name() = info.fileName();
    mutable_display_name() = info.fileName();
    mutable_type() = NODE_TYPE_FILE;
    mutable_last_read() = info.lastRead().toString(DATE_FORMAT);

    // Should not happen now. As the internal flash and removable
    // storage use FAT32 file system.
    if (info.isSymLink())
    {
        qWarning("Symbolink found, should not happen");
    }
}

FileNode::~FileNode()
{
}

ContentThumbnail & FileNode::thumbDB()
{
    return down_cast<BranchNode *>(mutable_parent())->thumbDB(this);
}

/// Update all information.
void FileNode::update()
{
    QFileInfo info(absolute_path());
    mutable_last_read() = info.lastRead().toString(DATE_FORMAT);
    metadata(true);
}

ContentNode & FileNode::metadata(bool force_update)
{
    if (!cms_node_)
    {
        cms_node_.reset(new ContentNode);
    }

    if (force_update || data_state_ == MD_TOSCAN)
    {
        mdb().getContentNode(*cms_node_, absolute_path(), false);
        data_state_ = MD_SCANNED;
    }
    return *cms_node_;
}

bool FileNode::hasRecord()
{
    return (metadata(true).id() != CMS_INVALID_ID);
}

bool FileNode::updateMetadata(ContentNode &data)
{
    // Make sure the content node exist.
    if (data.id() < 0)
    {
        mdb().createContentNode(data);
    }
    return mdb().updateContentNode(data);
}

bool FileNode::remove(QString &error)
{
    ContentNode node;
    if (mdb().getContentNode(node, absolute_path(), false))
    {
        mdb().removeContentNode(node);
    }

    QFile file(absolute_path());
    if (!file.remove())
    {
        error = file.errorString();
        return false;
    }
    return true;
}

bool FileNode::thumbnail(QImage & thumbnail)
{
    ContentThumbnail & db = thumbDB();
    ScopedDB<ContentThumbnail> lock(db);
    if (!db.loadThumbnail(name(), cms::THUMBNAIL_LARGE, thumbnail))
    {
        return false;
    }
    return true;
}

/// Check the document is returnable or not.
bool FileNode::isReturnable()
{
    ContentNode & node = metadata(true);
    QVariantMap attributes;
    node.attributes(attributes);
    if (attributes.contains(cms::CMS_IS_RETURNABLE))
    {
        if (attributes.value(cms::CMS_IS_RETURNABLE).toBool())
        {
            if (attributes.contains(CMS_HAS_RETURNED))
            {
                return !attributes.value(CMS_HAS_RETURNED).toBool();
            }
        }
    }
    return false;
}

/// Steve: Check whether the document has been returned
bool FileNode::hasReturned()
{
    ContentNode & node = metadata(true);
    QVariantMap attributes;
    node.attributes(attributes);
    if (attributes.contains(CMS_HAS_RETURNED))
    {
        return attributes.value(CMS_HAS_RETURNED).toBool();
    }
    return false;
}

/// Return the expired date in second since 1970-01-01T00:00:00
unsigned int FileNode::expriedDate()
{
    ContentNode & node = metadata(true);
    QVariantMap attributes;
    node.attributes(attributes);
    if (attributes.contains(cms::CMS_EXPIRED_DATE))
    {
        return attributes.value(cms::CMS_EXPIRED_DATE).toULongLong() / 1000;
    }
    return 0;
}

}  // namespace model

}  // namespace explorer
