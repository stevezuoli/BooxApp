#include "onyx/ui/keyboard_data.h"
#include "onyx/ui/onyx_keyboard.h"
#include "onyx/sys/sys.h"
#include "onyx/sys/platform.h"
#include "onyx/ui/languages.h"

namespace ui
{

const QString KeyboardData::BACKSPACE_TEXT = "backspace";
const QString KeyboardData::ENTER_TEXT = "enter";

KeyboardData::KeyboardData()
{
    ui::loadTranslator(QLocale::system().name());
    initTopKeySymbolCode();
    initMenuKeyCode();
}

KeyboardData::~KeyboardData()
{
    clearDatas(top_codes_);
    clearDatas(left_codes_);
    clearDatas(middle_codes_);
    clearDatas(right_codes_);
    clearDatas(bottom_codes_);
    clearDatas(menu_codes_);

    clearDatas(top_shift_codes_);
    clearDatas(left_shift_codes_);
    clearDatas(middle_shift_codes_);
    clearDatas(right_shift_codes_);
    clearDatas(bottom_shift_codes_);

    clearDatas(left_symbol_codes_);
    clearDatas(middle_symbol_codes_);
    clearDatas(right_symbol_codes_);
}

ODatas & KeyboardData::topCodes(bool shift, bool symbol)
{
    if (symbol)
    {
        return top_symbol_codes_;
    }
    return (!shift? top_codes_: top_shift_codes_);
}

ODatas & KeyboardData::leftCodes(bool shift)
{
    return (!shift? left_codes_: left_shift_codes_);
}

ODatas & KeyboardData::middleCodes(bool shift)
{
    return (!shift? middle_codes_: middle_shift_codes_);
}

ODatas & KeyboardData::rightCodes(bool shift)
{
    return (!shift? right_codes_: right_shift_codes_);
}

ODatas & KeyboardData::bottomCodes(bool shift)
{
    return (!shift? bottom_codes_: bottom_shift_codes_);
}

ODatas & KeyboardData::menuCodes()
{
    return menu_codes_;
}

ODatas & KeyboardData::leftSymbolCodes()
{
    return left_symbol_codes_;
}

ODatas & KeyboardData::middleSymbolCodes()
{
    return middle_symbol_codes_;
}

ODatas & KeyboardData::rightSymbolCodes()
{
    return right_symbol_codes_;
}

OData * KeyboardData::createData(const QString & value)
{
    OData * data = new OData;
    data->insert(TAG_TITLE, value);
    return data;
}

OData * KeyboardData::createBackspaceData()
{
    QPixmap backspace_pixmap(":/images/back_spac.png");
    OData * dd = new OData;
    dd->insert(TAG_COVER, backspace_pixmap);
    dd->insert(TAG_SPECIAL_KEY, Qt::Key_Backspace);
    dd->insert(TAG_SPECIAL_KEY_TEXT, BACKSPACE_TEXT);
    return dd;
}

OData * KeyboardData::createEnterData()
{
    QPixmap enter_pixmap(":/images/enter_key.png");
    OData * dd = new OData;
    dd->insert(TAG_COVER, enter_pixmap);
    dd->insert(TAG_SPECIAL_KEY, Qt::Key_Enter);
    dd->insert(TAG_SPECIAL_KEY_TEXT, ENTER_TEXT);
    return dd;
}

OData * KeyboardData::createSpaceData()
{
    QPixmap space_pixmap(":/images/space_key.png");
    OData * dd = new OData;
    dd->insert(TAG_COVER, space_pixmap);
    dd->insert(TAG_SPECIAL_KEY, Qt::Key_Space);
    dd->insert(TAG_SPECIAL_KEY_TEXT, " ");
    return dd;
}

void KeyboardData::initMenuKeyCode()
{
    ODataPtr dd(createData(QApplication::tr("Shift")));
    static const int MENU_FONT_SIZE = 20;
    dd->insert(TAG_MENU_TYPE, OnyxKeyboard::KEYBOARD_MENU_SHIFT);
    dd->insert(TAG_FONT_SIZE, MENU_FONT_SIZE);
    menu_codes_.push_back(dd);

    ODataPtr s(createData(QApplication::tr("Symbol")));
    s->insert(TAG_MENU_TYPE, OnyxKeyboard::KEYBOARD_MENU_SYMBOL);
    s->insert(TAG_FONT_SIZE, MENU_FONT_SIZE);
    menu_codes_.push_back(s);


    ODataPtr l(createData(QApplication::tr("Language")));
    l->insert(TAG_MENU_TYPE, OnyxKeyboard::KEYBOARD_MENU_LANGUAGE);
    l->insert(TAG_FONT_SIZE, MENU_FONT_SIZE);
    menu_codes_.push_back(l);

    if (SysStatus::instance().hasTouchScreen() && !sys::isAk98())
    {
        ODataPtr dd(createData(QApplication::tr("Write")));
        dd->insert(TAG_MENU_TYPE, OnyxKeyboard::KEYBOARD_MENU_WRITE);
        dd->insert(TAG_FONT_SIZE, MENU_FONT_SIZE);
        menu_codes_.push_back(dd);
    }
}

void KeyboardData::initTopKeySymbolCode()
{
    const QChar chs[] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};
    for (int i = 0; i < 10; i++)
    {
        ODataPtr dd(createData(QString(chs[i])));
        top_symbol_codes_.push_back(dd);
    }
}

}   // namespace ui
