#ifdef BUILD_FOR_ARM
#include <QtGui/qwsdisplay_qws.h>
#include <QtGui/qscreen_qws.h>
#endif

#include "onyx/cms/content_thumbnail.h"
#include "adobe_drm_engine/adobe_drm_handler.h"

#include "naboo_view.h"
#include "naboo_thumbnail.h"

using namespace ui;
using namespace vbf;
using namespace sketch;
using namespace adobe_view;
using namespace adobe_drm;

namespace naboo_reader
{

static const unsigned int REPAINT_INTERVAL = 1000;
static const unsigned int SLIDE_SHOW_INTERVAL = 3000;
static const unsigned int TTS_INTERVAL = 200;
static const unsigned int ANNOTATION_INTERVAL = 0;
static const unsigned int AUTO_FLIP_INTERVAL = 1000;
static const unsigned int MOVE_ERROR = 10;
static const int OVERLAP_DISTANCE = 100;
static const qreal DELTA = 0.01;

// viewport navigation
static const AdobeViewportLocation all_viewport_locations[] =
{
    LEFT_SPACE, RIGHT_SPACE, UP_SPACE, BOTTOM_SPACE
};

static const int hor_margin = 5;
static const int ver_margin = 5;

static RotateDegree getSystemRotateDegree()
{
    int degree = 0;
#ifdef BUILD_FOR_ARM
    degree = QScreen::instance()->transformOrientation();
#endif
    return static_cast<RotateDegree>(degree);
}

static int getIndexOfFontValue(const vector<qreal> & font_values, const double current_value)
{
    for (int idx = 0; idx < static_cast<int>(font_values.size()); ++idx)
    {
        if (qAbs(font_values[idx] - current_value) <= DELTA)
        {
            return idx;
        }
    }
    return -1;
}

static void initFontValues(vector<qreal> & font_values)
{
    font_values.push_back(1.0);
    font_values.push_back(1.25);
    font_values.push_back(1.50);
    font_values.push_back(1.75);
    font_values.push_back(2.0);
    font_values.push_back(2.25);
    font_values.push_back(2.50);
    font_values.push_back(2.75);
    font_values.push_back(3.0);
    font_values.push_back(3.25);
    font_values.push_back(3.50);
    font_values.push_back(3.75);
    font_values.push_back(4.0);
    font_values.push_back(4.25);
    font_values.push_back(4.50);
    font_values.push_back(4.75);
    font_values.push_back(5.0);
}

static bool isSpace(const QChar & c)
{
    ushort code = c.unicode();
    if (code == 0x0009 || (code >= 0x000a && code <= 0x000d) || code == 0x0020)
    {
        return true;
    }
    return false;
}

static bool isPuncuation(const QChar & c)
{
    return c.isPunct();
}

static bool isHyphen(const QChar & c)
{
    ushort code = c.unicode();
    if (code == 0x00ad)
    {
        return true;
    }
    return false;
}

static bool isNumber(const QChar & c)
{
    return c.isNumber();
}

// Implementation of NabooView --------------------------------------------------
NabooView::NabooView(QWidget *parent)
    : BaseView(parent, Qt::FramelessWindowHint)
    , model_(0)
    , render_client_(this)
    , search_view_(0)
    , is_screen_anno_dirty_(false)
    , sketch_client_(&render_client_)
    , hyperlink_assist_(this)
    , left_image_(0)
    , right_image_(0)
    , up_image_(0)
    , down_image_(0)
    , auto_flip_current_page_(1)
    , auto_flip_step_(5)
    , current_waveform_(onyx::screen::instance().defaultWaveform())
{
    initFontValues(font_values_);
    connect(&status_mgr_, SIGNAL(stylusChanged(const int)), this, SLOT(onStylusChanged(const int)));
    connect(&sketch_proxy_, SIGNAL(requestUpdateScreen()), this, SLOT(onRequestFastUpdateScreen()));

    // set drawing area to sketch agent
    sketch_proxy_.setDrawingArea(this);
    sketch_proxy_.setWidgetOrient(getSystemRotateDegree());

    // set up repaint timer
    repaint_timer_.setSingleShot(true);
    repaint_timer_.setInterval(REPAINT_INTERVAL);
    connect(&repaint_timer_, SIGNAL(timeout()), this, SLOT(onRequestFullUpdateScreen()));

    slide_show_timer_.setSingleShot(true);
    slide_show_timer_.setInterval(SLIDE_SHOW_INTERVAL);
    connect(&slide_show_timer_, SIGNAL(timeout()), this, SLOT(autoFlip()));

    tts_timer_.setSingleShot(true);
    tts_timer_.setInterval(TTS_INTERVAL);
    connect(&tts_timer_, SIGNAL(timeout()), this, SLOT(playVoiceOnCurrentScreen()));

    annotation_timer_.setSingleShot(true);
    annotation_timer_.setInterval(ANNOTATION_INTERVAL);
    connect(&annotation_timer_, SIGNAL(timeout()), this, SLOT(onAnnotationFinished()));

    flip_page_timer_.setInterval(AUTO_FLIP_INTERVAL);
    connect(&flip_page_timer_, SIGNAL(timeout()), this, SLOT(autoFlipMultiplePages()));

    connect(&render_client_, SIGNAL(externalHyperlinkClicked(const QString &)),
            this, SLOT(onExternalHyperlinkClicked(const QString &)));
    connect(&render_client_, SIGNAL(thumbnailReady(QImage, const double &)),
            this, SLOT(onThumbnailReady(QImage, const double &)));
    connect(&render_client_, SIGNAL(renderConfigurationUpdated()),
            this, SLOT(onRenderConfigurationUpdated()));
    connect(&render_client_, SIGNAL(renderRequestSent()),
            this, SLOT(onRenderRequestSent()));
    connect(&render_client_, SIGNAL(navigationMatrixUpdated()),
            this, SLOT(onNavigationMatrixUpdated()));
}

NabooView::~NabooView(void)
{
}

void NabooView::onSaveViewOptions()
{
    // save reading history
    vbf::Configuration & conf = model()->getConf();
    conf.options[CONFIG_FLASH_TYPE] = onyx::screen::instance().defaultWaveform();

    // save the reading progress
    QString progress("%1 / %2");
    AdobeLocationPtr cur_loc = render_client_.getCurrentLocation();
    int current_pos = static_cast<int>(cur_loc->getPagePosition()) + 1;
    int total = static_cast<int>(model_->documentClient()->getPageCount());
    conf.info.mutable_progress() = progress.arg(current_pos).arg(total);

    // save all of the sketches
    sketch_proxy_.save();
    render_client_.saveOptions();
}

void NabooView::onDocumentClose()
{
    saveThumbnail();
    render_client_.onDocumentClose();
}

void NabooView::onDocumentRequestLicense(const QString & type,
                                         const QString & resource_id,
                                         const QByteArray & request_data)
{
}

void NabooView::onDocumentReady()
{
    current_waveform_ = static_cast<onyx::screen::ScreenProxy::Waveform>( model()->getConf().options[CONFIG_FLASH_TYPE].toInt() );
    if ( current_waveform_ == onyx::screen::ScreenProxy::GU )
    {
        onyx::screen::instance().setDefaultWaveform( current_waveform_ );
    }
    else
    {
        onyx::screen::instance().setDefaultWaveform( onyx::screen::ScreenProxy::GC );
    }
    sketch_proxy_.open(model_->path());
}

void NabooView::onDocumentError(const QString & error)
{
    if (sys::SysStatus::instance().isSystemBusy())
    {
        // if it is the first time rendering, set busy to be false
        sys::SysStatus::instance().setSystemBusy(false);
    }

    bool license_required = model_->documentClient()->hasLicenseRequired();
    if (error.startsWith("E_ADEPT_CORE_PASSHASH_NOT_FOUND") && !license_required)
    {
        QString url = error.right(error.size() - QString("E_ADEPT_CORE_PASSHASH_NOT_FOUND").size());
        if (handleHashPasswordRequest(url.trimmed()))
        {
            return;
        }
        license_required = true;
    }

    QString err(QCoreApplication::tr("Cannot open the document.\n%1"));
    if (license_required)
    {
        err = QCoreApplication::tr("No permission to open the document.\nPlease check the password or DRM settings.");
    }
    else
    {
        err = err.arg(error);
    }
    handleError(err);
}

void NabooView::onRequestPassword()
{
    sys::SysStatus::instance().setSystemBusy(false);
    PasswordDialog dialog(this);
    QString password;
    if (dialog.popup(password) != QDialog::Accepted)
    {
        return;
    }
    password = dialog.password();
    model_->documentClient()->setPassword(password);
}

#ifdef MAIN_WINDOW_TOC_ON
void NabooView::onTreeViewReturn( const QModelIndex& index )
{
    dynamic_cast<MainWindow*>(parentWidget())->activateView(NABOO_VIEW);
    if (!index.isValid())
    {
        emit currentPageChanged(1, model()->documentClient()->getPageCount());
        return;
    }

    double dst_pos = model()->getPositionByIndex( index );
    if (dst_pos >= 0)
    {
        gotoPosition( dst_pos );
    }
}
#endif

void NabooView::onNeedThumbnailForNewPage(const QString bookmark, const QSize &size)
{
    render_client_.disableRepaint();
    AdobeRenderConf & render_conf = render_client_.renderConf();
    AdobeLocationPtr position = model()->documentClient()->getLocationFromBookmark(bookmark);
    render_conf.setPagePosition( position );
    render_conf.setThumbnailSize( size );

    // update viewport if necessary
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view != 0)
    {
        ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);
        QSize viewport_size = thumbnail_view->size();
        if (viewport_size.width() > viewport_size.height())
        {
            viewport_size.transpose();
        }
        render_conf.setViewport(viewport_size);
    }

    render_conf.setOperation( RENDER_THUMBNAIL_NAVIGATE_TO_LOCATION );
    render_conf.exec();
}

void NabooView::onNeedNextThumbnail(const QString current_bookmark, const QSize &size)
{
    render_client_.disableRepaint();
    AdobeRenderConf & render_conf = render_client_.renderConf();
    render_conf.setThumbnailSize( size );
    if (!current_bookmark.isEmpty())
    {
        AdobeLocationPtr position = model()->documentClient()->getLocationFromBookmark(current_bookmark);
        render_conf.setPagePosition( position );
    }

    // update viewport if necessary
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view != 0)
    {
        ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);
        QSize viewport_size = thumbnail_view->size();
        if (viewport_size.width() > viewport_size.height())
        {
            viewport_size.transpose();
        }
        render_conf.setViewport(viewport_size);
    }

    render_conf.setOperation( RENDER_THUMBNAIL_NEXT_SCREEN );
    render_conf.exec();
}

void NabooView::onNeedPreviousThumbnail(const QString current_bookmark, const QSize &size)
{
    render_client_.disableRepaint();
    AdobeRenderConf & render_conf = render_client_.renderConf();
    render_conf.setThumbnailSize( size );
    if (!current_bookmark.isEmpty())
    {
        AdobeLocationPtr position = model()->documentClient()->getLocationFromBookmark(current_bookmark);
        render_conf.setPagePosition( position );
    }

    // update viewport if necessary
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view != 0)
    {
        ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);
        QSize viewport_size = thumbnail_view->size();
        if (viewport_size.width() > viewport_size.height())
        {
            viewport_size.transpose();
        }
        render_conf.setViewport(viewport_size);
    }

    render_conf.setOperation( RENDER_THUMBNAIL_PREVIOUS_SCREEN );
    render_conf.exec();
}

void NabooView::onThumbnailReturnToReading(const QString bookmark)
{
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view == 0)
    {
        return;
    }
    ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);
    deattachThumbnailView(thumbnail_view);
    down_cast<MainWindow*>(parentWidget())->activateView(NABOO_VIEW);

    // reattach sketch proxy
    sketch_proxy_.setDrawingArea(this);

    // reset waveform
    onyx::screen::instance().setDefaultWaveform(current_waveform_);

    render_client_.disableRepaint();
    AdobeRenderConf render_conf = thumbnail_view->originalRenderConf();
    render_conf.setOperation(RENDER_RESTORE);
    if (!bookmark.isEmpty())
    {
        render_conf.setOperation(RENDER_NAVIGATE_TO_LOCATION);
        AdobeLocationPtr loc = model()->documentClient()->getLocationFromBookmark(bookmark);
        render_conf.setPagePosition(loc);
    }
    render_client_.renderConf() = render_conf;
    render_client_.sendRenderRequest(&(render_client_.renderConf()));
}

void NabooView::attachThumbnailView(ThumbnailView *thumb_view)
{
    thumb_view->setModel(model_);
    thumb_view->setRendererClient(&render_client_);
    thumb_view->setOriginalRenderConfiguration(render_client_.renderConf());
    connect(thumb_view, SIGNAL(needThumbnailForNewPage(const QString, const QSize&)),
            this, SLOT(onNeedThumbnailForNewPage(const QString, const QSize&)));
    connect(thumb_view, SIGNAL(needNextThumbnail(const QString, const QSize&)),
            this, SLOT(onNeedNextThumbnail(const QString, const QSize&)));
    connect(thumb_view, SIGNAL(needPreviousThumbnail(const QString, const QSize&)),
            this, SLOT(onNeedPreviousThumbnail(const QString, const QSize&)));
    connect(thumb_view, SIGNAL(returnToReading(const QString)),
            this, SLOT(onThumbnailReturnToReading(const QString)));
}

void NabooView::deattachThumbnailView(ThumbnailView *thumb_view)
{
    disconnect(thumb_view, SIGNAL(needThumbnailForNewPage(const QString, const QSize&)),
               this, SLOT(onNeedThumbnailForNewPage(const QString, const QSize&)));
    disconnect(thumb_view, SIGNAL(needNextThumbnail(const QString, const QSize&)),
               this, SLOT(onNeedNextThumbnail(const QString, const QSize&)));
    disconnect(thumb_view, SIGNAL(needPreviousThumbnail(const QString, const QSize&)),
               this, SLOT(onNeedPreviousThumbnail(const QString, const QSize&)));
    disconnect(thumb_view, SIGNAL(returnToReading(const QString)),
               this, SLOT(onThumbnailReturnToReading(const QString)));
}

void NabooView::displayThumbnailView()
{
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view == 0)
    {
        return;
    }

    ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);
    attachThumbnailView(thumbnail_view);
    thumbnail_view->attachSketchProxy(&sketch_proxy_);
    down_cast<MainWindow*>(parentWidget())->activateView(THUMBNAIL_VIEW);
    down_cast<ThumbnailView*>(thumbnail_view)->setCurrentPage(render_client_.getCurrentLocation()->getBookmark());
}

