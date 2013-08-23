
#include "games_node.h"
#include "branch_node.h"
#include "util/util.h"

namespace explorer {

namespace model {

GamesNode::GamesNode(Node * parent)
    : BranchNode(parent)
{
}

GamesNode::~GamesNode()
{
}


NodePtrs& GamesNode::updateChildrenInfo()
{
    updateChildren();
    return children_;
}

void GamesNode::updateChildren()
{
    util::DeletePtrContainer(&children_);

    Node *game = new Node(this);
    game->mutable_type() = NODE_TYPE_GAME_SUDOKU;
    game->mutable_name() = nodeName(game->type());
    game->mutable_display_name() = nodeDisplayName(game->type());
    children_.push_back(game);
}


}

}
