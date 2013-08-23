#ifndef FILE_NODE_H_
#define FILE_NODE_H_

#include "onyx/base/down_cast.h"
#include "node.h"
#include "onyx/cms/content_manager.h"
#include "onyx/cms/content_thumbnail.h"

using namespace cms;

namespace explorer {

namespace model {


/// File Node implement. The file node can contain
/// another virtual file system.
class FileNode :  public Node
{
public:
    FileNode(Node * parent, const QFileInfo & info);
    ~FileNode();

public:
    qint64 fileSize() const { return size_; }
    const QString & suffix() const;

    void update();
    ContentNode & metadata(bool force_update = false);
    bool hasRecord();
    bool updateMetadata(ContentNode &);
    bool remove(QString &error);

    bool thumbnail(QImage & thumbnail);

    // DRM
    bool isReturnable();
    bool hasReturned();
    unsigned int expriedDate();

private:
    ContentThumbnail & thumbDB();

private:
    enum MetaDataState
    {
        MD_INVALID = -1,
        MD_TOSCAN,              ///< Need to scan.
        MD_SCANNED              ///< Alaredy scanned.
    };

private:
    MetaDataState data_state_;
    mutable scoped_ptr<ContentNode> cms_node_;
    mutable qint64 size_;
    QString suffix_;
};

inline const QString & FileNode::suffix() const
{
    return suffix_;
}

}  // namespace model

}  // namespce explorer

#endif