void NabooView::onThumbnailReady(QImage thumbnail, const double & zoom_value)
{
    AdobeRenderConf & render_conf = render_client_.renderConf();
    AdobeRenderOperation opt = render_conf.getOperation();
    if (opt == RENDER_THUMBNAIL_SAVE)
    {
        // save the thumbnail
        if (cmt_ != 0 && !thumbnail.isNull())
        {
            QFileInfo file_info(model_->path());
            cmt_->storeThumbnail(file_info.fileName(), THUMBNAIL_LARGE, thumbnail);
        }
        return;
    }

    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view == 0)
    {
        return;
    }
    ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);
    shared_ptr< NabooThumbnail > thumb(new NabooThumbnail(render_conf.getPagePosition(), zoom_value));
    thumb->setImage(new QImage(thumbnail));

    switch (opt)
    {
    case RENDER_THUMBNAIL_NAVIGATE_TO_LOCATION:
        {
            thumbnail_view->setThumbnail(thumb);
        }
        break;
    case RENDER_THUMBNAIL_NEXT_SCREEN:
        {
            thumbnail_view->setNextThumbnail(thumb);
        }
        break;
    case RENDER_THUMBNAIL_PREVIOUS_SCREEN:
        {
            thumbnail_view->setPreviousThumbnail(thumb);
        }
        break;
    default:
        break;
    }
}

void NabooView::onExternalHyperlinkClicked(const QString & url)
{
    qDebug("External Hyperlink Clicked:%s", url.toUtf8().constData());
}

void NabooView::onRenderRequestSent()
{
    // save annotations and sketches
    model()->annotationMgr()->save();
    sketch_proxy_.save();
}

void NabooView::onNavigationMatrixUpdated()
{
    // clear the sketch assist if the scale factor of navigation matrix changed
    sketch_client_.clearCache();
}

void NabooView::requestPlayingVoice()
{
    // play the voice if need
    tts_timer_.start();
}

void NabooView::ttsGotoPage()
{
    if (tts_engine_ != 0 && tts_engine_->state() == TTS_PLAYING)
    {
        pauseTTS();
        render_client_.renderConf().setNeedPlayVoice( true );
    }
}

void NabooView::prevScreen()
{
    ttsGotoPage();
    render_client_.prevScreen();
}

void NabooView::nextScreen()
{
    ttsGotoPage();
    render_client_.nextScreen();
}

bool NabooView::gotoPosition( AdobeLocationPtr pos )
{
    ttsGotoPage();
    return render_client_.gotoPosition(pos);
}

bool NabooView::canMoveViewport()
{
    if (render_client_.getPagingMode() == PM_HARD_PAGES ||
        render_client_.getPagingMode() == PM_SCROLL_PAGES)
    {
        if (render_client_.getPagingMode() == PM_HARD_PAGES && render_client_.zoomingMode() == ZOOM_HIDE_MARGIN)
        {
            return false;
        }

        AdobeViewportLocations locations = render_client_.viewportLocation();
        if (locations != NO_SPACE)
        {
            return true;
        }
    }
    return false;
}

bool NabooView::paintViewportMarks(QPainter & painter)
{
    AdobeViewportLocations locations = render_client_.viewportLocation();
    if (locations == NO_SPACE)
    {
        return false;
    }

    const int size = sizeof(all_viewport_locations)/sizeof(all_viewport_locations[0]);
    for(int i = 0; i < size; ++i)
    {
        if (locations.testFlag(all_viewport_locations[i]))
        {
            switch (all_viewport_locations[i])
            {
            case LEFT_SPACE:
                {
                    if (left_image_ == 0)
                    {
                        left_image_.reset(new QImage(":/images/left.png"));
                    }
                    QPoint left_pt(hor_margin, ((height() - left_image_->height()) >> 1));
                    painter.drawImage(left_pt, *left_image_);
                }
                break;
            case RIGHT_SPACE:
                {
                    if (right_image_ == 0)
                    {
                        right_image_.reset(new QImage((":/images/right.png")));
                    }
                    QPoint right_pt(width() - right_image_->width() - hor_margin, ((height() - right_image_->height()) >> 1));
                    painter.drawImage(right_pt, *right_image_);
                }
                break;
            case UP_SPACE:
                {
                    if (up_image_ == 0)
                    {
                        up_image_.reset(new QImage(":/images/up.png"));
                    }
                    QPoint up_pt(((width() - up_image_->width()) >> 1), ver_margin);
                    painter.drawImage(up_pt, *up_image_);
                }
                break;
            case BOTTOM_SPACE:
                {
                    if (down_image_ == 0)
                    {
                        down_image_.reset(new QImage(":/images/down.png"));
                    }
                    QPoint down_pt(((width() - down_image_->width()) >> 1), height() - down_image_->height() - ver_margin);
                    painter.drawImage(down_pt, *down_image_);
                }
                break;
            default:
                break;
            }
        }
    }
    return true;
}

bool NabooView::moveViewportByScreenSize(AdobeViewportLocation location)
{
    AdobeViewportLocations locations = render_client_.viewportLocation();
    if (locations.testFlag(location))
    {
        switch (location)
        {
        case LEFT_SPACE:
            render_client_.moveViewport(OVERLAP_DISTANCE - width(), 0);
            break;
        case RIGHT_SPACE:
            render_client_.moveViewport(width() - OVERLAP_DISTANCE, 0);
            break;
        case UP_SPACE:
            render_client_.moveViewport(0, OVERLAP_DISTANCE - height());
            break;
        case BOTTOM_SPACE:
            render_client_.moveViewport(0, height() - OVERLAP_DISTANCE);
            break;
        default:
            return false;
        }
        return true;
    }
    return false;
}

bool NabooView::hitTestViewportMarks(const QPoint & pos, AdobeViewportLocation & location)
{
    AdobeViewportLocations locations = render_client_.viewportLocation();
    if (locations == NO_SPACE)
    {
        return false;
    }

    location = NO_SPACE;
    const int size = sizeof(all_viewport_locations)/sizeof(all_viewport_locations[0]);
    for(int i = 0; i < size; ++i)
    {
        if (locations.testFlag(all_viewport_locations[i]))
        {
            switch (all_viewport_locations[i])
            {
            case LEFT_SPACE:
                {
                    if (left_image_ == 0)
                    {
                        left_image_.reset(new QImage(":/images/left.png"));
                    }
                    QRect left_rect(hor_margin,
                                    ((height() - left_image_->height()) >> 1),
                                    left_image_->width(),
                                    left_image_->height());
                    if (left_rect.contains(pos))
                    {
                        location = LEFT_SPACE;
                        return true;
                    }
                }
                break;
            case RIGHT_SPACE:
                {
                    if (right_image_ == 0)
                    {
                        right_image_.reset(new QImage((":/images/right.png")));
                    }
                    QRect right_rect(width() - right_image_->width() - hor_margin,
                                     ((height() - right_image_->height()) >> 1),
                                     right_image_->width(),
                                     right_image_->height());
                    if (right_rect.contains(pos))
                    {
                        location = RIGHT_SPACE;
                        return true;
                    }
                }
                break;
            case UP_SPACE:
                {
                    if (up_image_ == 0)
                    {
                        up_image_.reset(new QImage(":/images/up.png"));
                    }
                    QRect up_rect(((width() - up_image_->width()) >> 1), ver_margin, up_image_->width(), up_image_->height());
                    if (up_rect.contains(pos))
                    {
                        location = UP_SPACE;
                        return true;
                    }
                }
                break;
            case BOTTOM_SPACE:
                {
                    if (down_image_ == 0)
                    {
                        down_image_.reset(new QImage(":/images/down.png"));
                    }
                    QRect down_rect(((width() - down_image_->width()) >> 1),
                                    height() - down_image_->height() - ver_margin,
                                    down_image_->width(),
                                    down_image_->height());
                    if (down_rect.contains(pos))
                    {
                        location = BOTTOM_SPACE;
                        return true;
                    }
                }
                break;
            default:
                break;
            }
        }
    }
    return false;
}

// This function is called when executing render request so that we need make sure
// there is no pending
void NabooView::onRenderConfigurationUpdated()
{
    if (sys::SysStatus::instance().isSystemBusy())
    {
        // if it is the first time rendering, set busy to be false
        sys::SysStatus::instance().setSystemBusy(false);
    }

    // search by the pattern if necessary
    searchInCurrentScreen();
    if (status_mgr_.isErasing() || status_mgr_.isSketching())
    {
        updateSketchProxy();
    }

    if (status_mgr_.isFreePen())
    {
        Range screen_range;
        if (render_client_.getScreenRange(screen_range))
        {
            Range first_word;
            if ((anno_ctx_.lastLocation() < screen_range.start || anno_ctx_.firstLocation() > screen_range.end) &&
                firstWord(first_word))
            {
                // deselect previous selections if there is any
                anno_ctx_.reset();
                anno_ctx_.addLocation(first_word.start);
                anno_ctx_.addLocation(first_word.end);
            }
        }
    }

    if (onyx::screen::instance().userData() == 0)
    {
        ++onyx::screen::instance().userData();
    }
    emit requestUpdateParent(false);
    update();

    if ( render_client_.renderConf().getOperation() != RENDER_RESTORE )
    {
        // save the reading history besides the restored one
        saveReadingContext();
    }

    if ( render_client_.renderConf().needPlayVoice() )
    {
        requestPlayingVoice();
    }

    AdobeLocationPtr cur_loc = render_client_.getCurrentLocation();
    if (cur_loc != 0)
    {
        emit currentPageChanged(static_cast<int>(cur_loc->getPagePosition()),
                                static_cast<int>(model()->documentClient()->getPageCount()));
    }
    else
    {
        QString error(QCoreApplication::tr("Error of browsing document!"));
        handleError(error);
    }

    if ( status_mgr_.isSlideShow() )
    {
        slide_show_timer_.start();
    }
}

void NabooView::autoFlip()
{
    if (!render_client_.isValid()) return;

#ifdef DUMP_PAGES
    QString path;
#ifdef Q_WS_QWS
    path = ("/media/sd");
#else
    path = QDir::home().path();
#endif
    QDir dir(path);
    if (!dir.exists("snapshot") && !dir.mkdir("snapshot"))
    {
        return;
    }
    dir.cd("snapshot");

    QFileInfo file_info(model_->path());
    QString file_name = file_info.fileName();
    //file_name = file_name.left(file_name.indexOf("."));
    if (!dir.exists(file_name) && !dir.mkdir(file_name))
    {
        return;
    }

    if (dir.cd(file_name))
    {
        AdobeLocationPtr cur_loc = render_client_.getScreenBeginning();
        QString snapshot_name("%1.png");
        snapshot_name = snapshot_name.arg(cur_loc->getPagePosition());
        QString snapshot_path = dir.absoluteFilePath(snapshot_name);
        sys::SysStatus::instance().snapshot(snapshot_path);
    }
#endif

    if (render_client_.isAtEnd())
    {
        AdobeLocationPtr beginning = model()->documentClient()->getBeginning();
        gotoPosition( beginning );
    }
    else
    {
        nextScreen();
    }
}

void NabooView::autoFlipMultiplePages()
{
    int last_page = static_cast<int>(model()->documentClient()->getPageCount()) - 1;
    if (auto_flip_current_page_ < last_page)
    {
        auto_flip_current_page_ += auto_flip_step_;
        if (auto_flip_current_page_ > last_page)
        {
            auto_flip_current_page_ = last_page;
        }
        if (auto_flip_current_page_ < 1)
        {
            auto_flip_current_page_ = 1;
        }
        emit currentPageChanged(auto_flip_current_page_, last_page + 1);
    }
}

void NabooView::generateZoomSettings( std::vector<ZoomFactor> & zoom_settings )
{
    zoom_settings.clear();
    if (model()->documentClient()->type() == REFLOWABLE_DOCUMENT)
    {
        for ( int i = 0; i <= 4; ++i )
        {
            zoom_settings.push_back( i );
        }
    }
    else
    {
        // set optional zoom items
        zoom_settings.push_back(ZOOM_HIDE_MARGIN);
        zoom_settings.push_back(ZOOM_TO_PAGE);
        zoom_settings.push_back(ZOOM_TO_WIDTH);
        zoom_settings.push_back(ZOOM_TO_HEIGHT);
        if (SysStatus::instance().hasTouchScreen() &&
            model()->documentClient()->type() == FIX_PAGE_DOCUMENT &&
            !render_client_.isInFontIndexMode())
        {
            zoom_settings.push_back(ZOOM_SELECTION);
        }
        zoom_settings.push_back(75.0f);
        zoom_settings.push_back(100.0f);
        zoom_settings.push_back(125.0f);
        zoom_settings.push_back(150.0f);
        zoom_settings.push_back(175.0f);
        zoom_settings.push_back(200.0f);
        zoom_settings.push_back(300.0f);
        zoom_settings.push_back(400.0f);
    }
}

