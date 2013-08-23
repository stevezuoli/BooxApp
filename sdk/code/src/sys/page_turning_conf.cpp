// Authors: John

#ifdef BUILD_FOR_ARM
#include <QtGui/qscreen_qws.h>
#endif

/// Public header of the system configuration library.
#include "onyx/sys/page_turning_conf.h"


namespace sys
{
int PageTurningConfig::direction_ = 1;
int PageTurningConfig::THRESHOLD = -1;

/// Page turning configuration. By default, from right to left or
/// from bottom to top is interpreted as next.
PageTurningConfig::PageTurningConfig()
{
}

PageTurningConfig::~PageTurningConfig()
{
}

bool PageTurningConfig::makeSureTableExist(QSqlDatabase& database)
{
    QSqlQuery query(database);
    return query.exec("create table if not exists page_turning_conf ("
                      "key text primary key, "
                      "value text "
                      ")");
}

bool PageTurningConfig::load(QSqlDatabase& db)
{
    // TODO.
    return true;
}

bool PageTurningConfig::save(QSqlDatabase& db)
{
    // TODO.
    return true;
}

int PageTurningConfig::direction(const QPoint & old_position, const QPoint & new_position)
{
    int delta_x = new_position.x() - old_position.x();
    int delta_y = new_position.y() - old_position.y();

    int dist = std::max(abs(delta_x), abs(delta_y));
    if (dist < distance())
    {
        return 0;
    }

    int delta = 0;
    if (abs(delta_x) > abs(delta_y))
    {
        delta = delta_x;
    }
    else
    {
        delta = delta_y;
    }

    if (delta < -distance())
    {
        return direction_;
    }
    else if (delta > distance())
    {
        return -direction_;
    }
    return 0;
}

void PageTurningConfig::setDirection(int conf)
{
    if (conf > 0)
    {
        direction_ = 1;
    }
    else if (conf < 0)
    {
        direction_ = -1;
    }
}

/// Return the distance threshold.
int PageTurningConfig::distance()
{
    if (THRESHOLD <= 0)
    {
        THRESHOLD = qgetenv("SWIPE_DISTANCE").toInt();
    }
    if (THRESHOLD <= 0)
    {
        THRESHOLD = 10;
    }
    return THRESHOLD;
}

int PageTurningConfig::whichArea(const QPoint & old_position, const QPoint & new_position)
{
    QRect screen = QApplication::desktop()->screenGeometry();
    int degree = 0;
#ifdef BUILD_FOR_ARM
    degree = QScreen::instance()->transformOrientation() * 90;
#endif
    if (degree == 90 || degree == 270)
    {
        screen.setSize(QSize(screen.height(), screen.width()));
    }

    if (new_position.x() < screen.width() / 3)
    {
        return -1;
    }
    else if (new_position.x() > screen.width() * 2 / 3)
    {
        return 1;
    }
    return 0;
}

}   // namespace sys.
