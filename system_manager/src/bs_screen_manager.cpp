#include <QtGui/QtGui>
#include <algorithm>
#include <math.h>
#ifdef ENABLE_EINK_SCREEN
#include "bs_cmd.h"
#include <cstdio>
#include <QtGui/qscreen_qws.h>
#include <QtGui/qwsdisplay_qws.h>
#endif

#include "bs_screen_manager.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys_utils.h"

using namespace std;
using namespace onyx::screen;

#ifdef ENABLE_EINK_SCREEN
// I still keep the following code here, as it's more flexible.
static int default_waveform = WAVEFORM_8_MODE;
#endif
std::vector<unsigned char> img;

static QVector<QRgb> color_table;

/// Screen mananger is designed to help applications to:
/// - Copy data from Qt framebuffer.
/// - Update the screen by specified waveform.
/// There are some good reasons that we move the data copying from
/// Qt screen driver to application level:
/// - More easy to deploy. Don't need to hack the Qt transform driver.
/// - More robust.
/// - More flexible, application can control everything. It's even possible
/// for application to maintain several buffer blocks.
/// - Better performance. Don't need to copy all changes to controller especially
/// window switching.
BSScreenManager::BSScreenManager()
: busy_(false)
, busy_index_(0)
, screen_saver_index_(-1)
, sleeping_(true)
, enable_(true)
, data_(0)
, screen_width_(600)
, screen_height_(800)
{
    reset();
#ifdef ENABLE_EINK_SCREEN
    bs_cmd_flag_hif_mode_cmd();
    ensureRunning();

    screen_width_ = qgetenv("SCREEN_WIDTH").toInt();
    if (screen_width_ < 0)
    {
        screen_width_ = 600;
    }

    screen_height_ = qgetenv("SCREEN_HEIGHT").toInt();
    if (screen_height_ < 0)
    {
        screen_height_ = 800;
    }

    // Check gray scale at first, by default, we use the 8 gray level waveform.
    default_waveform = qgetenv("WAVEFORM").toInt();
    if (default_waveform < 0)
    {
        default_waveform = WAVEFORM_8_MODE;
    }

    // Report temp.
    printf("temp %d\n\n", bs_cmd_rd_temp());
#endif
    start();

    // setup connection.
    busy_timer_.setInterval(1200);
    connect(&busy_timer_, SIGNAL(timeout()), this, SLOT(onBusyTimeout()));
}

BSScreenManager::~BSScreenManager()
{
    stop();
}

void BSScreenManager::enableUpdate(bool enable)
{
    enable_ = enable;
}

void BSScreenManager::start()
{
#ifdef BUILD_FOR_ARM
    data_ = QScreen::instance()->base();
#endif
    socket_.bind(QHostAddress(QHostAddress::LocalHost), PORT);
    connect(&socket_, SIGNAL(readyRead()), this, SLOT(onReadyRead()), Qt::QueuedConnection);
}

void BSScreenManager::stop()
{
    qDebug("broadsheet stop.");
#ifdef BUILD_FOR_ARM
    data_ = 0;
#endif
}

bool BSScreenManager::setGrayScale(int colors)
{
    QString parameter;
#ifdef BUILD_FOR_ARM
    if ((colors == 16 && default_waveform == WAVEFORM_16_MODE) ||
        (colors == 8 && default_waveform == WAVEFORM_8_MODE))
    {
        return false;
    }

    if (colors == 16)
    {
        parameter.setNum(colors);
        default_waveform = WAVEFORM_16_MODE;
    }
    else
    {
        parameter.setNum(8);
        default_waveform = WAVEFORM_8_MODE;
    }
#endif
    QStringList args;
    args << parameter;
    sys::runScriptBlock("update_waveform.sh", args, 10 * 1000);

    // Reset controller.
    reset();
    return true;
}

int BSScreenManager::grayScale()
{
#ifdef BUILD_FOR_ARM
    if (default_waveform == WAVEFORM_16_MODE)
    {
        return 16;
    }
#endif
    return 8;
}

QVector<QRgb> & BSScreenManager::colorTable()
{
    if (color_table.size() <= 0)
    {
        for(int i = 0; i < 256; ++i)
        {
            color_table.push_back(qRgb(i, i, i));
        }
    }
    return color_table;
}

