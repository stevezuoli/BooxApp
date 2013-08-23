
#include <QColor>
#include <QTime>
#include <qglobal.h>
#include "kermit_screen.h"
#include "kermit/kermit_api.h"

static const int DEFAULT_WIDTH = 800;
static const int DEFAULT_HEIGHT = 600;

KermitScreen::KermitScreen(int displayId)
: QScreen(displayId)
, memory_(0)
{
    qDebug("Onyx Kermit screen created!");
}

KermitScreen::~KermitScreen()
{
    if (memory_)
    {
        delete memory_;
        memory_ = 0;
    }
}

// Ref: http://doc.trolltech.com/4.2/qtopiacore-svgalib.html
// http://doc.trolltech.com/main-snapshot/ipc-sharedmemory-dialog-cpp.html
bool KermitScreen::connect(const QString &displaySpec)
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

    QScreen::d = 32;                     // Color depth.

    QScreen::lstep = width * d / 8;
    QScreen::w = width;
    QScreen::h = height;
    QScreen::dw = width;
    QScreen::dh = height;

    QScreen::size = lstep * height;
    QScreen::grayscale = false;

    setPixelFormat(QImage::Format_ARGB32);

    if (memory_ == 0)
    {
        memory_ = new uchar[size];
        mapsize = size;
        qDebug("map size is %d in argb", mapsize);
    }
    QScreen::data = memory_;

    // Maybe need to change the dpi later.
    const int dpi = 72;
    QScreen::physWidth = qRound(QScreen::dw * 25.4 / dpi);
    QScreen::physHeight = qRound(QScreen::dh * 25.4 / dpi);
    return true;
}

bool KermitScreen::initDevice()
{
    return true;
}

void KermitScreen::shutdownDevice()
{
}

void KermitScreen::disconnect()
{
}

void KermitScreen::exposeRegion(QRegion r, int changing)
{
    QScreen::exposeRegion(r, changing);
}

void KermitScreen::blit(const QImage &img, const QPoint &topLeft, const QRegion &reg)
{
    bool do_fallback = true;
    // QTime t; t.start();

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

    // qDebug("blit time %d", t.elapsed());
}

void KermitScreen::solidFill(const QColor &color, const QRegion &region)
{
    //qDebug("KermitScreen::solidFill  ");
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
    // qDebug("KermitScreen::solidFill done");
}