/// Popup menu
bool NabooView::updateActions()
{
    bool ret = false;
    if ( render_client_.isValid())
    {
        // Reading Tools
        std::vector<ReadingToolsType> reading_tools;
        if ( !status_mgr_.isSlideShow() )
        {
            reading_tools.push_back( SEARCH_TOOL );
            if ( model()->documentClient()->hasToc() )
            {
                reading_tools.push_back( TOC_VIEW_TOOL );
            }

            if (SysStatus::instance().hasTouchScreen() || SysStatus::instance().isDictionaryEnabled())
            {
                reading_tools.push_back( DICTIONARY_TOOL );
            }

            if (SysStatus::instance().hasTouchScreen() || SysStatus::instance().isTTSEnabled())
            {
                reading_tools.push_back( TEXT_TO_SPEECH );
            }
        }

        if (tts_engine_ == 0 || tts_engine_->state() != TTS_PLAYING)
        {
            reading_tools.push_back( SLIDE_SHOW );
        }
        reading_tools_actions_.generateActions( reading_tools, false );
        reading_tools_actions_.setActionStatus( SLIDE_SHOW,
                                                status_mgr_.isSlideShow() );

        if ( !status_mgr_.isSlideShow() )
        {
            reading_tools.clear();

            Range screen_range;
            if (render_client_.getScreenRange(screen_range))
            {
                if (model_->hasBookmark(screen_range))
                {
                    reading_tools.push_back( DELETE_BOOKMARK );
                }
                else
                {
                    reading_tools.push_back( ADD_BOOKMARK );
                }
            }
            reading_tools.push_back( SHOW_ALL_BOOKMARKS );
            reading_tools_actions_.generateActions( reading_tools, true );

            reading_tools.clear();
            reading_tools.push_back( PREVIOUS_VIEW );
            reading_tools.push_back( NEXT_VIEW );
            reading_tools.push_back( GOTO_PAGE );
            reading_tools_actions_.generateActions(reading_tools, true);

            reading_tools.clear();
            reading_tools.push_back( DISPLAY_HYPERLINKS );
            reading_tools.push_back( CLOCK_TOOL );
            reading_tools_actions_.generateActions(reading_tools, true);
            reading_tools_actions_.setActionStatus(DISPLAY_HYPERLINKS, status_mgr_.isDisplayHyperlinks());

            if (SysStatus::instance().hasTouchScreen())
            {
                reading_tools.clear();
                reading_tools.push_back( SCROLL_PAGE );
                reading_tools.push_back( RETRIEVE_WORD );
                reading_tools.push_back( COPY_CONTENT );
                reading_tools_actions_.generateActions(reading_tools, true);
                reading_tools_actions_.setActionStatus(SCROLL_PAGE, status_mgr_.isPan());
            }

            // Zoom Settings is only used in fixed page document
            std::vector<ZoomFactor> zoom_settings;
            generateZoomSettings(zoom_settings);
            zoom_setting_actions_.generateActions(zoom_settings);
            if (model()->documentClient()->type() == FIX_PAGE_DOCUMENT)
            {
                zoom_setting_actions_.setCurrentZoomValue(render_client_.zoomingMode());
            }
            else
            {
                zoom_setting_actions_.setCurrentZoomValue(render_client_.renderConf().getFontSize());
            }

            // Font Actions
            font_actions_.generateActions(font_values_, render_client_.renderConf().getFontRatio());

            // View Settings
            // TODO. Add Face to Face mode adn scroll mode when we found it works.
            PageLayouts page_layouts;
            //page_layouts.push_back( SCROLL_LAYOUT );
            //page_layouts.push_back( FACE_TO_FACE_LAYOUT );
            if (model()->documentClient()->type() == FIX_PAGE_DOCUMENT)
            {
                page_layouts.push_back( PAGE_LAYOUT );
                page_layouts.push_back( CONTINUOUS_LAYOUT );
                //page_layouts.push_back( REFLOWABLE_LAYOUT );
            }
            page_layouts.push_back( THUMBNAIL_LAYOUT );
            view_actions_.generatePageLayoutActions( page_layouts );

            switch (render_client_.getPagingMode())
            {
            case PM_HARD_PAGES:
                view_actions_.setCurrentPageLayout(PAGE_LAYOUT);
                break;
            case PM_SCROLL_PAGES:
                view_actions_.setCurrentPageLayout(CONTINUOUS_LAYOUT);
                break;
            case PM_FLOW_PAGES:
                view_actions_.setCurrentPageLayout(REFLOWABLE_LAYOUT);
                break;
            case PM_SCROLL:
                view_actions_.setCurrentPageLayout(SCROLL_LAYOUT);
                break;
            case PM_HARD_PAGES_2UP:
                view_actions_.setCurrentPageLayout(FACE_TO_FACE_LAYOUT);
                break;
            default:
                break;
            }

            sketch_actions_.clear();
            AnnotationModes anno_modes;
            SketchModes     sketch_modes;
            SketchColors    sketch_colors;
            SketchShapes    sketch_shapes;

            anno_modes.push_back(ADD_ANNOTATION);
            anno_modes.push_back(DELETE_ANNOTATION);
            anno_modes.push_back(DIAPLAY_ALL_ANNOTATIONS);
            if (!render_client_.isInFontIndexMode())
            {
                sketch_modes.push_back(MODE_SKETCHING);
                sketch_modes.push_back(MODE_ERASING);

                sketch_colors.push_back(SKETCH_COLOR_WHITE);
                //sketch_colors.push_back(SKETCH_COLOR_LIGHT_GRAY);
                //sketch_colors.push_back(SKETCH_COLOR_DARK_GRAY);
                sketch_colors.push_back(SKETCH_COLOR_BLACK);

                sketch_shapes.push_back(SKETCH_SHAPE_0);
                sketch_shapes.push_back(SKETCH_SHAPE_1);
                sketch_shapes.push_back(SKETCH_SHAPE_2);
                sketch_shapes.push_back(SKETCH_SHAPE_3);
                sketch_shapes.push_back(SKETCH_SHAPE_4);
            }

            sketch_actions_.generateAnnotationMode(anno_modes);
            sketch_actions_.generateSketchMode(sketch_modes);
            if (status_mgr_.isSketching())
            {
                sketch_actions_.setSketchMode(MODE_SKETCHING, true);
            }
            else if (status_mgr_.isErasing())
            {
                sketch_actions_.setSketchMode(MODE_ERASING, true);
            }
            else if (status_mgr_.isAddAnnotation())
            {
                sketch_actions_.setAnnotationMode(ADD_ANNOTATION, true);
            }
            else if (status_mgr_.isDeleteAnnotation())
            {
                sketch_actions_.setAnnotationMode(DELETE_ANNOTATION, true);
            }
            else
            {
                sketch_actions_.setAnnotationMode(ADD_ANNOTATION, false);
                sketch_actions_.setSketchMode(MODE_SKETCHING, false);
            }

            if (!sketch_colors.empty())
            {
                sketch_actions_.generateSketchColors(sketch_colors,
                                                     sketch_proxy_.getColor());
            }
            if (!sketch_shapes.empty())
            {
                sketch_actions_.generateSketchShapes(sketch_shapes,
                                                     sketch_proxy_.getShape());
            }
            if (!status_mgr_.isSketching())
            {
                sketch_actions_.setSketchColor( INVALID_SKETCH_COLOR );
                sketch_actions_.setSketchShape( INVALID_SKETCH_SHAPE );
            }
        }
        ret = true;
    }

    // regenerate system actions
    std::vector<int> all;
    all.push_back(ROTATE_SCREEN);
    if (isFullScreenCalculatedByWidgetSize())
    {
        all.push_back(EXIT_FULL_SCREEN);
    }
    else
    {
        all.push_back(FULL_SCREEN);
    }
    all.push_back(MUSIC);
    all.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(all);
    return ret;
}

