#include "stdio.h"
#include "alien-event.h"
#include "alien-error.h"
#include "alien-notify.h"
#include "alien-pointer.h"
#include "alien-screen.h"
#include "alien-request.h"
#include "alien-timer.h"
#include "alien-filesys-data.h"
#include "alien-memory.h"
#include "alien-debug.h"
#include "alien-config.h"
#include "alien-context.h"
#include "preferences.h"
#include "touchscreen.h"
#include "picsel-entrypoint.h"
#include "picsel-config.h"
#include "picsel-config-fileviewer.h"
#include "picsel-agent.h"
#include "picsel-pixelblock.h"
#include "picsel-fileviewer.h"
#include "picsel-pointer.h"
#include "picsel-flowmode.h"
#include "picsel-debug.h"
#include "linux-alien.h"
#include "linux-alien-config-fv.h"
#include "picsel-version.h"
#include "picsel-focus.h"
#include "picsel-app.h"
#include "picsel-version.h"
#include "picsel-language.h"
#include "picsel-locale.h"
#include "picsel-encoding.h"
#include "picsel-search.h"


#include <QApplication>
#include <QDesktopWidget>
#include "onyx_office.h"
#include "onyx/data/configuration.h"
#include "onyx/ui/ui_global.h"
#include "onyx/cms/user_db.h"

