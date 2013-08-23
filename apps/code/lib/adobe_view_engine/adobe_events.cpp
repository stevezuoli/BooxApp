#include "dp_all.h"
#include "adobe_events.h"

namespace adobe_view
{

class AdobeKeyboardEventPrivate : public dpdoc::KeyboardEvent
{
public:
    AdobeKeyboardEventPrivate(dpdoc::KeyboardEventType type,
                              const QString & key_identifier,
                              unsigned int modifiers,
                              dpdoc::KeyLocation key_location);
    virtual ~AdobeKeyboardEventPrivate();

    virtual int getEventKind() { return dpdoc::EK_KEYBOARD; }
    virtual int getEventType() { return type_; }
    virtual void reject() { rejected_ = true; }

    virtual dp::String getKeyIdentifier() { return key_identifier_; }
    virtual unsigned int getModifiers() { return modifiers_; }
    virtual int getKeyLocation() { return key_location_; }

private:
    dpdoc::KeyboardEventType    type_;
    dp::String                  key_identifier_;
    unsigned int                modifiers_;
    dpdoc::KeyLocation          key_location_;
    bool                        rejected_;
};

class AdobeMouseEventPrivate : public dpdoc::MouseEvent
{
public:
    AdobeMouseEventPrivate(dpdoc::MouseEventType type,
                           int button,
                           unsigned int modifiers,
                           int x,
                           int y);
    virtual ~AdobeMouseEventPrivate();

    virtual int getEventKind() { return dpdoc::EK_MOUSE; }
    virtual int getEventType() { return type_; }
    virtual void reject() { rejected_ = true; }

    virtual int getButton() { return button_; }
    virtual unsigned int getModifiers() { return modifiers_; }
    virtual int getX() { return x_; }
    virtual int getY() { return y_; }

    bool isRejected() { return rejected_; }

private:
    dpdoc::MouseEventType   type_;
    int                     button_;
    unsigned int            modifiers_;
    int                     x_;
    int                     y_;
    bool                    rejected_;
};

class AdobeTextEventPrivate : public dpdoc::TextEvent
{
public:
    AdobeTextEventPrivate(const QString & text);
    virtual ~AdobeTextEventPrivate();

    virtual int getEventKind() { return dpdoc::EK_TEXT; }
    virtual int getEventType() { return dpdoc::TEXT_INPUT; }
    virtual void reject() { rejected_ = true; }

    virtual dp::String getData();

private:
    dp::String data_;
    bool       rejected_;
};

AdobeKeyboardEventPrivate::AdobeKeyboardEventPrivate(dpdoc::KeyboardEventType type,
                                                     const QString & key_identifier,
                                                     unsigned int modifiers,
                                                     dpdoc::KeyLocation key_location)
    : type_(type)
    , key_identifier_(key_identifier.toUtf8().constData())
    , modifiers_(modifiers)
    , key_location_(key_location)
{
}

AdobeKeyboardEventPrivate::~AdobeKeyboardEventPrivate()
{
}

AdobeKeyboardEvent::AdobeKeyboardEvent(AdobeKeyboardEventType type,
                                       const QString & key_identifier,
                                       unsigned int modifiers,
                                       AdobeKeyLocation key_location)
    : private_(new AdobeKeyboardEventPrivate(static_cast<dpdoc::KeyboardEventType>(type),
                                             key_identifier,
                                             modifiers,
                                             static_cast<dpdoc::KeyLocation>(key_location)))
{
}

AdobeKeyboardEvent::~AdobeKeyboardEvent()
{
}

QString AdobeKeyboardEvent::getKeyIdentifier()
{
    return private_->getKeyIdentifier().utf8();
}

unsigned int AdobeKeyboardEvent::getModifiers()
{
    return private_->getModifiers();
}

int AdobeKeyboardEvent::getKeyLocation()
{
    return private_->getKeyLocation();
}

AdobeMouseEventPrivate::AdobeMouseEventPrivate(dpdoc::MouseEventType type,
                                               int button,
                                               unsigned int modifiers,
                                               int x,
                                               int y)
    : type_(type)
    , button_(button)
    , modifiers_(modifiers)
    , x_(x)
    , y_(y)
    , rejected_(false)
{
}

AdobeMouseEventPrivate::~AdobeMouseEventPrivate()
{
}

AdobeMouseEvent::AdobeMouseEvent(AdobeMouseEventType type,
                                 int button,
                                 unsigned int modifiers,
                                 int x,
                                 int y)
    : private_(new AdobeMouseEventPrivate(static_cast<dpdoc::MouseEventType>(type),
                                          button,
                                          modifiers,
                                          x,
                                          y))
{
}

AdobeMouseEvent::~AdobeMouseEvent()
{
}

int AdobeMouseEvent::getButton()
{
    return private_->getButton();
}

unsigned int AdobeMouseEvent::getModifiers()
{
    return private_->getModifiers();
}

int AdobeMouseEvent::getX()
{
    return private_->getX();
}

int AdobeMouseEvent::getY()
{
    return private_->getY();
}

AdobeTextEventPrivate::AdobeTextEventPrivate(const QString & text)
    : data_(text.toUtf8().constData())
{
}

AdobeTextEventPrivate::~AdobeTextEventPrivate()
{
}

dp::String AdobeTextEventPrivate::getData()
{
    return data_;
}

AdobeTextEvent::AdobeTextEvent(const QString & text)
: private_(new AdobeTextEventPrivate(text))
{
}

AdobeTextEvent::~AdobeTextEvent()
{
}

QString AdobeTextEvent::getData()
{
    return private_->getData().utf8();
}

}
