#ifndef ONYX_KEYBOARD_DATA_FACTORY_H_
#define ONYX_KEYBOARD_DATA_FACTORY_H_

#include "ui_global.h"

namespace ui
{

class KeyboardData;

// The factory to build keyboard data for different locales.
//
class KeyboardDataFactory
{
public:
    KeyboardDataFactory();
    ~KeyboardDataFactory();

    bool registerKeyboardData(const QLocale & locale,
            KeyboardData *data);
    KeyboardData * getKeyboardData(const QLocale & locale);

private:
    QMap<QString, KeyboardData *> data_map_;
};

}   // namespace ui

#endif