void BSScreenManager::setBusy(bool busy, bool show_indicator)
{
    busy_ = busy;
    busy_index_ = 0;
    busy_canvas_.reset(0);
    if (busy_ && show_indicator)
    {
        onBusyTimeout();
        busy_timer_.start();
    }
    else
    {
        busy_timer_.stop();
    }
}

void BSScreenManager::fillScreen(unsigned char color)
{
    ensureRunning();
    ScreenCommand command;
    command.type = ScreenCommand::FILL_SCREEN;
    command.color = color;
    command.update_flags = ScreenCommand::FULL_UPDATE;
    command.waveform = onyx::screen::ScreenProxy::GC;
    command.wait_flags = ScreenCommand::WAIT_BEFORE_UPDATE;
    fillScreen(command);
}

void BSScreenManager::refreshScreen(onyx::screen::ScreenProxy::Waveform waveform)
{
    ensureRunning();
    ScreenCommand command;
    command.type = ScreenCommand::SYNC_AND_UPDATE;
    command.update_flags = ScreenCommand::FULL_UPDATE;
    command.left = 0;
    command.top = 0;
    command.width = screen_width_;
    command.height = screen_height_;
    command.waveform = waveform;
    command.wait_flags = ScreenCommand::WAIT_BEFORE_UPDATE;
    updateWidget(command);
}

void BSScreenManager::drawImage(const QImage &image)
{
    QRect rc(0, 0, screen_width_, screen_height_);
    blit(rc, image.bits(), onyx::screen::ScreenProxy::GC);
}

void BSScreenManager::fadeScreen()
{
#ifdef ENABLE_EINK_SCREEN
    // Have to use two images as QPainter does not support all features on index 8 image.
    QImage image(imageFromScreen());
    QRect rc(0, 0, screen_width_, screen_height_);

    {
        QImage argb = image.convertToFormat(QImage::Format_ARGB32);
        QPainter p(&argb);
        QBrush brush(QColor(0, 0, 0, 100), Qt::DiagCrossPattern);
        p.fillRect(rc, brush);
        image = argb.convertToFormat(QImage::Format_Indexed8, image.colorTable());
    }

    blit(rc, image.bits(), onyx::screen::ScreenProxy::GU);
#endif
}

/// Show USB connection image on screen.
void BSScreenManager::showUSBScreen()
{
    QRect rc(0, 0, screen_width_, screen_height_);
    QImage argb(rc.size(), QImage::Format_ARGB32);
    argb.fill(qRgb(0xff, 0xff, 0xff));

    int degree = 0;
#ifdef BUILD_FOR_ARM
    degree = QScreen::instance()->transformOrientation() * 90;
#endif
    QImage usb_image(":/images/usb_connection.png");
    if (degree != 0)
    {
        QMatrix rotateMatrix;
        usb_image = usb_image.transformed(rotateMatrix.rotate(360 - degree));
    }

    int x = (screen_width_  - usb_image.width()) / 2;
    int y = (screen_height_  - usb_image.height()) / 2;
    {
        QPainter p(&argb);
        p.drawImage(x, y, usb_image);
    }
    argb = argb.convertToFormat(QImage::Format_Indexed8, colorTable());
    blit(rc, argb.bits(), onyx::screen::ScreenProxy::GC);
}

void BSScreenManager::showCurrentDeepSleepScreen()
{
    QImage image(currentScreenSaverImage());
    if (!image.isNull())
    {
        drawImage(image);
    }
    else
    {
        fillScreen(0xff);
    }
}

void BSScreenManager::showDeepSleepScreen()
{
    QImage image(nextScreenSaverImage());
    if (!image.isNull())
    {
        drawImage(image);
    }
    else
    {
        fillScreen(0xff);
    }
}

void BSScreenManager::blit(const QRect & rc,
                         const unsigned char *src,
                         onyx::screen::ScreenProxy::Waveform waveform)
{
    ensureRunning();
#ifdef ENABLE_EINK_SCREEN
    bs_cmd_ld_img_area_data(DEFAULT_PIXEL_FORMAT,
                            rc.left(),
                            rc.top(),
                            rc.width(),
                            rc.height(),
                            const_cast<unsigned char *>(src));
    if (waveform == onyx::screen::ScreenProxy::GU)
    {
        bs_cmd_upd_part(default_waveform, 0, 0);
    }
    else
    {
        bs_cmd_upd_full(default_waveform, 0, 0);
    }
#endif
}

