#ifndef HEBREW_KEYBOARD_DATA_H
#define HEBREW_KEYBOARD_DATA_H

#include "keyboard_data.h"

namespace ui
{

class HebrewKeyboardData: public KeyboardData
{
public:
    HebrewKeyboardData();
    virtual ~HebrewKeyboardData();

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

} //end ui

#endif // HEBREW_KEYBOARD_DATA_H
