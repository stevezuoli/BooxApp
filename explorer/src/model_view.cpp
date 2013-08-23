#include <algorithm>

#include "onyx/ui/ui.h"
#include "onyx/ui/time_zone_dialog.h"
#include "onyx/ui/calendar.h"
#include "onyx/ui/clock_dialog.h"

#include "onyx/screen/screen_proxy.h"

#include "onyx/wireless/dialup_dialog.h"
#include "onyx/wireless/wifi_dialog.h"

#include "onyx/cms/cms_tags.h"
#include "onyx/sys/sys_utils.h"
#include "onyx/data/web_history.h"

#include "about_dialog.h"
#include "model_view.h"
#include "explorer_conf.h"
#include "system_controller.h"
#include "dir_node.h"
#include "library_node.h"
#include "locale_dialog.h"
#include "date_dialog.h"
#include "waveform_dialog.h"
#include "search_dialog.h"
#include "mdb.h"
#include "notes_node.h"
#include "shortcuts_node.h"
#include "writepad_node.h"
#include "rename_dialog.h"
#include "url_dialog.h"
#include "websites_node.h"
#include "font_management_dialog.h"
#include "filetype_dialog.h"
#include "startup_dialog.h"
#include "screen_update_dialog.h"

using namespace std;
using namespace ui;

namespace explorer
{

using model::Node;
using namespace controller;

namespace view
{

static const QString VIEW_TYPE_TAG = "ModelView/view_type";
static const QString ACTIVE_NODE_TAG   = "ModelView/active_node";
static const int NO_SELECTION               = -1;

ModelView::ModelView(QWidget *parent, ModelTree & model)
#ifdef _WINDOWS
    : QWidget(parent, 0)
#else
    : QWidget(parent, 0) // Qt::FramelessWindowHint)
#endif
    , sys_status_(sys::SysStatus::instance())
    , node_views_()
    , view_mode_(THUMBNAIL_VIEW)
    , ver_layout_(this)
    , grid_layout_(0)
    , path_bar_(0, model)
    , header_view_(this)
    , status_bar_(this)
    , model_tree_(model)
    , rows_(1)
    , columns_(1)
    , items_per_page_(1)
    , first_visible_(0)
    , selected_(0)
    , current_page_(0)
    , page_count_(0)
    , pressed_node_(0)
    , pressed_point_()
    , metadata_pos_(0)
    , enable_extract_(false)
    , current_state_(SHUTDOWN_REASON_NONE)
    , can_shutdown_(false)
{
    // Load settings.
    loadSettings();

    // Background color.
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    // create layout.
    createLayout();

    connect(&controller::SystemController::instance(),
            SIGNAL(metadataReady(const QString&)),
            this,
            SLOT(onMetadataReady(const QString &)));
}

ModelView::~ModelView()
{
    saveSettings();
}

Node * ModelView::firstVisibleNode()
{
    NodePtrs & ref = children_nodes(false, false);
    if (first_visible_ >= 0 &&
        first_visible_ < static_cast<int>(ref.size()))
    {
        return ref[first_visible_];
    }
    return 0;
}

Node * ModelView::firstSelectedNode()
{
    NodePtrs & ref = children_nodes(false, false);
    if (selected_ >= 0 &&
        selected_ < static_cast<int>(ref.size()))
    {
        return ref[selected_];
    }
    return 0;
}

bool ModelView::isFileNodeSelected()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }
    return (ptr->type() == NODE_TYPE_FILE);
}

bool ModelView::isFolderNodeSelected()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }
    return (ptr->type() == NODE_TYPE_DIRECTORY);
}

bool ModelView::isNoteSelected()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }
    return (ptr->type() == NODE_TYPE_NOTE);
}

bool ModelView::isCreateNoteSelected()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }
    return (ptr->type() == NODE_TYPE_NEW_NOTE);
}

bool ModelView::isReturnable()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }
    if (ptr->type() == NODE_TYPE_FILE)
    {
        return down_cast<FileNode *>(ptr)->isReturnable();
    }
    return false;
}

bool ModelView::isCreateWritePadSelected()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }
    return (ptr->type() == NODE_TYPE_NEW_WRITEPAD);
}

bool ModelView::isInWebSite()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }
    return  (ptr->type() == NODE_TYPE_WEBSITE);
}

bool ModelView::isWebSiteSelected()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }
    if (ptr->type() == NODE_TYPE_WEBSITE)
    {
        const QString & name = ptr->name();
        if (name.compare("google") && name.compare("onyx") &&  name.compare("adobe") && name.compare("wiki"))
        {
            return true;
        }
    }

    return false;
}

bool ModelView::isWritePadSelected()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }
    return (ptr->type() == NODE_TYPE_WRITEPAD);
}

bool ModelView::isRecentDocumentsSelected()
{
    return (model_tree_.currentType() == NODE_TYPE_RECENT_DOCS);
}

bool ModelView::isShortcutsSelected()
{
    return (model_tree_.currentType() == NODE_TYPE_SHORTCUTS);
}

bool ModelView::isInSDCard()
{
    return (model_tree_.currentType() == NODE_TYPE_SD);
}

bool ModelView::isInFlash()
{
    return (model_tree_.currentType() == NODE_TYPE_LIBRARY);
}

void ModelView::loadSettings()
{
    view_mode_ = static_cast<ViewType>(explorerOption().value(VIEW_TYPE_TAG, THUMBNAIL_VIEW).toInt());
}

void ModelView::saveSettings()
{
    model_tree_.saveSettings();
    explorerOption().setValue(VIEW_TYPE_TAG, view_mode_);
    explorerOption().sync();
}

/// Change the default update type.
void ModelView::useDWUpdate(bool use_fastest)
{
    if (use_fastest)
    {
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
    }
    else
    {
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
    }
}

void ModelView::createLayout()
{
    // The margins are defined in main window. For all child window
    // we always use the margin 0.
    ver_layout_.setSpacing(2);
    ver_layout_.setContentsMargins(0, 0, 0, 0);

    grid_layout_.setSpacing(0);
    grid_layout_.setContentsMargins(0, 0, 0, 0);

    // Path bar.
    ver_layout_.addWidget(&path_bar_);
    connect(&path_bar_, SIGNAL(desktopClicked()),
            this, SLOT(onDesktopClicked()));
    connect(&path_bar_, SIGNAL(branchClicked(const QString&)),
            this, SLOT(onBranchClicked(const QString&)));
    connect(&path_bar_, SIGNAL(pathClicked(const QString &)),
            this, SLOT(onPathClicked(const QString &)));
    connect(&path_bar_, SIGNAL(closeClicked()),
            this, SLOT(onPathCloseClicked()));

    // Header view
    header_view_.updateNode(model_tree_.currentNode());
    ver_layout_.addWidget(&header_view_);
    connect(&header_view_, SIGNAL(fieldClicked(Field, SortOrder)),
            this, SLOT(changeSortCriteria(Field, SortOrder)));
    connect(&header_view_, SIGNAL(fieldResized(Field, int,int)),
            this, SLOT(onFieldResized(Field, int,int)));

    // Add the grid layout into big vertical layout.
    ver_layout_.addLayout(&grid_layout_);

    // Add the status bar.
    ver_layout_.addWidget(&status_bar_);
    connect(&status_bar_,
            SIGNAL(progressClicked(const int, const int)),
            this,
            SLOT(onStatusItemClicked(const int, const int)));

    connect(&status_bar_,
            SIGNAL(menuClicked()),
            this,
            SLOT(onMenuClicked()));
}

void ModelView::showChildren(bool visible)
{
    // Also need to close dialog that opened by statusbar.
    status_bar_.closeChildrenDialogs();

    // Close all active dialog at first.
    if (!visible)
    {
        QList<QDialog *> dialogs = findChildren<QDialog *>();
        foreach(QDialog *dialog, dialogs)
        {
            if (dialog && dialog->isActiveWindow())
            {
                dialog->close();
            }
        }
        QApplication::processEvents();
    }

    for(NodeViewPtrIter iter = node_views_.begin(); iter != node_views_.end(); ++iter)
    {
        (*iter)->setVisible(visible);
    }

    path_bar_.setVisible(visible);
    header_view_.setVisible(visible);
    status_bar_.setVisible(visible);
}