void BSScreenManager::onReadyRead()
{
    // qDebug("onReadyRead at %s sleeping %d", qPrintable(QTime::currentTime().toString("mm:ss.zzz")), sleeping_);

    // Wakeup controller.
    ensureRunning();

    ScreenCommand command;
    QHostAddress address;
    quint16 port;
    int size = sizeof(ScreenCommand);

    while (socket_.hasPendingDatagrams())
    {
        int read = socket_.readDatagram(reinterpret_cast<char *>(&command), sizeof(command), &address, &port);
        if (isBusy() || !isUpdateEnabled())
        {
            continue;
        }

        if (read == size)
        {
            // qDebug("command %d", command.type);
            switch (command.type)
            {
              case ScreenCommand::WAIT_FOR_FINISHED:
                ensureUpdateFinished();
                break;
            case ScreenCommand::SYNC:
                sync(command);
                break;
            case ScreenCommand::UPDATE:
                updateScreen(command);
                break;
            case ScreenCommand::SYNC_AND_UPDATE:
                updateWidget(command);
                break;
            case ScreenCommand::DRAW_LINE:
                drawLine(command, 0);
                break;
            case ScreenCommand::DRAW_LINES:
                drawLines(command);
                break;
            case ScreenCommand::FILL_SCREEN:
                fillScreen(command);
                break;
            default:
                break;
            }
        }

        if (command.wait_flags & ScreenCommand::WAIT_COMMAND_FINISH)
        {
            socket_.writeDatagram(reinterpret_cast<const char *>(&command), sizeof(command), address, port);
        }
    }
}

/// This function uses screen coordinates. TODO: Optimize it later.
void BSScreenManager::onBusyTimeout()
{
#ifdef _WINDOWS
    return;
#endif

    // Using current screen data as the image.
    if (!busy_canvas_)
    {
        QImage image(imageFromScreen());
        if (image.format() == QImage::Format_Indexed8)
        {
            busy_canvas_.reset(new QImage(image.convertToFormat(QImage::Format_ARGB32, image.colorTable())));
        }
        else
        {
            busy_canvas_.reset(new QImage(image.convertToFormat(QImage::Format_ARGB32)));
        }
    }

    // TODO, we may need to rotate here.
    QImage busy_image = busyImage(busy_index_++);
    int x = (screen_width_  - busy_image.width()) / 2;
    int y = (screen_height_  - busy_image.height()) / 2;
    QRect rc(x, y, busy_image.width(), busy_image.height());
    QImage dst = busy_canvas_->copy(rc);
    {
        QPainter painter(&dst);
        painter.drawImage(0, 0, busy_image);
    }
    dst = dst.convertToFormat(QImage::Format_Indexed8, colorTable());
    blit(rc, dst.bits(), onyx::screen::ScreenProxy::GU);
}

/// Make sure the update request has been processed.
void BSScreenManager::ensureUpdateFinished()
{
#ifdef ENABLE_EINK_SCREEN
    bsc.wait_for_ready();
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
#endif
}

/// This function copies data from Qt framebuffer to controller.
void BSScreenManager::sync(ScreenCommand & command)
{
    // qDebug("sync data %d %d %d %d %d", command.left, command.top, command.width, command.height, screen_width_);
#ifdef BUILD_FOR_ARM
    bs_cmd_ld_img_area_data(DEFAULT_PIXEL_FORMAT,
                            command.left,
                            command.top,
                            command.width,
                            command.height,
                            data_,
                            screen_width_);
#endif
    // qDebug("sync data done");
}

/// This function copies data for the specified widget from Qt framebuffer
/// to display controller and updates the screen by using the waveform.
void BSScreenManager::updateWidget(ScreenCommand & command)
{
    sync(command);
    updateScreen(command);
}

/// Update the specified region of the widget.
void BSScreenManager::updateWidgetRegion(ScreenCommand & command)
{
    sync(command);
    updateScreen(command);
}