void NabooView::popupMenu()
{
    // Make sure the display update is finished, otherwise
    // user can not see the menu on the screen.
    onyx::screen::instance().ensureUpdateFinished();

    if ( onyx::screen::instance().defaultWaveform() == onyx::screen::ScreenProxy::DW )
    {
        // stop fastest update mode to get better image quality.
        if ( current_waveform_ == onyx::screen::ScreenProxy::DW )
        {
            current_waveform_ = onyx::screen::ScreenProxy::GC;
        }
        onyx::screen::instance().setDefaultWaveform(current_waveform_);
    }

    ui::PopupMenu menu(this);
    if (updateActions())
    {
        if ( !status_mgr_.isSlideShow() )
        {
            menu.addGroup(&font_actions_);
            if (model()->documentClient()->type() == FIX_PAGE_DOCUMENT)
            {
                menu.addGroup(&zoom_setting_actions_);
            }
            if (SysStatus::instance().hasTouchScreen())
            {
                menu.addGroup(&sketch_actions_);
            }
            menu.addGroup(&view_actions_);
        }
        menu.addGroup(&reading_tools_actions_);
    }
    menu.setSystemAction(&system_actions_);
    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    stopTTS();
    QAction * group = menu.selectedCategory();
    bool disable_update = true;
    if (group == reading_tools_actions_.category())
    {
        int tool = reading_tools_actions_.selectedTool();
        switch (tool)
        {
        case SEARCH_TOOL:
            {
                displaySearchView( true );
            }
            break;
        case TOC_VIEW_TOOL:
            {
                displayTOC( true );
            }
            break;
        case DICTIONARY_TOOL:
            {
                displayDictionary( true );
            }
            break;
        case TEXT_TO_SPEECH:
            {
                startTTS();
                disable_update = false;
            }
            break;
        case DISPLAY_HYPERLINKS:
            {
                FunctionStatus status = ( status_mgr_.isDisplayHyperlinks() ? FUNC_NORMAL : FUNC_SELECTED );
                status_mgr_.setStatus( ID_DISPLAY_HYPERLINKS, status );
                update();
                disable_update = false;
            }
            break;
        case SCROLL_PAGE:
            {
                status_mgr_.setStatus( ID_PAN, FUNC_SELECTED );
                anno_ctx_.reset();
                disable_update = false;
            }
            break;
        case PREVIOUS_VIEW:
            {
                disable_update = back();
            }
            break;
        case NEXT_VIEW:
            {
                disable_update = forward();
            }
            break;
        case ADD_BOOKMARK:
            {
                disable_update = addBookmark();
            }
            break;
        case DELETE_BOOKMARK:
            {
                disable_update = deleteBookmark();
            }
            break;
        case SHOW_ALL_BOOKMARKS:
            {
                displayBookmarks();
            }
            break;
        case GOTO_PAGE:
            {
                emit popupJumpPageDialog();
            }
            break;
        case SLIDE_SHOW:
            {
                if (status_mgr_.isSlideShow())
                {
                    stopSlideShow();
                }
                else
                {
                    startSlideShow();
                }
            }
            break;
        case RETRIEVE_WORD:
            {
                enterWordRetrievingMode();
                disable_update = false;
            }
            break;
        case COPY_CONTENT:
            {
                copyToClipboard();
                disable_update = false;
            }
            break;
        case CLOCK_TOOL:
            {
                emit clockClicked();
            }
            break;
        default:
            break;
        }
    }
    else if (group == zoom_setting_actions_.category())
    {
        bool ret = false;
        if (model()->documentClient()->type() == FIX_PAGE_DOCUMENT)
        {
            ret = zooming(zoom_setting_actions_.getSelectedZoomValue());
        }
        else
        {
            ret = render_client_.scaleFontSize(static_cast<int>(zoom_setting_actions_.getSelectedZoomValue()));
        }
        disable_update = !ret;
    }
    else if (group == font_actions_.category())
    {
        disable_update = render_client_.scaleFontRatio(font_actions_.selectedMultiplier());
    }
    else if (group == view_actions_.category())
    {
        disable_update = false;
        ViewActionsType type = INVALID_VIEW_TYPE;
        int value = -1;

        type = view_actions_.getSelectedValue(value);
        switch (type)
        {
        case VIEW_ROTATION:
            rotate();
            break;
        case VIEW_PAGE_LAYOUT:
            {
                switch (static_cast<PageLayoutType>(value))
                {
                case PAGE_LAYOUT:
                    disable_update = render_client_.switchPagingMode(PM_HARD_PAGES);
                    break;
                case CONTINUOUS_LAYOUT:
                    disable_update = render_client_.switchPagingMode(PM_SCROLL_PAGES);
                    break;
                case FACE_TO_FACE_LAYOUT:
                    disable_update = render_client_.switchPagingMode(PM_HARD_PAGES_2UP);
                    break;
                case SCROLL_LAYOUT:
                    disable_update = render_client_.switchPagingMode(PM_SCROLL);
                    break;
                case REFLOWABLE_LAYOUT:
                    disable_update = render_client_.switchPagingMode(PM_FLOW_PAGES);
                    break;
                case THUMBNAIL_LAYOUT:
                    displayThumbnailView();
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            break;
        }
    }
    else if (group == sketch_actions_.category())
    {
        SketchActionsType type = INVALID_SKETCH_TYPE;
        int value = -1;
        bool checked = false;

        type = sketch_actions_.getSelectedValue( value, checked );
        switch (type)
        {
        case ANNOTATION_MODE:
            if ( static_cast<AnnotationMode>(value) == DIAPLAY_ALL_ANNOTATIONS )
            {
                displayAnnotations( checked );
            }
            else
            {
                setAnnotationMode( static_cast<AnnotationMode>(value), checked );
            }
            break;
        case SKETCH_MODE:
            setSketchMode( static_cast<SketchMode>(value), checked );
            break;
        case SKETCH_COLOR:
            setSketchColor( static_cast<SketchColor>(value) );
            break;
        case SKETCH_SHAPE:
            setSketchShape( static_cast<SketchShape>(value) );
            break;
        default:
            break;
        }
        disable_update = false;
    }
    else if (group == system_actions_.category())
    {
        SystemAction system_action = system_actions_.selected();
        switch (system_action)
        {
        case RETURN_TO_LIBRARY:
            returnToLibrary();
            break;
        case SCREEN_UPDATE_TYPE:
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, true);
            onyx::screen::instance().toggleWaveform();
            current_waveform_ = onyx::screen::instance().defaultWaveform();
            disable_update = false;
            break;
        case FULL_SCREEN:
            {
                emit fullScreen(true);
            }
            break;
        case EXIT_FULL_SCREEN:
            {
                emit fullScreen(false);
            }
            break;
        case ROTATE_SCREEN:
            rotate();
            break;
        case MUSIC:
            openMusicPlayer();
            break;
        default:
            break;
        }
    }

    if (!disable_update)
    {
        emit requestUpdateParent(true);
    }
}

void NabooView::attachModel(BaseModel * model)
{
    if (model_ == model)
    {
        return;
    }

    // Record the model.
    model_ = down_cast<NabooModel*>(model);
    render_client_.attachDocumentClient(model_->documentClient());
    connect(model_->documentClient(), SIGNAL(documentReadySignal()), this, SLOT(onDocumentReady()));
    connect(model_->documentClient(), SIGNAL(documentErrorSignal(const QString &)), this, SLOT(onDocumentError(const QString &)));
    connect(model_->documentClient(), SIGNAL(documentCloseSignal()), this, SLOT(onDocumentClose()));
    connect(model_->documentClient(), SIGNAL(documentRequestLicense(const QString &, const QString &, const QByteArray &)),
            this, SLOT(onDocumentRequestLicense(const QString &, const QString &, const QByteArray &)));
    connect(model_->documentClient(), SIGNAL(searchSucceeded()), this, SLOT(onSearchSucceeded()));
    connect(model_->documentClient(), SIGNAL(searchNoMoreResults()), this, SLOT(onSearchNoMoreResults()));
    connect(model_->documentClient(), SIGNAL(requestPassword()), this, SLOT(onRequestPassword()));
    connect(model_, SIGNAL(requestSaveAllOptions()), this, SLOT(onSaveViewOptions()));
}

void NabooView::deattachModel()
{
    if (model_ == 0)
    {
        return;
    }

    disconnect(model_->documentClient(), SIGNAL(documentReadySignal()), this, SLOT(onDocumentReady()));
    disconnect(model_->documentClient(), SIGNAL(documentErrorSignal(const QString &)), this, SLOT(onDocumentError(const QString &)));
    disconnect(model_->documentClient(), SIGNAL(documentCloseSignal()), this, SLOT(onDocumentClose()));
    disconnect(model_->documentClient(), SIGNAL(documentRequestLicense(const QString &, const QString &, const QByteArray &)),
               this, SLOT(onDocumentRequestLicense(const QString &, const QString &, const QByteArray &)));
    disconnect(model_->documentClient(), SIGNAL(searchSucceeded()), this, SLOT(onSearchSucceeded()));
    disconnect(model_->documentClient(), SIGNAL(searchNoMoreResults()), this, SLOT(onSearchNoMoreResults()));
    disconnect(model_, SIGNAL(requestSaveAllOptions()), this, SLOT(onSaveViewOptions()));
    render_client_.deattachDocumentClient();
    model_ = 0;
}

void NabooView::attachMainWindow(MainWindow *main_window)
{
    connect(this, SIGNAL(currentPageChanged(const int, const int)),
            main_window, SLOT(handlePositionChanged(const int, const int)));
    connect(this, SIGNAL(itemStatusChanged(const StatusBarItemType, const int)),
            main_window, SLOT(handleItemStatusChanged(const StatusBarItemType, const int)));
    connect(this, SIGNAL(requestUpdateParent(bool)),
            main_window, SLOT(handleRequestUpdate(bool)));
    connect(this, SIGNAL(popupJumpPageDialog()),
            main_window, SLOT(handlePopupJumpPageDialog()));
    connect(this, SIGNAL(fullScreen(bool)),
            main_window, SLOT(handleFullScreen(bool)));
    connect(this, SIGNAL(clockClicked()),
            main_window, SLOT(handleClockClicked()));

    connect(main_window, SIGNAL(pagebarClicked(const int, const int)),
            this, SLOT(onPagebarClicked(const int, const int)));
    connect(main_window, SIGNAL(popupContextMenu()),
            this, SLOT(onPopupContextMenu()));
    status_mgr_.setStatus(ID_PAN, FUNC_SELECTED);
}

void NabooView::deattachMainWindow(MainWindow *main_window)
{
    disconnect(this, SIGNAL(currentPageChanged(const int, const int)),
               main_window, SLOT(handlePositionChanged(const int, const int)));
    disconnect(this, SIGNAL(itemStatusChanged(const StatusBarItemType, const int)),
               main_window, SLOT(handleItemStatusChanged(const StatusBarItemType, const int)));
    disconnect(this, SIGNAL(requestUpdateParent(bool)),
               main_window, SLOT(handleRequestUpdate(bool)));
    disconnect(this, SIGNAL(popupJumpPageDialog()),
               main_window, SLOT(handlePopupJumpPageDialog()));
    disconnect(this, SIGNAL(fullScreen(bool)),
               main_window, SLOT(handleFullScreen(bool)));
    disconnect(this, SIGNAL(clockClicked()),
               main_window, SLOT(handleClockClicked()));

    disconnect(main_window, SIGNAL(pagebarClicked(const int, const int)),
               this, SLOT(onPagebarClicked(const int, const int)));
    disconnect(main_window, SIGNAL(popupContextMenu()),
               this, SLOT(onPopupContextMenu()));
}

void NabooView::attachTreeView(TreeViewDialog *tree_view)
{
    connect(tree_view, SIGNAL(returnToReading(const QModelIndex&)),
            this, SLOT(onTreeViewReturn(const QModelIndex&)));
}

void NabooView::deattachTreeView(TreeViewDialog *tree_view)
{
    disconnect(tree_view, SIGNAL(returnToReading(const QModelIndex&)),
               this, SLOT(onTreeViewReturn(const QModelIndex&)));
}

bool NabooView::flip( int direction )
{
    if (render_client_.isLocked() || !render_client_.isValid()) return true;

    if ( direction > 0 )
    {
        nextScreen();
        return !render_client_.isAtEnd();
    }
    else if (direction < 0)
    {
        prevScreen();
        return !render_client_.isAtBeginning();
    }
    return true;
}

void NabooView::paintEvent(QPaintEvent *pe)
{
    if (sys::SysStatus::instance().isSystemBusy() || model_ == 0 ||
        !model_->isReady() || render_client_.isLocked() || !render_client_.isValid())
    {
        return;
    }

    QPainter painter(this);

    // Clip region to get better performance. It works pretty well especially
    // when you only need to update a small region of widget.
    painter.setClipRect(pe->rect());

    // Paint raw image at very first
#ifndef USE_PIXMAP
    painter.drawImage( pe->rect().topLeft(), render_client_.surface()->image(), pe->rect() );
#else
    painter.drawPixmap( pe->rect().topLeft(), render_client_.surface()->image(), pe->rect() );
#endif

    // Paint the bounding rectangles of hyperlinks
    paintHyperlinks(painter);
    paintBookmark(painter);

    // John: disable it for 1.5.
    // paintViewportMarks(painter);

    // second layer, sketches
    paintSketchesInCurrentScreen(painter);

    // annotations
    painter.setCompositionMode( QPainter::RasterOp_SourceXorDestination );
    QBrush brush( Qt::white );
    paintAnnotationsInScreen( painter, brush );
    paintSelectings( painter, brush );
    paintSearchResults( painter, brush );
    paintSelectedHyperlink( painter, brush );
}

void NabooView::fastRepaint( const QRect & area )
{
    // Disable screen update at first, as repaint triggers UpdateRequest event
    onyx::screen::instance().enableUpdate( false );
    repaint( area );
    onyx::screen::instance().enableUpdate( true );

    // Now we can copy the data to controller.
    onyx::screen::instance().updateWidgetRegion( this, area, onyx::screen::ScreenProxy::DW, false, onyx::screen::ScreenCommand::WAIT_COMMAND_FINISH );
}

bool NabooView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if (key_event->key() == Qt::Key_PageDown || key_event->key() == Qt::Key_PageUp)
        {
            keyReleaseEvent(key_event);
            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

void NabooView::mousePressEvent(QMouseEvent *me)
{
    switch (me->button())
    {
    case Qt::LeftButton:
        if (status_mgr_.isZoomIn())
        {
            zoomInPress( me );
        }
        else if(status_mgr_.isSketching())
        {
        }
        else if (status_mgr_.isErasing())
        {
        }
        else if(status_mgr_.isPan())
        {
            panPress( me );
        }
        else if( status_mgr_.isFreePen() ||
                 status_mgr_.isAddAnnotation() ||
                 status_mgr_.isDeleteAnnotation() )
        {
            penPress( me );
        }
        break;
    default:
        break;
    }
    me->accept();
}

void NabooView::onPagebarClicked(const int percent, const int value)
{
    int dst = value;
    int count = static_cast<int>( model_->documentClient()->getPageCount() );
    if ( dst < 0 )
    {
        dst = 0;
    }
    if ( dst >= count )
    {
        dst = count - 1;
    }

    AdobeLocationPtr loc = model_->documentClient()->getLocationFromPagePosition(dst);
    gotoPosition( loc );
}

void NabooView::onPopupContextMenu()
{
    popupMenu();
}

void NabooView::mouseReleaseEvent(QMouseEvent *me)
{
    switch (me->button())
    {
    case Qt::LeftButton:
        if (status_mgr_.isZoomIn())
        {
            zoomInRelease( me );
        }
        else if (status_mgr_.isSketching())
        {
        }
        else if (status_mgr_.isErasing())
        {
        }
        else if (status_mgr_.isPan())
        {
            panRelease( me );
        }
        else if ( status_mgr_.isFreePen() ||
                  status_mgr_.isAddAnnotation() ||
                  status_mgr_.isDeleteAnnotation() )
        {
            penRelease( me );
        }
        break;
    case Qt::RightButton:
        {
            popupMenu();
        }
        break;
    default:
        break;
    }
    me->accept();
}

void NabooView::returnToLibrary()
{
    qApp->exit();
}

void NabooView::mouseMoveEvent(QMouseEvent *me)
{
    if (status_mgr_.isZoomIn())
    {
        zoomInMove( me );
    }
    else if (status_mgr_.isSketching())
    {
    }
    else if (status_mgr_.isErasing())
    {
    }
    else if ( status_mgr_.isFreePen() ||
              status_mgr_.isAddAnnotation() ||
              status_mgr_.isDeleteAnnotation() )
    {
        penMove( me );
    }
    me->accept();
}

void NabooView::keyPressEvent( QKeyEvent *ke )
{
    switch (ke->key())
    {
    case Qt::Key_PageUp:
        {
            AdobeLocationPtr current_loc = render_client_.getCurrentLocation();
            auto_flip_current_page_ = static_cast<int>(current_loc->getPagePosition());
            auto_flip_step_ = -5;
            onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
            flip_page_timer_.start();
        }
        break;
    case Qt::Key_PageDown:
        {
            AdobeLocationPtr current_loc = render_client_.getCurrentLocation();
            auto_flip_current_page_ = static_cast<int>(current_loc->getPagePosition());
            auto_flip_step_ = 5;
            onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
            flip_page_timer_.start();
        }
        break;
    default:
        break;
    }
}

void NabooView::keyReleaseEvent(QKeyEvent *ke)
{
    switch(ke->key())
    {
    case Qt::Key_PageUp:
        {
            onyx::screen::instance().setDefaultWaveform(current_waveform_);
            flip_page_timer_.stop();
            AdobeLocationPtr current_loc = render_client_.getCurrentLocation();
            int current_page = static_cast<int>(current_loc->getPagePosition());
            if (current_page == auto_flip_current_page_)
            {
                prevScreen();
            }
            else
            {
                AdobeLocationPtr dst = model_->documentClient()->getLocationFromPagePosition(auto_flip_current_page_);
                gotoPosition( dst );
            }
        }
        break;
    case Qt::Key_PageDown:
        {
            onyx::screen::instance().setDefaultWaveform(current_waveform_);
            flip_page_timer_.stop();
            AdobeLocationPtr current_loc = render_client_.getCurrentLocation();
            int current_page = static_cast<int>(current_loc->getPagePosition());
            if (current_page == auto_flip_current_page_)
            {
                nextScreen();
            }
            else
            {
                AdobeLocationPtr dst = model_->documentClient()->getLocationFromPagePosition(auto_flip_current_page_);
                gotoPosition( dst );
            }
        }
        break;
    case Qt::Key_Right:
        {
            if (canMoveViewport())
            {
                if (render_client_.zoomingMode() != ZOOM_HIDE_MARGIN)
                {
                    moveViewportByScreenSize(RIGHT_SPACE);
                }
                else
                {
                    moveViewportByScreenSize(BOTTOM_SPACE);
                }
            }
            else if ( status_mgr_.isFreePen() || status_mgr_.isAddAnnotation() )
            {
                moveToNextWord();
            }
            else
            {
                //forward();
                nextScreen();
            }
        }
        break;
    case Qt::Key_F:
        {
            render_client_.scaleSurface( ZOOM_TO_PAGE );
        }
        break;
    case Qt::Key_R:
        {
            rotate();
        }
        break;
    case Qt::Key_Left:
        {
            if (canMoveViewport())
            {
                if (render_client_.zoomingMode() != ZOOM_HIDE_MARGIN)
                {
                    moveViewportByScreenSize(LEFT_SPACE);
                }
                else
                {
                    moveViewportByScreenSize(UP_SPACE);
                }
            }
            else if ( status_mgr_.isFreePen() || status_mgr_.isAddAnnotation() )
            {
                moveToPreviousWord();
            }
            else
            {
                //back();
                prevScreen();
            }
        }
        break;
    case Qt::Key_A:
        {
            addBookmark();
        }
        break;
    case Qt::Key_D:
        {
            deleteBookmark();
        }
        break;
    case Qt::Key_Z:
        {
            zooming(ZOOM_SELECTION);
        }
        break;
    case Qt::Key_S:
        {
            emit testSuspend();
        }
        break;
    case Qt::Key_V:
        {
            emit testWakeUp();
        }
        break;
    case Qt::Key_W:
        {
            render_client_.scaleSurface(ZOOM_TO_WIDTH);
        }
        break;
    case Qt::Key_H:
        {
            render_client_.scaleSurface(ZOOM_TO_HEIGHT);
        }
        break;
    case Qt::Key_1:
        {
            render_client_.scaleSurface(50);
        }
        break;
    case Qt::Key_2:
        {
            render_client_.scaleSurface(100);
        }
        break;
    case Qt::Key_3:
        {
            render_client_.scaleSurface(200);
        }
        break;
    case Qt::Key_4:
        {
            render_client_.scaleSurface(300);
        }
        break;
    case Qt::Key_F1:
        {
            render_client_.switchPagingMode(PM_HARD_PAGES);
        }
        break;
    case Qt::Key_F2:
        {
            render_client_.switchPagingMode(PM_HARD_PAGES_2UP);
        }
        break;
    case Qt::Key_F3:
        {
            render_client_.switchPagingMode(PM_FLOW_PAGES);
        }
        break;
    case Qt::Key_F4:
        {
            render_client_.switchPagingMode(PM_SCROLL);
        }
        break;
    case Qt::Key_F5:
        {
            render_client_.switchPagingMode(PM_SCROLL_PAGES);
        }
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
    case Qt::Key_Up:
        {
            if (status_mgr_.isDisplayHyperlinks())
            {
                previousHyperlink();
            }
            else
            {
                if (canMoveViewport())
                {
                    moveViewportByScreenSize(UP_SPACE);
                }
                else if ( status_mgr_.isFreePen() || status_mgr_.isAddAnnotation() )
                {
                    moveToUpWord();
                }
                else
                {
                    increaseFontSize();
                }
            }
        }
        break;
    case Qt::Key_Down:
    case Qt::Key_Minus:
        {
            if (status_mgr_.isDisplayHyperlinks())
            {
                nextHyperlink();
            }
            else
            {
                if (canMoveViewport())
                {
                    moveViewportByScreenSize(BOTTOM_SPACE);
                }
                else if ( status_mgr_.isFreePen() || status_mgr_.isAddAnnotation() )
                {
                    moveToDownWord();
                }
                else
                {
                    decreaseFontSize();
                }
            }
        }
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        {
            if (status_mgr_.isDisplayHyperlinks())
            {
                navigateToCurrentHyperlink();
            }
            else if ( status_mgr_.isFreePen())
            {
                textSelectionDone();
            }
            else
            {
                emit popupJumpPageDialog();
            }
        }
        break;
    case Qt::Key_Escape:
        {
            if ( status_mgr_.isSlideShow() )
            {
                stopSlideShow();
            }
            else if ((tts_engine_ != 0 && tts_engine_->state() == TTS_PLAYING) ||
                     (tts_view_ != 0 && tts_view_->isVisible()))
            {
                stopTTS();
            }
            else
            {
                returnToLibrary();
            }
        }
        break;
    case ui::Device_Menu_Key:
        {
            popupMenu();
        }
        break;
    case Qt::Key_T:
        {
            displayThumbnailView();
        }
        break;
    default:
        {
        }
        break;
    }
}

void NabooView::increaseFontSize()
{
    int current_font_idx = getIndexOfFontValue(font_values_, render_client_.renderConf().getFontRatio());
    if (current_font_idx >= static_cast<int>(font_values_.size()) - 1)
    {
        return;
    }
    current_font_idx++;
    render_client_.scaleFontRatio(font_values_[current_font_idx]);
}

void NabooView::decreaseFontSize()
{
    int current_font_idx = getIndexOfFontValue(font_values_, render_client_.renderConf().getFontRatio());
    if (current_font_idx <= 0)
    {
        return;
    }
    current_font_idx--;
    render_client_.scaleFontRatio(font_values_[current_font_idx]);
}

void NabooView::nextHyperlink()
{
    if (hyperlink_assist_.nextHyperlink())
    {
        // Disable screen update at first, as repaint triggers UpdateRequest event
        onyx::screen::instance().enableUpdate( false );
        repaint();
        onyx::screen::instance().enableUpdate( true );

        // Now we can copy the data to controller.
        onyx::screen::instance().updateWidgetRegion( this, rect(), onyx::screen::ScreenProxy::GU, false, onyx::screen::ScreenCommand::WAIT_COMMAND_FINISH );
    }
}

void NabooView::previousHyperlink()
{
    if (hyperlink_assist_.prevHyperlink())
    {
        // Disable screen update at first, as repaint triggers UpdateRequest event
        onyx::screen::instance().enableUpdate( false );
        repaint();
        onyx::screen::instance().enableUpdate( true );

        // Now we can copy the data to controller.
        onyx::screen::instance().updateWidgetRegion( this, rect(), onyx::screen::ScreenProxy::GU, false, onyx::screen::ScreenCommand::WAIT_COMMAND_FINISH );
    }
}

void NabooView::navigateToCurrentHyperlink()
{
    AdobeLocationPtr target;
    Range range = hyperlink_assist_.getCurrentHyperlink(target);
    if (range.isValid())
    {
        // fast draw the area of hyperlinks then jump to the destination
        QRect area = sketch_client_.getDirtyAreaOfRange(range);
        fastRepaint( area );
        render_client_.navigateToLink( target );
    }
}

void NabooView::resizeEvent( QResizeEvent *re )
{
    if ( model() == 0 || !render_client_.isValid() )
    {
        return;
    }

    if ( dict_widget_ != 0 && dict_widget_->isVisible() )
    {
        dict_widget_->ensureVisible( getDirtyArea( anno_ctx_ ) );
    }

    if ( tts_view_ != 0 && tts_view_->isVisible() )
    {
        tts_view_->ensureVisible();
    }

    if ( search_view_ != 0 && search_view_->isVisible())
    {
        search_view_->ensureVisible();
    }

    AdobeRenderConf & render_conf = render_client_.renderConf();
    render_conf.setOperation( RENDER_NAVIGATE_TO_LOCATION );
    if ( !render_client_.needRestore() )
    {
        render_conf.setViewport( re->size() );
    }
    render_client_.sendRenderRequest( &render_conf );
}

#ifndef QT_NO_WHEELEVENT
void NabooView::wheelEvent( QWheelEvent *we )
{
    int offset = we->delta();
    switch (render_client_.renderConf().getRotateDegree())
    {
    case 0:
        render_client_.moveViewport(0, -offset);
        break;
    case 90:
        render_client_.moveViewport(offset, 0);
        break;
    case 180:
        render_client_.moveViewport(0, offset);
        break;
    case 270:
        render_client_.moveViewport(-offset, 0);
        break;
    default:
        break;
    }
}
#endif

void NabooView::attachSketchProxy()
{
    if (status_mgr_.isErasing())
    {
        sketch_proxy_.setMode(MODE_ERASING);
    }
    else if (status_mgr_.isSketching())
    {
        sketch_proxy_.setMode(MODE_SKETCHING);
    }
    sketch_proxy_.attachWidget(this);
    updateSketchProxy();
}

void NabooView::deattachSketchProxy()
{
    sketch_proxy_.deattachWidget(this);
}

void NabooView::updateSketchProxy()
{
    // deactivate all pages
    sketch_proxy_.deactivateAll();

    //RepaintHold hold(&render_client_);
    AdobeDocumentClient * doc_client = model()->documentClient();
    Range screen_range;
    if (!render_client_.getScreenRange(screen_range))
    {
        return;
    }

    // make sure sketch on correct display context
    ZoomFactor zoom_value = render_client_.getRealZoomFactor();
    sketch_proxy_.setWidgetOrient( getSystemRotateDegree() );
    sketch_proxy_.setZoom( zoom_value );
    sketch_proxy_.setContentOrient( ROTATE_0_DEGREE );

    int start_page_number = static_cast<int>(screen_range.start->getPagePosition());
    int end_page_number   = render_client_.getPagingMode() == PM_SCROLL_PAGES ?
        static_cast<int>(screen_range.end->getPagePosition()) : start_page_number;
    if (end_page_number >= doc_client->getPageCount())
    {
        end_page_number = static_cast<int>(doc_client->getPageCount() - 1);
    }

    for ( int i = start_page_number; i <= end_page_number; ++i )
    {
        AdobeLocationPtr page_loc = doc_client->getLocationFromPagePosition(i);
        QRect region;
        if ( !sketch_client_.getPageDisplayRegion( page_loc, region ) )
        {
            continue;
        }
        sketch::PageKey page_key;
        page_key.setNum( static_cast<double>( i ) );
        sketch_proxy_.activatePage(model_->path(), page_key);
        sketch_proxy_.updatePageDisplayRegion(model_->path(), page_key, region);
    }
}

void NabooView::rotate()
{
    // Not use the native rotation any more
    //if (render_client_->getPagingMode() != PM_HARD_PAGES &&
    //    render_client_->getPagingMode() != PM_HARD_PAGES_2UP )
    //{
    //    // other mode does not support rotation
    //    qDebug("Only Hard Page Mode supports rotation");
    //    return;
    //}

    //AdobeRenderConf & render_conf = render_client_.renderConf();
    //render_conf.setOperation( RENDER_ROTATION );
    //render_conf.setRotateDegree( rotate_degree );
    //render_client_.sendRenderRequest( &render_conf );

#ifndef Q_WS_QWS
    RotateDegree prev_degree = getSystemRotateDegree();
    if (prev_degree == ROTATE_0_DEGREE)
    {
        resize(800, 600);
    }
    else
    {
        resize(600, 800);
    }
#else
    emit rotateScreen();
#endif

    RotateDegree degree = getSystemRotateDegree();
    sketch_proxy_.setWidgetOrient( degree );
}

void NabooView::zoomInPress( QMouseEvent *me )
{
    current_waveform_ = onyx::screen::instance().defaultWaveform();
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
    stroke_area_.initArea(me->pos());
    if (rubber_band_ == 0)
    {
        rubber_band_.reset(new QRubberBand(QRubberBand::Rectangle, this));
    }
    rubber_band_->setGeometry(QRect(stroke_area_.getOriginPosition(),
                                    QSize()));
    rubber_band_->show();
}

void NabooView::zoomInMove( QMouseEvent *me )
{
    stroke_area_.expandArea(me->pos());
    rubber_band_->setGeometry(QRect(stroke_area_.getOriginPosition(),
                                    me->pos()).normalized());
}

void NabooView::zoomInRelease( QMouseEvent *me )
{
    stroke_area_.expandArea(me->pos());

    rubber_band_->hide();

    // clear the background
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);

    // return to previous waveform
    onyx::screen::instance().setDefaultWaveform(current_waveform_);

    sys::SysStatus::instance().setSystemBusy(true, false);
    render_client_.selectZoom(stroke_area_.getRect());
    status_mgr_.setStatus(ID_ZOOM_IN, FUNC_NORMAL);
}

bool NabooView::zooming( double zoom_setting )
{
    if (zoom_setting == ZOOM_SELECTION)
    {
        status_mgr_.setStatus(ID_ZOOM_IN, FUNC_SELECTED);
    }
    else
    {
        return render_client_.scaleSurface(zoom_setting);
    }
    return true;
}

void NabooView::panPress( QMouseEvent *me )
{
    pan_area_.setStartPoint(me->pos());
}

void NabooView::panMove( QMouseEvent *me )
{
    // Not handle the pan-move event
}

bool NabooView::tryInternalHyperlink( const QPoint & pos )
{
    if ( !render_client_.isValid() ||
         render_client_.isLocked() ||
         model_->documentClient()->type() != REFLOWABLE_DOCUMENT )
         return false;

    AdobeLocationPtr target;
    bool ret = false;
    if ( hyperlink_assist_.hitTest( pos, selected_hyperlink_, target ) && target != 0 )
    {
        // fast draw the area of hyperlinks then jump to the destination
        QRect area = sketch_client_.getDirtyAreaOfRange( selected_hyperlink_ );
        fastRepaint( area );

        render_client_.navigateToLink( target );
        ret = true;
    }

    selected_hyperlink_.reset();
    return ret;
}

void NabooView::panRelease( QMouseEvent *me )
{
    if ( render_client_.isLocked() || !render_client_.isValid() ) return;

    pan_area_.setEndPoint(me->pos());
    int sys_offset = sys::SystemConfig::direction( pan_area_.getStart(), pan_area_.getEnd() );
    int offset_x = 0, offset_y = 0;
    pan_area_.getOffset(offset_x, offset_y);
    if ( sys_offset == 0 )
    {
        // Try to move the viewport
        if (canMoveViewport())
        {
            AdobeViewportLocation location = NO_SPACE;
            if (hitTestViewportMarks(me->pos(), location))
            {
                moveViewportByScreenSize(location);
                return;
            }
        }

        // Manually handle the internal hyperlinks because sometimes the handleEvent does
        // not work.
        // Disable this function for PDF document because it does not work on RM SDK 9.1.
        if ( !tryInternalHyperlink( me->pos() ) )
        {
            AdobeMouseEvent release_event( MOUSE_CLICK,
                                           0,
                                           1,
                                           me->pos().x(),
                                           me->pos().y() );
            render_client_.enableRepaint();
            render_client_.handleMouseEvent( &release_event );
        }

        // Hit test annotation
        Annotation anno;
        if ( hitTestAnnotation( me->pos(), anno ) )
        {
            requestUpdateAnnotation( anno );
        }
    }
    else if (render_client_.getPagingMode() == PM_FLOW_PAGES ||
             (render_client_.getPagingMode() == PM_HARD_PAGES &&
             (render_client_.renderConf().getZoomSetting() == ZOOM_TO_PAGE ||
              render_client_.renderConf().isFitForScreen())) ||
              (render_client_.zoomingMode() == ZOOM_HIDE_MARGIN &&
               !render_client_.isInFontIndexMode()))
    {
        if ( sys_offset > 0)
        {
            if (render_client_.zoomingMode() == ZOOM_HIDE_MARGIN && render_client_.getPagingMode() == PM_SCROLL_PAGES)
            {
                moveViewportByScreenSize(BOTTOM_SPACE);
            }
            else
            {
                nextScreen();
            }
        }
        else if ( sys_offset < 0 )
        {
            if (render_client_.zoomingMode() == ZOOM_HIDE_MARGIN && render_client_.getPagingMode() == PM_SCROLL_PAGES)
            {
                moveViewportByScreenSize(UP_SPACE);
            }
            else
            {
                prevScreen();
            }
        }
    }
    else
    {
        render_client_.moveViewport(offset_x, offset_y);
    }
}

void NabooView::textSelectionDone()
{
    if ( !render_client_.isValid() || !anno_ctx_.isValid() ||
         dict_widget_ == 0 || !dict_widget_->isVisible() )
    {
        return;
    }

    QString text = anno_ctx_.getText( model_->documentClient() );
    if ( !text.isEmpty() )
    {
        dict_widget_->lookup( text );
        dict_widget_->ensureVisible( getDirtyArea( anno_ctx_ ) );
    }
}

void NabooView::displaySearchView( bool )
{
    if (search_view_ == 0)
    {
        search_view_.reset( new SearchWidget( this, search_ctx_ ) );
        connect( search_view_.get(),
                 SIGNAL( search( BaseSearchContext & ) ),
                 this,
                 SLOT( onSearchNext( BaseSearchContext & ) ) );

        connect( search_view_.get(),
                 SIGNAL( closeClicked() ),
                 this,
                 SLOT( onSearchViewClose() ) );

        /*
        connect( search_view_.get(),
                 SIGNAL( isHighlightAllChanged() ),
                 this,
                 SLOT( onSearchViewIsHighlightAll() ) );
        */
    }

    // hide dictionary view
    if ( dict_widget_ != 0 && dict_widget_->isVisible() )
    {
        dict_widget_->hide();
    }

    search_view_->ensureVisible();
    emit requestUpdateParent( true );
}

void NabooView::displayDictionary( bool )
{
    if ( dict_mgr_== 0 )
    {
        dict_mgr_.reset( new DictionaryManager );
    }

    if ( tts_engine_ == 0 )
    {
        tts_engine_.reset(new TTS(QLocale::system()));
    }

    if ( dict_widget_ == 0 )
    {
        dict_widget_.reset( new DictWidget( this, *dict_mgr_, tts_engine_.get() ) );
        connect( dict_widget_.get(), SIGNAL( closeClicked() ),
                 this, SLOT( onDictClosed() ) );
        connect( dict_widget_.get(), SIGNAL( keyReleaseSignal(int) ),
                 this, SLOT( onDictKeyReleased(int) ) );
    }

    // hide search view
    if ( search_view_ != 0 && search_view_->isVisible() )
    {
        search_view_->hide();
    }

    enterWordRetrievingMode();

    // When dictionary widget is not visible, it's necessary to update the text view.
    QString text = anno_ctx_.getText( model()->documentClient() );
    dict_widget_->lookup( text );
    dict_widget_->ensureVisible( getDirtyArea( anno_ctx_ ), true );
    emit requestUpdateParent( true );
}

void NabooView::onDictClosed()
{
    status_mgr_.setStatus( ID_FREE_PEN, FUNC_NORMAL );
    anno_ctx_.reset();
}

void NabooView::onDictKeyReleased(int key)
{
    switch (key)
    {
    case Qt::Key_Up:
        {
            moveToUpWord();
        }
        break;
    case Qt::Key_Left:
        {
            moveToPreviousWord();
        }
        break;
    case Qt::Key_Right:
        {
            moveToNextWord();
        }
        break;
    case Qt::Key_Down:
        {
            moveToDownWord();
        }
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        {
            textSelectionDone();
        }
        break;
    default:
        break;
    }
}

void NabooView::onAnnotationFinished()
{
    sys::SysStatus::instance().setSystemBusy(false);
    if ( notes_dialog_ == 0 )
    {
        notes_dialog_.reset( new NotesDialog( QString(), this ) );
    }

    int ret = notes_dialog_->popup(QString());
    if (ret == QDialog::Accepted)
    {
        QString content = notes_dialog_->inputText();
        anno_ctx_.setExplanation(content);
    }
    if ( !addSelectionToAnnotations() )
    {
        clearCurrentSelection();
    }
    anno_ctx_.reset();
}

void NabooView::onSearchViewClose()
{
    model_->documentClient()->setSearchBusy(false);
    resetSearchResults();
}

void NabooView::openMusicPlayer()
{
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
    sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
}

void NabooView::onSearchViewIsHighlightAll()
{
    bool is_highlight_all = true;//search_view_->isHighlightAll();
    bool ret = false;
    if ( is_highlight_all )
    {
        ret = searchInCurrentScreen();
    }
    else
    {
        ret = model()->documentClient()->searchConf().clearSearchResultsButKeepCurrent();
    }

    if ( ret )
    {
        // mandatory update the view
        emit requestUpdateParent(false);
        update();
    }
}

bool NabooView::searchInCurrentScreen()
{
    if ( search_view_ != 0 &&
         search_view_->isVisible() &&
         //search_view_->isHighlightAll() &&
         !model()->documentClient()->searchConf().results().empty() )
    {
        Range screen_range;
        if ( render_client_.getScreenRange( screen_range ) )
        {
            return model()->documentClient()->searchConf().searchAllInRange( screen_range );
        }
    }
    return false;
}

void NabooView::onSearchNext( BaseSearchContext & ctx )
{
    if ( !render_client_.isValid() )
    {
        qDebug("Empty Renderer used in searching");
        return;
    }

    // set the search flags
    unsigned int flags = 0;
    if ( !ctx.forward() )
    {
        flags |= SF_BACK;
    }
    if ( ctx.case_sensitive() )
    {
        flags |= SF_MATCH_CASE;
    }
    if ( ctx.match_whole_word() )
    {
        flags |= SF_WHOLE_WORD;
    }
    AdobeSearchConf & search_conf = model()->documentClient()->searchConf();
    search_conf.setFlags( flags );

    // set the start & end
    // TODO. Improve the search view to support more options
    AdobeLocationPtr start;
    AdobeLocationPtr end;
    if ( search_conf.pattern() == ctx.pattern() )
    {
        // search from last result
        if ( ctx.forward() )
        {
            Range last_result;
            if ( !search_conf.results().empty() )
            {
                last_result = search_conf.results().back();
                start = last_result.end;
            }

            if ( start == 0 )
            {
                // cannot get the result, just search from current location
                start = render_client_.getCurrentLocation();
            }
            end = model()->documentClient()->getEnd();
        }
        else
        {
            Range first_result;
            if ( !search_conf.results().empty() )
            {
                first_result = search_conf.results().front();
                end = first_result.start;
            }

            if ( end == 0 )
            {
                // cannot get the result, just search from
                end = render_client_.getCurrentLocation();
            }
            start = model()->documentClient()->getBeginning();
        }
    }
    else
    {
        // search from current page
        if ( ctx.forward() )
        {
            start = render_client_.getScreenBeginning();
            end = model()->documentClient()->getEnd();
        }
        else
        {
            end = render_client_.getScreenEnd();
            start = model()->documentClient()->getBeginning();
        }
    }
    search_conf.setStart(start);
    search_conf.setEnd(end);

    // set the pattern
    search_conf.setPattern( ctx.pattern() );

    // send search request
    model()->documentClient()->sendSearchRequest( &search_conf );
}

void NabooView::onStylusChanged( const int type )
{
    switch (type)
    {
    case ID_SKETCHING:
    case ID_ERASING:
        attachSketchProxy();
        break;
    case ID_DELETE_ANNOTATION:
        anno_ctx_.reset();
    default:
        deattachSketchProxy();
        break;
    }
    emit itemStatusChanged(STYLUS, type);
}

void NabooView::onSearchSucceeded()
{
    Range first_result = model()->documentClient()->searchConf().results().front();
    AdobeRenderConf & render_conf = render_client_.renderConf();
    render_conf.setOperation(RENDER_NAVIGATE_TO_LOCATION);
    render_conf.setPagePosition(first_result.start);
    render_client_.sendRenderRequest(&render_conf);
}

void NabooView::onSearchNoMoreResults()
{
    if (search_view_ != 0 && search_view_->isVisible())
    {
        search_view_->noMoreMatches();
    }
}

void NabooView::resetSearchResults()
{
    model()->documentClient()->searchConf().clearSearchResults();
}

bool NabooView::isSearchResultsInRange( const Range & search_result,
                                        const Range & range )
{
    if (search_result.start >= range.start && search_result.end <= range.end)
    {
        return true;
    }
    return false;
}

void NabooView::paintSearchResults( QPainter & painter, QBrush & brush )
{
    AdobeSearchConf & search_conf = model()->documentClient()->searchConf();
    if (search_conf.results().empty())
    {
        return;
    }

    //RepaintHold hold(&render_client_);
    Range screen_range;
    if (!render_client_.getScreenRange(screen_range))
    {
        return;
    }

    SearchResults::const_iterator iter = search_conf.results().begin();
    for ( ; iter != search_conf.results().end(); ++iter )
    {
        Range result = *iter;
        if (isSearchResultsInRange(result, screen_range))
        {
            paintSelections(painter, brush, result);
        }
    }
}

void NabooView::enterWordRetrievingMode()
{
    if (status_mgr_.isAddAnnotation())
    {
        status_mgr_.setStatus(ID_ADD_ANNOTATION, FUNC_NORMAL);
    }
    else if (status_mgr_.isDeleteAnnotation())
    {
        status_mgr_.setStatus(ID_DELETE_ANNOTATION, FUNC_NORMAL);
    }
    status_mgr_.setStatus(ID_FREE_PEN, FUNC_SELECTED);

    Range first_word;
    if (firstWord(first_word))
    {
        // deselect previous selections if there is any
        clearCurrentSelection();
        anno_ctx_.addLocation(first_word.start);
        anno_ctx_.addLocation(first_word.end);
        bool is_dirty = false;
        if ( status_mgr_.isAddAnnotation())
        {
            is_dirty = displayCurrentSelection();
            sys::SysStatus::instance().setSystemBusy(true, false);
            annotation_timer_.start();
        }
        else
        {
            is_dirty = displayCurrentSelection();
        }
        is_screen_anno_dirty_ = ( is_screen_anno_dirty_ || is_dirty );
    }
}

void NabooView::displayTOC( bool )
{
#ifdef MAIN_WINDOW_TOC_ON
    QWidget* tree_view = dynamic_cast<MainWindow*>(parentWidget())->getView(TOC_VIEW);
    if (tree_view == 0)
    {
        return;
    }

    down_cast<MainWindow*>(parentWidget())->activateView(TOC_VIEW);
    down_cast<TreeViewDialog*>(tree_view)->setModel( model_->getTOCModel() );
    down_cast<TreeViewDialog*>(tree_view)->initialize( tr("Table of Contents") );
#else
    QStandardItemModel * toc_model = model()->documentClient()->getTOCModel();
    if (toc_model == 0)
    {
        qDebug("No TOC");
        return;
    }

    TreeViewDialog toc_view(this);
    toc_view.setModel(toc_model);
    toc_view.tree().showHeader(true);

    QVector<int> percentages;
    percentages.push_back(80);
    percentages.push_back(20);
    toc_view.tree().setColumnWidth(percentages);
    int ret = toc_view.popup( tr("Table of Contents") );

    // Returned from the TOC view
    onyx::screen::instance().enableUpdate( false );
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate( true );

    if (ret != QDialog::Accepted)
    {
        return;
    }

    QModelIndex index = toc_view.selectedItem();
    if ( !index.isValid() )
    {
        return;
    }

    sys::SysStatus::instance().setSystemBusy(true, false);
    gotoPosition( model()->documentClient()->getPositionByTOCIndex( index ) );
#endif
}

void NabooView::displayAnnotations( bool )
{
    QStandardItemModel anno_model;
    if ( !model()->annotationMgr()->getAnnotationsModel(anno_model) )
    {
        qDebug("No annotations");
    }

    TreeViewDialog anno_view( this );
    anno_view.setModel( &anno_model );
    anno_view.tree().showHeader(true);

    QVector<int> percentages;
    percentages.push_back(80);
    percentages.push_back(20);
    anno_view.tree().setColumnWidth(percentages);
    int ret = anno_view.popup(tr("Annotations"));

    onyx::screen::instance().enableUpdate(false);
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);

    if (ret != QDialog::Accepted)
    {
        return;
    }
    QModelIndex index = anno_view.selectedItem();
    if (!index.isValid())
    {
        return;
    }

    sys::SysStatus::instance().setSystemBusy(true, false);
    gotoPosition( model()->annotationMgr()->getPositionByAnnoIndex( index ) );
}

