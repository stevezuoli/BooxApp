#ifndef ONYX_RUSSIAN_KEYBOARD_LAYOUT_H_
#define ONYX_RUSSIAN_KEYBOARD_LAYOUT_H_

#include "keyboard_data.h"

namespace ui
{

class RussianKeyboardData: public KeyboardData
{
public:
    RussianKeyboardData();
    virtual ~RussianKeyboardData();

protected:
    virtual void initTopKeyCode();
    virtual void initLeftKeyCode();
    virtual void initMiddleKeyCode();
    virtual void initRightKeyCode();
    virtual void initBottomKeyCode();

    virtual void initTopKeyShiftCode();
    virtual void initLeftKeyShiftCode();
    virtual void initMiddleKeyShiftCode();
    virtual void initRightKeyShiftCode();
    virtual void initBottomKeyShiftCode();

    virtual void initLeftKeySymbolCode();
    virtual void initMiddleKeySymbolCode();
    virtual void initRightKeySymbolCode();
};

};

#endif
