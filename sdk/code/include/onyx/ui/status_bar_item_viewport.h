#ifndef ONYX_STATUS_BAR_ITEM_VIEWPORT_H_
#define ONYX_STATUS_BAR_ITEM_VIEWPORT_H_

#include "status_bar_item.h"

namespace ui
{

class StatusBarItemViewport : public StatusBarItem
{
    Q_OBJECT

public:
    StatusBarItemViewport(QWidget *parent);
    virtual ~StatusBarItemViewport(void);

public Q_SLOTS:
    void setViewport(const QRect & parent, const QRect & child, int current_column, int total);

private:
    virtual void paintEvent(QPaintEvent *);

private:
    QRect parent_;
    QRect child_;
    int total_;
    int current_;
};

};  // namespace ui

#endif  // ONYX_STATUS_BAR_ITEM_VIEWPORT_H_