namespace onyx {

static Alien_Context context;
static const PicselApp_StartFlags startFlags = PicselApp_ExpandingHeap;
static const QString VIEW_STATE_TAG = "office_doc_view_state";

struct LessByPosition {
    bool operator()(const vbf::Bookmark& a, const vbf::Bookmark& b) const {
        return a.data().value<int>() < b.data().value<int>();
    }
};

static void dumpViewState(PicselControl_ViewListState *state)
{
    printf("dump view state:\n\n");
    printf("full screen %d\n", state->fullScreen);
    printf("rotation %d\n", state->rotation);
    printf("view count %d\n", state->viewCount);
    for(int i = 0; i < state->viewCount; ++i)
    {
        printf("view %d\n", i);
        printf("flow mode %d\n", state->views[i]->flowMode);
        printf("width %d\n", state->views[i]->layoutWidth);
        printf("number %d\n", state->views[i]->pageNumber);
        printf("url %s\n", state->views[i]->url);
        printf("x scroll %d\n", state->views[i]->xScroll);
        printf("y scroll %d\n", state->views[i]->yScroll);
        printf("zoom %d\n", state->views[i]->zoomValue);
    }
    printf("\n\n");
}

static void dumpReadingContext(ReadingContext & context)
{
    printf("dump reading context: \n\n");
    printf("page mode %d\n", context.page_mode_);
    printf("zoom value %f\n", context.zoom_value_);
    printf("font index %d\n", context.font_index_);
    printf("\n\n");
}

QDataStream & operator<< ( QDataStream & out, const PicselControl_ViewListState & state )
{
    out << state.fullScreen;
    out << state.rotation;
    out << state.viewCount;
    for(int i = 0; i < state.viewCount; ++i)
    {
        out << state.views[i]->flowMode;
        out << state.views[i]->layoutWidth;
        out << state.views[i]->pageNumber;
        out << state.views[i]->xScroll;
        out << state.views[i]->yScroll;
        out << state.views[i]->zoomValue;
        QByteArray t(state.views[i]->url);
        out << t;
    }
    return out;
}

QDataStream & operator>> ( QDataStream & in, PicselControl_ViewListState & state )
{
    int d = 0;
    in >> state.fullScreen;
    in >> d; state.rotation = static_cast<PicselRotation>(d);
    in >> state.viewCount;
    state.views = (PicselControl_ViewState **)malloc(sizeof(PicselControl_ViewState *) * state.viewCount);
    for(int i = 0; i < state.viewCount; ++i)
    {
        state.views[i] = (PicselControl_ViewState *)malloc(sizeof(PicselControl_ViewState));
        in >> d; state.views[i]->flowMode = static_cast<PicselFlowMode>(d);
        in >> state.views[i]->layoutWidth;
        in >> state.views[i]->pageNumber;
        in >> state.views[i]->xScroll;
        in >> state.views[i]->yScroll;
        in >> state.views[i]->zoomValue;

        QByteArray t;
        in >> t;
        state.views[i]->url = (char *)malloc(sizeof(char) * t.size());
        strncpy(state.views[i]->url, t.constBegin(), t.size());
    }
    return in;
}


QDataStream & operator<< ( QDataStream & out, const ReadingContext & context )
{
    out << context.font_index_;
    out << context.page_mode_;
    out << context.zoom_value_;
    return out;
}

QDataStream & operator>> ( QDataStream & in, ReadingContext & context )
{
    int d = 0;
    in >> d; context.font_index_ = static_cast<OfficeFontSizeIndex>(d);
    in >> context.page_mode_;
    in >> context.zoom_value_;
    return in;
}

OfficeReader::OfficeReader()
: opened_(false)
, image_(0)
, timer_count_(0)
, job_finished_(false)
, page_repeat_(30)
, pan_limits_(PicselPanLimit_Base)
, search_init_(false)
, broadcast_(true)
, page_mode_process_(false)
, screen_resize_finished_(false)
, zooming_process_(false)
{
    font_dir_ = "/opt/font_kai/2";
    sub_font_dir_ = "";
}

OfficeReader::OfficeReader(OfficeReader & ref)
{
}

OfficeReader::~OfficeReader()
{
}

QByteArray OfficeReader::viewState()
{
    enableBroadcast(false);
    PicselControl_getViewState(context.picselContext);
    view_state_process_ = false;
    backendProcess(view_state_process_, 5000);
    enableBroadcast(true);
    return view_state_;
}

bool OfficeReader::restoreViewState(const QByteArray & data)
{
    if (data.isEmpty())
    {
        qWarning("Try to restore empty view state.");
        return false;
    }
    view_state_ = data;
    PicselControl_ViewListState state;
    QBuffer buffer(&view_state_);
    buffer.open(QIODevice::ReadOnly);
    QDataStream stream(&buffer);
    stream >> state;
    stream >> reading_context_;
    dumpViewState(&state);
    dumpReadingContext(reading_context_);

    PicselControl_setViewState(context.picselContext, &state);
    return true; 
}

/// Restore view state by using reading context.
/// In screen rotation, the view port does not work, so we
/// have to maintain state in reading context. 
bool OfficeReader::restoreViewState()
{
    if (reading_context_.page_mode_)
    {
        setZoomRatio(reading_context_.zoom_value_); 
    }
    else
    {
        setFontSize(reading_context_.font_index_);
    }
    return true;
}

// initialize or re-initialize backend library.
bool OfficeReader::initialize(const QSize &size)
{
    resize(size);
    context.inputType = InputType_Command;

    // threading.
    context.mainThread = AlienThread_create(&context, NULL, NULL, 0);
    context.currentThread = context.mainThread;
    context.useExpandingHeap = (startFlags & PicselApp_ExpandingHeap) != 0;
    context.initFn = Picsel_EntryPoint_FileViewer();
    context.picselInitComplete = 1;
    return true;
}

bool OfficeReader::open(const QString &path, int ms)
{
    if (path.isEmpty())
    {
        return false;
    }
    current_file_ = path;

    // TODO: Need to make sure the file command will be release correctly.
    PicselFileCommand* command = new PicselFileCommand;
    Picsel_ThreadModelFn threadFn;

    threadFn = Picsel_ThreadModel_alienThreads();
    context.currentFile = strdup(current_file_.toLocal8Bit().constData());

    command->fileContents   = strdup(current_file_.toLocal8Bit().constData());
    command->fileLength       = 0;
    command->fileExtension[0] = '\0';
    PicselApp_start(&context, command, context.initFn, threadFn, startFlags);
    backendProcess(opened_, ms);
    if (!isOpened())
    {
        qDebug("Could not open document.");
        return false;
    }
    emit documentOpened();
    return true;
}

bool OfficeReader::isOpened()
{
    return opened_;
}

const QString & OfficeReader::path()
{
    return current_file_;
}

bool OfficeReader::close()
{
    PicselApp_shutDown(context.picselContext);
    opened_ = false;
    return true;
}

QImage & OfficeReader::image()
{
    // If crashed, check caller.
    return *image_;
}

// An universal way to call office reader.
// John: use switch case now, also separate
bool OfficeReader::doAction(const OfficeAction action, QVariant parameter)
{
    switch (action)
    {
    case INCREASE_FONT_SIZE:
        {
            return increaseFontSize();
        }
    case DECREASE_FONT_SIZE:
        {
            return decreaseFontSize();
        }
    case SET_FONT_SIZE:
        {
            return setFontSize(static_cast<OfficeFontSizeIndex>(parameter.toInt()));
        }
    case SET_FONT_FAMILY:
        {
            return false;
        }
    case NEXT_PAGE:
        {
            return nextPage();
        }
    case PREV_PAGE:
        {
            return prevPage();
        }
    case GOTO_PAGE:
        {
            return gotoPage(parameter.toInt());
        }
    case FIT_TO_PAGE:
        {
            return fitToPage();
        }
    case FIT_TO_WIDTH:
        {
            return fitToWidth();
        }
    case FIT_TO_HEIGHT:
        {
            return fitToHeight();
        }
    case ZOOM_IN:
        {
            return zoomIn();
        }
    case ZOOM_OUT:
        {
            return zoomOut();
        }
    case SET_ZOOM_RATIO:
        {
            return setZoomRatio(parameter.toDouble());
        }
    case TO_REFLOW_MODE:
        {
            return toReflowMode();
        }
    case TO_PAGE_MODE:
        {
            return toPageMode();
        }
    case PAN:
        {
            return pan(parameter.toSize());
        }
    case OPEN_LINK:
        {
            return openLink(parameter.toPoint());
        }
    case SMART_PAN_LEFT:
        {
            return smartPanLeft();
        }
    case SMART_PAN_RIGHT:
        {
            return smartPanRight();
        }
    case PAN_LEFT:
        {
            return panLeft();
        }
    case PAN_RIGHT:
        {
            return panRight();
        }
    case SCROLL_UP:
        {
            return scrollUp();
        }
    case SCROLL_DOWN:
        {
            return scrollDown();
        }
    case SEARCH_NEXT:
        {
            return searchForward();
        }
    case SEARCH_PREV:
        {
            return searchBack();
        }
    case CANCEL_SEARCH:
        {
            return cancelSearch();
        }
    case RESIZE:
        {
            return resize(parameter.toSize());
        }
    case ADD_BOOKMARK:
        {
            return addBookmark(currentPage());
        }
    case DELETE_BOOKMARK:
        {
            return deleteBookmark(currentPage());
        }
    case INVALID_ACTION:
        {
            return false;
        }
    default:
        return false;
    }
    return true;
}

OfficeFontSizeIndex OfficeReader::fontSize()
{
    return reading_context_.font_index_;
}

int OfficeReader::currentPage()
{
    return current_page_;
}

int OfficeReader::totalPage()
{
    return total_page_;
}

double OfficeReader::zoomRatio()
{
    return reading_context().zoom_value_;
}

unsigned int OfficeReader::panLimits()
{
    return pan_limits_;
}

void OfficeReader::setPageRepeat(const int repeat)
{
    page_repeat_ = repeat;
}

int OfficeReader::pageRepeat()
{
    return page_repeat_;
}

void OfficeReader::panLimits(bool &up, bool &down, bool &left, bool &right)
{
    up = ((pan_limits_ - PicselPanLimit_Base) & PicselPanLimit_Top);
    down  = ((pan_limits_ - PicselPanLimit_Base) & PicselPanLimit_Bottom);
    left  = ((pan_limits_ - PicselPanLimit_Base) & PicselPanLimit_Left);
    right = ((pan_limits_ - PicselPanLimit_Base) & PicselPanLimit_Right);
}

/// return true if there is any limit.
bool OfficeReader::allLimits()
{
    return true;
}

bool OfficeReader::leftLimit()
{
    bool up, down, left, right;
    panLimits(up, down, left, right);
    return left;
}

bool OfficeReader::rightLimit()
{
    bool up, down, left, right;
    panLimits(up, down, left, right);
    return right;
}

bool OfficeReader::topLimit()
{
    bool up, down, left, right;
    panLimits(up, down, left, right);
    return up;
}

bool OfficeReader::bottomLimit()
{
    bool up, down, left, right;
    panLimits(up, down, left, right);
    return down;
}

bool OfficeReader::nextPage()
{
    PicselFileviewer_turnPage(context.picselContext, PicselPageTurn_Next);
    return true;
}

bool OfficeReader::prevPage()
{
    bool ret =false;
    ret = PicselFileviewer_turnPage(context.picselContext,
                                    PicselPageTurn_Previous) == 1;
    return ret;
}

bool OfficeReader::gotoPage(const int page)
{
    PicselFileviewer_gotoPage(context.picselContext, page);
    return true;
}

bool OfficeReader::zoomIn()
{
    if (reading_context().zoom_value_ < 975.0f)
    {
        return setZoomRatio(reading_context().zoom_value_ + 25.0f);
    }
    return false;
}

bool OfficeReader::zoomOut()
{
    if (reading_context().zoom_value_ > 30.0f)
    {
        return setZoomRatio(reading_context().zoom_value_ - 25.0f);
    }
    return false;
}

bool OfficeReader::fitToPage()
{
    enableBroadcast(false);
    if (isInReflowMode())
    {
        toPageMode();
    }
    enableBroadcast(true);
    bool ret = PicselApp_fitDocument(context.picselContext, PicselFitPage);
    return ret;
}

bool OfficeReader::fitToWidth()
{
    enableBroadcast(false);
    if (isInReflowMode())
    {
        toPageMode();
    }
    enableBroadcast(true);
    bool ret = PicselApp_fitDocument(context.picselContext, PicselFitWidth);
    return ret;
}

bool OfficeReader::fitToHeight()
{
    enableBroadcast(false);
    if (isInReflowMode())
    {
        toPageMode();
    }
    enableBroadcast(true);
    bool ret = PicselApp_fitDocument(context.picselContext, PicselFitHeight);
    return ret;
}

bool OfficeReader::pan(const QSize & offset)
{
    PicselControl_pan(context.picselContext, offset.width(), offset.height(), PicselControl_Start);
    PicselControl_pan(context.picselContext, offset.width(), offset.height(), PicselControl_End);
    return true;
}

/// pan left is possible. if it reaches the limit, scroll up.
bool OfficeReader::smartPanLeft()
{
    if (!leftLimit())
    {
    return pan(QSize(screen_size_.width() - pageRepeat(), 0));
    }
    return scrollDown();
}

bool OfficeReader::smartPanRight()
{
    if (!rightLimit())
    {
    return pan(QSize(-screen_size_.width() + pageRepeat(), 0));
    }
    return scrollUp();
}

bool OfficeReader::panLeft()
{
    return pan(QSize(screen_size_.width() - pageRepeat(), 0));
}

bool OfficeReader::panRight()
{
    return pan(QSize(-screen_size_.width() + pageRepeat(), 0));
 }

bool OfficeReader::scrollUp()
{
    if ( !topLimit())
    {
        return pan(QSize(0, screen_size_.height() - pageRepeat()));
    }
    else
    {
        bool ret =false;
        if (currentPage() != 1)
        {
            enableBroadcast(false);
            ret = prevPage();
            ret = PicselControl_panPosition(context.picselContext, PicselPan_BottomLeft)==1;
            enableBroadcast(true);
        }
        return ret;
    }
}

bool OfficeReader::scrollDown()
{
    if(!bottomLimit())
    {
        return pan(QSize(0, -screen_size_.height() + pageRepeat()));
    }
    else
    {
        return nextPage();
    }
}

bool OfficeReader::isInPageMode()
{
    return reading_context_.page_mode_;
}

bool OfficeReader::isInReflowMode()
{
    return !reading_context_.page_mode_;
}

bool OfficeReader::increaseFontSize()
{
    OfficeFontSizeIndex index = reading_context_.font_index_;
    switch(reading_context_.font_index_)
    {
    case Minimum:
        index = Small;
        break;
    case Small:
        index = Medium;
        break;
    case Medium:
        index = Large;
        break;
    case Large:
        index = Maximum;
        break;
    case Maximum:
        index = Maximum;
        break;
    default:
        index = Maximum;
    }
    return setFontSize(index);
}

bool OfficeReader::decreaseFontSize()
{
    OfficeFontSizeIndex index = reading_context_.font_index_;
    switch(reading_context_.font_index_)
    {
    case Maximum:
        index = Large;
        break;
    case Large:
        index = Medium;
        break;
    case Medium:
        index = Small;
        break;
    case Small:
        index = Minimum;
        break;
    case Minimum:
        index = Minimum;
        break;
    default:
        index = Medium;
    }
    return setFontSize(index);
}

void OfficeReader::onPageChanged(int current, int total)
{
    current_page_ = current;
    total_page_ = total;
}

void OfficeReader::updateImage(void               *buffer,
                               unsigned int        width,
                               unsigned int        height,
                               unsigned int        widthBytes,
                               unsigned int        updateX,
                               unsigned int        updateY,
                               unsigned int        updateWidth,
                               unsigned int        updateHeight,
                               unsigned int        xTopLeft,
                               unsigned int        yTopLeft) {
    resetImage(QSize(width, height));
    if (isOpened())
    {
        printf("going to copy data.\n");
        Picsel_PixelBlock_copy_16bpp((const short unsigned int*) buffer,
                                    width,
                                    height,
                                    widthBytes,
                                    (ushort*) image_->bits(),
                                    widthBytes,
                                    updateX,
                                    updateY,
                                    updateWidth,
                                    updateHeight,
                                    context.rotation);
        printf("copy data done.\n");
    }
}

static void dumpState(PicselControl_ViewListState *state)
{
    printf("full screen %d\n", state->fullScreen);
    printf("rotation %d\n", state->rotation);
    printf("view count %d\n", state->viewCount);
    for(int i = 0; i < state->viewCount; ++i)
    {
        printf("view %d\n", i);
        printf("flow mode %d\n", state->views[i]->flowMode);
        printf("width %d\n", state->views[i]->layoutWidth);
        printf("number %d\n", state->views[i]->pageNumber);
        printf("url %s\n", state->views[i]->url);
        printf("x scroll %d\n", state->views[i]->xScroll);
        printf("y scroll %d\n", state->views[i]->yScroll);
        printf("zoom %d\n", state->views[i]->zoomValue);
    }
}

/// When there is no enough memory, this callback function is invoked.
void OfficeReader::onInsufficientMemory()
{
    emit insufficientMemory();
}

/// Receive information event form picsel library. we can broadcast some signal here.
void OfficeReader::informationEvent(int index, void *eventData)
{
    AlienInformation_Event  event = static_cast<AlienInformation_Event>(index);
    //  const char *str = PicselDebug_getInformationEvent (event, eventData);
    switch (event)
    {
    case AlienInformation_DocumentLoaded:
        {
            AlienInformation_DocumentLoadedInfo *info;
            info = (AlienInformation_DocumentLoadedInfo *) eventData;
            LinuxAlien_displayResourceUsage();

            if (info->status == PicselLoadedStatus_FullyLoaded)
            {
                opened_ = true;
                page_mode_process_ = true;
                zooming_process_ = true;
            }
        }
        break;

    case AlienInformation_GetViewStateResult:
        {
            AlienInformation_ViewStateInfo *info = static_cast<AlienInformation_ViewStateInfo *>(eventData);
            PicselControl_ViewListState * state = info->state;
            dumpState(state);
            onViewStateReady(state);
            view_state_process_ = true;
        }
        break;
    case AlienInformation_SearchResult:
        {
            AlienInformation_SearchResultInfo *search_info;
            search_info = (AlienInformation_SearchResultInfo *) eventData;
            qDebug("search result %d", search_info->result);
            if (search_info->result == PicselSearch_Found)
            {
                printf("AlienInformation_SearchResult\n");
                PicselSearch_snapToResult (context.picselContext);
            }
            else if (search_info->result == PicselSearch_NotFound ||
                     search_info->result == PicselSearch_EndOfDocument ||
                     search_info->result == PicselSearch_Error)
            {
                emit searchStateChanged(NotFound);
            }
        }
        break;
    case AlienInformation_SearchListResult:
        {
            AlienInformation_SearchListResultInfo *search_list_info;
            search_list_info = (AlienInformation_SearchListResultInfo *) eventData;
            if (search_list_info->result == PicselSearch_Found)
            {
                //printf("AlienInformation_SearchListResult\n");
                PicselSearch_snapToResultId (context.picselContext, search_list_info->id);
            }
        }
        break;
    case AlienInformation_FileInfoResult:
        {
            AlienInformation_FileInfo *fileinfo = (AlienInformation_FileInfo*) eventData;
            // Update metadata.
            printf("Title : %s\n", fileinfo->title);
            printf("URL : %s\n", fileinfo->url);
            printf("MIME : %s\n", fileinfo->mime);

            title_ = QString::fromUtf8(reinterpret_cast<char *>(fileinfo->title));
        }
        break;
    case AlienInformation_Pan :
        {
            AlienInformation_PanInfo *paninfo = (AlienInformation_PanInfo*) eventData;
            pan_pos_ = QPoint(paninfo->x, paninfo->y );
        }
        break;
    case AlienInformation_Zoom:
        {
            zooming_process_ = true;
            AlienInformation_ZoomInfo *zoomData = (AlienInformation_ZoomInfo *) eventData;
            reading_context().zoom_value_ = (zoomData->zoom / 655.360);
            qDebug() << "Update zoom info "<< reading_context().zoom_value_;
        }
        break;
    case AlienInformation_ScreenResized:
        {
            screen_resize_finished_ = true;
        }
        break;
    case AlienInformation_InitComplete:
        {
            //AlienDebug_output ("Init complete.\n");
            //BUG have to place a AlienDebug_output here, otherwise it would tear
            AlienDebug_output("");
            LinuxAlien_displayResourceUsage();
            LinuxAlien_resetResourceUsage();
        }
        break;

    case AlienInformation_splashScreenDone:
        {
            emit splashScreenDone();
        }
        break;

    case AlienInformation_FlowMode:
        {
            AlienInformation_FlowModeInfo *resultInfo;
            resultInfo = (AlienInformation_FlowModeInfo *) eventData;
            //set flow mode by picsel
            qDebug() << "resultInfo " << resultInfo->result;
            context.flowMode = resultInfo->flowMode;
            context.changingFlowMode = 0;
        }
        break;

    case AlienInformation_FocusInformation:
        {
            PicselFocus_Information *focusInfo;
            focusInfo = (PicselFocus_Information *) eventData;
            if (focusInfo->targetUrl != NULL)
            {
                printf("targetUrl = %s\n", focusInfo->targetUrl);
            }
            break;
        }
    case AlienInformation_FocusResult:
        {
            PicselFocus_Result *focusResult;
            focusResult = (PicselFocus_Result *) eventData;

            printf("Focus result: error = %d", focusResult->failure);

            if (focusResult->failure == 0)
            {
                /* retrieve information about the currently focussed link */
                PicselFocus_getInformation(context.picselContext);
            }
        }
        break;
    case AlienInformation_RequestShutdown:
        {
            printf("Picsel requested shutdown\n");
        }
        break;
    case AlienInformation_SetPointerThreshold:
        {
            AlienInformation_SetPointerThresholdInfo *data = (AlienInformation_SetPointerThresholdInfo*) eventData;
            context.pointerSizeThreshold = data->threshold;
        }
        break;
    case AlienInformation_FileProgressResult:
        {
        }
        break;
    case AlienInformation_ZoomLimitReached:
        {
        }
        break;
    case AlienInformation_PanLimitsReached:
        {
            //use for paint pan direction flags
            AlienInformation_PanLimitsReachedInfo *pan_info =(AlienInformation_PanLimitsReachedInfo*) eventData;
            pan_limits_ = pan_info->panLimits;
            qDebug()<<"pan_limits_"<<pan_limits_;
        }
        break;
    case AlienInformation_ThumbnailDone:
        break;
    default:
        break;
    }
}


void OfficeReader::reportError(int, void *)
{
}

void OfficeReader::userRequest(void *d)
{
    int ret = 0;
    PicselUserRequest_Request  *request = static_cast<PicselUserRequest_Request *>(d);
    AlienUserRequest_Request *alien_request = new AlienUserRequest_Request;
    alien_request->ac = &context;
    alien_request->picselRequest = request;

    switch (request->type)
    {
    case PicselUserRequest_Type_Transition:
        {
            if (request->requestData->transition.type != PicselUserRequest_Transition_Type_Popup) {
                request->requestData->transition.response = PicselUserRequest_Transition_Accept;

                PicselUserRequest_notify(alien_request->ac->picselContext,
                    alien_request->picselRequest);
                ret = 1;
            }
        }
        break;
    case PicselUserRequest_Type_Document_Password:
        {
            QString password;
            emit passwordRequest(password);
            if (!password.isEmpty())
            {
                alien_request->picselRequest->requestData->password.password = password.toLocal8Bit().data();
                alien_request->picselRequest->result = PicselUserRequest_Result_Accepted;
            }
            else
            {
                alien_request->picselRequest->requestData->password.password = NULL;
                alien_request->picselRequest->result = PicselUserRequest_Result_Rejected;
            }

            PicselUserRequest_notify(alien_request->ac->picselContext, alien_request->picselRequest);
            ret = 1;
        }
        break;

    default:
        printf("AlienUserRequest_request type %s not handled.",
            PicselDebug_getUserRequestType(request->type));
    }

    request->result = PicselUserRequest_Result_Accepted;
}

void OfficeReader::timerRequest(unsigned long  *reference, unsigned long ms)
{
    ++timer_count_;
    *reference = TIMER_ID;
    QTimer::singleShot(ms, this, SLOT(onRequestTimeout()));
}

void OfficeReader::onRequestTimeout()
{
    if (PicselApp_timerExpiry(context.picselContext, TIMER_ID))
        --timer_count_;

    if (timer_count_ <= 0)
    {
        timer_count_ = 0;
        job_finished_ = true;
        if (ableToBroadcast())
        {
            QTimer::singleShot(0, this, SLOT(broadcastJobFinish()));
        }
    }
}

void OfficeReader::broadcastJobFinish()
{
    if (timer_count_ <= 0)
    {
        emit jobFinished();
        qDebug("job finished.\n");
    }
}

void OfficeReader::cancelTimer(unsigned long  *reference) {
    // We need to cancel timer, decrease timer count
    qWarning("Timer is canceled %ld.", *reference);
    //--timer_count_;
}

void OfficeReader::configReady()
{
    Fv_AlienConfig_ready(&context);
    PicselApp_setKeyBehaviour(context.picselContext, 1, 0);
    PicselApp_setTimeSlice(context.picselContext, 0, timeSlice());
    Preferences_read(context.picselContext, context.preferences);
    PicselConfig_setString(context.picselContext, PicselConfig_fontsDirectory, getFontsDir());

    Picsel_Encoding_Register_zhcn(context.picselContext);
    Picsel_Encoding_Register_zhtw(context.picselContext);
    Picsel_Encoding_Register_jajp(context.picselContext);
    Picsel_Encoding_Register_kokr(context.picselContext);

    Picsel_PdfLanguage_Register_jajp(context.picselContext);
    Picsel_PdfLanguage_Register_zhtw(context.picselContext);
    Picsel_PdfLanguage_Register_zhcn(context.picselContext);
    Picsel_PdfLanguage_Register_kokr(context.picselContext);

    Picsel_Font_Register_zhtw(context.picselContext);
    Picsel_Font_Register_zhcn(context.picselContext);
    Picsel_Font_Register_jajp(context.picselContext);
    Picsel_Font_Register_kokr(context.picselContext);
    Picsel_Font_Register_arar(context.picselContext);
    Picsel_Font_Register_bnin(context.picselContext);
    Picsel_Font_Register_latin(context.picselContext);
//     qDebug() << "PicselConfig_fontSize "<< getFontSize();
//     PicselConfig_setInt(context.picselContext,
//                         PicselConfig_fontSize,  getFontSize() * (1<<16));
//     qDebug() << "PicselConfig_flowMode "<< getFlowMode();
//     PicselConfig_setInt(context.picselContext,
//                         PicselConfig_flowMode, getFlowMode());
//     qDebug() << "PicselConfigFV_initialZoom" << zoomRatio() ;
//     PicselConfig_setInt (context.picselContext,
//                          PicselConfigFV_initialZoom, zoomRatio() *(1<<16));
//
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_maxImageSize, 4 * 1024 * 1024);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_useHqCacheZoom, 0);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_applyFitOnPageChange, 0);
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_readInLargeBlocks, 1);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_adaptiveRedraw, PicselConfigFV_RedrawMode_AdaptiveHigh);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_RedrawMode, PicselConfigFV_RedrawMode_AdaptiveHigh);

    // To improve image quality.
    PicselConfig_setInt(context.picselContext, PicselConfigFV_StaticSubsampleThresholdOverride, 800);
