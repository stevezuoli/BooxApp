#ifndef ONYX_STATUS_BAR_ITEM_CLOCK_H_
#define ONYX_STATUS_BAR_ITEM_CLOCK_H_

#include "status_bar_item.h"

namespace ui
{

class StatusBarItemClock : public StatusBarItem
{
    Q_OBJECT

public:
    StatusBarItemClock(QWidget *parent);
    virtual ~StatusBarItemClock(void);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public:
    QDateTime & startDateTime() { return start_; }

Q_SIGNALS:
    void clicked();

private:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

    void createLayout();
    bool setTimeText();

private:
    QDateTime start_;
    QString time_text_;
    scoped_ptr<QFontMetrics> metrics_;

private:
    static const QString DATE_FORMAT;
};

};  // namespace ui

#endif  // ONYX_STATUS_BAR_ITEM_CLOCK_H_
