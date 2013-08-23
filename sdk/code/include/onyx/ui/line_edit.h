#ifndef ONYX_LINE_EDIT_H_
#define ONYX_LINE_EDIT_H_

#include <QtGui/QtGui>

namespace ui
{

/// Line edit for eink device. Remove unnecessary updates.
class OnyxLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    OnyxLineEdit(QWidget *parent);
    OnyxLineEdit(const QString & text, QWidget *parent);
    ~OnyxLineEdit();

protected:
    void focusInEvent(QFocusEvent *e);

Q_SIGNALS:
    void getFocus(OnyxLineEdit *object);
    void setCheckByMouse(OnyxLineEdit *object);
    void outOfRange(QKeyEvent *ke);

protected:
    void keyReleaseEvent(QKeyEvent *ke);
    void keyPressEvent(QKeyEvent * ke);
    void mouseReleaseEvent(QMouseEvent * event);

private:
    bool out_of_range_;
};

};

#endif  // ONYX_LINE_EDIT_H_
