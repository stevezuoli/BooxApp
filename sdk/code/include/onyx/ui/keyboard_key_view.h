#ifndef ONYX_KEYBOARD_KEY_H_
#define ONYX_KEYBOARD_KEY_H_

#include "onyx_keyboard_utils.h"
#include "onyx/ui/content_view.h"

namespace ui
{

class KeyboardData;

class KeyboardKeyView : public ContentView
{
    Q_OBJECT

public:
    KeyboardKeyView(QWidget *parent = 0);
    ~KeyboardKeyView();

    virtual void updateView();
    static const QString type();

private:
    void paintEvent(QPaintEvent * event);
    void drawIcon(QPainter & painter, QRect rect);
    void drawText(QPainter & painter, QRect rect);

};

}   // namespace ui

#endif
