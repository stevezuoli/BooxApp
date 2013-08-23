#include <algorithm>
#include <map>
#include "util/util.h"
#include "desktop_node.h"
#include "folder_node.h"
#include "explorer_conf.h"
#include "library_node.h"
#include "shortcuts_node.h"
#include "notes_node.h"
#include "writepad_node.h"
#include "websites_node.h"
#include "onyx/base/device.h"
#include "onyx/sys/sys_status.h"
#include "onyx/sys/platform.h"
#include "system_controller.h"
#include "sites_database.h"
#include "games_node.h"


namespace explorer {

namespace model {

static const QString LIBRARY_ROOT_TAG       = "Model/library/root";
static const QString LIBRARY_LOCATION_TAG   = "Model/library/location";

static const QString USB_ROOT_TAG           = "Model/usb/root";
static const QString USB_LOCATION_TAG       = "Model/usb/location";

static const QString SD_ROOT_TAG            = "Model/sd/root";
static const QString SD_LOCATION_TAG        = "Model/sd/location";

static const QString DOWNLOAD_ROOT_TAG      = "Model/download/root";
static const QString DOWNLOAD_LOCATION_TAG  = "Model/download/location";

static const QString SORT_BY_TAG            = "Model/sort_by";
static const QString SORT_ORDER_TAG            = "Model/sort_order";

static const NodeType SETTINGS[] = { NODE_TYPE_DATE,
                                     NODE_TYPE_TIMEZONE,
                                     NODE_TYPE_LOCALE,
                                     NODE_TYPE_PM,
                                     NODE_TYPE_SCREEN_CALIBRATION,
                                     NODE_TYPE_3G_CONNECTION,
                                     NODE_TYPE_SSH_SERVER,
                                     NODE_TYPE_FORMAT_FLASH,
                                     NODE_TYPE_USER_MANUAL,
                                     NODE_TYPE_WAVEFORM_SETTINGS,
                                     NODE_TYPE_FONTMANAGEMENT,
                                     NODE_TYPE_FILETYPE_SETTINGS,
                                     NODE_TYPE_STARTUP,
                                     NODE_TYPE_SCREEN_UPATE_SETTING,
                                     NODE_TYPE_ABOUT };

static const NodeType APPLICATIONS[] = { NODE_TYPE_CALENDAR,
                                         NODE_TYPE_FULL_SCREEN_CLOCK };

/// Put all initial directories here.
/// Get Last access location.
static QString getLibraryRoot()
{
#ifdef Q_WS_WIN32
    return explorerOption().value(LIBRARY_ROOT_TAG, "C://").toString();
#else
    return explorerOption().value(LIBRARY_ROOT_TAG, LIBRARY_ROOT).toString();
#endif
}

static QString getUSBRoot()
{
#ifdef Q_WS_WIN32
    return explorerOption().value(USB_ROOT_TAG, "D://").toString();
#else
    return explorerOption().value(USB_ROOT_TAG, USB_ROOT).toString();
#endif
}

static QString getSDRoot()
{
#ifdef Q_WS_WIN32
    return explorerOption().value(SD_ROOT_TAG, "D:/").toString();
#else
    return explorerOption().value(SD_ROOT_TAG, SDMMC_ROOT).toString();
#endif
}

static QString getDownloadRoot()
{
#ifdef Q_WS_WIN32
    return explorerOption().value(DOWNLOAD_ROOT_TAG, "F:\\").toString();
#else
    return explorerOption().value(DOWNLOAD_ROOT_TAG, DOWNLOAD_ROOT).toString();
#endif
}

DesktopNode::DesktopNode(Node * p)
    : BranchNode(p)
{
    mutable_type() = NODE_TYPE_ROOT;
    updateChildren();
}

DesktopNode::~DesktopNode()
{
    clearChildren();
}

void DesktopNode::updateChildren()
{
    util::DeletePtrContainer(&children_);
    name_filters_.clear();

    children_.push_back(createLibraryNode());
    children_.push_back(createSDNode());
    children_.push_back(createRecentDocumentsNode());
    children_.push_back(createShortcutsNode());

    if (explorer::controller::SystemController::instance().hasTouch())
    {
        children_.push_back(createWritePadContainer());
        children_.push_back(createDictionaryNode());
        children_.push_back(createNotesNode());
    }
    if (explorer::controller::SystemController::instance().hasTouch())
    {
        children_.push_back(createWebSitesNode());
        //children_.push_back(createFeedReaderNode());
    }

    children_.push_back(createGamesNode());
    children_.push_back(createSettingsNode());
    children_.push_back(createApplicationsNode());
    
    if (!QFile::exists("/root/Settings/vcom") && sys::is166E())
    {
        children_.push_back(createVCOMManager());
    }

    // For release 1.5, it's disabled.
    // children_.push_back(createOnlineShopNode());
}

/// Update children node but does not re-generate the child list.
NodePtrs& DesktopNode::updateChildrenInfo()
{
    return children_;
}

void DesktopNode::updateDisplayNames()
{
    const NodeType types[] = {NODE_TYPE_LIBRARY,
                              NODE_TYPE_SD,
                              NODE_TYPE_RECENT_DOCS,
                              NODE_TYPE_SYS_SETTINGS,
                              NODE_TYPE_SHORTCUTS,
                              NODE_TYPE_NOTE_CONTAINER,
                              NODE_TYPE_WEBSITES,
                              NODE_TYPE_APPLICATIONS};
    int size = sizeof(types) / sizeof(types[0]);
    for(int i = 0; i < size; ++i)
    {
        NodePtr ptr = node(types[i]);
        if (ptr)
        {
            ptr->mutable_display_name() = nodeDisplayName(types[i]);
        }
    }

    // Update settings node.
    BranchNode *settings = down_cast<BranchNode *>(node(NODE_TYPE_SYS_SETTINGS));
    size = sizeof(SETTINGS) / sizeof(SETTINGS[0]);
    for(int i = 0; i < size; ++i)
    {
        // Some nodes may be not created, let's have a check.
        NodePtr ptr = settings->node(SETTINGS[i]);
        if (ptr)
        {
            ptr->mutable_display_name() = nodeDisplayName(SETTINGS[i]);
        }
    }

    // Update applications node.
    BranchNode *apps = down_cast<BranchNode *>(node(NODE_TYPE_APPLICATIONS));
    size = sizeof(APPLICATIONS) / sizeof(APPLICATIONS[0]);
    for(int i = 0; i < size; ++i)
    {
        // Some nodes may be not created, let's have a check.
        NodePtr ptr = apps->node(APPLICATIONS[i]);
        if (ptr)
        {
            ptr->mutable_display_name() = nodeDisplayName(APPLICATIONS[i]);
        }
    }

    // Update others
    if (node(NODE_TYPE_NOTE_CONTAINER))
    {
        NotesNode * notes = down_cast<NotesNode *>(node(NODE_TYPE_NOTE_CONTAINER));
        notes->mutable_display_name() = nodeDisplayName(NODE_TYPE_NOTE_CONTAINER);
        notes->actionNode().mutable_display_name() = nodeDisplayName(NODE_TYPE_NEW_NOTE);
    }

    if (node(NODE_TYPE_WRITEPAD_CONTAINER))
    {
        WritePadContainer * notes = down_cast<WritePadContainer *>(node(NODE_TYPE_WRITEPAD_CONTAINER));
        notes->mutable_display_name() = nodeDisplayName(NODE_TYPE_WRITEPAD_CONTAINER);
        notes->actionNode().mutable_display_name() = nodeDisplayName(NODE_TYPE_NEW_WRITEPAD);
    }

    if (node(NODE_TYPE_DICTIONARY))
    {
        Node* n = node(NODE_TYPE_DICTIONARY);
        n->mutable_display_name() = nodeDisplayName(NODE_TYPE_DICTIONARY);
    }
}

BranchNode* DesktopNode::createLibraryNode()
{
    FolderNode *ptr = new FolderNode(this, getLibraryRoot());
    ptr->mutable_type() = NODE_TYPE_LIBRARY;
    ptr->changeSortCriteria(sort_field(), sort_order());
    ptr->mutable_name() = nodeName(ptr->type());
    ptr->mutable_display_name() = nodeDisplayName(ptr->type());
    return ptr;
}

BranchNode* DesktopNode::createUSBNode()
{
    FolderNode *ptr = new FolderNode(this, getUSBRoot());
    ptr->mutable_type() = NODE_TYPE_USB;
    ptr->changeSortCriteria(sort_field(), sort_order());
    return ptr;
}

BranchNode* DesktopNode::createSDNode()
{
    FolderNode *ptr = new FolderNode(this, getSDRoot());
    ptr->mutable_type() = NODE_TYPE_SD;
    ptr->changeSortCriteria(sort_field(), sort_order());
    ptr->mutable_name() = nodeName(ptr->type());
    ptr->mutable_display_name() = nodeDisplayName(ptr->type());
    return ptr;
}

BranchNode* DesktopNode::createRecentDocumentsNode()
{
    LibraryNode *ptr = new LibraryNode(this, cms::ContentManager::RECENT_DOCUMENTS_ID);
    ptr->mutable_type() = NODE_TYPE_RECENT_DOCS;
    ptr->mutable_name() = nodeName(ptr->type());
    ptr->mutable_display_name() = nodeDisplayName(ptr->type());
    return ptr;
}

BranchNode* DesktopNode::createShortcutsNode()
{
    ShortcutsNode *ptr = new ShortcutsNode(this);
    ptr->mutable_type() = NODE_TYPE_SHORTCUTS;
    ptr->mutable_name() = nodeName(ptr->type());
    ptr->mutable_display_name() = nodeDisplayName(ptr->type());
    return ptr;
}

BranchNode* DesktopNode::createNotesNode()
{
    NotesNode *ptr = new NotesNode(this);
    ptr->mutable_type() = NODE_TYPE_NOTE_CONTAINER;
    ptr->mutable_name() = nodeName(ptr->type());
    ptr->mutable_display_name() = nodeDisplayName(ptr->type());

    // Special child node.
    ptr->actionNode().mutable_name() = nodeName(NODE_TYPE_NEW_NOTE);
    ptr->actionNode().mutable_display_name() = nodeDisplayName(NODE_TYPE_NEW_NOTE);
    return ptr;
}

BranchNode* DesktopNode::createDownloadNode()
{
    FolderNode *ptr = new FolderNode(this, getDownloadRoot());
    ptr->mutable_type() = NODE_TYPE_DOWNLOAD;
    ptr->changeSortCriteria(sort_field(), sort_order());
    return ptr;
}

bool DesktopNode::isSettingAllowed(NodeType type)
{
    int tg = qgetenv("TG").toInt();
    if (type == NODE_TYPE_3G_CONNECTION)
    {
        return (tg != 0);
    }

    if (type == NODE_TYPE_WAVEFORM_SETTINGS ||
        type == NODE_TYPE_USER_MANUAL ||
        type == NODE_TYPE_SSH_SERVER)
    {
        return false;
    }

    if (!explorer::controller::SystemController::instance().hasTouch() &&
        type == NODE_TYPE_SCREEN_CALIBRATION)
    {
        return false;
    }
    return true;
}

BranchNode* DesktopNode::createSettingsNode()
{
    BranchNode *settings = new BranchNode(this);
    settings->mutable_type() = NODE_TYPE_SYS_SETTINGS;
    settings->mutable_name() = nodeName(settings->type());
    settings->mutable_display_name() = nodeDisplayName(settings->type());
    settings->changeSortCriteria(sort_field(), sort_order());

    // Create the setting nodes.
    const int size = sizeof(SETTINGS) / sizeof(SETTINGS[0]);
    for(int i = 0; i < size; ++i)
    {
        if (isSettingAllowed(SETTINGS[i]))
        {
            Node *ptr = new Node(settings);
            ptr->mutable_type() = SETTINGS[i];
            ptr->mutable_name() = nodeName(ptr->type());
            ptr->mutable_display_name() = nodeDisplayName(ptr->type());
            settings->mutable_children().push_back(ptr);
        }
    }
    return settings;
}

BranchNode* DesktopNode::createApplicationsNode()
{
    BranchNode *apps = new BranchNode(this);
    apps->mutable_type() = NODE_TYPE_APPLICATIONS;
    apps->mutable_name() = nodeName(apps->type());
    apps->mutable_display_name() = nodeDisplayName(apps->type());
    apps->changeSortCriteria(sort_field(), sort_order());

    // Create the setting nodes.
    const int size = sizeof(APPLICATIONS) / sizeof(APPLICATIONS[0]);
    for(int i = 0; i < size; ++i)
    {
        Node *ptr = new Node(apps);
        ptr->mutable_type() = APPLICATIONS[i];
        ptr->mutable_name() = nodeName(ptr->type());
        ptr->mutable_display_name() = nodeDisplayName(ptr->type());
        apps->mutable_children().push_back(ptr);
    }
    return apps;
}

BranchNode* DesktopNode::createWebSitesNode()
{
    BranchNode *sites = new WebSitesNode(this);
    sites->mutable_type() = NODE_TYPE_WEBSITES;
    sites->mutable_name() = nodeName (sites->type());
    sites->mutable_display_name() = nodeDisplayName (sites->type());
    return sites;
}

BranchNode* DesktopNode::createOnlineShopNode()
{
    BranchNode *shops = new BranchNode(this);
    shops->mutable_type() = NODE_TYPE_ONLINESHOPS;
    shops->mutable_name() = nodeName(shops->type());
    shops->mutable_display_name() = nodeDisplayName(shops->type());
    return shops;
}

BranchNode* DesktopNode::createWritePadContainer()
{
    BranchNode *container = new WritePadContainer(this);
    container->mutable_type() = NODE_TYPE_WRITEPAD_CONTAINER;
    container->mutable_name() = nodeName(container->type());
    container->mutable_display_name() = nodeDisplayName(container->type());
    return container;
}

Node* DesktopNode::createDictionaryNode()
{
    Node *dict = new Node(this);
    dict->mutable_type() = NODE_TYPE_DICTIONARY;
    dict->mutable_name() = nodeName(dict->type());
    dict->mutable_display_name() = nodeDisplayName(dict->type());

    return dict;
}

Node* DesktopNode::createFeedReaderNode()
{
    Node *reader = new Node(this);
    reader->mutable_type() = NODE_TYPE_FEEDREADER;
    reader->mutable_name() = nodeName(reader->type());
    reader->mutable_display_name() = nodeDisplayName(reader->type());
    return reader;
}

Node* DesktopNode::createVCOMManager()
{
    Node *manager = new Node(this);
    manager->mutable_type() = NODE_TYPE_APPLICATION;
    manager->mutable_name() = nodeName(manager->type());
    manager->mutable_display_name() = nodeDisplayName(manager->type());
    return manager;
}
BranchNode* DesktopNode::createGamesNode()
{
    BranchNode *games = new GamesNode(this);
    games->mutable_type() = NODE_TYPE_GAMES;
    games->mutable_name() = nodeName(games->type());
    games->mutable_display_name() = nodeDisplayName(games->type());
    return games;
}

}  // namespace model

}  // namespace explorer