/// Create node views or update node views according to the nodes.
/// This function update nodes views from the \first_position of
/// \children. It also updates the view mode of these node views.
/// \first_position The first visible node.
/// \selected_ The selected node position.
/// \count The items per page.
void ModelView::updateChildNodeViews(NodePtrs & children,
                                     const size_t first_position,
                                     const size_t selected,
                                     const size_t count)
{
    for(size_t i = node_views_.size(); i < count; ++i)
    {
        NodeViewPtr view(new NodeView(this));
        connect(view.get(),
                SIGNAL(nodeClicked(Node *)),
                this,
                SLOT(onNodeClicked(Node*)));

        connect(view.get(),
                SIGNAL(nodePressed(Node *, const QPoint)),
                this,
                SLOT(onNodePressed(Node *, const QPoint)));

        connect(view.get(),
                SIGNAL(nodeReleased(Node *, const QPoint)),
                this,
                SLOT(onNodeReleased(Node *, const QPoint)));

        node_views_.push_back(view);
    }

    // Associate the node view with the node.
    size_t up = min(children.size(), first_position + count);
    NodeViewPtrIter iter = node_views_.begin();
    for(size_t pos = first_position; pos < up; ++pos)
    {
        (*iter)->setNode(children[pos]);
        (*iter)->setViewType(view_mode_);
        if (pos == selected)
        {
            (*iter)->select(true);
        }
        else
        {
            (*iter)->select(false);
        }

        if ((*iter)->needPaint())
        {
            (*iter)->update();
        }
        ++iter;
    }

    // Update the other node views.
    NodeViewPtrIter end = node_views_.end();
    while (iter != end)
    {
        (*iter)->setNode(0);
        (*iter)->setViewType(view_mode_);
        if ((*iter)->needPaint())
        {
            (*iter)->update();
        }
        ++iter;
    }

    // TODO: Optimize here.

}

/// Change layout. Make sure the node views have been created already.
/// This function is usually called when layout is created or layout
/// is to be changed.
void ModelView::updateGridLayout(const int rows, const int columns)
{
    assert(static_cast<int>(node_views_.size()) >= rows * columns);

    if (grid_layout_.count() > 0)
    {
        for(NodeViewPtrIter it = node_views_.begin();
            it != node_views_.end();
            ++it)
        {
            grid_layout_.removeWidget(it->get());
            (*it)->hide();
        }
    }

    for(int i = 0; i < rows; ++i)
    {
        for(int j = 0; j < columns; ++j)
        {
            grid_layout_.addWidget(node_views_[i * columns + j].get(), i, j);
            node_views_[i * columns + j]->show();
        }
    }
}

void ModelView::mousePressEvent(QMouseEvent *me)
{
    me->accept();
}

void ModelView::mouseReleaseEvent(QMouseEvent *me)
{
    /// Internal controller.
    me->accept();
}

void ModelView::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();

    // By default, we disable the fastest update.
    useDWUpdate(false);
    switch(ke->key())
    {
    case Qt::Key_1:
        {
            setViewType(LIST_VIEW);
        }
        break;
    case Qt::Key_2:
        {
            setViewType(DETAILS_VIEW);
        }
        break;
    case Qt::Key_3:
        {
            setViewType(THUMBNAIL_VIEW);
        }
        break;
    case Qt::Key_S:
        {
            if (model_tree_.sort_order() == ASCENDING)
            {
                changeSortCriteria(SIZE, DESCENDING);
            }
            else
            {
                changeSortCriteria(SIZE, ASCENDING);
            }
        }
        break;
    case Qt::Key_N:
        {
            if (model_tree_.sort_order() == ASCENDING)
            {
                changeSortCriteria(NAME, DESCENDING);
            }
            else
            {
                changeSortCriteria(NAME, ASCENDING);
            }
        }
        break;
    case Qt::Key_T:
        {
            if (model_tree_.sort_order() == ASCENDING)
            {
                changeSortCriteria(NODE_TYPE, DESCENDING);
            }
            else
            {
                changeSortCriteria(NODE_TYPE, ASCENDING);
            }
        }
        break;
    case Qt::Key_R:
        {
            if (model_tree_.sort_order() == ASCENDING)
            {
                changeSortCriteria(RATING, DESCENDING);
            }
            else
            {
                changeSortCriteria(RATING, ASCENDING);
            }
        }
        break;
    case Qt::Key_C:
        {
            if (ke->modifiers() & Qt::ControlModifier)
            {
                addToClipboard(firstSelectedNode());
            }
        }
        break;
    case Qt::Key_X:
        {
            if (ke->modifiers() & Qt::ControlModifier)
            {
                addToClipboard(firstSelectedNode());
            }
        }
        break;
    case Qt::Key_Delete:
        {
            clearClipboard();
            addToClipboard(firstSelectedNode());
            deleteFile();
        }
        break;
    case Qt::Key_PageDown:
        if (nextPage())
        {
            updateChildNodes(false, false);
            onVisibleNodesChanged();
            updateStatusBar();
        }
        break;
    case Qt::Key_PageUp:
        if (prevPage())
        {
            updateChildNodes(false, false);
            onVisibleNodesChanged();
            updateStatusBar();
        }
        break;
    case Qt::Key_Escape:
        if (gotoUp())
        {
            updateAll();
            return;
        }
        emit outOfRoot();
        break;
    case Qt::Key_Up:
        if (selectItem(-columns_))
        {
            useDWUpdate(true);
            updateChildNodes(false, false);
            onVisibleNodesChanged();
            updateStatusBar();
        }
        break;
    case Qt::Key_Down:
        {
            navigateKeyDown();
        }
        break;
    case Qt::Key_Left:
    // case Qt::Key_VolumeUp:
        if (selectItem(-1))
        {
            useDWUpdate(true);
            updateChildNodes(false, false);
            onVisibleNodesChanged();
            updateStatusBar();
        }
        break;
    case Qt::Key_Right:
    // case Qt::Key_VolumeDown:
        if (selectItem(1))
        {
            useDWUpdate(true);
            updateChildNodes(false, false);
            onVisibleNodesChanged();
            updateStatusBar();
        }
        break;
    case Qt::Key_Return:
        onNodeClicked(firstSelectedNode());
        break;
    case ui::Device_Menu_Key:
    case Qt::Key_F10:
        popupMenu();
        break;
    case Qt::Key_F11:
        controller::SystemController::instance().stopAllApplications(false);
        break;
    case Qt::Key_F2:
        rename(firstSelectedNode());
        break;
    case Qt::Key_U:
        fileSystemChanged(model::NODE_TYPE_SD, false);
        break;
    case Qt::Key_F:
        search();
        break;
    case Qt::Key_O:
        triggerOnlineService();
        break;
    default:
        break;
    }
}

void ModelView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        model_tree_.updateDisplayNames();
        updateAll(false);
    }
    else
    {
        QWidget::changeEvent(event);
    }
}

void ModelView::resizeEvent ( QResizeEvent * event )
{
    calculatePositions();

    // Make sure the node views have been created.
    updateChildNodeViews(children_nodes(false, false),
                            first_visible_,
                            selected_,
                            items_per_page_);
    updateGridLayout(rows_, columns_);

    updateChildNodes(false, false);

    updateStatusBar();
}

void ModelView::paintEvent(QPaintEvent *event)
{
    event->accept();
    if (internal_state() == SHUTDOWN_REASON_NONE)
    {
        return;
    }
    else if (internal_state() == SHUTDOWN_REASON_USER_REQUEST)
    {
        // Shutdown by user request.
        QPainter painter(this);

        // Check if we have any shutdown image.
        QDir dir(SHARE_ROOT);
        dir.cd("explorer/images");
        QString path = dir.filePath("shutdown.png");
        if (QFile::exists(path))
        {
            int degree = sys_status_.screenTransformation();
            if (degree == 0)
            {
                painter.drawImage(rect(), QImage(path));
            }
            else
            {
                QMatrix matrix;
                matrix.rotate(degree);
                QImage image(path);
                image = image.transformed(matrix);
                painter.drawImage(rect(), image);
            }
        }
        else
        {
            painter.fillRect(rect(), QBrush(Qt::white));
        }
        enableShutdown(true);
    }
    else if (internal_state() == SHUTDOWN_REASON_LOW_BATTERY)
    {
        QPainter painter(this);
        QFont font;
        font.setPointSize(30);
        painter.setFont(font);
        QRect rc(50, 0, width() - 100, height());
        QTextOption option(Qt::AlignCenter);
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        painter.drawText(rc, QApplication::tr("Low Battery, Please charge."), option);
        enableShutdown(true);
    }

}


