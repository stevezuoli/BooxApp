#include <QtGui/QtGui>
#include <algorithm>
#include <math.h>
#ifdef ENABLE_EINK_SCREEN
#include <cstdio>
#include <QtGui/qscreen_qws.h>
#include <QtGui/qwsdisplay_qws.h>
#endif

#include "kermit_screen_manager.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys_utils.h"
#include "kermit/kermit_api.h"

using namespace std;
using namespace onyx::screen;

static const int WAVEFORM_8_MODE = 3;
static const int WAVEFORM_16_MODE = 2;

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
KermitScreenManager::KermitScreenManager()
: busy_(false)
, busy_index_(0)
, screen_saver_index_(-1)
, sleeping_(true)
, enable_(true)
, kermit_buffer_(0)
, kermit_buffer_size_(0)
, screen_width_(800)
, screen_height_(600)
{
#ifdef ENABLE_EINK_SCREEN
    screen_width_ = qgetenv("SCREEN_WIDTH").toInt();
    if (screen_width_ < 0)
    {
        screen_width_ = 800;
    }

    screen_height_ = qgetenv("SCREEN_HEIGHT").toInt();
    if (screen_height_ < 0)
    {
        screen_height_ = 600;
    }

    // By default, we use the 8 gray level waveform.
    default_waveform = qgetenv("WAVEFORM").toInt();
    if (default_waveform < 0)
    {
        default_waveform = WAVEFORM_8_MODE;
    }

#endif
    start();

    // setup connection.
    busy_timer_.setInterval(1200);
    connect(&busy_timer_, SIGNAL(timeout()), this, SLOT(onBusyTimeout()));
}

KermitScreenManager::~KermitScreenManager()
{
    stop();
}

void KermitScreenManager::enableUpdate(bool enable)
{
    enable_ = enable;
}

void KermitScreenManager::start()
{
#ifdef BUILD_FOR_ARM
    int ret = kermit_init();
    kermit_buffer_size_ = 0;
    kermit_buffer_ = (uchar *)kermit_mmap((long unsigned int *)&kermit_buffer_size_);
    qDebug("kermit init done %d buffer %p size %d", ret, kermit_buffer_, kermit_buffer_size_);

    kermit_display_on();
    kermit_panel_on();
#endif
    socket_.bind(QHostAddress(QHostAddress::LocalHost), PORT);
    connect(&socket_, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void KermitScreenManager::stop()
{
#ifdef BUILD_FOR_ARM
    kermit_display_off();
    kermit_panel_off();
    kermit_free(kermit_buffer_, kermit_buffer_size_);
    kermit_exit();
#endif
}

bool KermitScreenManager::setGrayScale(int colors)
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

int KermitScreenManager::grayScale()
{
#ifdef BUILD_FOR_ARM
    if (default_waveform == WAVEFORM_16_MODE)
    {
        return 16;
    }
#endif
    return 8;
}

QVector<QRgb> & KermitScreenManager::colorTable()
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

void KermitScreenManager::setBusy(bool busy, bool show_indicator)
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

void KermitScreenManager::fillScreen(unsigned char color)
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

void KermitScreenManager::refreshScreen(onyx::screen::ScreenProxy::Waveform waveform)
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

void KermitScreenManager::drawImage(const QImage &image)
{
    QRect rc(0, 0, screen_width_, screen_height_);
    blit(rc, image.bits(), onyx::screen::ScreenProxy::GC);
}

void KermitScreenManager::fadeScreen()
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
void KermitScreenManager::showUSBScreen()
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

void KermitScreenManager::showDeepSleepScreen()
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

void KermitScreenManager::blit(const QRect & rc,
                               const unsigned char *src,
                               onyx::screen::ScreenProxy::Waveform waveform)
{
    // Convert from Qt framebuffer to marvell buffer.
#ifdef ENABLE_EINK_SCREEN
    flashScreen(waveform);
#endif
}

void KermitScreenManager::onReadyRead()
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
            case ScreenCommand::DRAW_POINTS:
                drawLine(command);
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
void KermitScreenManager::onBusyTimeout()
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
    QImage & busy_image = busyImage(busy_index_++);
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
void KermitScreenManager::ensureUpdateFinished()
{
#ifdef ENABLE_EINK_SCREEN

#endif
}

/// This function copies data from Qt framebuffer to controller.
void KermitScreenManager::sync(ScreenCommand & command)
{
#ifdef BUILD_FOR_ARM
    qDebug("sync data now...%d %d %d %d", command.left, command.top, command.width, command.height);
    QTime t;t.start();
    uchar * dst  = kermit_buffer_ + command.top * screen_width_ / 2 + command.left / 2;
    uchar * base = QScreen::instance()->base();
    base = base + command.top * QScreen::instance()->linestep() + command.left * 4;
    for(int i = 0; i < command.height; ++i)
    {
        uint * s = reinterpret_cast<uint *>(base);
        uchar * t = dst;
        for(int j = 0; j < command.width/2; ++j, ++t)
        {
            *t = (qGray(*s++) >> 4);
            *t += (qGray(*s++) & 0xf0);
        }
        dst += screen_width_ / 2;
        base += QScreen::instance()->linestep();
    }
    qDebug("elapsed %d ms", t.elapsed());
#endif
}

/// This function copies data for the specified widget from Qt framebuffer
/// to display controller and updates the screen by using the waveform.
void KermitScreenManager::updateWidget(ScreenCommand & command)
{
    sync(command);
    updateScreen(command);
}

/// Update the specified region of the widget.
void KermitScreenManager::updateWidgetRegion(ScreenCommand & command)
{
    sync(command);
    updateScreen(command);
}

/// Update screen without copying any data.
void KermitScreenManager::updateScreen(ScreenCommand & command)
{
#ifdef ENABLE_EINK_SCREEN
    if (command.wait_flags & ScreenCommand::WAIT_BEFORE_UPDATE)
    {
        ensureUpdateFinished();
    }

    if (command.waveform == onyx::screen::ScreenProxy::DW)
    {
        flashScreen(command.waveform);
        return;
    }

    if (command.update_flags == ScreenCommand::FULL_UPDATE)
    {
        if (command.waveform == onyx::screen::ScreenProxy::GU)
        {
            flashScreen(command.waveform);
        }
        else if (command.waveform == onyx::screen::ScreenProxy::GC)
        {
            flashScreen(command.waveform);
        }
    }
    else
    {
        // Only update the specified region.
        if (command.waveform == onyx::screen::ScreenProxy::GU)
        {
            flashScreen(command.waveform);
        }
        else if (command.waveform == onyx::screen::ScreenProxy::GC)
        {
            flashScreen(command.waveform);
        }
    }
#endif
}

void KermitScreenManager::reset()
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

bool KermitScreenManager::ensureRunning()
{
    kermit_panel_on();
    return true;
}

bool KermitScreenManager::sleep()
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

        // Don't use standby now.
        // bs_cmd_stby();
        // qDebug("after sleep to wait_until_ready.");
        // bsc.wait_until_ready();
        kermit_display_off();

        kermit_panel_off();

        sleeping_ = true;
    }
#endif
    return true;
}