//     PicselConfig_setInt(context.picselContext,
//                         PicselConfig_thumbnailsMax, -1);
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_leftAlignLargeDocs, 0);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_panToPageEnable, 0);
//     PicselConfig_setInt(context.picselContext,
//                         PicselConfig_thumbnailsMaxScaled, -1);
    PicselConfig_setInt (context.picselContext,
                         PicselConfigFV_pageFitDefault, PicselConfigFV_PageFit_None/*PicselConfigFV_PageFit_Screen*/ );
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_panBorderWidth, 0);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_panToPageLeftRight, 0);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_dispmanBufferSize, 150 * 1024);
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_searchBorderColour, 0xffffffff);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_singlePageLoad, 0);
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_searchFillColour, 0x80808080);
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_searchBorderWidth, 1 << 10);
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_searchListBorderColour, 0x000000ff);
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_searchListFillColour, 0x10101010);
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_searchListBorderWidth, 1 << 10);
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_flowModeKeepZoom, 1);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_zoomAfterResize, 0);
    PicselConfig_setInt(context.picselContext,
                        PicselConfig_startFullscreen, 1);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_zoomNotify , 1);
    PicselConfig_setInt(context.picselContext,
                        PicselConfigFV_bgColour, 0xffffffff);
 }

void OfficeReader::screenConfiguration(void *d)
{
    AlienScreenConfiguration *config = static_cast<AlienScreenConfiguration *>(d);
    config->landscape = 0;
    config->xTopLeft = 0;
    config->yTopLeft = 0;
    config->picselScreenWidth    = context.overrideScreenWidth;
    config->picselScreenHeight   = context.overrideScreenHeight;
    config->physicalScreenWidth  = context.overrideScreenWidth;
    config->physicalScreenHeight = context.overrideScreenHeight;
    config->maxNonRotateWidth    = context.overrideScreenWidth;
    config->maxNonRotateHeight   = context.overrideScreenHeight;
    config->format    = PicselScreenFormat_b5g6r5;
    config->alignment = BufferAlignmentDefault;
}

