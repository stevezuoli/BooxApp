
#include <QColor>
#include <QTime>
#include <qglobal.h>
#include "naboo_screen.h"

static const int DEFAULT_WIDTH = 600;
static const int DEFAULT_HEIGHT = 800;
static bool screen_debug = false;

NabooScreen::NabooScreen(int displayId)
: QScreen(displayId)
, memory_(0)
{
    // qDebug("Naboo screen created!");
    screen_debug = (qgetenv("DEBUG_SCREEN_DRIVER").toInt() > 0);
}

NabooScreen::~NabooScreen()
{
    if (memory_)
    {
        delete [] memory_;
        memory_ = 0;
    }
}

// Ref: http://doc.trolltech.com/4.2/qtopiacore-svgalib.html
// http://doc.trolltech.com/main-snapshot/ipc-sharedmemory-dialog-cpp.html
bool NabooScreen::connect(const QString &displaySpec)
{
    Q_UNUSED(displaySpec);

    // Read environment variables.
    int width = qgetenv("SCREEN_WIDTH").toInt();
    int height = qgetenv("SCREEN_HEIGHT").toInt();

    // Fallback if not available.
    if (width <= 0)
    {
        width = DEFAULT_WIDTH;
    }
    if (height <= 0)
    {
        height = DEFAULT_HEIGHT;
    }

    QScreen::lstep = width;
    QScreen::w = width;
    QScreen::h = height;
    QScreen::dw = width;
    QScreen::dh = height;

    QScreen::d = 8;                     // Color depth.
    QScreen::size = width * height;
    QScreen::grayscale = true;
    QScreen::screencols = 256;

    setPixelFormat(QImage::Format_Indexed8);

    for(int i = 0; i < 256; ++i)
        screenclut[i] = qRgb(i, i, i);

    // When we can not create the shared memory, it could be caused that system manager crashed
    // or killed. On Linux, the shared memory is still there, so we can try to attach to the
    // existing shared memory.
    memory_ = new uchar[QScreen::size];
    QScreen::data = memory_;

    // Maybe need to change the dpi later.
    const int dpi = 72;
    QScreen::physWidth = qRound(QScreen::dw * 25.4 / dpi);
    QScreen::physHeight = qRound(QScreen::dh * 25.4 / dpi);
    return true;
}

bool NabooScreen::initDevice()
{
    return true;
}

void NabooScreen::shutdownDevice()
{
}

void NabooScreen::disconnect()
{
}

void NabooScreen::exposeRegion(QRegion r, int changing)
{
    QScreen::exposeRegion(r, changing);
}

void NabooScreen::blit(const QImage &img, const QPoint &topLeft, const QRegion &reg)
{
    bool do_fallback = true;
    static QTime t;
    if (screen_debug)
    {
        t.start();
    }

    // Only handle the indexed 8, as the eink screen works best with that.
    // Not sure yet. It should be gray color instead of indexed 8.
    // qDebug("blit img format %d image color depth %d indexed8 %d",
    //         img.format(), img.depth(), QImage::Format_Indexed8);
    if (depth() == 8 && img.format() == QImage::Format_Indexed8)
        do_fallback = false;

    if (do_fallback)
    {
        // qDebug( "call QScreen::blit now topLeft (%d,%d)- (%d,%d)",
        //         topLeft.x(), topLeft.y(),
        //         reg.boundingRect().width(), reg.boundingRect().height());
        QScreen::blit(img, topLeft, reg);
    }
    else
    {
        qDebug("Implement it later.");
    }

    if (screen_debug)
    {
        qDebug("blit time %d", t.elapsed());
    }
}

void NabooScreen::solidFill(const QColor &color, const QRegion &region)
{
    //qDebug("NabooScreen::solidFill  ");
    if (depth() != 8)
    {
        qWarning("Fallback to QScreen as the depth is not 8 %d", depth());
        QScreen::solidFill(color, region);
        return;
    }

    unsigned char *base = QScreen::data;
    if (base == 0)
    {
        qWarning("Framebuffer is empty.");
        return;
    }

    unsigned char gray = (qGray(color.rgba()) & 0xff);
    const QVector<QRect> rects = region.rects();
    for (int i = 0; i < rects.size(); ++i)
    {
        const QRect r = rects.at(i);

        for(int y = r.top(); y < r.top() + r.height(); ++y)
        {
            unsigned char *p = base + y * QScreen::w + r.left();
            memset(p, r.width(), gray);
        }
    }
    // qDebug("NabooScreen::solidFill done");
}





