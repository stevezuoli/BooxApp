
#include <QtGui/QtGui>
#include "view.h"
#include "model_manager.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys.h"

using namespace ui;
using namespace vbf;

namespace reader
{

static const int PAGE_REPEAT = 20;
static const int DELTA = 10;

ReaderView::ReaderView(QWidget *parent)
    : QWebView(parent)
    , progress_(0)
    , scrollbar_hidden_(0)
    , update_type_(onyx::screen::ScreenProxy::GU)
    , enable_text_selection_(false)
    , change_view_port_(true)
{
    // In order to receive link clicked event.
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    // Setup connections.
    connect(this, SIGNAL(linkClicked(const QUrl &)), this, SLOT(onLinkClicked(const QUrl &)));
    connect(this, SIGNAL(loadStarted(void)), this, SLOT(onLoadStarted(void)));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(onLoadProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
    connect(page(), SIGNAL(repaintRequested(const QRect &)), this, SLOT(onRepaintRequested(const QRect&)));

    // Use queued connection to make sure selected text is reported correctly.
    connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()), Qt::QueuedConnection);

    connect(&sys::SysStatus::instance(), SIGNAL(musicPlayerStateChanged(int)),
            this, SLOT(onMusicPlayerStateChanged(int)));

    QFile file;
    file.setFileName(":/res/jquery.min.js");
    if (file.open(QIODevice::ReadOnly))
    {
        jquery_ = file.readAll();
        file.close();
    }
}

ReaderView::~ReaderView()
{
    onyx::screen::instance().setDefaultWaveform(system_update_type_);
}

bool ReaderView::open(const QString &path)
{
    model_.reset(ModelManager::instance().createInstance(path));

    if (!model_)
    {
        qWarning("No plugin found for %s", qPrintable(path));
        return false;
    }

    if (!model_->open(path))
    {
        qWarning("Could not open document %s", qPrintable(path));
        return false;
    }

    if (!manager_)
    {
        manager_.reset(new ReaderNetworkManager(model_.get()));
        page()->setNetworkAccessManager(manager_.get());
    }

    path_ = path;
    loadOptions(path);
    load(current_location());
    return true;
}

bool ReaderView::close()
{
    saveOptions(path_);
    return true;
}

void ReaderView::saveOptions()
{
    saveOptions(path_);
}

void ReaderView::onLinkClicked(const QUrl &url)
{
    qDebug("url clicked %s", qPrintable(url.toString()));
    load(url);
}

void ReaderView::onLoadStarted(void)
{
    // In order to use javascript to hide scrollbar.
    scrollbar_hidden_ = 0;
    progress_ = 0;

    // Store the screen update type.
    resetUpdateRect();
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
}

void ReaderView::onLoadProgress(int progress)
{
    // For both vertical and horizontal.
    if (scrollbar_hidden_ <= 2)
    {
        QVariant var = page()->currentFrame()->evaluateJavaScript("document.body.style.overflow = 'hidden';");
        if (!var.isNull())
        {
            ++scrollbar_hidden_;
        }
    }

    progress_ = progress;
    emit progressChangedSignal(progress, 100);
}

void ReaderView::onLoadFinished(bool ok)
{
    // Restore the screen update type.
    onyx::screen::instance().setDefaultWaveform(update_type_);
    update_rect_ = rect();
    progress_ = 100;

    if (change_view_port_)
    {
        change_view_port_ = false;
        setViewport(conf().options[CONFIG_VIEW_PORT].toPointF());
    }

    if (ok)
    {
        page()->currentFrame()->evaluateJavaScript(jquery_);
        //hideGif();
        hideScrollbar();
    }

    updateViewportRange();
}

void ReaderView::hideGif()
{
    QString code = "$('[src*=gif]').hide()";
    page()->currentFrame()->evaluateJavaScript(code);
}

void ReaderView::hideScrollbar()
{
    QString code = "$('body').css('overflow', 'hidden')";
    page()->currentFrame()->evaluateJavaScript(code);
}

void ReaderView::onRangeClicked(const int percentage,
                                 const int value)
{
    int height = page()->currentFrame()->contentsSize().height();
    QPointF pt = currentOffset();
    pt.ry() = height * percentage / 100 - rect().height();
    myScrollTo(pt.toPoint());
    updateViewportRange();
}

void ReaderView::myScroll(int dx, int dy)
{
    page()->currentFrame()->scroll(dx, dy);
    updateViewportRange();
}

