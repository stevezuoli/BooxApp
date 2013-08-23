#ifndef ONYX_KEYBOARD_DATA_H_
#define ONYX_KEYBOARD_DATA_H_

#include "onyx_keyboard_utils.h"
#include "onyx/data/data.h"

namespace ui
{

class KeyboardData
{
public:
    static const QString BACKSPACE_TEXT;
    static const QString ENTER_TEXT;

public:
    KeyboardData();
    virtual ~KeyboardData();

    ODatas & topCodes(bool shift = false, bool symbol = false);
    ODatas & leftCodes(bool shift = false);
    ODatas & middleCodes(bool shift = false);
    ODatas & rightCodes(bool shift = false);
    ODatas & bottomCodes(bool shift = false);
    ODatas & menuCodes();

    ODatas & leftSymbolCodes();
    ODatas & middleSymbolCodes();
    ODatas & rightSymbolCodes();

protected:
    // Should insert a QString value of TAG_TITLE for normal keys (like a, b,
    // c, etc). And for special keys (like back-space), should insert both 1) a
    // QPixmap instance of TAG_COVER ; 2) a QString value of TAG_SPECIAL_KEY.
    virtual void initTopKeyCode() = 0;
    virtual void initLeftKeyCode() = 0;
    virtual void initMiddleKeyCode() = 0;
    virtual void initRightKeyCode() = 0;
    virtual void initBottomKeyCode() = 0;
    virtual void initMenuKeyCode();

    virtual void initTopKeyShiftCode() = 0;
    virtual void initLeftKeyShiftCode() = 0;
    virtual void initMiddleKeyShiftCode() = 0;
    virtual void initRightKeyShiftCode() = 0;
    virtual void initBottomKeyShiftCode() = 0;

    virtual void initLeftKeySymbolCode() = 0;
    virtual void initMiddleKeySymbolCode() = 0;
    virtual void initRightKeySymbolCode() = 0;
    virtual void initTopKeySymbolCode();

    OData * createData(const QString & value);
    OData * createBackspaceData();
    OData * createEnterData();
    OData * createSpaceData();

protected:
    // Use these data for initialization.
    ODatas top_codes_;
    ODatas left_codes_;
    ODatas middle_codes_;
    ODatas right_codes_;
    ODatas bottom_codes_;
    ODatas menu_codes_;

    // Use these data when shift is clicked.
    ODatas top_shift_codes_;
    ODatas left_shift_codes_;
    ODatas middle_shift_codes_;
    ODatas right_shift_codes_;
    ODatas bottom_shift_codes_;

    // Use these data when symbol is clicked.
    ODatas left_symbol_codes_;
    ODatas middle_symbol_codes_;
    ODatas right_symbol_codes_;
    ODatas top_symbol_codes_;
};

}   // namespace ui

#endif
