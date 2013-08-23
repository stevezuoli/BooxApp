#ifndef WEB_KEY_GENERATOR_H_
#define WEB_KEY_GENERATOR_H_

#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "onyx/ui/ui.h"

using namespace ui;

namespace webapp
{

class KeyGen
{
public:
    KeyGen();
    ~KeyGen();

public:
    enum Complexity
    {
        SIMPLE = 2,
        COMPLEX = 4
    };
    QString generateKey(int length, Complexity cpl);

private:
    QChar generateChar(Complexity cpl);

private:
    QStringList candidates;
};

};   // namespace webapp

#endif