/// Model event handler. When model changed, it's necessary to update the
/// header node, all children nodes and the status bar.
/// The first visible position is calculated by caller.
void ModelView::updateAll(bool rescan)
{
    updateHeaderNode();
    
    updateChildNodes(rescan, false);
    
    onVisibleNodesChanged();
    
    updateStatusBar();
}

void ModelView::onNodePressed(Node * node, const QPoint &pos)
{
    pressed_node_ = node;
    pressed_point_ = pos;
}

void ModelView::onNodeReleased(Node * node, const QPoint &pos)
{
    int direction = sys::SystemConfig::direction(pressed_point_, pos);

    if (direction == 0)
    {
        onNodeClicked(node);
        return;
    }

    if (direction > 0)
    {
        // want to jump to next
        if (nextPage())
        {
            updateChildNodes(false, false);
            onVisibleNodesChanged();
            updateStatusBar();
        }
    }
    else if (direction < 0)
    {
        if (prevPage())
        {
            updateChildNodes(false, false);
            onVisibleNodesChanged();
            updateStatusBar();
        }
    }
}

void ModelView::onNodeClicked(Node * node)
{
    if (node == NULL)
    {
        return;
    }

    guiSelectNode(node);
    // Only handle the node it can hanlde, for those nodes it can
    // not handle, it would delegate the node to state controller
    // for further process, which may cause state transmission.
    model::NodeType type = node->type();
    switch (type)
    {
    case model::NODE_TYPE_DIRECTORY:
        onDirNodeClicked(node);
        break;
    case model::NODE_TYPE_FILE:
        onFileNodeClicked(node);
        break;
    case model::NODE_TYPE_LOCALE:
    case model::NODE_TYPE_DATE:
    case model::NODE_TYPE_PM:
    case model::NODE_TYPE_ABOUT:
    case model::NODE_TYPE_FONTMANAGEMENT:
    case model::NODE_TYPE_SCREEN_CALIBRATION:
    case model::NODE_TYPE_FORMAT_FLASH:
    case model::NODE_TYPE_FORMAT_SD:
    case model::NODE_TYPE_3G_CONNECTION:
    case model::NODE_TYPE_SSH_SERVER:
    case model::NODE_TYPE_USER_MANUAL:
    case model::NODE_TYPE_TIMEZONE:
    case model::NODE_TYPE_WAVEFORM_SETTINGS:
    case model::NODE_TYPE_FILETYPE_SETTINGS:
    case model::NODE_TYPE_STARTUP:
    case model::NODE_TYPE_SCREEN_UPATE_SETTING:
        onSettingNodeClicked(node);
        break;
    case model::NODE_TYPE_CALENDAR:
    case model::NODE_TYPE_FULL_SCREEN_CLOCK:
        onApplicationsClicked(node);
        break;
    case model::NODE_TYPE_WEBSITE:
        onWebSiteNodeClicked(node);
        break;
    case model::NODE_TYPE_NEW_NOTE:
        onNodeCreateClicked(node);
        break;
    case model::NODE_TYPE_NOTE:
        onNodeEditingClicked(node);
        break;
    case model::NODE_TYPE_WRITEPAD:
        onWritePadClicked(node);
        break;
    case model::NODE_TYPE_NEW_WRITEPAD:
        onCreateWritePad(node);
        break;
    case model::NODE_TYPE_DICTIONARY:
        onDictionaryClicked(node);
        break;
    case model::NODE_TYPE_FEEDREADER:
        onFeedReaderClicked(node);
        break;
    case model::NODE_TYPE_GAME_SUDOKU:
        onSudokuClicked(node);
        break;
    case model::NODE_TYPE_APPLICATION:
        onVCOMManagerClicked(node);
        break;
    default:
        onBranchNodeClicked(node);
        break;
    }
}

void ModelView::onDirNodeClicked(Node * node)
{
    // Check current node.
    if (model_tree_.currentNode()->type() == NODE_TYPE_SHORTCUTS)
    {
        ModelPath model_path;
        model_tree_.modelPath(node->absolute_path(), model_path);
        model_tree_.cdPath(model_path);
    }
    else
    {
        model_tree_.cd(node->name());
    }
    resetCursor();
    updateAll(true);
}

void ModelView::onFileNodeClicked(Node * node)
{

    // Before opening it, we need to check the node is valid or not.
    QString path = node->absolute_path();

    // Disable screen update.
    onyx::screen::instance().enableUpdate(false);

    // Try to open it.
    if (processFileNode(path))
    {
        // Disable any other user event now.
        // TODO: not sure we still need to setBusy here.
        // sys_status_.setSystemBusy(true);

        // If we are able to open the document, it's better
        // to stop the thumbnail rendering. So that the viewer
        // can get more resource.
        stopBackgroundJobs();
    }
    else
    {
        // If we can not open it, enable screen update then.
        onyx::screen::instance().enableUpdate(true);
    }
}

/// This function actually is not really necessary. We can reuse
/// onFileNodeClicked.
void ModelView::onWebSiteNodeClicked(Node *node)
{
    // Disable screen update.
    onyx::screen::instance().enableUpdate(false);

    // Try to open it.
    if (SystemController::instance().navigateTo(node->absolute_path()))
    {
        // Disable any other user event now.
        // sys_status_.setSystemBusy(true);
        // If we are able to open the document, it's better
        // to stop the thumbnail rendering. So that the viewer
        // can get more resource.
        stopBackgroundJobs();
    }
    else
    {
        // If we can not open it, enable screen update then.
        onyx::screen::instance().enableUpdate(true);
    }
}

void ModelView::onFeedReaderClicked(Node *node)
{
    SystemController::instance().startFeedReader();
}

void ModelView::onSudokuClicked(Node *node)
{
    SystemController::instance().startSudoku();
}

void ModelView::onVCOMManagerClicked(Node *node)
{
    SystemController::instance().startVCOMManager();
}

void ModelView::onSettingNodeClicked(Node *node)
{
    switch (node->type())
    {
    case NODE_TYPE_LOCALE:
        {
            // Always use short connection.
            sys::SystemConfig conf;
            LocaleDialog dialog(this, conf);
            dialog.exec();
            break;
        }
    case NODE_TYPE_DATE:
        {
            DateDialog dialog(this);
            dialog.exec();
            break;
        }
    case NODE_TYPE_TIMEZONE:
        {
            showTimezoneDialog();
            break;
        };
    case NODE_TYPE_PM:
        {
            status_bar_.onBatteryClicked();
            break;
        }
    case NODE_TYPE_FORMAT_FLASH:
        {
            formatFlash();
            break;
        }
    case NODE_TYPE_FORMAT_SD:
        {
            formatSD();
            break;
        }
    case NODE_TYPE_ABOUT:
        {
            showAboutDialog();
            break;
        }
    case NODE_TYPE_FONTMANAGEMENT: 
        {
            sys::SystemConfig conf;
            showFontManagement(conf);
            break;
        }
    case NODE_TYPE_SCREEN_CALIBRATION:
        {
            controller::SystemController::instance().startCalibration();
            break;
        }
    case NODE_TYPE_3G_CONNECTION:
        {
            DialUpDialog dialog(this, sys::SysStatus::instance());
            dialog.popup();
            break;
        }
    case NODE_TYPE_SSH_SERVER:
        {
            // Start dropbear server and show the address to end user.
            break;
        }
    case NODE_TYPE_USER_MANUAL:
        {
            openUserManual();
            break;
        }
    case NODE_TYPE_WAVEFORM_SETTINGS:
        {
            showWaveformDialog();
            break;
        }
    case NODE_TYPE_FILETYPE_SETTINGS:
        {
            FileTypeDialog dialog(this);
            dialog.exec();
            break;
        }
    case NODE_TYPE_STARTUP:
        {
            sys::SystemConfig conf;
            showStartupDialog(conf);
            break;
        }
    case NODE_TYPE_SCREEN_UPATE_SETTING:
        {
            sys::SystemConfig conf;
            showScreenUpdateDialog(conf);
            break;
        }

    default:
        break;
    }
}