void NabooView::setSketchMode(const SketchMode mode, bool selected)
{
    FunctionStatus s = selected ? FUNC_SELECTED : FUNC_NORMAL;
    FunctionID id = ID_UNKNOWN;
    switch ( mode )
    {
    case MODE_SKETCHING:
        id = ID_SKETCHING;
        break;
    case MODE_ERASING:
        id = ID_ERASING;
        break;
    case ADD_ANNOTATION:
        id = ID_ADD_ANNOTATION;
        break;
    case DELETE_ANNOTATION:
        id = ID_DELETE_ANNOTATION;
        break;
    default:
        return;
    }
    status_mgr_.setStatus(id, s);
}

void NabooView::setAnnotationMode( const AnnotationMode mode, bool selected )
{
    FunctionStatus s = selected ? FUNC_SELECTED : FUNC_NORMAL;
    FunctionID id = ID_UNKNOWN;
    switch ( mode )
    {
    case ADD_ANNOTATION:
        id = ID_ADD_ANNOTATION;
        break;
    case DELETE_ANNOTATION:
        id = ID_DELETE_ANNOTATION;
        break;
    default:
        return;
    }
    status_mgr_.setStatus(id, s);
}

void NabooView::setSketchColor( const SketchColor color )
{
    sketch_proxy_.setColor( color );
    status_mgr_.setStatus( ID_SKETCHING, FUNC_SELECTED );
}

