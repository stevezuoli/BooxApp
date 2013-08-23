#ifndef MULTI_TOUCH_H_
#define MULTI_TOUCH_H_

#include <QtGui/QtGui>
#include "onyx/base/base.h"

/// Class for multi touch. Caller can use this class to
/// calculate the new zooming factor and new image position.
/// This class generates a low quality image during multi touch holding.
class MultiTouch
{
public:
    MultiTouch();
    ~MultiTouch();

public:
    void onHoldDetectedBand(QWidget *wnd, QRect r1, QRect r2, int prev, int now);
    void onHoldDetectedPixmap(QWidget *wnd, QRect r1, QRect r2, int prev, int now);
    void onHoldReleaseDetected(QRect r1, QRect r2);

    QRubberBand * band() { return band_.get(); }
    QPixmap * pixmap();

private:
    int diagonal(const QRect & rc);

private:
    scoped_ptr<QRubberBand> band_;
    scoped_ptr<QPixmap> pixmap_;
    QPixmap result_;
    QRect rc_touched_;
    bool dirty_;
    qreal zoom_;
};

#endif