void ModelView::onApplicationsClicked(Node * node)
{
    switch (node->type())
    {
    case NODE_TYPE_CALENDAR:
        {
            onCalendarClicked(node);
            break;
        }
    case NODE_TYPE_FULL_SCREEN_CLOCK:
        {
            onClockClicked(node);
            break;
        }

    default:
        break;
    }
}

void ModelView::onCalendarClicked(Node * node)
{
    Calendar calendar(this);
    calendar.exec();
}

void ModelView::onClockClicked(Node * node)
{
    FullScreenClock clock(this);
    clock.exec();
}

void ModelView::onBranchNodeClicked(Node *node)
{
    model_tree_.cd(node->name());
    resetCursor();
    updateAll(true);
}

void ModelView::onNodeCreateClicked(Node * node)
{
    return controller::SystemController::instance().createNote(mdb().suggestedNoteName());
}

void ModelView::onDictionaryClicked(Node * node)
{
    return controller::SystemController::instance().openDictionary();
}

void ModelView::onWritePadClicked(Node * node)
{
    controller::SystemController::instance().startWritePad(node->absolute_path());
}

void ModelView::onCreateWritePad(Node * node)
{
    controller::SystemController::instance().startWritePad("");
}

void ModelView::onNodeEditingClicked(Node * node)
{
    if (!node)
    {
        return;
    }
    NoteNode *note = down_cast<NoteNode *>(node);
    if (note)
    {
        return controller::SystemController::instance().createNote(note->info().name());
    }
}

/// For file node, it's more complex than the other nodes.
/// Use a dedicated function to handle that.
bool ModelView::processFileNode(const QString &path)
{
    // Node::ChildListType::const_iterator it = std::find(begin_, end_, node);
    return controller::SystemController::instance().openContentFile(path);
}

void ModelView::search()
{
    // Popup search dialog.
    static SearchContext ctx;
    ctx.setNode(model_tree_.currentNode());
    SearchDialog dialog(this, ctx);
    if (dialog.popup(status_bar_.height()) == QDialog::Rejected)
    {
        model_tree_.currentNode()->clearNameFilters();
        updateAll(true);
    }
    else
    {
        updateAll(false);
    }
}

/// Add shortcut link.
void ModelView::addShortcut(Node *node)
{
    if (node == 0)
    {
        return;
    }
    QString path = node->absolute_path();
    cms::ContentManager & db = mdb();
    int count = db.links(path);
    QFileInfo info(path);
    QString target = info.fileName();
    QString postfix("(1%)");
    if (count > 0)
    {
        target += postfix.arg(count);
    }
    db.link(path, target);
}

void ModelView::rename(Node *node)
{
    if (node == 0)
    {
        return;
    }

    if (node->type() != NODE_TYPE_FILE && node->type() != NODE_TYPE_DIRECTORY)
    {
        return;
    }

    QString name = node->name();
    QString result;
    RenameDialog dialog(this);
    result = dialog.popup(name);
    if (result.isEmpty() || result == name)
    {
        return;
    }

    QFileInfo info(node->absolute_path());
    QDir dir = info.absoluteDir();

    if (node->type() == NODE_TYPE_DIRECTORY)
    {
        dir.cdUp();
    }

    if (!dir.rename(name, result))
    {
        ErrorDialog dialog(tr("Rename failed."));
        dialog.exec();
    }
    else
    {
        updateAll(true);
    }
}

void ModelView::triggerOnlineService()
{
    if (sys::SystemConfig::hasHomePage())
    {
        navigateUrl(sys::SystemConfig::homeUrl());
    }
    else
    {
        // Check whether there is download.txt
        const QString name = "download.txt";
        NodePtrs & ref = children_nodes(false, false);
        for(int i = 0; i < static_cast<int>(ref.size()); ++i)
        {
            if (ref[i]->name().compare(name, Qt::CaseInsensitive) == 0)
            {
                QFile file(ref[i]->absolute_path());
                file.open(QIODevice::ReadOnly);
                QString link = file.readAll();
                QStringList list;
                list << link;
                list << "fulfill";
                sys_status_.startDRMService(list);
                return;
            }
        }
    }
}

void ModelView::onDownloadStateChanged(const QString & path,
                                       int percentage,
                                       bool open)
{
    updateAll(true);
    if (open)
    {
        if (isActiveWindow() && SystemController::instance().runningViewers() <= 0)
        {
            processFileNode(path);
        }
        else
        {
            // Just record the document and it will be opened later.
            pending_list_.push_back(path);
        }
    }
}

void ModelView::onViewerClosed()
{
    sys_status_.updateBatteryStatus();
    if (!pending_list_.isEmpty())
    {
        processFileNode(pending_list_.takeFirst());
    }
    else
    {
        onVisibleNodesChanged();
    }
}

/// Re-scan recent documents list.
/*
void ModelView::switchToRecentDocument()
{
    current_type_ = NODE_TYPE_RECENT_DOCS;

    updateHeaderNode(model_tree_->node(NODE_TYPE_RECENT_DOCS));
    header_node_->children(true);
    header_node_->sort(LAST_ACCESS_TIME, DESCENDING);

    first_visible_ = 0;
    selected_ = first_visible_;

    updateChildNodes(false);
    updateStatusBar();
}
*/

void ModelView::resort()
{
    updateChildNodes(false, true);
}

/// Reset content in clipboard to the specified node.
/// The content will be used later.
void ModelView::addToClipboard(Node * node)
{
    if (node)
    {
        source_nodes_=node->absolute_path();
    }
}

void ModelView::clearClipboard()
{
    source_nodes_.clear();
}

/// Check if the clipboard contains any content or not.
bool ModelView::isClipboardEmpty()
{
    return source_nodes_.isEmpty();
}

/// Copy the content from clipboard to here.
bool ModelView::copy(const QString & path)
{
    if (path.isEmpty())
    {
        return false;
    }

    if ( QFileInfo(source_nodes_).isDir() && QFileInfo(path).isFile() )
    {
        ErrorDialog dialog(tr("The target is a file, can not do that."));
        dialog.exec();
        return false;
    }
    if (source_nodes_.compare(path) ==0 )
    {
        ErrorDialog dialog(tr("The target is same, can not do that."));
        dialog.exec();
        return false;
    }

    sys::SysStatus::instance().setSystemBusy(true);
    QString program = "cp";
    QStringList arguments;
    arguments << "-rf" <<  source_nodes_ << path ;

    int code = sys::runScriptBlock(program, arguments, 3000 * 1000);
    if (code != 0)
    {
        sys::SysStatus::instance().setSystemBusy(false);

        ErrorDialog dialog(tr("pasted failed."));
        dialog.exec();
        return false;
    }

    sys::SysStatus::instance().setSystemBusy(false);
    return true;
}

/// Move the content from clipboard to here and clear the clipboard.
bool ModelView::move(const QString& path)
{
    if (path.isEmpty())
    {
        return false;
    }

    if (QFileInfo(source_nodes_).isDir() && QFileInfo(path).isFile())
    {
        ErrorDialog dialog(tr("The target is a file, can not do that."));
        dialog.exec();
        return false;
    }
    if (source_nodes_.compare(path) ==0 )
    {
        ErrorDialog dialog(tr("The target is same, can not do that."));
        dialog.exec();
        return false;
    }

    sys::SysStatus::instance().setSystemBusy(true);
    QString program = "mv";
    QStringList arguments;
    arguments << source_nodes_ << path;

    int code = sys::runScriptBlock(program, arguments, 3000 * 1000);
    if (code !=0 )
    {
        sys::SysStatus::instance().setSystemBusy(false);

        ErrorDialog dialog(tr("pasted failed."));
        dialog.exec();
        return false;
    }

    sys::SysStatus::instance().setSystemBusy(false);
    clearClipboard();
    return true;
}