void ReaderView::myScrollTo(const QPoint &pt)
{
    page()->currentFrame()->setScrollPosition(pt);
    updateViewportRange();
}


QPointF ReaderView::currentOffset()
{
    return page()->currentFrame()->scrollPosition();
}

void ReaderView::updateViewportRange()
{
    // Get current location.
    QSizeF s = page()->currentFrame()->contentsSize();
    QPointF pt = currentOffset();
    emit viewportRangeChangedSignal(static_cast<int>(pt.y()),
                                    static_cast<int>(rect().height()),
                                    static_cast<int>(s.height()));
}


void ReaderView::onRepaintRequested(const QRect&rc)
{
    update_rect_ = update_rect_.united(rc);

    // Improve performance of text selection.
    if (isTextSelectionEnabled())
    {
        selected_rect_ = selected_rect_.unite(rc);
        selected_rect_ = selected_rect_.intersect(rect());
    }
}

void ReaderView::loadOptions(const QString &path)
{
    // Open the database to read options.
    if (openDatabase(path, database_))
    {
        loadDocumentOptions(database_, path, conf());
    }

    Configuration & cfg = conf();

    // Location.
    current_location_.setScheme(model_->scheme());
    QString location = cfg.options[CONFIG_LAST_LOCATION].toString();
    if (!location.isEmpty())
    {
        current_location_.setPath(location);
    }
    else
    {
        current_location_ = model_->home();
    }

    // Text size.
    bool ok = false;
    qreal factor = cfg.options[CONFIG_FONT_SIZE].toDouble(&ok);
    if (ok)
    {
        setTextSizeMultiplier(factor);
    }

    // Screen update type.
    onyx::screen::ScreenProxy::Waveform type = static_cast<onyx::screen::ScreenProxy::Waveform>(cfg.options[CONFIG_FLASH_TYPE].toInt());
    if (type == onyx::screen::ScreenProxy::GC)
    {
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
    }
}

void ReaderView::saveOptions(const QString &path)
{
    Configuration & cfg = conf();
    cfg.options[CONFIG_LAST_LOCATION] = url().path();
    cfg.options[CONFIG_FONT_SIZE] = textSizeMultiplier();
    cfg.options[CONFIG_FLASH_TYPE] = onyx::screen::instance().defaultWaveform();
    cfg.options[CONFIG_VIEW_PORT] = viewport();

    // Always reading. TODO
    QString progress(tr("Reading"));
    cfg.info.mutable_progress() = progress;
    saveDocumentOptions(database_, path, cfg);
}

void ReaderView::gotoHome()
{
    load(model_->home());
}

void ReaderView::showTableOfContents()
{
    QStandardItemModel toc_model;
    model_->tableOfContents(toc_model);

    TreeViewDialog dialog(this, toc_model);

    // Search the url in the model.
    QModelIndex selected;
    model_->indexFromUrl(toc_model, url(), selected);
    int ret = dialog.popup(tr("Table Of Contents"), selected);
    if (ret != QDialog::Accepted)
    {
        update();
        return;
    }

    QUrl url;
    if (!model_->urlFromIndex(toc_model, dialog.selectedItem(), url))
    {
        return;
    }
    load(url);
}

void ReaderView::updateActions()
{
    QString font = QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont);
    font_family_actions_.generateActions(font, true);

    std::vector<qreal> size;
    text_size_actions_.generateActions(size, textSizeMultiplier());
    navigation_actions_.generateActions(history());

    if (doc_map_actions_.actions().size() <= 0)
    {
        if (model_->supportTableOfContents())
        {
            doc_map_actions_.generateActions();
        }
    }

    // Reading tools
    if (reading_tool_actions_.actions().size() <= 0)
    {
        std::vector<ReadingToolsType> tools;
        tools.push_back(SEARCH_TOOL);
        // tools.push_back(TEXT_TO_SPEECH);
        tools.push_back(DICTIONARY_TOOL);
        tools.push_back(CLOCK_TOOL);
        reading_tool_actions_.generateActions(tools);
    }

    if (system_actions_.actions().size() <= 0)
    {
        system_actions_.generateActions();
    }
}

