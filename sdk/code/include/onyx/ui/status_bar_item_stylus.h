#ifndef ONYX_STATUS_BAR_ITEM_STYLUS_H_
#define ONYX_STATUS_BAR_ITEM_STYLUS_H_

#include "status_bar_item.h"

namespace ui
{

class StatusBarItemStylus : public StatusBarItem
{
    Q_OBJECT

public:
    StatusBarItemStylus(QWidget *parent);
    virtual ~StatusBarItemStylus(void);

Q_SIGNALS:
    void clicked();

public Q_SLOTS:
    void setStylusState(const int state);

private:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    QImage & image();
    QString resourcePath();

private:
    Images images_;
};

};  // namespace ui

#endif  // ONYX_STATUS_BAR_ITEM_STYLUS_H_