bool ModelView::deleteSelectedNode()
{
    if (isRecentDocumentsSelected())
    {
        deleteSelectedRecentDoc();
    }
    else if (isShortcutsSelected())
    {
        deleteSelectedShortcut();
    }
    else if (isFileNodeSelected() || isFolderNodeSelected())
    {
        return deleteFile();
    }
    else if (isNoteSelected())
    {
        return deleteNote();
    }
    else if (isWritePadSelected())
    {
        return deleteWritePad();
    }
    return false;
}

QString ModelView::selectedNodePath()
{
    QString path;
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return path;
    }
    if (ptr->type() != NODE_TYPE_FILE &&
        ptr->type() != NODE_TYPE_DIRECTORY)
    {
        return path;
    }

    path = ptr->absolute_path();
    return path;
}

bool ModelView::deleteSelectedRecentDoc()
{
    BranchNode * node = model_tree_.currentNode();
    if (node->type() == NODE_TYPE_RECENT_DOCS)
    {
        down_cast<LibraryNode *>(node)->removeRecentDocument(selectedNodePath());
        updateAll(true);
        return true;
    }
    return false;
}

bool ModelView::deleteSelectedShortcut()
{
    BranchNode * node = model_tree_.currentNode();
    if (node->type() == NODE_TYPE_SHORTCUTS)
    {
        down_cast<ShortcutsNode *>(node)->removeShortcut(selectedNodePath());
        updateAll(true);
        return true;
    }
    return false;
}

bool ModelView::removeDirectory(const QString &path)
{
    sys::SysStatus::instance().setSystemBusy(true);
    QStringList args;
    args << "-rf";
    args << path;
    sys::runScriptBlock("rm", args);
    sys::SysStatus::instance().setSystemBusy(false);
    return true;
}

void ModelView::clearAllWritePads()
{
    BranchNode * node = model_tree_.branchNode(NODE_TYPE_WRITEPAD_CONTAINER);
    if (node)
    {
        down_cast<WritePadContainer *>(node)->clearAll();
    }
}


/// Delete the content in clipboard and clear the clipboard.
bool ModelView::deleteFile(bool ask)
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }

    QString path = ptr->absolute_path();
    if (ptr->type() != NODE_TYPE_FILE &&
        ptr->type() != NODE_TYPE_DIRECTORY &&
        ptr->type() != NODE_TYPE_WRITEPAD)
    {
        return false;
    }

    if (ask)
    {
        DeleteFileDialog dialog(path, this);
        int ret = dialog.exec();
        if (ret != QMessageBox::Yes)
        {
            return false;
        }
    }

    QString error;
    bool dirty = false;
    if (ptr->type() == NODE_TYPE_FILE ||
        ptr->type() == NODE_TYPE_WRITEPAD)
    {
        if (down_cast<FileNode *>(ptr)->remove(error))
        {
            dirty = true;
        }
    }
    else if (ptr->type() == NODE_TYPE_DIRECTORY)
    {
        removeDirectory(path);
        dirty = true;
    }

    if (!error.isEmpty())
    {
    }

    if (dirty)
    {
        updateAll(true);
    }
    return true;
}

bool ModelView::deleteNote()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }

    if (ptr->type() != NODE_TYPE_NOTE)
    {
        return false;
    }

    mdb().removeNote(ptr->name());
    updateAll(true);
    return true;
}

bool ModelView::deleteWritePad()
{
    return deleteFile(false);
}
bool ModelView::deleteWebSite()
{
    Node * ptr = firstSelectedNode();
    if (ptr == 0)
    {
        return false;
    }

    const QString & name = ptr->name(); 
    qDebug("delete website %s",qPrintable(name));
    BranchNode* website = model_tree_.currentNode();
    down_cast<WebSitesNode*>(website)->removeWebSite(name);
    updateAll(true);

    return true;
}

bool ModelView::addWebSite()
{
    QString result;
    UrlDialog dialog(this);
    result = dialog.popup(result);
    if (result.isEmpty())
    {
        return false;
    }

    qDebug("addWebSite %s",qPrintable(result));
    BranchNode* website = model_tree_.currentNode();
    if (!down_cast<WebSitesNode*>(website)->addWebSite(result))
    {
        ErrorDialog error(tr("url invalid"),this);
        error.exec();
        return false; 
    }
    updateAll(true);

    return true;
}

void ModelView::onDesktopClicked()
{
    model_tree_.cdDesktop();
    updateAll(true);
}

void ModelView::onBranchClicked(const QString &name)
{
    model_tree_.cdBranch(name);
    updateAll(true);
}

void ModelView::onPathClicked(const QString &path)
{
    // Either cd or updateAll(true) can rescan the children nodes.
    QStringList p;
    model_tree_.modelPath(path, p);
    model_tree_.cdPath(p);
    updateAll(true);
}

void ModelView::onPathCloseClicked()
{
    model_tree_.currentNode()->clearNameFilters();
    updateAll(true);
}

/// Update title node and all child nodes.
/// Set the first visible according to the offset.
void ModelView::updateHeaderNode(BranchNode *node)
{
    // Update pathbar.
    path_bar_.updateAll();

    // Update header view.
    header_view_.updateNode(model_tree_.currentNode());
    if (view_mode_ == LIST_VIEW)
    {
        header_view_.show();
    }
    else
    {
        header_view_.hide();
    }
}

/// Update children nodes.
/// \regenerate will remove all existing children nodes and scan them again.
/// \update does not remove existing children nodes and only ask the node to
/// update its information only.
void ModelView::updateChildNodes(bool regenerate, bool update)
{
    NodePtrs & children  = children_nodes(regenerate, update);

    // May have to adjust first_visible_ and selected_ item.
    if (first_visible_ >= static_cast<int>(children.size()))
    {
        first_visible_ = static_cast<int>(children.size() - 1) / items_per_page_;
        first_visible_ *= items_per_page_;
    }
    if (first_visible_ < 0)
    {
        first_visible_ = 0;
    }

    // The selected item.
    if (selected_ < 0)
    {
        selected_ = 0;
    }
    else if (selected_ >= static_cast<int>(children.size()))
    {
        selected_ = first_visible_;
    }

    updateChildNodeViews(children, first_visible_, selected_, items_per_page_);
}

/// Change view mode. When view mode is changed, we also need to
/// re-calculate the first visible item position or the selected
/// item position. To make sure user can still see the previous visible items.
bool ModelView::setViewType(ViewType mode)
{
    if (view_mode_ == mode || mode == INVALID_VIEW)
    {
        return false;
    }

    view_mode_ = mode;

    updateHeaderNode();

    calculatePositions();

    switch (view_mode_)
    {
    case LIST_VIEW:
        updateChildNodeViews(children_nodes(false, false),
                             first_visible_,
                             selected_,
                             items_per_page_);
        updateGridLayout(rows_, columns_);
        break;
    case DETAILS_VIEW:
        updateChildNodeViews(children_nodes(false, false),
                             first_visible_,
                             selected_,
                             items_per_page_);
        updateGridLayout(rows_, columns_);
        break;
    case THUMBNAIL_VIEW:
        updateChildNodeViews(children_nodes(false, false),
                             first_visible_,
                             selected_,
                             items_per_page_);
        updateGridLayout(rows_, columns_);

        // Try to render thumbnail if any.
        onVisibleNodesChanged();
        break;
    default:
        assert(false);
        break;
    }
    updateStatusBar();
    return true;
}

void ModelView::updateStatusBar()
{
    int new_page = first_visible_ / (items_per_page_) + 1;
    int new_count = model_tree_.currentNode()->children().size() / (items_per_page_);
    if (new_count * items_per_page_ <
        static_cast<int>(model_tree_.currentNode()->children().size()))
    {
        ++new_count;
    }
    if (new_count == 0)
    {
        new_count = 1;
    }

    if (current_page_ != new_page ||
        page_count_ != new_count)
    {
        useDWUpdate(false);
        current_page_ = new_page;
        page_count_ = new_count;
        status_bar_.setProgress(current_page_, page_count_);
    }
}

void ModelView::resetCursor()
{
    first_visible_ = 0;
    selected_ = first_visible_;
}

