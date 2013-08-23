#include "model_tree.h"
#include "file_node.h"
#include "library_node.h"
#include "folder_node.h"
#include "explorer_conf.h"
#include "onyx/cms/content_manager.h"
#include "onyx/base/device.h"

namespace explorer
{

namespace model
{

ModelTree::ModelTree()
: root_node_(0)
, current_node_(0)
{
    initialize();
}

ModelTree::~ModelTree()
{
    saveSettings();
}

/// Initialize data model. It will initialize the data model.
/// By default it will insert all nodes that at top and second level
/// into the node tree.
/// If preference data is available, it will also initialize according
/// to the preference data.
void ModelTree::initialize()
{
    // by_ = static_cast<Field>(explorerOption().value(SORT_BY_TAG, NAME).toInt());
    // order_ = static_cast<SortOrder>(explorerOption().value(SORT_ORDER_TAG, ASCENDING).toInt());
    changeCurrentNode(&root_node_);
}

BranchNode * ModelTree::changeCurrentNode(BranchNode *node)
{
    current_node_ = node;
    folderPath(current_node_->type(), current_path_);
    return current_node_;
}

// Update model path according to current node.
void ModelTree::updatePath()
{

}

bool ModelTree::folderPath(NodeType type, ModelPath & path)
{
    path.clear();
    if (type == NODE_TYPE_ROOT)
    {
        return true;
    }

    BranchNode * branch = branchNode(type);
    FolderNode * folder = folderNode(branch);
    if (branch)
    {
        path << branch->name();
    }
    if (folder)
    {
        path << folder->currentPath();
    }
    return true;
}

/// Downcast the node to FolderNode.
FolderNode *ModelTree::folderNode(Node *node)
{
    if (node == 0)
    {
        return 0;
    }

    NodeType type = node->type();
    if (type == NODE_TYPE_SD ||
        type == NODE_TYPE_LIBRARY)
    {
        return down_cast<FolderNode *>(node);
    }
    return 0;
}

/// Uninitialize data model. Store current context into preference
/// file.
void ModelTree::saveSettings()
{
    // At first, record the current node and the location.
    // Not correct.
    /*
    explorerOption().setValue(LIBRARY_LOCATION_TAG,
                                 node(NODE_TYPE_LIBRARY)->currentPath());
    explorerOption().setValue(USB_LOCATION_TAG,
                                 node(NODE_TYPE_USB)->currentPath());
    explorerOption().setValue(SD_LOCATION_TAG,
                                 node(NODE_TYPE_SD)->currentPath());
    */

    // explorerOption().setValue(SORT_BY_TAG, by_);
    // explorerOption().setValue(SORT_ORDER_TAG, order_);
}

BranchNode * ModelTree::root()
{
    return &root_node_;
}

/// Retrieve current node for current active type.
BranchNode * ModelTree::currentNode()
{
    return current_node_;
}

NodeType ModelTree::currentType()
{
    return current_node_->type();
}

/// Return the top level branch node according to the type.
BranchNode * ModelTree::branchNode(NodeType type)
{
    BranchNode * node = down_cast<BranchNode *>(root_node_.node(type));
    return node;
}

Field ModelTree::sort_field() const
{
    return current_node_->sort_field();
}

SortOrder ModelTree::sort_order() const
{
    return current_node_->sort_order();
}

BranchNode * ModelTree::cdDesktop()
{
    return changeCurrentNode(&root_node_);
}

BranchNode * ModelTree::cdBranch(NodeType type)
{
    Node* ptr = root_node_.node(type);
    FolderNode *folder = folderNode(ptr);
    if (folder)
    {
        folder->cdRoot();
        return changeCurrentNode(folder);
    }
    return changeCurrentNode(down_cast<BranchNode *>(ptr));
}

BranchNode * ModelTree::cdBranch(const QString &name)
{
    Node* ptr = root_node_.node(name);
    FolderNode *folder = folderNode(ptr);
    if (folder)
    {
        folder->cdRoot();
        return changeCurrentNode(folder);
    }

    return changeCurrentNode(down_cast<BranchNode *>(ptr));
}

BranchNode * ModelTree::cd(const QString &name)
{
    FolderNode *folder = folderNode(current_node_);
    if (folder)
    {
        folder->cd(name);
        return changeCurrentNode(folder);
    }

    // Get child node.
    NodePtr ptr = current_node_->node(name);
    folder = folderNode(ptr);
    if (folder)
    {
        folder->cd(name);
        return changeCurrentNode(folder);
    }

    return changeCurrentNode(down_cast<BranchNode *>(ptr));
}

/// Convert file system path to ModelPath.
bool ModelTree::modelPath(const QString & path, ModelPath & model_path)
{
    FolderNode *sd = folderNode(root()->node(NODE_TYPE_SD));
    FolderNode *flash = folderNode(root()->node(NODE_TYPE_LIBRARY));

    model_path.clear();
    if (path.startsWith(sd->root(), Qt::CaseSensitive))
    {
        model_path << sd->name();
    }
    else if (path.startsWith(flash->root(), Qt::CaseSensitive))
    {
        model_path << flash->name();
    }
    else
    {
        return false;
    }

    model_path << path;
    return true;
}

bool ModelTree::cdPath(const ModelPath &path)
{
    if (path.size() <= 0)
    {
        return false;
    }

    NodePtr p = root()->node(path.front());
    FolderNode * folder = folderNode(p);
    changeCurrentNode(folder);

    for(int i = 1; i < path.size(); ++i)
    {
        folder->cd(path.at(i));
    }
    return true;
}

BranchNode * ModelTree::cdUp()
{
    if (!canGoUp())
    {
        return 0;
    }

    // Check.
    BranchNode *node = currentNode();
    FolderNode *folder = folderNode(node);
    if (folder)
    {
        if (folder->canGoUp())
        {
            folder->cdUp();
            return changeCurrentNode(folder);
        }
    }
    return changeCurrentNode(down_cast<BranchNode *>(node->mutable_parent()));
}

bool ModelTree::canGoUp()
{
    return (current_node_ != &root_node_);
}

void ModelTree::updateDisplayNames()
{
    root_node_.updateDisplayNames();
}

void ModelTree::changeSortCriteria(Field by, SortOrder order)
{
    by_ = by;
    order_ = order;
}


}  // namespace model

}  // namespace explorer

