
#ifndef ONYX_FULL_SCREEN_CALENDAR_H_
#define ONYX_FULL_SCREEN_CALENDAR_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/ui_global.h"
#include "onyx/screen/screen_proxy.h"

namespace ui
{

/// Fullscreen calendar
class Calendar : public QDialog
{
    Q_OBJECT
public:
    Calendar(QWidget *parent);
    ~Calendar(void);

public:
    int exec();

private:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    bool event(QEvent *e);
    void createLayout();
    void paintEvent(QPaintEvent *);
    void showEvent(QShowEvent *);
    void drawMonthName(QPainter* painter, const QRect &month_name_rect,
            const int month);
    void drawMonth(QPainter* painter,
                   int inix,
                   int iniy,
                   int monthWth,
                   int monthHth,
                   int dayWth,
                   int dayHth,
                   int year,
                   int month);
    void drawPage(QPainter*, const QDate& date);
    void drawArrow(QPainter* painter, int total_width, int total_height, int hor_space, int ver_space, int year_height);
    void drawYear(QPainter* painter, const QRect& rect, int year_height, int year);
    void setColAndRow(int& col, int& row, int total_width, int total_height, int hor_space, int ver_space);
    void month_loc(int& x,int& y, int col,int month);
    int firstMonth(int curr_month);
    void stylusPan(const QPoint &now, const QPoint &old);

private Q_SLOTS:
    void onReturn();
    void onOkClicked(bool);
    void onCloseClicked();

private:
    int page_tag_;
    int month_count_;
    QPoint begin_point_;
    sys::SystemConfig conf_;

    QRect left_arrow_rect_;
    QRect right_arrow_rect_;
    onyx::screen::ScreenProxy::Waveform flush_type_;
};

}   // namespace ui

#endif