/// Re-calculate rows, columns, first_visible item according to current
/// view mode.Make sure the selected item is alwasy visible when
/// view mode switched.
void ModelView::calculatePositions()
{
    int width = size().width();
    int height = size().height() - header_view_.height() - status_bar_.height();
    if (view_mode_ == LIST_VIEW)
    {
        static const int HEIGHT = 50;
        rows_ =  height / HEIGHT;
        columns_ = 1;
    }
    else if (view_mode_ == DETAILS_VIEW)
    {
        static const int HEIGHT = 98;
        rows_ =  height / HEIGHT;
        columns_ = 1;
    }
    else if (view_mode_ == THUMBNAIL_VIEW)
    {
        static const int WIDTH  = 120;
        static const int HEIGHT = 150;
        rows_ =  height / HEIGHT;
        columns_ = width / WIDTH;
    }

    // It happens only when the widget is too small to display anything.
    if (rows_ <= 0) rows_ = 1;
    if (columns_ <= 0) columns_ = 1;
    items_per_page_ = rows_ * columns_;

    if (selected_ == NO_SELECTION)
    {
        first_visible_ = 0;
    }
    else
    {
        first_visible_ = selected_ / (items_per_page_) * (items_per_page_);
    }
}

void ModelView::navigateKeyDown()
{
    int offset = static_cast<int>(children_nodes(false, false).size()) - selected_ - 1;
    if (offset < 0)
    {
        return;
    }

    if (offset > columns_)
    {
        offset = columns_;
    }

    if (selectItem(offset))
    {
        useDWUpdate(true);
        updateChildNodes(false, false);
        onVisibleNodesChanged();
        updateStatusBar();
    }
}

bool ModelView::nextPage()
{
    if (first_visible_ + items_per_page_ >=
        static_cast<int>(children_nodes(false, false).size()))
    {
        return false;
    }

    first_visible_ += items_per_page_;
    selected_ = first_visible_;
    return true;
}

bool ModelView::prevPage()
{
    if (first_visible_ <= 0)
    {
        return false;
    }

    first_visible_ -= items_per_page_;
    if (first_visible_ < 0)
    {
        first_visible_  = 0;
    }
    selected_ = first_visible_;
    return true;
}

bool ModelView::jumpPage(const int page)
{
    if (current_page_ == page ||
        page > page_count_)
    {
        return false;
    }

    // Update the first visible position.
    first_visible_ = (page - 1)* items_per_page_;
    selected_ = first_visible_;

    updateChildNodes(false, false);
    onVisibleNodesChanged();
    updateStatusBar();
    return true;
}

bool ModelView::gotoUp()
{
    // Before we goto up, we need to check the position.
    if (model_tree_.canGoUp())
    {
        QString name;
        BranchNode * node = model_tree_.currentNode();
        FolderNode * folder = model_tree_.folderNode(node);
        if (folder)
        {
            name = folder->leafNodeName();
        }
        else if (node)
        {
            name = node->name();
        }
        model_tree_.cdUp();

        // Recalculate the offset, bug, need to consider the sort order.
        selected_ = model_tree_.currentNode()->nodePosition(name);
        calculatePositions();
        return true;
    }
    return false;
}

/// Depends on current node.
bool ModelView::changeSortCriteria(Field by, SortOrder order)
{
    if (model_tree_.sort_order() == order &&
        model_tree_.sort_field() == by)
    {
        return false;
    }

    // Tell the model to remember the new sort criterial.
    model_tree_.changeSortCriteria(by, order);

    // Record the current selected one.
    // We don't need to make selected item visible.
    //NodePtrs& children = children_nodes(false, false);
    //Node *ptr = 0;
    //if (selected_ > NO_SELECTION && selected_ < static_cast<int>(children.size()))
    //{
    //    ptr = children[selected_];
    //}
    //else
    //{
    //    selected_ = NO_SELECTION;
    //}

    // Sort.
    model_tree_.currentNode()->sort(by, order);
    header_view_.updateNode(model_tree_.currentNode());

    // Update the selected index.
    //NodePtrsIter pos = children.end();
    //if (ptr)
    //{
    //    pos = std::find(children.begin(), children.end(), ptr);
    //}
    //if (pos != children.end())
    //{
    //    selected_ = static_cast<int>(pos - children.begin());
    //    first_visible_ = selected_ / (items_per_page_) * (items_per_page_);
    //}
    //else
    //{
    //    first_visible_ = 0;
    //}


    updateChildNodeViews(children_nodes(false, false),
                         first_visible_,
                         selected_,
                         items_per_page_);
    updateStatusBar();

    return true;
}

void ModelView::onFieldResized(Field field, int x, int width)
{
    // Conver the size to position.
    if (view_mode_ != LIST_VIEW)
    {
        return;
    }

    for(int i = 0; i < static_cast<int>(node_views_.size()); ++i)
    {
        node_views_[i]->fieldResized(field, x, width);
    }
}

/// File system signal handler. File system changed means some
/// mount point has been changed.
void ModelView::fileSystemChanged(const NodeType type, bool mount)
{
    // Update model no matter it's mounted or not.
    BranchNode* node   = model_tree_.branchNode(type);
    FolderNode* folder = model_tree_.folderNode(node);
    if (folder)
    {
        folder->fileSystemChanged();
    }

    // Update user interface is it's mounted or it's visible now.
    if (mount || node == model_tree_.currentNode())
    {
        model_tree_.cdBranch(type);
        resetCursor();
        updateAll(true);
    }
}

void ModelView::onStatusItemClicked(const int percent, const int value)
{
    jumpPage(value);
}

void ModelView::onMenuClicked()
{
    popupMenu();
}

/// Change the selection item position according to the offset.
/// It returns false if selection position is not changed.
bool ModelView::selectItem(const int offset)
{
    int new_selected = selected_ + offset;

    if (new_selected >= static_cast<int>(children_nodes(false, false).size()))
    {
        return false;
    }
    else if (new_selected < 0)
    {
        return false;
    }
    selected_ = new_selected;

    // Have to update the first visible position.
    first_visible_ = selected_ / (items_per_page_) * (items_per_page_);
    return true;
}


bool ModelView::selectNode(Node *node)
{
    NodePtrs& children = children_nodes(false, false);
    NodePtrsIter iter = std::find(children.begin(),
                                  children.end(),
                                  node);
    if (iter == children.end())
    {
        return false;
    }

    // Check if it's already selected.
    int new_selected = iter - children.begin();
    if (selected_ == new_selected)
    {
        return true;
    }
    else
    {
        selected_ = new_selected;
    }

    // Update the node view.
    updateChildNodeViews(children, first_visible_, selected_, items_per_page_);
    return true;
}

bool ModelView::guiSelectNode(Node *node)
{
    // Disable screen update now.
    onyx::screen::instance().enableUpdate(false);

    // Make the node selected.
    if (selectNode(node))
    {
        // Process all pending events to update user interface.
        repaint();
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW, true);
    }
    onyx::screen::instance().enableUpdate(true);
    return true;
}

void ModelView::popupMenu()
{
    ui::PopupMenu menu(this);
    updateMenuActions();

    // Add to group.
    menu.addGroup(&view_type_actions_);

    // Check size.
    if (file_actions_.actions().size() > 0)
    {
        menu.addGroup(&file_actions_);
    }

    // Check if recent document has been selected.
    if (isRecentDocumentsSelected())
    {
        menu.addGroup(&history_actions_);
    }
    menu.addGroup(&sys_setting_actions_);
    menu.setSystemAction(&device_actions_);

    // Check website.
    if (website_actions_.actions().size() > 0)
    {
        menu.addGroup(&website_actions_);
    }

    // Show menu
    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == view_type_actions_.category())
    {
        handleViewTypeActions(view_type_actions_);
    }
    else if (group == file_actions_.category())
    {
        // To clear the menu background.
        handleFileActions(file_actions_);
    }
    else if (group == history_actions_.category())
    {
        handleRecentDocumentsActions(history_actions_);
    }
    else if (group == sys_setting_actions_.category())
    {
        // To clear the menu background.
        showSettingsDialog(sys_setting_actions_.selected());
    }
    else if (group == device_actions_.category())
    {
        execDeviceAction(device_actions_.selected());
    }
    else if (group == website_actions_.category())
    {
        handleWebSiteActions(website_actions_);
    }
}

