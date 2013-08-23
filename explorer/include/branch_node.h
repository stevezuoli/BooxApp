#ifndef BRANCH_NODE_H_
#define BRANCH_NODE_H_

#include "node.h"
#include "dir_node.h"
#include "mdb.h"
#include "onyx/cms/content_thumbnail.h"

using namespace cms;

namespace explorer {

namespace model {

/// Branch node represents a virtual container contains nodes.
/// It could be a directory or a category in cms library.
/// The branch node implements a directory based container.
class BranchNode :  public Node
{
public:
    BranchNode(Node * parent);
    virtual ~BranchNode();

public:
    virtual const NodePtrs& children(bool rescan = false);
    virtual NodePtrs& mutable_children(bool rescan = false);
    virtual NodePtrs& updateChildrenInfo();

    NodePtr node(const QString &name);
    NodePtr node(NodeType type);
    size_t nodePosition(Node * node);
    size_t nodePosition(const QString &name);

    Field sort_field() const { return by_field_; }
    SortOrder sort_order() const { return sort_order_; }

    void changeSortCriteria(Field by, SortOrder order);
    virtual bool sort(Field by, SortOrder order = ASCENDING);

    virtual bool search(const QStringList &filters, bool recursive, bool & stop);
    void clearNameFilters();
    const QStringList & name_filters() { return name_filters_; }

    ContentThumbnail & thumbDB(Node *node);

protected:
    virtual void updateChildren();
    virtual bool sort(NodePtrs &result, Field by, SortOrder order = ASCENDING);
    void clearChildren();

protected:
    Field by_field_;
    SortOrder sort_order_;
    QStringList name_filters_;
    NodePtrs children_;

    typedef shared_ptr<ContentThumbnail> ThumbnailPtr;
    QHash<QString, ThumbnailPtr> tdb_instances_;
};

/// Define simple sort type for all nodes.
struct LessByName
{
    bool operator()( const Node * a, const Node *b ) const
    {
        return (a->name().compare(b->name(), Qt::CaseInsensitive) < 0);
    }
};

struct GreaterByName
{
    bool operator()( const Node * a, const Node *b ) const
    {
        return (a->name().compare(b->name(), Qt::CaseInsensitive) > 0);
    }
};

struct LessByLastRead
{
    bool operator()( Node * a,  Node *b ) const
    {
        return (a->last_read() < b->last_read());
    }
};

struct GreaterByLastRead
{
    bool operator()( Node * a, Node *b ) const
    {
        return (a->last_read() > b->last_read());
    }
};

struct LessByNodetype
{
    bool operator()( Node * a,  Node *b ) const
    {
        return (a->type() < b->type());
    }
};

struct GreaterByNodeType
{
    bool operator()( Node * a, Node *b ) const
    {
        return (a->type() > b->type());
    }
};

}  // namespace model

}  // namespce explorer

#endif  // BRANCH_NODE_H_

