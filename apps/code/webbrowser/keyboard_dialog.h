#ifndef KEYBOARD_DIALOG_H_
#define KEYBOARD_DIALOG_H_

#include "onyx/ui/ui.h"

using namespace ui;
namespace webbrowser
{

class KeyboardDialog : public OnyxDialog
{
    Q_OBJECT
public:
    explicit KeyboardDialog(QWidget *parent = 0);
    ~KeyboardDialog(void);

    void ensureVisible();
    void clearText();
    QSize sizeHint() const;
    QSize minimumSize () const;
    QSize maximumSize () const;

Q_SIGNALS:
    void textFinsihed(const QString & text);

protected:
    void mouseMoveEvent(QMouseEvent *me);
    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);
    void keyReleaseEvent(QKeyEvent *);
    void keyPressEvent(QKeyEvent * ke);
    bool event(QEvent * event);
    void moveEvent(QMoveEvent *e);
    void resizeEvent(QResizeEvent *e);

private Q_SLOTS:
    void onOKClicked();
    void onTimeout();
    void onClose();

private:
    void createLayout();
    void adjustPosition();

private:
    QHBoxLayout       hbox_;

    KeyBoard          keyboard_;     ///< Keyboard.
    OnyxLineEdit     text_edit_;    ///< Input edit.
    OnyxPushButton   ok_button_;    ///< OK
    OnyxPushButton   clear_button_; ///< Clear the text.
    QTimer            timer_;        ///< Timer to update the screen.

};

};
#endif
