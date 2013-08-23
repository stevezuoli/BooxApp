#ifndef EXPLORER_MODEL_NODE_H__
#define EXPLORER_MODEL_NODE_H__

#include "onyx/base/base.h"
#include <QString>
#include "node_types.h"

namespace explorer
{

namespace model
{

class Node;
typedef Node* NodePtr;
typedef vector<NodePtr> NodePtrs;
typedef NodePtrs::iterator NodePtrsIter;
typedef QStringList ModelPath;

/// Node base class. The abstract node can represent
/// - File system, such as file and directory including removable storage.
/// - Nodes collection and so on.
class Node
{
public:
    Node(Node* parent);
    virtual ~Node(void);

    /// Retrieve parent node.  Every node has a parent except the root
    /// for which parent_ == NULL.
    const Node* parent() const { return parent_; }
    Node* mutable_parent() const { return parent_; }

    /// The name of the node. It can be served as the
    /// unique id of the node if possible.
    /// For file or directory, the name can be the same
    /// as the file name or directory name.
    const QString & name() const { return name_; }
    QString & mutable_name() { return name_; }

    /// Metadata or title for display.
    const QString & display_name() const { return display_name_; }
    QString & mutable_display_name() { return display_name_; }

    const QString & description() const { return description_; }
    QString & mutable_description() { return description_; }

    const QString & last_read() const { return last_read_; }
    QString & mutable_last_read() { return last_read_; }

    /// Absolute path in file system. It can also be url if necessary.
    /// When it's a path, it does not contain file:/// prefix.
    /// For url, it's caller's responsibility to interpret the url.
    const QString & absolute_path() const { return absolute_path_; }
    QString& mutable_absolute_path() { return absolute_path_; }

    /// Retrieve node type. It's used to identify node type in runtime.
    /// Note: different node type can share the same implementation.
    /// For example, USB and SD node can use the DirNode but return
    /// NODE_TYPE_USB and NODE_TYPE_SD.
    NodeType type() const { return type_; }
    NodeType & mutable_type() { return type_; }

public:
    static const size_t INVALID_ORDER;
    static const QString DATE_FORMAT;

private:
    Node* parent_;
    QString name_;
    QString display_name_;
    QString absolute_path_;
    QString description_;
    QString last_read_;
    NodeType type_;
    NO_COPY_AND_ASSIGN(Node);
};

QString nodeDisplayName(NodeType type);
QString nodeName(NodeType type);


}  // namespace model

}  // namespace explorer

#endif  // EXPLORER_MODEL_NODE_H__