int OfficeReader::timeSlice()
{
    return 250;
}

/// Process events. When cond changes from false to true or specified timer expired
/// this function returns. it's helpful when you are not sure a pending request has been
/// processed or not.
bool OfficeReader::backendProcess(bool &cond, int ms)
{
    QTime t;
    t.start();
    while (t.elapsed() <= ms && !cond)
    {
        QApplication::processEvents();
    }
    return (t.elapsed() > ms || cond);
}

const char * OfficeReader::getFontsDir()
{
    return (font_dir_ + sub_font_dir_).toLocal8Bit().constData();
}

OfficeFontSizeIndex OfficeReader::getFontSize()
{
    return reading_context().font_index_;
}

bool OfficeReader::setFontSize(OfficeFontSizeIndex index)
{
    reading_context_.font_index_ = index;
    unsigned int f;
    double d = 100;
    switch (index)
    {
    case Maximum:
        d = 300;
        break;
    case Large:
        d = 250;
        break;
    case Medium:
        d = 200;
        break;
    case Small:
        d = 150;
        break;
    case Minimum:
        d = 100;
        break;
    default:
        d = 100;
        break;
    }

    reading_context_.zoom_value_ = d;
    f = (unsigned int) qRound((d / 25.0) * (1 << 14));
    enableBroadcast(false);
    zooming_process_ = false;
    PicselControl_setZoom(context.picselContext, f, screen_size_.width() / 2, screen_size_.height() / 2);
    backendProcess(zooming_process_, 100 * 1000);
    enableBroadcast(true);

    toReflowMode();
    return true;
}

