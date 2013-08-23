#ifndef DESKTOP_NODE_H_
#define DESKTOP_NODE_H_

#include "branch_node.h"
#include "dir_node.h"
#include "file_node.h"
#include "onyx/cms/content_manager.h"
#include "onyx/cms/content_thumbnail.h"

using namespace cms;

namespace explorer {

namespace model {

/// Branch node represents a virtual container contains nodes.
/// It could be a directory or a category in cms library.
/// The branch node implements a directory based container.
class DesktopNode :  public BranchNode
{
public:
    DesktopNode(Node * parent);
    virtual ~DesktopNode();

public:
    NodePtrs& updateChildrenInfo();
    void updateDisplayNames();

protected:
    virtual void updateChildren();

private:
    BranchNode* createLibraryNode();
    BranchNode* createUSBNode();
    BranchNode* createSDNode();
    BranchNode* createRecentDocumentsNode();
    BranchNode* createDownloadNode();
    BranchNode* createShortcutsNode();
    BranchNode* createNotesNode();
    BranchNode* createSettingsNode();
    BranchNode* createApplicationsNode();
    BranchNode* createWebSitesNode();
    BranchNode* createOnlineShopNode();
    BranchNode* createWritePadContainer();
    Node* createDictionaryNode();
    Node* createFeedReaderNode();
    Node* createVCOMManager();
    Node* createCalendarNode();
    Node* createClockNode();
    BranchNode* createGamesNode();

    bool isSettingAllowed(NodeType type);

};

}  // namespace model

}  // namespce explorer

#endif  // BRANCH_NODE_H_

