
#ifndef ONYX_DATA_KEYS_H_
#define ONYX_DATA_KEYS_H_

namespace onyx
{

namespace data
{

static const int KEY_MENU =       0x01;
static const int KEY_ESC =        0x02;
static const int KEY_PAGEUP =     0x04;
static const int KEY_PAGEDOWN =   0x08;

static const int ENABLE_ALL = (KEY_MENU | KEY_ESC | KEY_PAGEUP | KEY_PAGEDOWN);
static const int ENABLE_MENU_ESC = (KEY_MENU | KEY_ESC);
static const int ENABLE_PAGE_UP_DOWN = (KEY_PAGEUP | KEY_PAGEDOWN);

static const int DISABLE_MENU_ESC = (~KEY_MENU) & (~KEY_ESC);
static const int DISABLE_PAGE_UP_DOWN = (~KEY_PAGEUP) & (~KEY_PAGEDOWN);

static const int DISABLE_ALL = 0;

}   // namespace data

}   // namespace onyx

#endif

