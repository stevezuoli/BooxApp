/*
 * french_keyboard_data.h
 *
 *  Created on: 2012-2-9
 *      Author: weihuahuang
 */

#ifndef ONYX_FRENCH_KEYBOARD_DATA_H_
#define ONYX_FRENCH_KEYBOARD_DATA_H_

#include "keyboard_data.h"

namespace ui
{

class FrenchKeyboardData: public KeyboardData
{
public:
    FrenchKeyboardData();
    virtual ~FrenchKeyboardData();

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



#endif /* FRENCH_KEYBOARD_DATA_H_ */
