#include <QtGui/QtGui>
#include "../inc/power_manager.h"

class MessageWidget : public QWidget
{
    Q_OBJECT
public:
    MessageWidget(PowerManager  & pm);
    ~MessageWidget();

protected:
    void paintEvent(QPaintEvent *event);
    bool event(QEvent * event);

private Q_SLOTS:
    void onTimeout();

private:
    PowerManager & watcher_;
    QTimer timer_;
    int count_;
    QFont font_;
};

