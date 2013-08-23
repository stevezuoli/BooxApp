#ifndef GAMES_NODE_H_
#define GAMES_NODE_H_

#include "node.h"
#include "branch_node.h"

namespace explorer {

namespace model {

/// Game node.
class GamesNode :  public BranchNode
{
public:
    GamesNode(Node * parent);
    ~GamesNode();

public:
    NodePtrs& updateChildrenInfo();

protected:
    virtual void updateChildren();

private:

};


}  // namespace model

}  // namespce explorer

#endif  // GAMES_NODE_H_