void KermitScreenManager::shutdown()
{
    // Don't need to shutdown controller now, as kernel close it now.
    // Ensure it's in running state.
    // reset();
    kermit_display_off();
    kermit_panel_off();
    // sleep();
}

void KermitScreenManager::remap()
{
}

bool KermitScreenManager::dbgStateTest()
{
    if (!ensureRunning())
    {
        return false;
    }

    refreshScreen(onyx::screen::ScreenProxy::GC);

    sleep();

    return true;
}

void KermitScreenManager::snapshot(const QString &path)
{
    QImage image(imageFromScreen());
    image.save(path, "png");
}

/// Draw line on screen directly. Make sure you call the enableFastestUpdate
/// before using this function.
void KermitScreenManager::drawLine(ScreenCommand & command)
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
    int px1 = command.x1 - rad;
    int py1 = command.y1 - rad;
    int px2 = command.x2 - rad;
    int py2 = command.y2 - rad;

#ifdef ENABLE_EINK_SCREEN
    // draw the end point
    // bs_cmd_ld_img_area_data(DEFAULT_PIXEL_FORMAT, px2, py2, command.size, command.size, &img[0]);
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
            // bs_cmd_ld_img_area_data(DEFAULT_PIXEL_FORMAT, y, x, command.size, command.size, &img[0]);
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
            // bs_cmd_ld_img_area_data(DEFAULT_PIXEL_FORMAT, x, y, command.size, command.size, &img[0]);
#endif
        }
    }
}

void KermitScreenManager::fillScreen(ScreenCommand & command)
{
#ifdef BUILD_FOR_ARM

#endif
    updateScreen(command);
}

QImage & KermitScreenManager::busyImage(int index)
{
    static QImage a(":/images/busy_a.png");
    static QImage b(":/images/busy_b.png");
    static QImage c(":/images/busy_c.png");
    int i = index % 3;
    if (i == 0)
    {
        return a;
    }
    else if (i == 1)
    {
        return b;
    }
    return c;
}

QDir KermitScreenManager::screenSaverDir()
{
    QDir dir(SHARE_ROOT);
    dir.cd("system_manager/images");
    return dir;
}

QImage KermitScreenManager::nextScreenSaverImage()
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
        if (result.format() != QImage::Format_Indexed8)
        {
            return result.convertToFormat(QImage::Format_Indexed8, colorTable());
        }
    }
    return result;
}

int KermitScreenManager::screenSaverCount()
{
    return screenSaverDir().entryInfoList(QDir::NoDotAndDotDot|QDir::Files).size();
}

void KermitScreenManager::flashScreen(int waveform)
{
    if (waveform == onyx::screen::ScreenProxy::GC)
    {
        kermit_set_waveform(WF_GC);
        kermit_flash_screen();
    }
    else if (waveform == onyx::screen::ScreenProxy::DW)
    {
        kermit_set_waveform(WF_MU);
    }
    else
    {
        kermit_set_waveform(WF_GU);
    }
    kermit_display_on();
    kermit_display_off();
}

QImage KermitScreenManager::imageFromScreen()
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