void ModelView::updateMenuActions()
{
    QAction * action = 0;

    // Update view actions.
    view_type_actions_.generateActions(view_mode_, model_tree_.sort_field(), model_tree_.sort_order());
    sys_setting_actions_.generateActions();
    device_actions_.generateActions();

    // Update according to current selection.
    FileActionTypes types;
    if (isFileNodeSelected())
    {
        types = FILE_DELETE|FILE_SEARCH|FILE_RENAME|FILE_CUT|FILE_COPY;
        if (isReturnable())
        {
            types |= FILE_DRM_RETURNABLE;
        }
        if (!isShortcutsSelected())
        {
            types |= FILE_CREATE_SHORTCUT;
        }
        if (!isClipboardEmpty())
        {
            types |= FILE_PASTE|FILE_PASTE_TO_DIR;
        }
    
    }
    else if (isFolderNodeSelected())
    {
        types = FILE_DELETE|FILE_SEARCH|FILE_RENAME|FILE_CUT|FILE_COPY;
        if (isShortcutsSelected())
        {
            types |= FILE_DELETE;
        }
        else
        {
            types |= FILE_CREATE_SHORTCUT;
        }
        if (!isClipboardEmpty())
        {
            types |= FILE_PASTE|FILE_PASTE_TO_DIR;
        }
    }

    else if (isNoteSelected())
    {
        types = FILE_CLEAR_ALL_NOTES|FILE_DELETE;
    }
    else if (isCreateNoteSelected())
    {
        types = FILE_CLEAR_ALL_NOTES;
    }
    else if (isWritePadSelected())
    {
        types = FILE_CLEAR_ALL_NOTES|FILE_DELETE;
    }
    else if (isCreateWritePadSelected())
    {
        types = FILE_CLEAR_ALL_WRITEPADS;
    }

    // Check if file is selected and we're in sd card or flash.
    if (!isClipboardEmpty() && (isInFlash() || isInSDCard()))
    {
        types |= FILE_PASTE;
    }

    file_actions_.generateActions(types);

    if (isRecentDocumentsSelected())
    {
        history_actions_.generateActions();
    }

    if (isInWebSite())
    {
        WebSiteActionTypes website_types = WEBSITE_ADD;
        if(isWebSiteSelected())
        {
            website_types |= WEBSITE_DELETE;
        }
        website_actions_.generateActions(website_types);
    }
    else
    {
        website_actions_.clear();
    }
}

void ModelView::showSettingsDialog(NodeType setting)
{
    // To clear dirty region.
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU, true, onyx::screen::ScreenCommand::WAIT_ALL);

    // The dialog will update screen itself.
    sys::SystemConfig conf;
    if (setting == NODE_TYPE_LOCALE)
    {
        LocaleDialog dialog(this, conf);
        dialog.exec();
    }
    else if (setting == NODE_TYPE_TIMEZONE)
    {
        showTimezoneDialog();
    }
    else if (setting == NODE_TYPE_PM)
    {
        status_bar_.onBatteryClicked();
    }
    else if (setting == NODE_TYPE_DATE)
    {
        DateDialog dialog(this);
        dialog.exec();
    }
    else if (setting == NODE_TYPE_ABOUT)
    {
        showAboutDialog();
    }
    else if (setting == NODE_TYPE_FONTMANAGEMENT)
    {
        showFontManagement(conf);
    }
    else if (setting == NODE_TYPE_SCREEN_CALIBRATION)
    {
        controller::SystemController::instance().startCalibration();
    }
    else if (setting == NODE_TYPE_NETWORK_PROFILE)
    {
        controller::SystemController::instance().startNetworkManager();
    }
    else if (setting == NODE_TYPE_FORMAT_FLASH)
    {
        formatFlash();
    }
    else if (setting == NODE_TYPE_USER_MANUAL)
    {
        openUserManual();
    }
    else if (setting == NODE_TYPE_WAVEFORM_SETTINGS)
    {
        showWaveformDialog();
    }
    else if (setting == NODE_TYPE_REMOVE_ACCOUNT_INFO)
    {
        removeAccountInfo();
    }
    else if (setting == NODE_TYPE_STARTUP)
    {
        showStartupDialog(conf);
    }
    else if (setting == NODE_TYPE_SCREEN_UPATE_SETTING)
    {
        showScreenUpdateDialog(conf);
    }
}

/// Show dialog with less screen update.
void ModelView::showDialog(QDialog & dialog)
{
    onyx::screen::instance().enableUpdate(false);
    dialog.show();
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
}

void ModelView::execDeviceAction(ExplorerDeviceAction action)
{
    if (action == DEVICE_UMOUNT_SD_CARD)
    {
        umountSD();
    }
    else if (action == DEVICE_UMOUNT_USB)
    {
        sys_status_.umountUSB();
    }
    else if (action == DEVICE_ROTATE_SCREEN)
    {
        sys_status_.rotateScreen();
    }
    else if (action == DEVICE_STANDBY)
    {
        suspend();
    }
    else if (action == DEVICE_SHUTDOWN)
    {
        shutdown(SHUTDOWN_REASON_USER_REQUEST);
    }
    else if (action == DEVICE_MUSIC)
    {
        // Start or show music player.
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
        sys_status_.setSystemBusy(true);
        controller::SystemController::instance().requestMusicService(sys::START_PLAYER);
    }
}

void ModelView::handleViewTypeActions(ViewTypeActions & actions)
{
    onyx::screen::instance().enableUpdate(false);
    bool changed = setViewType(actions.selectedViewType());
    changed |= changeSortCriteria(actions.selectedField(), actions.selectedOrder());
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);

    // If it's not changed, we still need to update the screen.
    if (!changed)
    {
        onyx::screen::instance().updateWidget(parentWidget(), onyx::screen::ScreenProxy::GU);
    }
    else
    {
        onyx::screen::instance().updateWidget(parentWidget(), onyx::screen::ScreenProxy::GC);
    }
}

void ModelView::handleFileActions(FileActions & actions)
{
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU, true, onyx::screen::ScreenCommand::WAIT_ALL);

    if (actions.selected() == ui::FILE_DELETE)
    {
        deleteSelectedNode();
    }
    else if (actions.selected() == ui::FILE_SEARCH)
    {
        search();
    }
    else if (actions.selected() == ui::FILE_CREATE_SHORTCUT)
    {
        addShortcut(firstSelectedNode());
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU, true, onyx::screen::ScreenCommand::WAIT_ALL);
    }
    else if (actions.selected() == ui::FILE_CLEAR_ALL_NOTES)
    {
        mdb().removeAllNotes();
        updateAll(true);
    }
    else if (actions.selected() == ui::FILE_CLEAR_ALL_WRITEPADS)
    {
        clearAllWritePads();
        updateAll(true);
    }
    else if (actions.selected() == ui::FILE_DRM_RETURNABLE)
    {
        returnBook(firstSelectedNode());
    }
    else if (actions.selected() == ui::FILE_RENAME)
    {
        rename(firstSelectedNode());
    }
    else if (actions.selected() == ui::FILE_CUT)
    {
        addToClipboard(firstSelectedNode());
        is_cut_=true;
    }
    else if (actions.selected() == ui::FILE_COPY)
    {
        addToClipboard(firstSelectedNode());
        is_cut_=false;
    }
    else if (actions.selected() == ui::FILE_PASTE_TO_DIR)
    {
        bool update = (is_cut_ && QFileInfo(source_nodes_).path().compare (model_tree_.currentNode()->absolute_path()) == 0 ) ;
        Node * ptr = firstSelectedNode();
        if (ptr)
        {
            QString path;
            path=ptr->absolute_path();
            is_cut_?move(path):copy(path);
        }

        if (update)
        {
            updateAll(true);
        }
        else
        {
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU, true, onyx::screen::ScreenCommand::WAIT_ALL);
        }
    }
    else if (actions.selected() == ui::FILE_PASTE)
    {
        QString path = model_tree_.currentNode()->absolute_path();
        if (!path.isEmpty())
        {
            is_cut_?move(path):copy(path);
        }

        updateAll(true);
    }
}
void ModelView::handleWebSiteActions(WebSiteActions & actions)
{
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU, true, onyx::screen::ScreenCommand::WAIT_ALL);

    if (actions.selected() == ui::WEBSITE_ADD)
    {
        addWebSite();
    }
    else if (actions.selected() == ui::WEBSITE_DELETE)
    {
        deleteWebSite();
    }
    
}