const int OfficeReader::getFlowMode( )
{
    return  context.flowMode;
}

bool OfficeReader::setZoomRatio(double zoom_ratio)
{
    enableBroadcast(false);
    if (!isInPageMode())
    {
        toPageMode();
    }
    enableBroadcast(true);

    unsigned int f;
    reading_context_.zoom_value_ = zoom_ratio;
    f = (unsigned int) qRound((zoom_ratio / 25.0) * (1 << 14));
    return PicselControl_setZoom(context.picselContext, f, screen_size_.width() / 2, screen_size_.height() / 2);
}


bool OfficeReader::resize(const QSize &size)
{
    resetImage(size);
    resizeBackendScreen(size);
    return true;
}

void OfficeReader::resetImage(const QSize &size)
{
    if (image_ && image_->size() == size)
    {
        return;
    }
    delete image_;
    image_ = new QImage(size, QImage::Format_RGB16);
}

void OfficeReader::resizeBackendScreen(const QSize & size)
{
    screen_size_ = size;
    context.overrideScreenWidth = screen_size_.width();
    context.overrideScreenHeight = screen_size_.height();
    context.rotation = PicselRotation0;
    enableBroadcast(false);
    screen_resize_finished_ = false;
    PicselScreen_resize(context.picselContext,  context.overrideScreenWidth
                              , context.overrideScreenHeight, 0, 0);
    backendProcess(screen_resize_finished_, 10 * 1000);
    enableBroadcast(true);
}

