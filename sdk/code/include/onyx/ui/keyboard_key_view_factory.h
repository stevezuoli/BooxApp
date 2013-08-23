#ifndef ONYX_KEYBOARD_KEY_FACTORY_H_
#define ONYX_KEYBOARD_KEY_FACTORY_H_

#include "onyx/ui/factory.h"

namespace ui
{

class KeyBoardKeyViewFactory : public Factory
{
public:
    KeyBoardKeyViewFactory();
    ~KeyBoardKeyViewFactory();

public:
    virtual ContentView * createView(QWidget *parent, const QString &type);

};

}   // namespace ui

#endif