void NabooView::setSketchShape( const SketchShape shape )
{
    sketch_proxy_.setShape( shape );
    status_mgr_.setStatus( ID_SKETCHING, FUNC_SELECTED );
}

void NabooView::penPress( QMouseEvent *me )
{
    // TODO. Refine it. Maybe the mouse event handling should be wrapped into a task?
    if ( render_client_.isLocked() || !render_client_.isValid() ) return;

    QPoint pos = me->pos();
    pan_area_.setStartPoint( pos );

    // deselect previous selections if there is any
    clearCurrentSelection();

    // deselect the search results
    resetSearchResults();

    // get current loction for hit test
    bool is_dirty = false;
    AdobeLocationPtr loc;
    Range ref_range( anno_ctx_.lastLocation(), anno_ctx_.currentLocation() );
    if ( sketch_client_.hitTest( pos, ref_range, loc ) )
    {
        if (!status_mgr_.isDeleteAnnotation())
        {
            annotation_timer_.stop();   // actually this sentence is useless because it is set to system busy after activating annotation timer
            anno_ctx_.reset();
            anno_ctx_.addLocation( loc );
        }
        else
        {
            // stop the previous repaint request
            repaint_timer_.stop();
            is_dirty = removeAnnotationsOnLocation( loc );
        }
    }

    is_screen_anno_dirty_ = ( is_screen_anno_dirty_ || is_dirty );
}

void NabooView::penMove( QMouseEvent *me )
{
    // TODO. Refine it. Maybe the mouse event handling should be wrapped into a task?
    if ( render_client_.isLocked() || !render_client_.isValid() ) return;

    QPoint pos = me->pos();
    bool is_dirty = false;
    AdobeLocationPtr loc;
    Range ref_range( anno_ctx_.lastLocation(), anno_ctx_.currentLocation() );
    if ( sketch_client_.hitTest( pos, ref_range, loc ) )
    {
        if (!status_mgr_.isDeleteAnnotation())
        {
            PagePosition current_page = static_cast<PagePosition>(loc->getPagePosition());
            PagePosition first_page = current_page;
            if (anno_ctx_.firstLocation() != 0)
            {
                first_page = static_cast<PagePosition>(anno_ctx_.firstLocation()->getPagePosition());
            }

            if (render_client_.getPagingMode() == PM_SCROLL_PAGES && first_page != current_page)
            {
                if (!addSelectionToAnnotations())
                {
                    clearCurrentSelection();
                }
                else
                {
                    anno_ctx_.reset();
                    anno_ctx_.addLocation( loc );
                }
            }
            else
            {
                anno_ctx_.addLocation( loc );
                // fast draw current selections
                if ( status_mgr_.isFreePen() )
                {
                    is_dirty = displayCurrentSelection();
                }
            }
        }
        else
        {
            is_dirty = removeAnnotationsOnLocation( loc );
        }
    }
    is_screen_anno_dirty_ = ( is_screen_anno_dirty_ || is_dirty );
}

void NabooView::penRelease( QMouseEvent *me )
{
    // TODO. Refine it. Maybe the mouse event handling should be wrapped into a task?
    if ( render_client_.isLocked() || !render_client_.isValid() ) return;

    QPoint pos = me->pos();
    pan_area_.setEndPoint( pos );

    int offset_x = 0;
    int offset_y = 0;
    pan_area_.getOffset( offset_x, offset_y );

    AdobeLocationPtr loc;
    bool select_valid_area = true;
    if ( !status_mgr_.isDeleteAnnotation() && abs(offset_x) < MOVE_ERROR && abs(offset_y) < MOVE_ERROR )
    {
        select_valid_area = penClick( me );
    }
    else
    {
        // get the latest selections
        Range ref_range( anno_ctx_.lastLocation(), anno_ctx_.currentLocation() );
        if ( !status_mgr_.isDeleteAnnotation() && sketch_client_.hitTest( pos, ref_range, loc ) )
        {
            anno_ctx_.addLocation( loc );
        }
    }

    bool is_dirty = false;
    if ( status_mgr_.isAddAnnotation() && select_valid_area )
    {
        is_dirty = displayCurrentSelection();
        sys::SysStatus::instance().setSystemBusy(true, false);
        annotation_timer_.start();
    }
    else if ( status_mgr_.isFreePen() && select_valid_area )
    {
        is_dirty = displayCurrentSelection();
        textSelectionDone();
    }
    else if ( status_mgr_.isDeleteAnnotation() )
    {
        is_dirty = removeAnnotationsOnLocation( loc );
    }
    else
    {
        return;
    }

    is_screen_anno_dirty_ = ( is_screen_anno_dirty_ || is_dirty );

    if ( !status_mgr_.isAddAnnotation() )
    {
        // repaint the dirty area at last
        repaint_timer_.start();
    }
}

bool NabooView::penClick( QMouseEvent *me )
{
    if ( !render_client_.isValid() || render_client_.isLocked() ) return false;

    if (status_mgr_.isAddAnnotation())
    {
        // Hit test annotation
        Annotation anno;
        if ( hitTestAnnotation( me->pos(), anno ) )
        {
            requestUpdateAnnotation( anno );
            return false;
        }
    }

    Range word_range;
    if ( retrieveWord( me->pos(), word_range ) )
    {
        anno_ctx_.reset();
        anno_ctx_.addLocation( word_range.start );
        anno_ctx_.addLocation( word_range.end );
        return true;
    }
    return false;
}

bool NabooView::retrieveWord( const QPoint & pos, Range & range )
{
    if ( render_client_.isLocked() || !render_client_.isValid() ) { return false; }
    range.reset();

    // get current loction for hit test
    AdobeLocationPtr cur_location;
    if (!render_client_.hitTest(pos, HF_SELECT, cur_location))
    {
        return false;
    }

    AdobeContentIteratorPtr iter = model()->documentClient()->getContentIterator(1, cur_location);

    // search for the beginning
    QString str = iter->previous(CI_JOIN_ALPHA);
    range.start = iter->getCurrentPosition();

    // search for the end
    str = iter->next(CI_JOIN_ALPHA);
    range.end = iter->getCurrentPosition();
    return range.isValid();
}

bool NabooView::nextWord( const Range & current_word, Range & result )
{
    if (!current_word.isValid())
    {
        return false;
    }

    AdobeLocationPtr end = render_client_.getScreenEnd();
    AdobeContentIteratorPtr iter = model()->documentClient()->getContentIterator(1, current_word.end);

    // search for the beginning
    QString str = " ";
    QChar ch = str.at(0);
    while (isSpace(ch) || isPuncuation(ch) || isHyphen(ch) || isNumber(ch))
    {
        result.start = iter->getCurrentPosition();
        str = iter->next(CI_JOIN_ALPHA);
        if (str.isEmpty())
        {
            return false;
        }
        ch = str.at(0);
        result.end = iter->getCurrentPosition();
        if (result.isValid() && result.start >= end)
        {
            return false;
        }
    }
    return true;
}