/// Update screen without copying any data.
void BSScreenManager::updateScreen(ScreenCommand & command)
{
    // qDebug("update screen");
#ifdef ENABLE_EINK_SCREEN
    if (command.wait_flags & ScreenCommand::WAIT_BEFORE_UPDATE)
    {
        ensureUpdateFinished();
    }

    if (command.waveform == ScreenProxy::DW)
    {
        bs_cmd_upd_part(WAVEFORM_2_MODE, 0, 0);
        return;
    }

    if (command.update_flags == ScreenCommand::FULL_UPDATE)
    {
        if (command.waveform == ScreenProxy::GU)
        {
            bs_cmd_upd_part(default_waveform, 0, 0);
        }
        else if (command.waveform == ScreenProxy::GC)
        {
            bs_cmd_upd_full(default_waveform, 0, 0);
        }
    }
    else
    {
        // Only update the specified region.
        if (command.waveform == ScreenProxy::GU)
        {
            bs_cmd_upd_part_area(default_waveform, 0, 0,
                                 command.left, command.top, command.width, command.height);
        }
        else if (command.waveform == ScreenProxy::GC)
        {
            bs_cmd_upd_full_area(default_waveform, 0, 0,
                                 command.left, command.top, command.width, command.height);
        }
    }
#endif
    // qDebug("update screen done");
}

void BSScreenManager::reset()
{
    // We need to re-init the controller sometimes. From experiment,
    // sometimes, the controller may deadlock when switch to sleep mode.
    // at this situation, we need to reset the controller and change the mode
    // to running mode.
    // qDebug("Reset controller now.");

    if (screen_width_ < 800)
    {
        sys::runScriptBlock("bs60_init", QStringList(), 3000);
    }

    // TODO, for the other screen resolution.
}

bool BSScreenManager::ensureRunning()
{
#ifdef ENABLE_EINK_SCREEN
    if (sleeping_)
    {
        bool ok = true;
        sleeping_ = false;
        int i = 0;
        while (i++ < 3 || !ok)
        {
            ok = true;
            if ((bsc.wait_for_ready() >= WAIT_MAX))
            {
                ok = false;
            }
            bs_cmd_run_sys();
            if ((bsc.wait_for_ready() >= WAIT_MAX))
            {
                // Need to reset the controller again.
                ok = false;
            }
        }
        return ok;
    }
#endif
    return true;
}

bool BSScreenManager::sleep()
{
#ifdef ENABLE_EINK_SCREEN
    if (!sleeping_)
    {
        // Ensure all previous screen updates have been processed.
        // Otherwise screen may look strange.
        ensureUpdateFinished();

        // Use sleep instead of standby. As sleep can save more power without
        // reducing performance. Sleep also cuts down the power to eink screen.
        // qDebug("Turn display controller into deep sleep mode.");
        bs_cmd_slp();

        // Don't use standby now.
        // bs_cmd_stby();
        // qDebug("after sleep to wait_until_ready.");
        // bsc.wait_until_ready();
        sleeping_ = true;
    }
#endif
    return true;
}

void BSScreenManager::shutdown()
{
    // Don't need to shutdown controller now, as kernel close it now.
    // Ensure it's in running state.
    // reset();
    ensureRunning();
    // sleep();
}

bool BSScreenManager::dbgStateTest()
{
    if (!ensureRunning())
    {
        return false;
    }

    refreshScreen(onyx::screen::ScreenProxy::GC);

    sleep();

    return true;
}

void BSScreenManager::snapshot(const QString &path)
{
    QImage image(imageFromScreen());
    image.save(path, "png");
}

/// Draw line on screen directly. Make sure you call the enableFastestUpdate
/// before using this function.
void BSScreenManager::drawLine(ScreenCommand & command, int index)
{
    // Check range.
    static const int MAX_SIZE = 50;
    if (command.size >= MAX_SIZE || command.size <= 0)
    {
        return;
    }

    int size = command.size * command.size;
    if (static_cast<int>(img.size()) < size)
    {
        img.resize(size);
    }
    memset(&img[0], command.color, size);

    int rad = command.size / 2;
    int px1 = command.points[index].x() - rad;
    int py1 = command.points[index].y() - rad;
    int px2 = command.points[index + 1].x() - rad;
    int py2 = command.points[index + 1].y() - rad;

#ifdef ENABLE_EINK_SCREEN
    // draw the end point
    bs_cmd_ld_img_area_data(DEFAULT_PIXEL_FORMAT, px2, py2, command.size, command.size, &img[0]);
#endif

    bool is_steep = abs(py2 - py1) > abs(px2 - px1);
    if (is_steep)
    {
        swap(px1, py1);
        swap(px2, py2);
    }

    // setup line draw
    int deltax   = abs(px2 - px1);
    int deltaerr = abs(py2 - py1);
    int error = 0;
    int x = px1;
    int y = py1;

    // setup step increment
    int xstep = (px1 < px2) ? 1 : -1;
    int ystep = (py1 < py2) ? 1 : -1;

    if (is_steep)
    {
        // draw swapped line
        for (int numpix = 0; numpix < deltax; numpix++)
        {
            x += xstep;
            error += deltaerr;

            if (2 * error > deltax)
            {
                y += ystep;
                error -= deltax;
            }

#ifdef ENABLE_EINK_SCREEN
            // draw the interposed point
            bs_cmd_ld_img_area_data(DEFAULT_PIXEL_FORMAT, y, x, command.size, command.size, &img[0]);
#endif
        }
    }
    else
    {
        // draw normal line
        for (int numpix = 0; numpix < deltax; numpix++)
        {
            x += xstep;
            error += deltaerr;

            if (2 * error > deltax)
            {
                y += ystep;
                error -= deltax;
            }

#ifdef ENABLE_EINK_SCREEN
            // draw the interposed point
            bs_cmd_ld_img_area_data(DEFAULT_PIXEL_FORMAT, x, y, command.size, command.size, &img[0]);
#endif
        }
    }
}

