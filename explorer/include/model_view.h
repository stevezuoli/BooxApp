// View represents model tree to end user.
// Author John

#ifndef MODEL_VIEW_H_
#define MODEL_VIEW_H_

#include "onyx/base/base.h"
#include "onyx/base/device.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/ui.h"

#include "model_tree.h"
#include "node_view.h"
#include "node_header_view.h"
#include "path_bar.h"
#include "view_type_actions.h"
#include "setting_actions.h"
#include "device_actions.h"
#include "file_actions.h"
#include "history_actions.h"
#include "website_actions.h"

using namespace ui;


namespace explorer
{

using namespace model;

namespace model
{
class Node;
}

namespace view
{

/// Model view displays the model and enables user to navigate through
/// the model.
class ModelView : public  QWidget
{
    Q_OBJECT

public:
    ModelView(QWidget *parent, ModelTree & model);
    ~ModelView();

public:
    void resort();

public Q_SLOTS:
    void fileSystemChanged(const NodeType type, bool mount);

    void onStatusItemClicked(const int percent, const int value);
    void onMenuClicked();
    void updateAll(bool rescan = false);
    void onNodePressed(Node * node, const QPoint &pos);
    void onNodeReleased(Node * node, const QPoint &pos);
    void onNodeClicked(Node * node);
    void onBranchNodeClicked(Node * node);
    void onNodeCreateClicked(Node * node);
    void onWritePadClicked(Node * node);
    void onCreateWritePad(Node * node);
    void onDictionaryClicked(Node *node);
    void onNodeEditingClicked(Node * node);
    void onDirNodeClicked(Node * node);
    void onFileNodeClicked(Node * node);
    void onSettingNodeClicked(Node *node);
    void onWebSiteNodeClicked(Node *node);
    void onFeedReaderClicked(Node *node);
    void onSudokuClicked(Node *node);
    void onVCOMManagerClicked(Node *node);
    void onApplicationsClicked(Node * node);
    void onCalendarClicked(Node * node);
    void onClockClicked(Node * node);
    bool processFileNode(const QString &path);

    void search();
    void addShortcut(Node *node);
    void rename(Node *ptr);

    void triggerOnlineService();
    void onDownloadStateChanged(const QString &, int, bool);
    void onViewerClosed();

    /// TODO, Should put them into the model tree.
    void addToClipboard(Node * node);
    void clearClipboard();
    bool isClipboardEmpty();
    bool copy(const QString & path);
    bool move(const QString & path);
    bool deleteSelectedNode();
    bool deleteFile(bool ask = true);
    bool deleteNote();
    bool deleteWritePad();
    bool deleteSelectedRecentDoc();
    bool deleteSelectedShortcut();
    bool deleteWebSite();
    bool addWebSite();

    bool removeDirectory(const QString &);
    void clearAllWritePads();

    QString selectedNodePath();

    void onDesktopClicked();
    void onBranchClicked(const QString&);
    void onPathClicked(const QString &path);
    void onPathCloseClicked();

    bool changeSortCriteria(Field by, SortOrder order);
    void onFieldResized(Field field, int x, int width);

    void extractMetadata();
    void stopExtractingMetadata();
    void enableExtractMetadata(bool enable = true) { enable_extract_ = enable; }
    bool isExtractMetadataEnabled() { return enable_extract_; }

    void stopBackgroundJobs();

    void suspend();
    void shutdown(int r = SHUTDOWN_REASON_NONE);

    ModelTree & model() { return model_tree_; }

    bool continueReading();

    void onMetadataReady(const QString &path);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyReleaseEvent(QKeyEvent *);
    void changeEvent(QEvent *event);
    void resizeEvent(QResizeEvent * event );
    void paintEvent( QPaintEvent * event );

Q_SIGNALS:
    void outOfRoot();

private:
    void loadSettings();
    void saveSettings();

    void useDWUpdate(bool fastest = true);

    void createLayout();
    void showChildren(bool visible);

    void updateChildNodeViews(NodePtrs & children,
                              const size_t first_position,
                              const size_t selected_,
                              const size_t items_per_page);
    void updateGridLayout(const int rows, const int columns);

