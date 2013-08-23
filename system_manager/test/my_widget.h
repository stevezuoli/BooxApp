#include <QtGui/QtGui>
#include <onyx/screen/screen_proxy.h>

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    MyWidget();

    ~MyWidget()
    {
    }

protected:
    void paintEvent(QPaintEvent *e);

    void keyReleaseEvent(QKeyEvent *ke)
    {
        if (ke->key() == Qt::Key_Escape)
        {
            qApp->exit();
        }
    }

private Q_SLOTS:
    void refresh();
    int  voltage();

};