/// Popup the menu.
void ReaderView::popupMenu()
{
    ui::PopupMenu menu(this);
    updateActions();

    menu.addGroup(&font_family_actions_);
    menu.addGroup(&text_size_actions_);
    menu.addGroup(&navigation_actions_);
    if (model_->supportTableOfContents())
    {
        menu.addGroup(&doc_map_actions_);
    }
    menu.addGroup(&reading_tool_actions_);
    menu.setSystemAction(&system_actions_);
    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == font_family_actions_.category())
    {
        changeFontFamily(font_family_actions_.selectedFont());
    }
    else if (group == text_size_actions_.category())
    {
        if (textSizeMultiplier() != text_size_actions_.selectedMultiplier())
        {
            setTextSizeMultiplier(text_size_actions_.selectedMultiplier());
        }
        else
        {
            update(rect());
        }
    }
    else if (group == navigation_actions_.category())
    {
        if (navigation_actions_.selected() == NAVIGATE_FORWARD)
        {
            forward();
        }
        else if (navigation_actions_.selected() == NAVIGATE_BACKWARD)
        {
            back();
        }
        else if (navigation_actions_.selected() == NAVIGATE_HOME)
        {
            // TODO. We may display thumbnails of recent access list here just like chrome.
            gotoHome();
        }
    }
    else if (group == doc_map_actions_.category())
    {
        if (doc_map_actions_.selected() == INDEX_TOC)
        {
            showTableOfContents();
        }
    }
    else if (group == reading_tool_actions_.category())
    {
        if (reading_tool_actions_.selectedTool() == DICTIONARY_TOOL)
        {
            startDictLookup();
        }
        else if (reading_tool_actions_.selectedTool() == TEXT_TO_SPEECH)
        {
            // startTTS();
        }
        else if (reading_tool_actions_.selectedTool() == SEARCH_TOOL)
        {
            showSearchWidget();
        }
        else if (reading_tool_actions_.selectedTool() == CLOCK_TOOL)
        {
            emit clockClicked();
        }
        return;
    }
    else if (group == system_actions_.category())
    {
        SystemAction system = system_actions_.selected();
        if (system == RETURN_TO_LIBRARY)
        {
            returnToLibrary();
        }
        else if (system == SCREEN_UPDATE_TYPE)
        {
            onyx::screen::instance().toggleWaveform();
            system_update_type_ = update_type_ = onyx::screen::instance().defaultWaveform();
            update();
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        }
        else if (system == MUSIC)
        {
            // Start or show music player.
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
            sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
        }
        else if (system == ROTATE_SCREEN)
        {
            emit rotateScreen();
        }
    }
}

QWebView *ReaderView::createWindow(QWebPage::WebWindowType type)
{
    return this;
}

void ReaderView::mousePressEvent(QMouseEvent*me)
{
    onyx::screen::instance().enableUpdate(false);
    if (isWidgetVisible(dict_widget_.get()))
    {
        enableTextSelection(true);
    }
    position_ = me->pos();
    return QWebView::mousePressEvent(me);
}

void ReaderView::mouseMoveEvent(QMouseEvent *me)
{
    /*
    if (isTextSelectionEnabled())
    {
        QWebView::mouseMoveEvent(me);
        repaint(selected_rect_);
        onyx::screen::instance().fastUpdateWidgetRegion(this, selected_rect_, false);
        selected_rect_.setCoords(0, 0, 0, 0);
    }
    else if(me->buttons() != Qt::NoButton)
    {
        QPoint delta = position_ - me->pos();
        page()->currentFrame()->scroll(delta.x(), delta.y());
        onyx::screen::instance().flush();
        onyx::screen::instance().enableUpdate(true);
        onyx::screen::instance().fastUpdateWidget(this, true);
        position_  = me->pos();
    }
    */
    me->accept();
}

void ReaderView::mouseReleaseEvent(QMouseEvent*me)
{
    QPoint delta = position_ - me->pos();
    if (isTextSelectionEnabled())
    {
        QWebView::mouseReleaseEvent(me);
        QContextMenuEvent ce(QContextMenuEvent::Mouse,
                             me->pos(),
                             me->globalPos(),
                             me->modifiers());
        page()->swallowContextMenuEvent(&ce);
        onyx::screen::instance().flush();
        onyx::screen::instance().updateWidgetRegion(this, selected_rect_, onyx::screen::ScreenProxy::DW, true);
        onyx::screen::instance().enableUpdate(true);
        emit selectionChanged();
        return;
    }

    onyx::screen::instance().enableUpdate(true);
    if (abs(delta.x()) < DELTA && abs(delta.y()) < DELTA)
    {
        // Could click a link.
        QWebView::mouseReleaseEvent(me);
        return;
    }

    // Pan.
    me->accept();
    page()->currentFrame()->scroll(delta.x(), delta.y());
}

