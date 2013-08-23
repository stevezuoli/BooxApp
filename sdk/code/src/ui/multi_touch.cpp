
#include "onyx/ui/multi_touch.h"
#include "onyx/sys/sys_status.h"


MultiTouch::MultiTouch()
{
}

MultiTouch::~MultiTouch()
{
}

void MultiTouch::onHoldDetectedBand(QWidget *wnd, QRect r1, QRect r2, int prev, int now)
{
    rc_touched_.setCoords(r1.center().x(), r1.center().y(), r2.center().x(), r2.center().y());
    rc_touched_ = rc_touched_.normalized();

    if (prev == 0)
    {
        if (!band_)
        {
            band_.reset(new QRubberBand(QRubberBand::Rectangle, wnd));
            band_->setGeometry(rc_touched_);
            band_->show();
        }
    }
    else
    {
        band_->setGeometry(rc_touched_);
    }
    sys::SysStatus::instance().requestMultiTouch();
}

void MultiTouch::onHoldDetectedPixmap(QWidget *wnd, QRect r1, QRect r2, int prev, int now)
{
    dirty_ = true;

    // Just touched.
    if (prev == 0)
    {
        pixmap_.reset(new QPixmap(QPixmap::grabWidget(wnd, wnd->rect())));
        rc_touched_.setCoords(r1.center().x(), r1.center().y(), r2.center().x(), r2.center().y());
        zoom_ = 1.0;
    }
    else
    {
        QTime t;
        t.start();
        QRect ra;
        ra.setCoords(r1.center().x(), r1.center().y(), r2.center().x(), r2.center().y());
        zoom_ = sqrt(static_cast<qreal>(diagonal(ra)) / static_cast<qreal>(diagonal(rc_touched_)));
        result_ = pixmap_->scaled(pixmap_->width() * zoom_, pixmap_->height() * zoom_);
        qDebug() << "pixmap zooming " << t.elapsed();
    }
    sys::SysStatus::instance().requestMultiTouch();
}

int MultiTouch::diagonal(const QRect & rc)
{
    int l = rc.width() * rc.width() + rc.height() * rc.height();
    return static_cast<int>(sqrt(static_cast<qreal>(l)));
}

void MultiTouch::onHoldReleaseDetected(QRect r1, QRect r2)
{
    if (band_)
    {
        band_->hide();
        band_.reset(0);
    }
    pixmap_.reset(0);
}

QPixmap * MultiTouch::pixmap()
{
    if (pixmap_)
    {
        return &result_;
    }
    return 0;
}