bool NabooView::previousWord( const Range & current_word, Range & result )
{
    if (!current_word.isValid())
    {
        return false;
    }

    AdobeLocationPtr beginning = render_client_.getScreenBeginning();
    AdobeContentIteratorPtr iter = model()->documentClient()->getContentIterator(1, current_word.start);

    // search for the beginning
    QString str = " ";
    QChar ch = str.at(0);
    while (isSpace(ch) || isPuncuation(ch) || isHyphen(ch) || isNumber(ch))
    {
        result.end = iter->getCurrentPosition();
        str = iter->previous(CI_JOIN_ALPHA);
        if (str.isEmpty())
        {
            return false;
        }
        ch = str.at(0);
        result.start = iter->getCurrentPosition();
        if (result.isValid() && result.end <= beginning)
        {
            return false;
        }
    }
    return true;
}

bool NabooView::upWord( const Range & current_word, Range & result )
{
    if (!current_word.isValid())
    {
        return false;
    }
    result.reset();

    AdobeLocationPtr beginning = render_client_.getScreenBeginning();
    AdobeContentIteratorPtr iter = model()->documentClient()->getContentIterator(1, current_word.start);
    int min_x_distance = -1;
    int min_y_distance = -1;
    bool found = false;
    Range range;
    while (!found)
    {
        // search for the beginning
        QString str = " ";
        QChar ch = str.at(0);
        while (isSpace(ch) || isPuncuation(ch) || isHyphen(ch) || isNumber(ch))
        {
            range.end = iter->getCurrentPosition();
            str = iter->previous(CI_JOIN_ALPHA | CI_IGNORE_TRAILING_PUNC);
            if (str.isEmpty())
            {
                return false;
            }
            ch = str.at(0);
            range.start = iter->getCurrentPosition();

            if (range.isValid() && range.end <= beginning)
            {
                break;
            }
        }

        if (range.end > beginning)
        {
            // found valid word?
            QRect range_area = sketch_client_.getDirtyAreaOfRange(range);
            QRect current_word_area = sketch_client_.getDirtyAreaOfRange(current_word);

            // check vertical position
            if (range_area.center().y() < current_word_area.top())
            {
                int distance_y = abs(current_word_area.top() - range_area.center().y());
                if (min_y_distance < 0 || min_y_distance >= distance_y)
                {
                    min_y_distance = distance_y;
                    int distance_x = abs(current_word_area.center().x() - range_area.center().x());

                    // record distance
                    if (min_x_distance < 0 || min_x_distance >= distance_x)
                    {
                        min_x_distance = distance_x;
                        result = range;
                    }
                }
                else
                {
                    found = true;
                }
            }
        }
        // exit loop if none of the word is found
        if (range.end <= beginning)
        {
            found = result.isValid();
            break;
        }
    }
    return found;
}

bool NabooView::downWord( const Range & current_word, Range & result )
{
    if (!current_word.isValid())
    {
        return false;
    }
    result.reset();

    AdobeLocationPtr end = render_client_.getScreenEnd();
    AdobeContentIteratorPtr iter = model()->documentClient()->getContentIterator(1, current_word.end);
    int min_x_distance = -1;
    int min_y_distance = -1;
    bool found = false;
    Range range;
    while (!found)
    {
        // search for the beginning
        QString str = " ";
        QChar ch = str.at(0);
        while (isSpace(ch) || isPuncuation(ch) || isHyphen(ch) || isNumber(ch))
        {
            range.start = iter->getCurrentPosition();
            str = iter->next(CI_JOIN_ALPHA);
            if (str.isEmpty())
            {
                return false;
            }
            ch = str.at(0);
            range.end = iter->getCurrentPosition();
            if (range.isValid() && range.start >= end)
            {
                break;
            }
        }

        if (range.start < end)
        {
            // found valid word?
            QRect range_area = sketch_client_.getDirtyAreaOfRange(range);
            QRect current_word_area = sketch_client_.getDirtyAreaOfRange(current_word);

            // check vertical position
            if (range_area.center().y() > current_word_area.bottom())
            {
                int distance_y = abs(range_area.center().y() - current_word_area.bottom());
                if (min_y_distance < 0 || min_y_distance >= distance_y)
                {
                    min_y_distance = distance_y;
                    int distance_x = abs(current_word_area.center().x() - range_area.center().x());

                    // record distance
                    if (min_x_distance < 0 || min_x_distance >= distance_x)
                    {
                        min_x_distance = distance_x;
                        result = range;
                    }
                }
                else
                {
                    found = true;
                }
            }
        }

        // exit loop if none of the word is found
        if (range.start >= end)
        {
            found = result.isValid();
            break;
        }
    }
    return found;
}

bool NabooView::firstWord( Range & result )
{
    AdobeLocationPtr beginning = render_client_.getScreenBeginning();
    AdobeContentIteratorPtr iter = model()->documentClient()->getContentIterator(1, beginning);

    // search for the beginning
    QString str = " ";
    QChar ch = str.at(0);
    Range last_result;
    while (isSpace(ch) || isPuncuation(ch) || isHyphen(ch) || isNumber(ch))
    {
        last_result = result;

        result.start = iter->getCurrentPosition();
        str = iter->next(CI_JOIN_ALPHA);
        if (str.isEmpty())
        {
            return false;
        }
        ch = str.at(0);
        result.end = iter->getCurrentPosition();

        if (last_result.isValid() && result.isValid() && result.end <= last_result.start)
        {
            return false;
        }
    }
    return true;
}

void NabooView::moveToNextWord()
{
    Range current_word(anno_ctx_.firstLocation(), anno_ctx_.currentLocation());
    Range result;
    if (nextWord(current_word, result))
    {
        // deselect previous selections if there is any
        clearCurrentSelection();
        anno_ctx_.addLocation(result.start);
        anno_ctx_.addLocation(result.end);
        bool is_dirty = false;
        if ( status_mgr_.isAddAnnotation())
        {
            is_dirty = displayCurrentSelection();
            sys::SysStatus::instance().setSystemBusy(true, false);
            annotation_timer_.start();
        }
        else
        {
            is_dirty = displayCurrentSelection();
        }
        is_screen_anno_dirty_ = ( is_screen_anno_dirty_ || is_dirty );

        // hide dictionary widget if necessary
        if (dict_widget_ != 0 && dict_widget_->isVisible())
        {
            dict_widget_->ensureVisible( getDirtyArea( anno_ctx_ ) );
        }
    }
}

void NabooView::moveToPreviousWord()
{
    Range current_word(anno_ctx_.firstLocation(), anno_ctx_.currentLocation());
    Range result;
    if (previousWord(current_word, result))
    {
        // deselect previous selections if there is any
        clearCurrentSelection();
        anno_ctx_.addLocation(result.start);
        anno_ctx_.addLocation(result.end);
        bool is_dirty = false;
        if ( status_mgr_.isAddAnnotation())
        {
            is_dirty = displayCurrentSelection();
            sys::SysStatus::instance().setSystemBusy(true, false);
            annotation_timer_.start();
        }
        else
        {
            is_dirty = displayCurrentSelection();
        }
        is_screen_anno_dirty_ = ( is_screen_anno_dirty_ || is_dirty );

        // hide dictionary widget if necessary
        if (dict_widget_ != 0 && dict_widget_->isVisible())
        {
            dict_widget_->ensureVisible( getDirtyArea( anno_ctx_ ) );
        }
    }
}

void NabooView::moveToUpWord()
{
    Range current_word(anno_ctx_.firstLocation(), anno_ctx_.currentLocation());
    Range result;
    if (upWord(current_word, result))
    {
        // deselect previous selections if there is any
        clearCurrentSelection();
        anno_ctx_.addLocation(result.start);
        anno_ctx_.addLocation(result.end);
        bool is_dirty = false;
        if ( status_mgr_.isAddAnnotation())
        {
            is_dirty = displayCurrentSelection();
            sys::SysStatus::instance().setSystemBusy(true, false);
            annotation_timer_.start();
        }
        else
        {
            is_dirty = displayCurrentSelection();
        }
        is_screen_anno_dirty_ = ( is_screen_anno_dirty_ || is_dirty );

        // hide dictionary widget if necessary
        if (dict_widget_ != 0 && dict_widget_->isVisible())
        {
            dict_widget_->ensureVisible( getDirtyArea( anno_ctx_ ) );
        }
    }
}

void NabooView::moveToDownWord()
{
    Range current_word(anno_ctx_.firstLocation(), anno_ctx_.currentLocation());
    Range result;
    if (downWord(current_word, result))
    {
        // deselect previous selections if there is any
        clearCurrentSelection();
        anno_ctx_.addLocation(result.start);
        anno_ctx_.addLocation(result.end);
        bool is_dirty = false;
        if ( status_mgr_.isAddAnnotation())
        {
            is_dirty = displayCurrentSelection();
            sys::SysStatus::instance().setSystemBusy(true, false);
            annotation_timer_.start();
        }
        else
        {
            is_dirty = displayCurrentSelection();
        }
        is_screen_anno_dirty_ = ( is_screen_anno_dirty_ || is_dirty );

        // hide dictionary widget if necessary
        if (dict_widget_ != 0 && dict_widget_->isVisible())
        {
            dict_widget_->ensureVisible( getDirtyArea( anno_ctx_ ) );
        }
    }
}

bool NabooView::copyToClipboard()
{
    // if it is in sketching mode, add the current selection to annotations
    if ( !anno_ctx_.isValid() )
    {
        qDebug("Nothing is selected");
        return false;
    }

    Range range( anno_ctx_.firstLocation(), anno_ctx_.currentLocation() );
    if ( !model_->annotationMgr()->isValidRange( range ) )
    {
        qDebug("Invalid selection Range");
        return false;
    }

    QString text = anno_ctx_.getText(model_->documentClient());
    QClipboard * clip_board = QApplication::clipboard();
    clip_board->setText(text);
    return true;
}

void NabooView::paintSketches( QPainter & painter, AdobeLocationPtr page_loc )
{
    if ( render_client_.isLocked() || !render_client_.isValid() ) return;

    sketch::PageKey page_key;
    int page_number = static_cast<int>(page_loc->getPagePosition());
    page_key.setNum( page_number );
    if ( !sketch_proxy_.isPageExisting( model_->path(), page_key ) )
    {
        return;
    }

    if ( !sketch_proxy_.loadPage( model_->path(), page_key, QString() ) )
    {
        return;
    }

    ZoomFactor zoom_value = render_client_.getRealZoomFactor();

    // make sure sketch on correct display context
    sketch_proxy_.setWidgetOrient( getSystemRotateDegree() );
    sketch_proxy_.setZoom( zoom_value );
    sketch_proxy_.setContentOrient( ROTATE_0_DEGREE );

    QRect region;
    if ( !sketch_client_.getPageDisplayRegion( page_loc, region ) )
    {
        return;
    }
    sketch_proxy_.updatePageDisplayRegion(model_->path(), page_key, region);

    // draw sketches in this page
    sketch_proxy_.paintPage(model_->path(), page_key, painter);
}

void NabooView::paintSketchesInCurrentScreen( QPainter & painter )
{
    if (!render_client_.isValid() || !sketch_client_.canSketch() || render_client_.isLocked())
    {
        return;
    }

    //RepaintHold hold(&render_client_);
    AdobeDocumentClient * doc_client = model()->documentClient();
    if (render_client_.getPagingMode() == PM_SCROLL_PAGES)
    {
        Range screen_range;
        if (!render_client_.getScreenRange(screen_range))
        {
            return;
        }

        int start_page_number = static_cast<int>(screen_range.start->getPagePosition());
        int end_page_number   = static_cast<int>(screen_range.end->getPagePosition());
        if (end_page_number >= doc_client->getPageCount())
        {
            end_page_number = static_cast<int>(doc_client->getPageCount() - 1);
        }

        for ( int i = start_page_number; i <= end_page_number; ++i )
        {
            AdobeLocationPtr page_loc = doc_client->getLocationFromPagePosition(i);
            paintSketches(painter, page_loc);
        }
    }
    else
    {
        AdobeLocationPtr current = render_client_.getCurrentLocation();
        paintSketches(painter, current);
    }
}

void NabooView::paintHyperlinks(QPainter & painter)
{
    if (!render_client_.isValid() || render_client_.isLocked() || !status_mgr_.isDisplayHyperlinks())
    {
        return;
    }

    QBrush brush( QColor( 0, 0, 0, 0x80 ) );
    QList<Range> ranges;
    AdobeLocationPtr target;
    Range current_link = hyperlink_assist_.getCurrentHyperlink(target);
    if (current_link.isValid())
    {
        ranges.push_back(current_link);
    }

    foreach ( Range range, ranges )
    {
        QList<QRect> rects = sketch_client_.getMergedDisplayRegionOfRange( range );
        foreach( QRect rect, rects )
        {
            painter.fillRect( rect, brush );
        }
    }
}

void NabooView::paintSelectedHyperlink( QPainter & painter, QBrush & brush )
{
    if (!render_client_.isLocked() &&
        render_client_.isValid() &&
        selected_hyperlink_.isValid() &&
        !selected_hyperlink_.isEmpty())
    {
        QList<QRect> rects = sketch_client_.getMergedDisplayRegionOfRange( selected_hyperlink_ );
        foreach( QRect rect, rects )
        {
            painter.fillRect( rect, brush );
        }
    }
}

bool NabooView::addSelectionToAnnotations()
{
    // if it is in sketching mode, add the current selection to annotations
    if ( !anno_ctx_.isValid() )
    {
        qDebug("Nothing is annotated");
        return false;
    }

    Range range( anno_ctx_.firstLocation(), anno_ctx_.currentLocation() );
    if ( !model_->annotationMgr()->isValidRange( range ) )
    {
        qDebug("Invalid Annotation Range");
        return false;
    }

    QString anno_str = anno_ctx_.getText(model_->documentClient());
    Annotation annotation;
    if (!model_->annotationMgr()->getAnnotationFromRange(range, anno_str, annotation ) )
    {
        qDebug("Cannot retrieve the annotation object");
        return false;
    }

    if ( !model_->annotationMgr()->addAnnotation(annotation) )
    {
        qDebug("Annotation cannot be inserted");
        return false;
    }
    return true;
}

bool NabooView::removeAnnotationsOnLocation( AdobeLocationPtr loc )
{
    if ( !render_client_.isValid() || render_client_.isLocked() || loc == 0 )
    {
        return false;
    }

    Range erased_range;
    if ( model()->annotationMgr()->removeAnnotation( loc, erased_range ) )
    {
        QRect dirty_area = sketch_client_.getDirtyAreaOfRange( erased_range );
        if ( dirty_area.isValid() )
        {
            fastRepaint( dirty_area );
            return true;
        }
    }
    return false;
}