void ReaderView::keyPressEvent(QKeyEvent *e)
{
    // We only handle key release event, so ignore some keys.
    switch (e->key())
    {
    case Qt::Key_Down:
    case Qt::Key_Up:
        break;
    default:
        QWebView::keyPressEvent(e);
        break;
    }
    e->accept();
}

void ReaderView::keyReleaseEvent(QKeyEvent *ke)
{
    switch (ke->key())
    {
    case ui::Device_Menu_Key:
        popupMenu();
        break;
    case Qt::Key_Left:
        myScroll(-rect().width() + PAGE_REPEAT, 0);
        break;
    case Qt::Key_Right:
        myScroll(rect().width() - PAGE_REPEAT, 0);
        break;
    case Qt::Key_PageDown:
    case Qt::Key_Down:
        scrollNext();
        break;
    case Qt::Key_PageUp:
    case Qt::Key_Up:
        scrollPrev();
        break;
    case Qt::Key_Escape:
        returnToLibrary();
        break;
    case Qt::Key_F:
        if (ke->modifiers() & Qt::ControlModifier)
        {
            showSearchWidget();
        }
        break;
    default:
        QWebView::keyReleaseEvent(ke);
        break;
    }
    ke->accept();
}

void ReaderView::contextMenuEvent(QContextMenuEvent * event)
{
    event->accept();
}

#ifndef QT_NO_WHEELEVENT
void ReaderView::wheelEvent(QWheelEvent *we)
{
    if (we->delta() < 0)
    {
        page()->currentFrame()->scroll(0, rect().height() - PAGE_REPEAT);
    }
    else if (we->delta() > 0)
    {
        page()->currentFrame()->scroll(0, -rect().height() + PAGE_REPEAT);
    }
    we->accept();
}
#endif

/// Ignore the double click event.
void ReaderView::mouseDoubleClickEvent(QMouseEvent*me)
{
    me->accept();
}

void ReaderView::onSelectionChanged()
{
    QString text = selectedText();
    if (selected_text_ == text)
    {
        return;
    }

    selected_text_ = text;
    // Now check if we need to update the dictionary widget or not.
    // Better to use a signal.
    if (!text.isEmpty() &&
        dict_widget_ &&
        dict_widget_->isVisible())
    {
        dict_widget_->lookup(text);
        dict_widget_->ensureVisible(selected_rect_);
    }
}

void ReaderView::startDictLookup()
{
    if (!dicts_)
    {
        dicts_.reset(new DictionaryManager);
    }

    if (!dict_widget_)
    {
        dict_widget_.reset(new DictWidget(this, *dicts_));
        connect(dict_widget_.get(), SIGNAL(closeClicked()), this, SLOT(stopDictLookup()));
    }

    hideHelperWidget(tts_widget_.get());
    hideHelperWidget(search_widget_.get());

    // When dictionary widget is not visible, it's necessary to update the text view.
    dict_widget_->lookup(selectedText());
    dict_widget_->ensureVisible(selected_rect_, true);
}

void ReaderView::stopDictLookup()
{
    hideHelperWidget(dict_widget_.get());
    enableTextSelection(false);
    selected_text_.clear();
    clearSelection();
}

void ReaderView::showSearchWidget()
{
    if (!search_widget_)
    {
        search_widget_.reset(new SearchWidget(this, search_context_));
        connect(search_widget_.get(), SIGNAL(search(BaseSearchContext &)),
                this, SLOT(onSearch(BaseSearchContext &)));
        connect(search_widget_.get(), SIGNAL(closeClicked()), this, SLOT(onSearchClosed()));
    }

    hideHelperWidget(dict_widget_.get());
    hideHelperWidget(tts_widget_.get());
    search_widget_->ensureVisible();
}

bool ReaderView::updateSearchCriteria()
{
    search_flags_ = 0;
    search_context_.stop(false);
    if (!search_context_.forward())
    {
        search_flags_ |= QWebPage::FindBackward;
    }
    if (search_context_.case_sensitive())
    {
        search_flags_ |= QWebPage::FindCaseSensitively;
    }
    return true;
}

void ReaderView::onSearch(BaseSearchContext&)
{
    bool ret = false;
    updateSearchCriteria();
    ret = doSearch();

    // No more search result.
    if (!ret)
    {
        // Need to update the search widget to indicate that there is no more
        // matched result.
        search_widget_->noMoreMatches();
    }
}

