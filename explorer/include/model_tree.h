#ifndef MODEL_TREE_H_
#define MODEL_TREE_H_

#include "onyx/base/base.h"
#include "node.h"
#include "desktop_node.h"
#include "folder_node.h"

namespace explorer {

namespace model {

/// Data model tree for explorer.
/// The view will query the data model to display data to end user.
/// The data model will query node to retrieve necessary information.
/// view <-> ModelTree <-> NodePtr
/// ModelTree maintains currnet active node type and the path.
class ModelTree
{
public:
    ModelTree();
    ~ModelTree();

public:
    BranchNode * root();
    BranchNode * currentNode();
    NodeType currentType();
    BranchNode * branchNode(NodeType type);

    const ModelPath & current_path() { return current_path_; }

    Field sort_field() const;
    SortOrder sort_order() const;

    BranchNode * cdDesktop();
    BranchNode * cdBranch(NodeType type);
    BranchNode * cdBranch(const QString &name);
    BranchNode * cd(NodePtr p);
    BranchNode * cdUp();

    bool modelPath(const QString & path, ModelPath & model);
    bool cdPath(const ModelPath &path);
    BranchNode * cd(const QString &name);
    bool canGoUp();

    void updateDisplayNames();
    void changeSortCriteria(Field by, SortOrder order);
    void saveSettings();

    FolderNode *folderNode(Node *node);

private:
    void initialize();

    BranchNode* changeCurrentNode(BranchNode *node);
    void updatePath();
    bool folderPath(NodeType type, ModelPath & path);

private:
    DesktopNode root_node_;
    BranchNode *current_node_;
    ModelPath current_path_;

    Field by_;
    SortOrder order_;

    NO_COPY_AND_ASSIGN(ModelTree);
};


}  // namespace model

}  // namespace explorer

#endif
