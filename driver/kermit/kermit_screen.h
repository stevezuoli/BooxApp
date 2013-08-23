/// Author John
/// Qt based Eink screen plugin for marvell kermit chipset.

#ifndef KERMIT_SCREEN_H_
#define KERMIT_SCREEN_H_

#include <QScreen>
#include <QSharedMemory>

/// Kermit Eink screen driver. It initialize the screen size
/// according to the environment variables. It provides
/// the memory chunk for qt screen driver. Check the
/// environment: SCREEN_WIDTH and SCREEN_HEIGHT.
/// The main idea to introduce this driver is to avoid implementing
/// a Linux kernel level framebuffer driver. This driver does
/// use any platform or hardware related API.
class KermitScreen : public QScreen
{
public:
    KermitScreen(int displayId);
    ~KermitScreen();

    bool connect(const QString &displaySpec);
    bool initDevice();
    void shutdownDevice();
    void disconnect();

    // TODO.
    void setMode(int, int, int) {}
    void blank(bool) {}

    void exposeRegion(QRegion r, int changing);
    void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
    void solidFill(const QColor &color, const QRegion &region);

private:
    uchar *memory_;
};

#endif  // KERMIT_SCREEN_H_

