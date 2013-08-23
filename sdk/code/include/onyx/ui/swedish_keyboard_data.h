#ifndef ONYX_SWEDISH_KEYBOARD_DATA_H_
#define ONYX_SWEDISH_KEYBOARD_DATA_H_

#include "keyboard_data.h"

namespace ui
{

class SwedishKeyboardData: public KeyboardData
{
public:
    SwedishKeyboardData();
    virtual ~SwedishKeyboardData();

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

}   // namespace ui

#endif