bool NabooView::hitTestAnnotation( const QPoint & pos, Annotation & anno )
{
    // get annotation list of current page
    Range screen_range;
    if (!render_client_.getScreenRange(screen_range))
    {
        return false;
    }

    QList<Annotation> annotations = model_->annotationMgr()->getAnnotationListByRange( screen_range );
    QList<Annotation>::iterator iter = annotations.begin();
    for (; iter != annotations.end(); ++iter)
    {
        Annotation & dst = *iter;
        Range range = model()->annotationMgr()->getRangeFromAnnotation( dst );
        if (range.isValid())
        {
            QList<QRect> merged_boxes = sketch_client_.getMergedDisplayRegionOfRange( range );
            for (QList<QRect>::iterator iter = merged_boxes.begin(); iter != merged_boxes.end(); ++iter)
            {
                if ((*iter).contains(pos))
                {
                    anno = dst;
                    return true;
                }
            }
        }
    }

    // check the hit area per annotation
    return false;
}

void NabooView::updateAnnotation( const Annotation & anno )
{
    model()->annotationMgr()->updateAnnotation( anno );
}

void NabooView::requestUpdateAnnotation( const Annotation & anno )
{
    if ( notes_dialog_ == 0 )
    {
        notes_dialog_.reset( new NotesDialog( QString(), this ) );
    }

    int ret = notes_dialog_->popup(anno.title());
    if (ret == QDialog::Accepted)
    {
        QString content = notes_dialog_->inputText();
        Annotation selected_anno = anno;
        if (selected_anno.data().isValid())
        {
            if (!content.isEmpty())
            {
                selected_anno.mutable_title() = content;
            }
            else
            {
                Range range = model_->annotationMgr()->getRangeFromAnnotation( selected_anno );
                if (range.isValid())
                {
                    selected_anno.mutable_title() = model_->documentClient()->getText( range );
                }
            }
            updateAnnotation(selected_anno);
        }
    }
}

void NabooView::paintAnnotationsInScreen( QPainter & painter, QBrush & brush )
{
    if ( render_client_.isLocked() || !render_client_.isValid() ) return;

    //RepaintHold hold(&render_client_);
    Range screen_range;
    if ( !render_client_.getScreenRange( screen_range ) )
    {
        return;
    }

    QList<Annotation> annotations = model()->annotationMgr()->getAnnotationListByRange(screen_range);
    QList<Annotation>::iterator iter = annotations.begin();
    for (; iter != annotations.end(); ++iter)
    {
        Annotation & anno = *iter;
        Range range = model()->annotationMgr()->getRangeFromAnnotation( anno );
        if (range.isValid())
        {
            paintSelections( painter, brush, range );
        }
    }
}

QRect NabooView::getDirtyArea( AnnotationCtx & anno_ctx )
{
    // fast draw current selection
    if ( !anno_ctx.isValid() )
    {
        //qDebug("Nothing is selected, Dirty area is empty");
        return QRect();
    }

    Range range1( anno_ctx.firstLocation(), anno_ctx.currentLocation() );
    QRect r1 = sketch_client_.getDirtyAreaOfRange( range1 );

    Range range2( anno_ctx.lastLocation(), anno_ctx.currentLocation() );
    QRect r2 = sketch_client_.getDirtyAreaOfRange( range2 );

    QRect result;
    if ( r1.isValid() && r2.isValid() )
    {
        result = r1.unite( r2 );
    }
    else if ( r1.isValid() )
    {
        result = r1;
    }
    else if ( r2.isValid() )
    {
        result = r2;
    }
    return result;
}

QString AnnotationCtx::getText( AdobeDocumentClient * document )
{
    QString text = explanation_;
    if ( !isValid() || !text.isEmpty())
    {
        return text;
    }

    Range range( firstLocation(), currentLocation() );
    text = document->getText(range);
    return text;
}

void NabooView::clearCurrentSelection()
{
    if ( !render_client_.isValid() || render_client_.isLocked() )
    {
        return;
    }

    QRect dirty_area = getDirtyArea( anno_ctx_ );
    anno_ctx_.reset();
    if ( !dirty_area.isValid() )
    {
        return;
    }
    fastRepaint( dirty_area );
}

bool NabooView::displayCurrentSelection()
{
    if (!render_client_.isValid() || render_client_.isLocked())
    {
        return false;
    }

    QRect dirty_area = getDirtyArea(anno_ctx_);
    /*qDebug("Dirty Area: x:%d, y:%d, width:%d, height:%d",
           dirty_area.left(),
           dirty_area.top(),
           dirty_area.width(),
           dirty_area.height());*/

    if (!dirty_area.isValid())
    {
        return false;
    }
    fastRepaint(dirty_area);
    return true;
}

void NabooView::paintSelections( QPainter & painter,
                                 QBrush & brush,
                                 Range & range )
{
    if (render_client_.isLocked() || !render_client_.isValid()) return;

    QList<QRect> merged_boxes = sketch_client_.getMergedDisplayRegionOfRange(range);
    for (QList<QRect>::iterator iter = merged_boxes.begin(); iter != merged_boxes.end(); ++iter)
    {
        painter.fillRect( *iter, brush );
    }
}

void NabooView::paintSelectings( QPainter & painter, QBrush & brush )
{
    // fast draw current selection
    if (!anno_ctx_.isValid() ||
        render_client_.isLocked() ||
        !render_client_.isValid() ||
        !status_mgr_.isFreePen())
    {
        //qDebug("Nothing is selected, Cannot continue painting");
        return;
    }

    // correct the order of locations
    Range range(anno_ctx_.firstLocation(), anno_ctx_.currentLocation());
    paintSelections(painter, brush, range);
}

void NabooView::saveReadingContext()
{
    // save the reading history
    QVariant item;
    item.setValue(render_client_.renderConf());
    reading_history_.addItem(item);
}

bool NabooView::back()
{
    if (reading_history_.canGoBack())
    {
        reading_history_.back();
        AdobeRenderConf render_conf = reading_history_.currentItem().value< AdobeRenderConf >();
        render_conf.setOperation(RENDER_RESTORE);
        render_client_.renderConf() = render_conf;
        render_client_.sendRenderRequest(&render_conf);
        return true;
    }
    return false;
}

bool NabooView::forward()
{
    if (reading_history_.canGoForward())
    {
        reading_history_.forward();
        AdobeRenderConf render_conf = reading_history_.currentItem().value< AdobeRenderConf >();
        render_conf.setOperation(RENDER_RESTORE);
        render_client_.renderConf() = render_conf;
        render_client_.sendRenderRequest(&render_conf);
        return true;
    }
    return false;
}

void NabooView::startTTS()
{
    if ( tts_engine_ == 0 )
    {
        tts_engine_.reset( new TTS( QLocale::system() ) );
    }

    if ( tts_view_ == 0 )
    {
        tts_view_.reset( new TTSWidget( this, *tts_engine_ ) );
        tts_view_->installEventFilter(this);
        connect(tts_view_.get(), SIGNAL(speakDone()), this, SLOT(onTTSPlayFinished()));
    }

    // hide dictionary view
    if ( dict_widget_ != 0 && dict_widget_->isVisible() )
    {
        dict_widget_->hide();
    }

    // hide search view
    if ( search_view_ != 0 && search_view_->isVisible() )
    {
        search_view_->hide();
    }

    if (!tts_view_->isVisible())
    {
        tts_view_->ensureVisible();
        requestPlayingVoice();
    }
}

void NabooView::playVoiceOnCurrentScreen()
{
    if (tts_view_ == 0 || !tts_view_->isVisible())
    {
        qDebug("TTS has finished");
        return;
    }

    AdobeDocumentClient* document = model()->documentClient();
    if (document == 0 || !render_client_.getScreenRange(tts_range_))
    {
        qDebug("Document is invalid, cannot launch TTS");
        return;
    }

    if (!render_client_.isValid() || render_client_.isLocked())
    {
        qDebug("Renderer is invalid, launch TTS then!");
        requestPlayingVoice();
        return;
    }

    QString text = document->getText(tts_range_);
    if (!text.isEmpty())
    {
        if (tts_engine_ != 0 && tts_view_->isVisible())
        {
            qDebug("Ensure TTS view");
            tts_view_->speak( text );
        }
    }
    else if (render_client_.isAtEnd())
    {
        pauseTTS();
    }
    render_client_.renderConf().setNeedPlayVoice( false );
}

void NabooView::onTTSPlayFinished()
{
    if (tts_view_ == 0 || !tts_view_->isVisible() ||
        tts_view_->state() == TTS_STOPPED || tts_view_->state() == TTS_PAUSED)
    {
        qDebug("TTS has finished");
        return;
    }

    AdobeRenderConf & render_conf = render_client_.renderConf();
    if (render_client_.isAtEnd())
    {
        // stop play voice
        render_conf.setNeedPlayVoice( false );
        pauseTTS();
        return;
    }

    // check range of current page and the tts range. If they are not the same,
    // read the text of current page
    Range screen_range;
    if (render_client_.getScreenRange(screen_range))
    {
        if (tts_range_.start == screen_range.start && tts_range_.end == screen_range.end)
        {
            render_conf.setOperation( RENDER_NAVIGATE_NEXT_SCREEN );
            render_conf.setNeedPlayVoice( true );
            render_client_.sendRenderRequest( &render_conf );
        }
        else
        {
            requestPlayingVoice();
        }
    }
}

void NabooView::stopTTS()
{
    if (tts_view_ != 0)
    {
        tts_view_->onCloseClicked(true);
    }
}

bool NabooView::pauseTTS()
{
    if (tts_view_ != 0)
    {
        return tts_view_->pause();
    }
    return false;
}

bool NabooView::resumeTTS()
{
    if (tts_view_ != 0)
    {
        return tts_view_->resume();
    }
    return false;
}

void NabooView::onRequestFullUpdateScreen()
{
    if ( is_screen_anno_dirty_ )
    {
        // mandatory update the view
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, false);
        is_screen_anno_dirty_ = false;
    }
}

void NabooView::onRequestFastUpdateScreen()
{
    onyx::screen::instance().enableUpdate(false);
    repaint();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget( this, onyx::screen::ScreenProxy::GU );
}

/// save the thumbnail of first page into database
bool NabooView::saveThumbnail()
{
    // Note: This function should better be called at the very beginning
    // or end
    if ( cmt_ == 0 )
    {
        QFileInfo file_info( model_->path() );
        cmt_.reset( new ContentThumbnail( file_info.dir().absolutePath() ) );
    }

    if ( cmt_->hasThumbnail( model_->path(), THUMBNAIL_LARGE ) )
    {
        // An issue here, the thumbnail should always be saved
        return true;
    }

    render_client_.disableRepaint();
    AdobeRenderConf & render_conf = render_client_.renderConf();
    AdobeLocationPtr beginning = model()->documentClient()->getBeginning();
    render_conf.setPagePosition( beginning );
    render_conf.setThumbnailSize( cms::thumbnailSize( THUMBNAIL_LARGE ) );
    render_conf.setOperation( RENDER_THUMBNAIL_SAVE );
    render_conf.exec();
    return true;
}

void NabooView::paintBookmark( QPainter & painter )
{
    Range screen_range;
    if (!render_client_.getScreenRange(screen_range))
    {
        return;
    }

    if (model_->hasBookmark(screen_range))
    {
        static QImage image(":/images/bookmark_flag.png");
        QPoint pt(rect().width()- image.width(), 0);
        painter.drawImage(pt, image);
    }
}

void NabooView::displayBookmarks()
{
    QStandardItemModel bookmarks_model;
    model_->getBookmarksModel( bookmarks_model );

    TreeViewDialog bookmarks_view( this );
    bookmarks_view.setModel( &bookmarks_model );
    bookmarks_view.tree().showHeader(true);

    QVector<int> percentages;
    percentages.push_back(80);
    percentages.push_back(20);
    bookmarks_view.tree().setColumnWidth(percentages);
    int ret = bookmarks_view.popup( tr("Bookmarks") );

    // Returned from the TOC view
    onyx::screen::instance().enableUpdate( false );
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate( true );

    if (ret != QDialog::Accepted)
    {
        return;
    }

    QModelIndex index = bookmarks_view.selectedItem();
    if ( !index.isValid() )
    {
        return;
    }

    sys::SysStatus::instance().setSystemBusy(true, false);
    QStandardItem *item = bookmarks_model.itemFromIndex( index );
    AdobeLocationPtr dst = item->data().value<AdobeLocationPtr>();
    gotoPosition( dst );
}

bool NabooView::addBookmark()
{
    if ( render_client_.isLocked() || !render_client_.isValid() ) return false;

    // get the beginning of current screen
    Range screen_range;
    if (render_client_.getScreenRange(screen_range) && model_->addBookmark(screen_range))
    {
        update();
        return true;
    }
    return false;
}

bool NabooView::deleteBookmark()
{
    if (render_client_.isLocked() || !render_client_.isValid()) return false;

    // get the beginning of current screen
    Range screen_range;
    if (render_client_.getScreenRange(screen_range) && model_->deleteBookmark(screen_range))
    {
        update();
        return true;
    }
    return false;
}

void NabooView::startSlideShow()
{
    status_mgr_.setStatus(ID_SLIDE_SHOW, FUNC_SELECTED);
    sys::SysStatus::instance().enableIdle(false);

    // reset the layout and zoom
    if (render_client_.getPagingMode() == PM_SCROLL_PAGES)
    {
        render_client_.switchPagingMode( PM_HARD_PAGES );
    }

    // enter full screen mode
    emit fullScreen(true);
}

void NabooView::stopSlideShow()
{
    slide_show_timer_.stop();
    status_mgr_.setStatus(ID_SLIDE_SHOW, FUNC_NORMAL);
    sys::SysStatus::instance().resetIdle();

    // exit full screen mode
    emit fullScreen(false);
}

bool NabooView::handleHashPasswordRequest(const QString & url)
{
    qDebug("Request hash info for:%s", qPrintable(url));
    SignInDialog dialog(this, QApplication::tr("Open PassHash Protected Document"));
    QString password;
    QString id;
    if (dialog.popup(id, password) != QDialog::Accepted)
    {
        return false;
    }

    password = dialog.password();
    id = dialog.id();
    if (password.isEmpty() || id.isEmpty())
    {
        return false;
    }

    AdobeDRMHandler drm_handler;
    if (drm_handler.runPassHash(url, id, password))
    {
        qDebug("Add PassHash Successfully");
        return true;
    }
    return false;
}

void NabooView::handleError(const QString & error)
{
    static bool need_exit = false;
    if (!need_exit)
    {
        ErrorDialog dialog(error, 0);
        dialog.exec();
        if (!model_->isReady() && !model_->documentClient()->tryingPassword())
        {
            need_exit = true;
            returnToLibrary();
        }
    }
}

bool NabooView::isFullScreenCalculatedByWidgetSize()
{
    if (parentWidget())
    {
        QSize parentSize = parentWidget()->size();
        // TODO find a better way to do this
        if (parentSize.height() == size().height())
        {
            return true;
        }
    }
    return false;
}

}