bool OfficeReader::searchForward()
{
    if (search_context_.pattern().isEmpty())
    {
        return false;
    }
    return PicselSearch_forward(context.picselContext) == 1;
}

bool OfficeReader::searchBack()
{
    if (search_context_.pattern().isEmpty())
    {
        return false;
    }
    return PicselSearch_back(context.picselContext) == 1;
}

bool OfficeReader::cancelSearch()
{
    search_init_ = false;
    return  PicselSearch_cancel(context.picselContext) == 1;
}

void OfficeReader::setFontfamily(QString& string)
{
    //TODO no way for now :-(
    sub_font_dir_ = string;
    //PicselApp_shutDown (context.picselContext);
    PicselConfig_setString(context.picselContext, PicselConfig_fontsDirectory, getFontsDir());
//
}

/// Notice: During page mode switch, we need to check it has been successfully changed or not,
/// otheriwse, zooming may not work correctly. so use page_mode_process_ and process event.
bool OfficeReader::toPageMode()
{
    int ret = 0;
    context.changingFlowMode = 1;
    ret = PicselFlowMode_set(context.picselContext, FlowMode_Normal);
    reading_context_.page_mode_ = true;
    page_mode_process_ = false;
    backendProcess(page_mode_process_, 10 * 1000);
    return ret;
}

