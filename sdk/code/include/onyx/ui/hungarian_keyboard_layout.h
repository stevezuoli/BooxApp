#ifndef ONYX_HUNGARIAN_KEYBOARD_LAYOUT_H_
#define ONYX_HUNGARIAN_KEYBOARD_LAYOUT_H_

#include "onyx/ui/keyboard_layout.h"

namespace ui
{

class HungarianKeyboardLayout: public KeyboardLayout
{
public:
    HungarianKeyboardLayout();
    ~HungarianKeyboardLayout();

    QSize getKeySize(int code);
protected:
    void initCode();
    void initKeys();
    void initShiftMap();
};

};

#endif