void ModelView::handleRecentDocumentsActions(HistoryActions & actions)
{
    if (actions.selected() == ui::HISTORY_CLEAR_ALL)
    {
        BranchNode * node = model_tree_.currentNode();
        if (node->type() == NODE_TYPE_RECENT_DOCS)
        {
            down_cast<LibraryNode *>(node)->clearAll();
        }
        updateAll(true);
    }
}

bool ModelView::continueReading()
{
    return (processFileNode(mdb().latestReading()));
}

bool ModelView::umountSD()
{
    // Jump to root if necessary, otherwise the sd card may not unmount correctly.
    if (model_tree_.currentType() == NODE_TYPE_SD)
    {
        stopBackgroundJobs();
        model_tree_.cdBranch(NODE_TYPE_SD);
    }

    sys_status_.umountSD();
    updateAll(true);
    // node->fileSystemChanged();
    if (!sys_status_.umountSD())
    {
        ui::MessageDialog dialog(QMessageBox::Warning,
                                 QApplication::tr("Warning"),
                                 QApplication::tr("Could not umount SD card. The file may be in used."),
                                 QMessageBox::Yes);
        dialog.exec();
        return false;
    }
    return true;
}

void ModelView::formatFlash()
{
    FormatFlashDialog dialog(this);
    if (dialog.exec() != QMessageBox::Yes)
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, true);
        return;
    }

    repaint();
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, true);
    SystemController::instance().formatFlash();
    update();
}

void ModelView::formatSD()
{
    FormatSDDialog dialog(this);
    if (dialog.exec() != QMessageBox::Yes)
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, true);
        return;
    }

    repaint();
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, true);
    SystemController::instance().formatFlash();
    update();
}

void ModelView::removeAccountInfo()
{
    MessageDialog dialog(QMessageBox::Information,
                         tr("Remove Account Information"),
                         tr("Are you sure to remove account information?"),
                         QMessageBox::Yes|QMessageBox::No);
    int ret = dialog.exec();
    if (ret != QMessageBox::Yes)
    {
        return;
    }

    // remove the activation record
    QString path;
#ifdef Q_WS_QWS
    path = ("/media/flash");
#else
    path = QDir::home().path();
#endif
    QDir dir(path);
    if (dir.cd(".adobe-digital-editions"))
    {
        path = dir.filePath("activation.xml");
        QFile activation_record(path);
        if (activation_record.exists())
        {
            activation_record.remove();
        }
    }

    // remove private conf db
    QFile private_conf_db(QDir::home().filePath("private_config.db"));
    if (private_conf_db.exists())
    {
        private_conf_db.remove();
    }
}

void ModelView::showTimezoneDialog()
{
    TimeZoneDialog dialog(this);
    if (dialog.popup(QCoreApplication::tr("Time Zone Settings")) != QDialog::Accepted)
    {
        return;
    }
    sys::SystemConfig::setTimezone(dialog.selectedTimeZone());
}

void ModelView::showAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void ModelView::showFontManagement(sys::SystemConfig& conf)
{   
    FontManagementDialog dialog(this,conf);
    dialog.exec();
    updateAll(true);
}

void ModelView::openUserManual()
{
    QDir dir(SHARE_ROOT);
    dir.cd("explorer/manual/");
    QString name(QLocale::system().name());
    name += ".pdf";

    // Before open it, we need to make a check.
    QString path = dir.absoluteFilePath(name);
    if (!QFile::exists(path))
    {
        // Fallback to en_US always.
        name = "en_US.pdf";
        path = dir.absoluteFilePath(name);
    }
    processFileNode(dir.absoluteFilePath(path));
}

void ModelView::showWaveformDialog()
{
    WaveformDialog dialog(this);
    dialog.exec();
}

void ModelView::showStartupDialog(sys::SystemConfig& conf)
{
    StartupDialog dialog(0, conf);
    dialog.exec();
}

void ModelView::showScreenUpdateDialog(sys::SystemConfig& conf)
{
    ScreenUpdateDialog dialog(0, conf);
    dialog.exec();
}

void ModelView::suspend()
{
    // Make sure all background jobs have been stopped.
    stopBackgroundJobs();

    // suspend.
    controller::SystemController::instance().suspend(sys_status_);
}

void ModelView::shutdown(int r)
{
    // Change the internal state if necessary.
    mutable_internal_state() = r;

    // Close all viewers.
    controller::SystemController::instance().stopAllApplications(true);

    // Ensure all settings have been stored.
    saveSettings();

    // Make sure all background jobs have been stopped.
    stopBackgroundJobs();

    // Hide all sub widgets.
    onyx::screen::instance().enableUpdate(false);
    showChildren(false);
    update();
    while (!canShutdown())
    {
        onyx::screen::instance().flush();
    }

    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC, true, onyx::screen::ScreenCommand::WAIT_ALL);

    // shutdown.
    controller::SystemController::instance().shutdown(sys_status_, r);
}

void ModelView::onVisibleNodesChanged()
{
    // Double check.
    if (view_mode_ != THUMBNAIL_VIEW && view_mode_ != DETAILS_VIEW)
    {
        return;
    }

    // Start metadata extraction timer only when it's active window.
    if (parentWidget() && parentWidget()->isActiveWindow())
    {
        metadata_pos_ = 0;
        enableExtractMetadata(true);

        // Use singleshot timer to ensure the screen update finished.
        QTimer::singleShot(0, this, SLOT(extractMetadata()));
    }
}

void ModelView::extractMetadata()
{
    while (metadata_pos_ < node_views_.size())
    {
        NodeView *ptr = node_views_[metadata_pos_].get();
        if (ptr->updateMetadata())
        {
            ++metadata_pos_;
            continue;
        }

        if (ptr->node() && ptr->node()->type() == NODE_TYPE_FILE && isExtractMetadataEnabled())
        {
            if (controller::SystemController::instance().extractMetadata(ptr->node()->absolute_path()))
            {
                break;
            }
        }
        ++metadata_pos_;
    }
}

void ModelView::onMetadataReady(const QString &path)
{
    // Update metadata now.
    for(size_t i = 0; i < node_views_.size(); ++i)
    {
        NodeView *ptr = node_views_[i].get();
        if (ptr && ptr->node())
        {
            if (ptr->node()->absolute_path() == path)
            {
                ptr->updateMetadata();
                break;
            }
        }
    }

    ++metadata_pos_;
    extractMetadata();
}

void ModelView::stopExtractingMetadata()
{
    enableExtractMetadata(false);
    controller::SystemController::instance().stopExtracting();
}

void ModelView::stopBackgroundJobs()
{
    stopExtractingMetadata();
}

void ModelView::enableShutdown(bool enable)
{
    can_shutdown_ = enable;
}

/// Check if we can really shutdown the device now.
bool ModelView::canShutdown()
{
    return can_shutdown_;
}

void ModelView::navigateUrl(const QString & url)
{
    // Disable screen update.
    onyx::screen::instance().enableUpdate(false);

    // Try to open it.
    if (SystemController::instance().navigateTo(url))
    {
        // Disable any other user event now.
        // sys_status_.setSystemBusy(true);
        // If we are able to open the document, it's better
        // to stop the thumbnail rendering. So that the viewer
        // can get more resource.
        stopBackgroundJobs();
    }
    else
    {
        // If we can not open it, enable screen update then.
        onyx::screen::instance().enableUpdate(true);
    }
}

/// Return the book.
void ModelView::returnBook(Node *ptr)
{
    WifiDialog dialog(0, sys::SysStatus::instance());
    dialog.popup();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU, true, onyx::screen::ScreenCommand::WAIT_ALL);

    if (ptr)
    {
        controller::SystemController::instance().returnLoanBook(ptr->absolute_path());
    }
}


}  // namespace view

}  // namespace explorer
