#ifndef ONYX_STATUS_BAR_ITEM_APP_H_
#define ONYX_STATUS_BAR_ITEM_APP_H_

#include "status_bar_item.h"

namespace ui
{

class StatusBarItemApp : public StatusBarItem
{
    Q_OBJECT

public:
    StatusBarItemApp(QWidget *parent, StatusBarItemType type = APP_DEFINED);
    virtual ~StatusBarItemApp(void);

    void setAppId(const int id);
    int appId();
    void setImage(const QImage & image);

Q_SIGNALS:
    void clicked(int);

private:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

    void createLayout();

private:
    QImage image_;
    int id_;
};

};  // namespace ui

#endif  // ONYX_STATUS_BAR_ITEM_VOLUME_H_