void BSScreenManager::drawLines(onyx::screen::ScreenCommand & command)
{
    for(int i = 0; i < command.point_count - 1; ++i)
    {
        drawLine(command, i);
    }
}

void BSScreenManager::fillScreen(ScreenCommand & command)
{
#ifdef BUILD_FOR_ARM
    bs_cmd_ld_img_area_data(DEFAULT_PIXEL_FORMAT,
                            0, 0, screen_width_, screen_height_,
                            command.color,
                            screen_width_);
#endif
    updateScreen(command);
}

QImage BSScreenManager::busyImage(int index)
{
    static QStringList images;
    if (images.size() <= 0)
    {
        images << ":/images/busy_a.png";
        images << ":/images/busy_b.png";
        images << ":/images/busy_c.png";
        images << ":/images/busy_d.png";
    }

    int i = index % 4;
    if (i >= 0 && i < images.size())
    {
        return QImage(images.at(i));
    }
    return QImage();
}

QDir BSScreenManager::screenSaverDir()
{
    QDir dir(SHARE_ROOT);
    dir.cd("system_manager/images");
    return dir;
}

QImage BSScreenManager::nextScreenSaverImage()
{
    if (++screen_saver_index_ >= screenSaverCount())
    {
        screen_saver_index_ = 0;
    }

    // Place images in $SHARE_ROOT/system_manager/images/
    QString path("screen_saver%1.png");
    path = path.arg(screen_saver_index_);
    path = screenSaverDir().absoluteFilePath(path);
    QImage result(path);
    if (!result.isNull())
    {
        if (result.width() != screen_width_ || result.height() != screen_height_)
        {
            result = result.scaled(screen_width_, screen_height_);
        }

        if (result.format() != QImage::Format_Indexed8)
        {
            return result.convertToFormat(QImage::Format_Indexed8, colorTable());
        }
    }
    return result;
}

QImage BSScreenManager::currentScreenSaverImage()
{
    // Place images in $SHARE_ROOT/system_manager/images/
    QString path("screen_saver%1.png");
    path = path.arg(screen_saver_index_);
    path = screenSaverDir().absoluteFilePath(path);
    QImage result(path);
    if (!result.isNull())
    {
        if (result.width() != screen_width_ || result.height() != screen_height_)
        {
            result = result.scaled(screen_width_, screen_height_);
        }

        if (result.format() != QImage::Format_Indexed8)
        {
            return result.convertToFormat(QImage::Format_Indexed8, colorTable());
        }
    }
    return result;
}

int BSScreenManager::screenSaverCount()
{
    return screenSaverDir().entryInfoList(QDir::NoDotAndDotDot|QDir::Files).size();
}

QImage BSScreenManager::imageFromScreen()
{
    // Takes a non-const data buffer, this version will never alter the contents of the buffer.
#ifdef BUILD_FOR_ARM
    QScreen *instance = QScreen::instance();
    const uchar *data = instance->base();
    int degree = 0;
    degree = instance->transformOrientation() * 90;
    int width = instance->width();
    int height = instance->height();
    if (degree == 90 || degree == 270)
    {
        std::swap(width, height);
    }
    QImage image(data, width, height, instance->pixelFormat());

    // Necessary to check as when rotation, it uses different pixel format.
    if (instance->pixelFormat() == QImage::Format_Indexed8)
    {
        image.setColorTable(colorTable());
    }
    return image;
#endif
    return QImage();
}


