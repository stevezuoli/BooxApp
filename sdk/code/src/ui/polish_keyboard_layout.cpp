#include "onyx/ui/polish_keyboard_layout.h"

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"

namespace ui
{

PolishKeyboardLayout::PolishKeyboardLayout()
: KeyboardLayout()
{
    initCode();
    initShiftMap();
    initSpecialMaps();
    initKeys();
}

PolishKeyboardLayout::~PolishKeyboardLayout()
{
}

QSize PolishKeyboardLayout::getKeySize(int code)
{
    const int space = 4;

    if (!standard_key_size_.isValid())
    {
#ifndef Q_WS_QWS
        QSize desktop_size = QSize(600, 800);
#else
        QSize desktop_size = qApp->desktop()->screenGeometry().size();
#endif
        int keyboard_width = std::min(desktop_size.width(), desktop_size.height());
        int key_length = (keyboard_width - 4 * (keys_[0].size() + 1) - 20) / keys_[0].size();
        standard_key_size_ = QSize(key_length, key_length);
    }

    QChar ch(code);
    if (ch.isNumber())
    {
        return standard_key_size_;
    }

    if (code < BSCode)
    {
        return standard_key_size_;
    }

    if (code > UnknownCode)
    {
        return standard_key_size_;
    }

    if (code == Blank)
    {
        bool has_touch = SysStatus::instance().hasTouchScreen();
        int len = standard_key_size_.width() * ( has_touch ? 2 : 3 );
        return QSize( len + ( space << ( has_touch ? 0 : 1 ) ), standard_key_size_.height() );
    }

    if (code == EnterCode)
    {
        return QSize( ( standard_key_size_.width() << 1 ) + space , standard_key_size_.height() );
    }

    if (code == BackSpace)
    {
        return standard_key_size_;
    }

    if (code == BackSlash)
    {
        return standard_key_size_;
    }

    if (code == DeleteCode)
    {
        return standard_key_size_;
    }

    if (code == ShiftCode)
    {
        return standard_key_size_;
    }

    if (code == HandWriting)
    {
        return standard_key_size_;
    }

    if (code == SwitchLanguage)
    {
        return standard_key_size_;
    }

    if (code == CapLock)
    {
        return standard_key_size_;
    }

    return standard_key_size_;
}

void PolishKeyboardLayout::initCode()
{
    code_vector_.clear();
    code_vector_.push_back(QChar('a'));     // 0
    code_vector_.push_back(QChar('b'));     // 1
    code_vector_.push_back(QChar('c'));     // 2
    code_vector_.push_back(QChar('d'));     // 3
    code_vector_.push_back(QChar('e'));     // 4
    code_vector_.push_back(QChar('f'));     // 5
    code_vector_.push_back(QChar('g'));     // 6
    code_vector_.push_back(QChar('h'));     // 7
    code_vector_.push_back(QChar('i'));     // 8
    code_vector_.push_back(QChar('j'));     // 9
    code_vector_.push_back(QChar('k'));     // 10
    code_vector_.push_back(QChar('l'));     // 11
    code_vector_.push_back(QChar('m'));     // 12
    code_vector_.push_back(QChar('n'));     // 13
    code_vector_.push_back(QChar('o'));     // 14
    code_vector_.push_back(QChar('p'));     // 15
    code_vector_.push_back(QChar('q'));     // 16
    code_vector_.push_back(QChar('r'));     // 17
    code_vector_.push_back(QChar('s'));     // 18
    code_vector_.push_back(QChar('t'));     // 19
    code_vector_.push_back(QChar('u'));     // 20
    code_vector_.push_back(QChar('v'));     // 21
    code_vector_.push_back(QChar('w'));     // 22
    code_vector_.push_back(QChar('x'));     // 23
    code_vector_.push_back(QChar('y'));     // 24
    code_vector_.push_back(QChar('z'));     // 25
    code_vector_.push_back(QChar(0x00D3));   //O acute  // 26
    code_vector_.push_back(QChar(0x00F3));   //o acute  // 27
    code_vector_.push_back(QChar(0x0104));  //A ogonek   // 28
    code_vector_.push_back(QChar(0x0105));  //a ogonek   // 29
    code_vector_.push_back(QChar(0x0106));  //C acute   // 30
    code_vector_.push_back(QChar(0x0107));  //c acute  // 31
    code_vector_.push_back(QChar(0x0118));  //E ogonek  // 32
    code_vector_.push_back(QChar(0x0119));  //e ogonek   // 33
    code_vector_.push_back(QChar(0x0143));  //N acute   // 34
    code_vector_.push_back(QChar(0x0144));  //n acute   // 35
    code_vector_.push_back(QChar(0x015A));  //S acute   // 36
    code_vector_.push_back(QChar(0x015B));  //s acute   // 37
    code_vector_.push_back(QChar(0x0179));  //Z acute  // 38
    code_vector_.push_back(QChar(0x017A));  //z acute   // 39
    code_vector_.push_back(QChar(0x017B));  //Z dot above   // 40
    code_vector_.push_back(QChar(0x017C));  //z dot above   // 41
    code_vector_.push_back(QChar(0x0141));  //L stroke  // 42
    code_vector_.push_back(QChar(0x0142));  //l stroke  // 43
    code_vector_.push_back(QChar('.'));  //dot  // 44
}

void PolishKeyboardLayout::initKeys()
{
    qDebug() << Q_FUNC_INFO;
    // resize keys
    keys_.clear();
    keys_.resize(5);
    int index = 0;

    // row 0
    keys_[0].resize(11);
    setKey(0, index++, '1');
    setKey(0, index++, '2');
    setKey(0, index++, '3');
    setKey(0, index++, '4');
    setKey(0, index++, '5');
    setKey(0, index++, '6');
    setKey(0, index++, '7');
    setKey(0, index++, '8');
    setKey(0, index++, '9');
    setKey(0, index++, '0');
    setKey(0, index++, code_vector_[27]);

    index = 0;
    keys_[1].resize(11);
    setKey(1, index++, code_vector_[16]);
    setKey(1, index++, code_vector_[22]);
    setKey(1, index++, code_vector_[4]);
    setKey(1, index++, code_vector_[17]);
    setKey(1, index++, code_vector_[19]);
    setKey(1, index++, code_vector_[24]);
    setKey(1, index++, code_vector_[20]);
    setKey(1, index++, code_vector_[8]);
    setKey(1, index++, code_vector_[14]);
    setKey(1, index++, code_vector_[15]);
    setKey(1, index++, code_vector_[31]);

    index = 0;
    keys_[2].resize(11);
    setKey(2, index++, code_vector_[0]);
    setKey(2, index++, code_vector_[18]);
    setKey(2, index++, code_vector_[3]);
    setKey(2, index++, code_vector_[5]);
    setKey(2, index++, code_vector_[6]);
    setKey(2, index++, code_vector_[7]);
    setKey(2, index++, code_vector_[9]);
    setKey(2, index++, code_vector_[10]);
    setKey(2, index++, code_vector_[11]);
    setKey(2, index++, code_vector_[12]);
    setKey(2, index++, code_vector_[35]);

    index = 0;
    keys_[3].resize(11);
    setKeyUnicode(3, index++, CapLock);
    setKey(3, index++, code_vector_[25]);
    setKey(3, index++, code_vector_[23]);
    setKey(3, index++, code_vector_[2]);
    setKey(3, index++, code_vector_[21]);
    setKey(3, index++, code_vector_[1]);
    setKey(3, index++, code_vector_[13]);
    setKey(3, index++, BackSpace);
    setKey(3, index++, code_vector_[43]);
    setKey(3, index++, code_vector_[29]);//0
    setKey(3, index++, code_vector_[33]);//1

    index = 0;
    bool has_touch = SysStatus::instance().hasTouchScreen();
    keys_[4].resize(has_touch ? 9 : 8);
    setKeyUnicode(4, index++, ShiftCode);
    setKey(4, index++, code_vector_[44]);
    setKeyUnicode(4, index++, Blank);
    setKeyUnicode(4, index++, EnterCode);
    setKeyUnicode(4, index++, SwitchLanguage);
    if (has_touch)
    {
        setKeyUnicode(4, index++, HandWriting);
    }
    setKey(4, index++, code_vector_[39]);
    setKey(4, index++, code_vector_[41]);
    setKey(4, index++, code_vector_[37]);//2
}

void PolishKeyboardLayout::initShiftMap()
{
    qDebug() << Q_FUNC_INFO;
    assert(!code_vector_.isEmpty());

    shift_map_.clear();
    shift_map_['1'] = '!';
    shift_map_['2'] = '@';
    shift_map_['3'] = '#';
    shift_map_['4'] = '$';
    shift_map_['5'] = '%';
    shift_map_['6'] = '^';
    shift_map_['7'] = QChar(0x0026);
    shift_map_['8'] = '*';
    shift_map_['9'] = '(';
    shift_map_['0'] = ')';
    shift_map_[code_vector_[27]] = '_';
    shift_map_[code_vector_[29]] = QChar(0x00B1); 

    shift_map_[code_vector_[16]] = '~';
    shift_map_[code_vector_[22]] = QChar(0x0060);
    shift_map_[code_vector_[4]] = QChar(0x221A);
    shift_map_[code_vector_[17]] = '-';
    shift_map_[code_vector_[19]] = '+';
    shift_map_[code_vector_[24]] = '=';
    shift_map_[code_vector_[20]] = '{';
    shift_map_[code_vector_[8]] = '}';
    shift_map_[code_vector_[14]] = '[';
    shift_map_[code_vector_[15]] = ']';
    shift_map_[code_vector_[31]] = QChar(0x00B5);
    shift_map_[code_vector_[33]] = QChar(0x00AE);

    shift_map_[code_vector_[0]] = ':';
    shift_map_[code_vector_[18]] = ';';
    shift_map_[code_vector_[3]] = '"';
    shift_map_[code_vector_[5]] = '\'';
    shift_map_[code_vector_[6]] = '<';
    shift_map_[code_vector_[7]] = '>';
    shift_map_[code_vector_[9]] = ',';
    shift_map_[code_vector_[10]] = '|';
    shift_map_[code_vector_[11]] = '?';
    shift_map_[code_vector_[12]] = QChar(0x00A9);
    shift_map_[code_vector_[35]] = QChar(0x00BD);
    shift_map_[code_vector_[37]] = QChar(0x201E);

    shift_map_[code_vector_[25]] = '\\';
    shift_map_[code_vector_[23]] = QChar(0x2243);
    shift_map_[code_vector_[2]] = '/';
    shift_map_[code_vector_[21]] = QChar(0x00A3);
    shift_map_[code_vector_[1]] = QChar(0x00F7);
    shift_map_[code_vector_[13]] = QChar(0x2030);
    shift_map_[code_vector_[43]] = QChar(0x2122);

    shift_map_[code_vector_[39]] = QChar(0x20AC);
    shift_map_[code_vector_[41]] = QChar(0x00B0);
}

}