bool OfficeReader::toReflowMode()
{
    int ret = 0;
    context.changingFlowMode = 1;
    ret = PicselFlowMode_set(context.picselContext, FlowMode_FitScreenWidth);
    reading_context_.page_mode_ = false;
    page_mode_process_ = false;
    backendProcess(page_mode_process_, 5 * 1000);
    return ret;
}

void *OfficeReader::backendContext()
{
    return context.picselContext;
}

bool OfficeReader::initSearch()
{
    if (search_init_)
    {
        return false;
    }
    if (search_context_.pattern().isEmpty())
    {
        return false;
    }
    if (PicselSearch_start(context.picselContext,
            (unsigned char*) qstrdup(search_context_.pattern().toLocal8Bit().constData()),
            (search_context_.case_sensitive() ? 1 : 0), 6, 72))
    {
        search_init_ = true;
        return search_init_;
    }
    return false;
}

void OfficeReader::search(BaseSearchContext& sc)
{
    initSearch();
    if (search_context_.forward()) {
        searchForward();
    } else {
        searchBack();
    }
}

BaseSearchContext & OfficeReader::searchContext()
{
    return search_context_;
}

vbf::Bookmarks & OfficeReader::bookmarks()
{
    return bookmarks_;
}

bool OfficeReader::hasBookmark(int page)
{
    vbf::BookmarksIter begin = bookmarks_.begin();
    vbf::BookmarksIter end   = bookmarks_.end();
    for (vbf::BookmarksIter iter  = begin; iter != end; ++iter)
    {
        if (iter->data().toInt() == page)
        {
            return true;
        }
    }
    return false;
}

