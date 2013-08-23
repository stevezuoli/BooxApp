#ifndef ONYX_POLISH_KEYBOARD_LAYOUT_H_
#define ONYX_POLISH_KEYBOARD_LAYOUT_H_

#include "onyx/ui/keyboard_layout.h"

namespace ui
{

class PolishKeyboardLayout: public KeyboardLayout
{
public:
    PolishKeyboardLayout();
    ~PolishKeyboardLayout();

    QSize getKeySize(int code);
protected:
    void initCode();
    void initKeys();
    void initShiftMap();
};

};

#endif
