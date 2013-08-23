#include "onyx/ui/keyboard_key_view_factory.h"
#include "onyx/ui/keyboard_key_view.h"

namespace ui
{

KeyBoardKeyViewFactory::KeyBoardKeyViewFactory()
    : Factory()
{

}

KeyBoardKeyViewFactory::~KeyBoardKeyViewFactory()
{

}

ContentView * KeyBoardKeyViewFactory::createView(QWidget *parent,
        const QString &type)
{
    return new KeyboardKeyView(parent);
}

}   // namespace ui
