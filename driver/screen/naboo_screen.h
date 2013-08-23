/// Author John
/// Qt based Eink screen plugin for Naboo project.

#ifndef NABOO_SCREEN_H_
#define NABOO_SCREEN_H_

#include <QScreen>
#include <QSharedMemory>

/// Eink screen driver. It initialize the screen size
/// according to the environment variables. It provides
/// the memory chunk for qt screen driver. Check the
/// environment: SCREEN_WIDTH and SCREEN_HEIGHT.
/// The main idea to introduce this driver is to avoid implementing
/// a Linux kernel level framebuffer driver. This driver does
/// use any platform or hardware related API.
class NabooScreen : public QScreen
{
public:
    NabooScreen(int displayId);
    ~NabooScreen();

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

#endif  // NABOO_SCREEN_H_