    void updateHeaderNode(BranchNode *node = 0);
    void updateChildNodes(bool regenerate, bool update);
    bool setViewType(ViewType mode);

    void updateStatusBar();

    void resetCursor();
    void calculatePositions();

    void navigateKeyDown();
    bool nextPage();
    bool prevPage();
    bool jumpPage(const int page);
    bool gotoUp();

    Node * firstVisibleNode();
    Node * firstSelectedNode();
    bool isFileNodeSelected();
    bool isFolderNodeSelected();
    bool isNoteSelected();
    bool isCreateNoteSelected();
    bool isReturnable();
    bool isCreateWritePadSelected();
    bool isWebSiteSelected();
    bool isWritePadSelected();
    bool isRecentDocumentsSelected();
    bool isShortcutsSelected();
    bool isInSDCard();
    bool isInFlash();
    bool isInWebSite();

    bool selectItem(const int offset);
    bool selectNode(Node *node);
    bool guiSelectNode(Node *node);

    NodePtrs& children_nodes(bool regenerate, bool update);

    // Menu
    void popupMenu();
    void updateMenuActions();
    void showSettingsDialog(NodeType setting);
    void showDialog(QDialog & dialog);
    void execDeviceAction(ExplorerDeviceAction action);
    void handleViewTypeActions(ViewTypeActions & actions);
    void handleFileActions(FileActions & actions);
    void handleRecentDocumentsActions(HistoryActions & actions);
    void handleWebSiteActions(WebSiteActions & actions);

    bool umountSD();
    void formatFlash();
    void formatSD();
    void removeAccountInfo();

    void showTimezoneDialog();
    void showFontManagement(sys::SystemConfig& conf);
    void showAboutDialog();
    void openUserManual();
    void showWaveformDialog();
    void showCalendar(sys::SystemConfig& conf);
    void showStartupDialog(sys::SystemConfig& conf);
    void showScreenUpdateDialog(sys::SystemConfig& conf);

    // Visible nodes changed.
    void onVisibleNodesChanged();

    void enableShutdown(bool enable = true);
    bool canShutdown();

    // internal state.
    int internal_state() { return current_state_; }
    int & mutable_internal_state() { return current_state_; }

    void navigateUrl(const QString & url);
    void returnBook(Node *ptr);

private:
    typedef shared_ptr<NodeView> NodeViewPtr;
    typedef vector<NodeViewPtr> NodeViewPtrs;
    typedef vector<NodeViewPtr>::iterator NodeViewPtrIter;

    SysStatus & sys_status_;

    NodeViewPtrs node_views_;
    ViewType view_mode_;

    QVBoxLayout ver_layout_;
    QGridLayout grid_layout_;

    PathBar path_bar_;

    NodeHeaderView header_view_;
    StatusBar status_bar_;

    ModelTree& model_tree_;

    int rows_;
    int columns_;
    int items_per_page_;
    int first_visible_;     ///< The first visible child node position.
    int selected_;          ///< Current selected child node position.

    int current_page_;      ///< Current page number.
    int page_count_;        ///< Total page number.

    Node * pressed_node_;   ///< Record the node has been pressed.
    QPoint pressed_point_;

    QString source_nodes_;       ///< Can be file or directory.
    QString target_path_;

    QStringList pending_list_;      ///< Documents need to be opened.

    ViewTypeActions view_type_actions_;
    FileActions file_actions_;
    HistoryActions history_actions_;
    SettingActions sys_setting_actions_;
    DeviceActions device_actions_;
    WebSiteActions website_actions_;

    size_t metadata_pos_;
    bool enable_extract_;

    int current_state_;
    bool can_shutdown_;
    bool is_cut_;

    NO_COPY_AND_ASSIGN(ModelView);
};

inline NodePtrs& ModelView::children_nodes(bool regenerate, bool update)
{
    if (update)
    {
        return model_tree_.currentNode()->updateChildrenInfo();
    }
    else
    {
        return model_tree_.currentNode()->mutable_children(regenerate);
    }
    static NodePtrs dummy;
    return dummy;
}

}  // namespace view

}  // namespace explorer

#endif
