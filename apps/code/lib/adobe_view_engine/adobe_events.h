#ifndef ADOBE_EVENT_H_
#define ADOBE_EVENT_H_

#include "adobe_utils.h"

namespace adobe_view
{

class AdobeKeyboardEventPrivate;
class AdobeKeyboardEvent
{
public:
    AdobeKeyboardEvent(AdobeKeyboardEventType type,
                       const QString & key_identifier,
                       unsigned int modifiers,
                       AdobeKeyLocation key_location);
    ~AdobeKeyboardEvent();

    QString getKeyIdentifier();
    unsigned int getModifiers();
    int getKeyLocation();

    AdobeKeyboardEventPrivate* realEvent() { return private_.get(); }
private:
    scoped_ptr<AdobeKeyboardEventPrivate> private_;
};

class AdobeMouseEventPrivate;
class AdobeMouseEvent
{
public:
    AdobeMouseEvent(AdobeMouseEventType type,
                    int button,
                    unsigned int modifiers,
                    int x,
                    int y);
    ~AdobeMouseEvent();

    int getButton();
    unsigned int getModifiers();
    int getX();
    int getY();

    AdobeMouseEventPrivate* realEvent() { return private_.get(); }
private:
    scoped_ptr<AdobeMouseEventPrivate> private_;
};

class AdobeTextEventPrivate;
class AdobeTextEvent
{
public:
    AdobeTextEvent(const QString & text);
    ~AdobeTextEvent();

    QString getData();
    AdobeTextEventPrivate* realEvent() { return private_.get(); }
private:
    scoped_ptr<AdobeTextEventPrivate> private_;
};

};

#endif
