#include "dir_node.h"
#include <QDate>


namespace explorer {

namespace model {

DirNode::DirNode(Node * p,  const QFileInfo & info)
        : Node(p)
{
    mutable_absolute_path() = info.absoluteFilePath();
    mutable_name() = info.fileName();
    mutable_display_name() = info.fileName();
    mutable_type() = NODE_TYPE_DIRECTORY;
    mutable_last_read() = info.lastRead().toString(Node::DATE_FORMAT);
}

DirNode::~DirNode()
{
}

}  // namespace model

}  // namespace explorer