// Use lazy load if possible.
bool OfficeReader::loadBookmarks()
{
    return true;
}

bool OfficeReader::saveBookmarks()
{
    return true;
}

bool OfficeReader::addBookmark(int page)
{
    QString title(tr("Bookmark %1"));
    title = title.arg(bookmarks_.size() + 1);
    return vbf::insertBookmark(bookmarks_, vbf::Bookmark(title,  page), LessByPosition());
}

bool OfficeReader::deleteBookmark(int page)
{
    vbf::BookmarksIter begin = bookmarks_.begin();
    vbf::BookmarksIter end   = bookmarks_.end();
    for (vbf::BookmarksIter iter  = begin; iter != end; ++iter)
    {
        if (iter->data().toInt() == page)
        {
            bookmarks_.erase(iter);
            return true;
        }
    }
    return false;
}

int OfficeReader::xScroll()
{
    return  pan_pos_.x();
}

int OfficeReader::yScroll()
{
    return pan_pos_.y();
}

const bool OfficeReader::ableToBroadcast()
{
    return broadcast_;
}

void OfficeReader::enableBroadcast(const bool broadcast)
{
    broadcast_ = broadcast;
}

void OfficeReader::onViewStateReady(void *data)
{
    PicselControl_ViewListState *state = static_cast<PicselControl_ViewListState *>(data);

    // Sync between state and reading context.
    if (state->viewCount > 0)
    {
        if (state->views[0]->flowMode == FlowMode_Normal)
        {
            reading_context_.page_mode_ = true;
        }
        else
        {
            reading_context_.page_mode_ = false;
        }
    }
    dumpViewState(state);
    dumpReadingContext(reading_context_);

    QBuffer buffer(&view_state_);
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);
    stream << *state;
    stream << reading_context_;
    printf("saved to viewport now.");
}

QString OfficeReader::title()
{
    return title_;
}

void OfficeReader::setPanPos(const QPoint & pt)
{
    PicselControl_setPan(context.picselContext, pt.x(), pt.y());
    QTime t;
    t.start();
    while (t.elapsed() <= 500 ) {
        QApplication::processEvents();
    }
}

bool OfficeReader::openLink(const QPoint &pt)
{
    PicselFocus_navigateScreen(context.picselContext, PicselFocus_Navigation_FocusAtPoint, pt.x(), pt.y());
    return true;
}

bool OfficeReader::waitForJobFinished(int ms)
{
    enableBroadcast(false);
    job_finished_ = false;
    backendProcess(job_finished_, ms);
    enableBroadcast(true);
    return true;
}

};