void ReaderView::onSearchClosed()
{
    // Clear all selected text.
    clearSelection();
}

bool ReaderView::doSearch()
{
    return findText(search_context_.pattern(), search_flags_);
}

void ReaderView::returnToLibrary()
{
    saveOptions(path_);
    qApp->quit();
}

void ReaderView::fastUpdateWidget(QRect &rect)
{
    onyx::screen::instance().enableUpdate(false);
    repaint();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidgetRegion(this, rect, onyx::screen::ScreenProxy::DW, false, onyx::screen::ScreenCommand::WAIT_COMMAND_FINISH);
    rect.setCoords(0, 0, 0, 0);
}

bool ReaderView::isWidgetVisible(QWidget * wnd)
{
    if (wnd)
    {
        return wnd->isVisible();
    }
    return false;
}

void ReaderView::hideHelperWidget(QWidget * wnd)
{
    if (wnd)
    {
        wnd->hide();
    }
}

void ReaderView::changeFontFamily(const QString & family)
{
    // It does not work for documents that specify font already.
    QWebSettings * settings = QWebSettings::globalSettings();
    if (settings->fontFamily(QWebSettings::StandardFont) == family)
    {
        update(rect());
        return;
    }

    for(int i = static_cast<int>(QWebSettings::StandardFont);
        i <= static_cast<int>(QWebSettings::FantasyFont); ++i)
    {
        settings->setFontFamily(static_cast<QWebSettings::FontFamily>(i), family);
    }
    update(rect());
}

void ReaderView::enableTextSelection(bool enable)
{
    enable_text_selection_ = enable;
}

bool ReaderView::isTextSelectionEnabled()
{
    return enable_text_selection_;
}

QPointF ReaderView::viewport()
{
    // Get current location.
    QSizeF s = page()->currentFrame()->contentsSize();
    QPointF pt = page()->currentFrame()->scrollPosition();
    return QPointF(pt.x() / s.width(), pt.y() / s.height());
}

void ReaderView::setViewport(QPointF percentage)
{
    QSizeF s = page()->currentFrame()->contentsSize();
    page()->currentFrame()->scroll(static_cast<int>(percentage.x() * s.width()),
                                   static_cast<int>(percentage.y() * s.height()));
}

bool ReaderView::atBeginning()
{
    QPointF pt = page()->currentFrame()->scrollPosition();
    return (pt.ry() <= 0);
}

bool ReaderView::atEnd()
{
    QSizeF s = page()->currentFrame()->contentsSize();
    QPointF pt = page()->currentFrame()->scrollPosition();
    return (pt.ry() + rect().height() >= s.height());
}

void ReaderView::scrollNext()
{
    if (atEnd())
    {
        QUrl new_url = url();
        if (nextUrl(new_url))
        {
            load(new_url);
            return;
        }
    }
    myScroll(0, rect().height() - PAGE_REPEAT);
}

void ReaderView::scrollPrev()
{
    if (atBeginning())
    {
        QUrl new_url = url();
        if (prevUrl(new_url))
        {
            load(new_url);
        }
    }
    myScroll(0, -rect().height() + PAGE_REPEAT);
}

/// To retrieve the next url.
bool ReaderView::nextUrl(QUrl & url)
{
    if (!model_->nextPage(url))
    {
        return false;
    }
    return true;
}

/// To retrieve the prev url.
bool ReaderView::prevUrl(QUrl & url)
{
    if (!model_->prevPage(url))
    {
        return false;
    }
    return true;
}

void ReaderView::resizeEvent(QResizeEvent *re)
{
    if (dict_widget_ && dict_widget_->isVisible())
    {
        dict_widget_->ensureVisible(selected_rect_);
    }

    if (tts_widget_ && tts_widget_->isVisible())
    {
        tts_widget_->ensureVisible();
    }

    if (search_widget_ && search_widget_->isVisible())
    {
        search_widget_->ensureVisible();
    }

    QWebView::resizeEvent(re);
}

/// Clear highlight text by using javascript.
void ReaderView::clearSelection()
{
    page()->currentFrame()->evaluateJavaScript("document.selection.empty;");
    page()->currentFrame()->evaluateJavaScript("window.getSelection().removeAllRanges();");
}

void ReaderView::onMusicPlayerStateChanged(int state)
{
    if (state == sys::HIDE_PLAYER || state == sys::STOP_PLAYER)
    {
        onyx::screen::instance().flush(this);
    }
}

}
