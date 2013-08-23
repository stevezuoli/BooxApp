#ifndef ONYX_TOUCH_EVENT_DATA_H_
#define ONYX_TOUCH_EVENT_DATA_H_


#include <QtCore/QtCore>

/// When using 4.5, we need to use struct, for 4.6 and above, it would
/// be not necessary.
struct OnyxTouchPoint
{
    int x;
    int y;
    int width;
    int height;
    int pressure;
    int buttons;
};

/// So far, supports two points only.
struct TouchData
{
    OnyxTouchPoint points[2];
};

static const QString TOUCH_SERVER_ADDRESS = "/tmp/onyx/touch";


#endif
