/*
 * bulgarian_keyboard_data.h
 *
 *  Created on: 2012-2-10
 *      Author: weihuahuang
 */

#ifndef ONYX_BULGARIAN_KEYBOARD_DATA_H_
#define ONYX_BULGARIAN_KEYBOARD_DATA_H_

#include "onyx/ui/keyboard_data.h"

namespace ui
{
class BulgarianKeyboardData: public KeyboardData
{

public:
    BulgarianKeyboardData();
    virtual ~BulgarianKeyboardData();

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


#endif /* BULGARIAN_KEYBOARD_DATA_H_ */
