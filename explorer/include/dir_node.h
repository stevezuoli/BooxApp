#ifndef DIR_NODE_H_
#define DIR_NODE_H_

#include "node.h"
#include <QFileInfo>

namespace explorer {

namespace model {

/// Directory node.
class DirNode : public Node
{
public:
    DirNode(Node * parent, const QFileInfo & info);
    ~DirNode();

};

}   // namespace model

}   // namespace explorer

#endif
