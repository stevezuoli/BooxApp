
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/input.h>
#include "naboo_keyboard_plugin.h"
#include <QtCore/QtCore>
#include <QtGui/qscreen_qws.h>

static const int TIMEOUT = 1000;        ///< TODO, if necessary, we can make it configurable.
static const int AUTOREPEAT = 600;        ///< TODO, if necessary, we can make it configurable.
static const int INVALID_KEY = -1;

static const int FLAG_EMPTY     = 0x0;
static const int FLAG_LONG_PRESS = 0x1;
static const int FLAG_AUTO_REPEAT = 0x2;


/// Define the map from Linux key value to key defined by Qt.
static const KeyMapEntry map_normal[] =
{
    {KEY_MENU,      Qt::Key_Menu,           Qt::Key_OpenUrl,    FLAG_LONG_PRESS},
    {KEY_ESC,       Qt::Key_Escape,         Qt::Key_Home,       FLAG_LONG_PRESS},
    {KEY_ENTER,     Qt::Key_Return,         Qt::Key_Stop,       FLAG_LONG_PRESS},
    {KEY_PAGEUP,    Qt::Key_PageUp,         Qt::Key_unknown,    FLAG_EMPTY},
    {KEY_PAGEDOWN,  Qt::Key_PageDown,       Qt::Key_unknown,    FLAG_EMPTY},

    {KEY_UP,        Qt::Key_Up,             Qt::Key_Up,         FLAG_AUTO_REPEAT},
    {KEY_DOWN,      Qt::Key_Down,           Qt::Key_Down,       FLAG_AUTO_REPEAT},
    {KEY_LEFT,      Qt::Key_Left,           Qt::Key_Left,       FLAG_AUTO_REPEAT},
    {KEY_RIGHT,     Qt::Key_Right,          Qt::Key_Right,      FLAG_AUTO_REPEAT},

    {KEY_VOLUMEUP,      Qt::Key_VolumeUp,   Qt::Key_Print,      FLAG_LONG_PRESS},
    {KEY_VOLUMEDOWN,    Qt::Key_VolumeDown, Qt::Key_Print,      FLAG_LONG_PRESS},
    {KEY_F21,           Qt::Key_F21,        Qt::Key_unknown,    FLAG_EMPTY},         // USB connection
    {KEY_F22,           Qt::Key_F22,        Qt::Key_unknown,    FLAG_EMPTY},         // Wifi
    {KEY_F23,           Qt::Key_F23,        Qt::Key_unknown,    FLAG_AUTO_REPEAT},   // Touch screen key event, not stylus switch.
    {KEY_F24,           Qt::Key_F24,        Qt::Key_unknown,    FLAG_EMPTY},         // TODO, change it later, stylus switch.
    {KEY_POWER,         Qt::Key_F35,        Qt::Key_Sleep,      FLAG_LONG_PRESS},    // See qwindowsystem_qws.cpp. F35 will never be blocked.
    {KEY_F20,           Qt::Key_F20,        Qt::Key_unknown,    FLAG_EMPTY},         // 3G power switch
};
static const int KEY_COUNT = sizeof(map_normal) / sizeof(map_normal[0]);

static const KeyMapEntry map_rotate_90[] =
{
    {KEY_MENU,      Qt::Key_Menu,           Qt::Key_OpenUrl,    FLAG_LONG_PRESS},
    {KEY_ESC,       Qt::Key_Escape,         Qt::Key_Home,       FLAG_LONG_PRESS},
    {KEY_ENTER,     Qt::Key_Return,         Qt::Key_Stop,       FLAG_LONG_PRESS},
    {KEY_PAGEUP,    Qt::Key_PageUp,         Qt::Key_unknown,    FLAG_EMPTY},
    {KEY_PAGEDOWN,  Qt::Key_PageDown,       Qt::Key_unknown,    FLAG_EMPTY},

    {KEY_UP,        Qt::Key_Right,          Qt::Key_Right,      FLAG_AUTO_REPEAT},
    {KEY_DOWN,      Qt::Key_Left,           Qt::Key_Left,       FLAG_AUTO_REPEAT},
    {KEY_LEFT,      Qt::Key_Up,             Qt::Key_Up,         FLAG_AUTO_REPEAT},
    {KEY_RIGHT,     Qt::Key_Down,           Qt::Key_Down,       FLAG_AUTO_REPEAT},

    {KEY_VOLUMEUP,      Qt::Key_VolumeUp,   Qt::Key_Print,      FLAG_LONG_PRESS},
    {KEY_VOLUMEDOWN,    Qt::Key_VolumeDown, Qt::Key_Print,      FLAG_LONG_PRESS},
    {KEY_F21,           Qt::Key_F21,        Qt::Key_unknown,    FLAG_EMPTY},         // USB connection
    {KEY_F22,           Qt::Key_F22,        Qt::Key_unknown,    FLAG_EMPTY},         // Wifi
    {KEY_F23,           Qt::Key_F23,        Qt::Key_unknown,    FLAG_AUTO_REPEAT},   // Touch screen key event, not stylus switch.
    {KEY_F24,           Qt::Key_F24,        Qt::Key_unknown,    FLAG_EMPTY},         // TODO, change it later, stylus switch.
    {KEY_POWER,         Qt::Key_F35,        Qt::Key_Sleep,      FLAG_LONG_PRESS},    // See qwindowsystem_qws.cpp. F35 will never be blocked.
    {KEY_F20,           Qt::Key_F20,        Qt::Key_unknown,    FLAG_EMPTY},         // 3G power switch
};



static const KeyMapEntry map_rotate_270[] =
{
    {KEY_MENU,      Qt::Key_Menu,           Qt::Key_OpenUrl,    FLAG_LONG_PRESS},
    {KEY_ESC,       Qt::Key_Escape,         Qt::Key_Home,       FLAG_LONG_PRESS},
    {KEY_ENTER,     Qt::Key_Return,         Qt::Key_Stop,       FLAG_LONG_PRESS},
    {KEY_PAGEUP,    Qt::Key_PageUp,         Qt::Key_unknown,    FLAG_EMPTY},
    {KEY_PAGEDOWN,  Qt::Key_PageDown,       Qt::Key_unknown,    FLAG_EMPTY},

    {KEY_UP,        Qt::Key_Left,           Qt::Key_Left,       FLAG_AUTO_REPEAT},
    {KEY_DOWN,      Qt::Key_Right,          Qt::Key_Right,      FLAG_AUTO_REPEAT},
    {KEY_LEFT,      Qt::Key_Down,           Qt::Key_Down,       FLAG_AUTO_REPEAT},
    {KEY_RIGHT,     Qt::Key_Up,             Qt::Key_Up,         FLAG_AUTO_REPEAT},

    {KEY_VOLUMEUP,      Qt::Key_VolumeUp,   Qt::Key_Print,      FLAG_LONG_PRESS},
    {KEY_VOLUMEDOWN,    Qt::Key_VolumeDown, Qt::Key_Print,      FLAG_LONG_PRESS},
    {KEY_F21,           Qt::Key_F21,        Qt::Key_unknown,    FLAG_EMPTY},         // USB connection
    {KEY_F22,           Qt::Key_F22,        Qt::Key_unknown,    FLAG_EMPTY},         // Wifi
    {KEY_F23,           Qt::Key_F23,        Qt::Key_unknown,    FLAG_AUTO_REPEAT},   // Touch screen key event, not stylus switch.
    {KEY_F24,           Qt::Key_F24,        Qt::Key_unknown,    FLAG_EMPTY},         // TODO, change it later, stylus switch.
    {KEY_POWER,         Qt::Key_F35,        Qt::Key_Sleep,      FLAG_LONG_PRESS},    // See qwindowsystem_qws.cpp. F35 will never be blocked.
    {KEY_F20,           Qt::Key_F20,        Qt::Key_unknown,    FLAG_EMPTY},         // 3G power switch
};
// Call stack of keyboard plugin
// QFactoryLoader::QFactoryLoader
// QKbdDriverFactory::create
// QWSServer::openKeyboard
// The plugin search path could be
// /usr/local/Qt/lib/plugins/kbddrivers/sharedlib.so
NabooKeyboardHandler::NabooKeyboardHandler(const QString & device_name)
: notify_(0)
{
    // qDebug("NabooKeyboardHandler created!");

    device_ = open(device_name.toAscii(), O_RDONLY);
    if (device_ < 0)
    {
        qWarning("Could not open device %s", qPrintable(device_name));
        return;
    }

    notify_ = new QSocketNotifier(device_, QSocketNotifier::Read);
    connect(notify_, SIGNAL(activated(int)), this, SLOT(onReceivedKeyboardEvent(int)));

    long_press_timer_.setSingleShot(true);
    connect(&long_press_timer_, SIGNAL(timeout()), this, SLOT(onLongPressTimeout()));

    auto_repeat_timer_.setSingleShot(true);
    connect(&auto_repeat_timer_, SIGNAL(timeout()), this, SLOT(myAutoRepeat()));
}

NabooKeyboardHandler::~NabooKeyboardHandler()
{
    close(device_);
    delete notify_;
}

void NabooKeyboardHandler::processKeyEvent(int unicode,
                                        int keycode,
                                        Qt::KeyboardModifiers modifiers,
                                        bool isPress,
                                        bool autoRepeat)
{
    QWSKeyboardHandler::processKeyEvent(unicode, keycode, modifiers, isPress, autoRepeat);
}

void NabooKeyboardHandler::onReceivedKeyboardEvent(int)
{
    // qDebug("Received keyboard event");
    unsigned char buff[20];
    struct input_event *key_event = 0;

    int j = read(device_, buff, 20);
    if (j < 0)
    {
        qDebug("Read error from device.");
        return;
    }

    const KeyMapEntry * map = &map_normal[0];
    int degree = QScreen::instance()->transformOrientation() * 90;
    if (degree == 90)
    {
        map = &map_rotate_90[0];
    }
    else if (degree == 270)
    {
         map = &map_rotate_270[0];
    }

    key_event = reinterpret_cast<input_event *>(&buff[0]);
    if (key_event->type == EV_KEY)
    {
        // qDebug("Raw key from linux %d", key_event->code);
        for(int i = 0; i < KEY_COUNT; ++i)
        {
            if (key_event->code == map[i].linux_key)
            {
                if (key_event->value)
                {
                    // Send pressed event to system.
                    current_key_ = map[i];
                    processKeyEvent(0, current_key_.qt_key, Qt::NoModifier, true, false);
                    long_press_timer_.start(TIMEOUT);
                }
                else
                {
                    // If long pressed can not be handled, send the release event.
                    if (!(map[i].flags & FLAG_LONG_PRESS) || current_key_.linux_key != INVALID_KEY)
                    {
                        processKeyEvent(0, map[i].qt_key, Qt::NoModifier, false, false);
                    }

                    // Stop.
                    current_key_.linux_key = INVALID_KEY;
                    long_press_timer_.stop();
                    stopAutoRepeat();
                }
                return;
            }
        }
        qDebug("Ignored the unknown key.");
    }
}

// Long press detected.
void NabooKeyboardHandler::onLongPressTimeout()
{
    // qDebug("Long press detected.");
    if (current_key_.linux_key == INVALID_KEY)
    {
        return;
    }

    if (current_key_.flags & FLAG_AUTO_REPEAT)
    {
        startAutoRepeat();
    }
    else
    {
        // Add a flag so that the release event will not be sent.
        if (current_key_.qt_long_press_key != Qt::Key_unknown)
        {
            processKeyEvent(0, current_key_.qt_long_press_key, Qt::NoModifier, false, false);
            current_key_.linux_key = INVALID_KEY;
        }
    }
}

void NabooKeyboardHandler::startAutoRepeat()
{
    auto_repeat_timer_.start(AUTOREPEAT);
}

void NabooKeyboardHandler::stopAutoRepeat()
{
    auto_repeat_timer_.stop();
}

// There is no way to change default delay time, so have to create our own.
void NabooKeyboardHandler::myAutoRepeat()
{
    // qDebug("autoRepeat %d", current_key_.qt_key);
    processKeyEvent(0, current_key_.qt_key, Qt::NoModifier, false, true);
    processKeyEvent(0, current_key_.qt_key, Qt::NoModifier, true, true);
    auto_repeat_timer_.start(AUTOREPEAT);
}

/// Implement the plugin shared library.
NabooKeyboardDriverPlugin::NabooKeyboardDriverPlugin(QObject *)
{
    // qDebug("NabooKeyboardDriverPlugin created!");
}

NabooKeyboardDriverPlugin::~NabooKeyboardDriverPlugin()
{
}

/// Should return all supported drivers list.
const QString NabooKeyboardDriverPlugin::driver_name = QLatin1String("NabooKeyboard");
QStringList NabooKeyboardDriverPlugin::keys() const
{
    QStringList lst(driver_name);
    return lst;
}

QWSKeyboardHandler* NabooKeyboardDriverPlugin::create(const QString& driver, const QString &device)
{
    // check driver name and device name. if matched create our own keyboard handler
    // otherwise should return 0.
    if (driver == driver_name);
    {
        return new NabooKeyboardHandler(device);
    }
    return 0;
}


Q_EXPORT_PLUGIN2(NabooKeyboard, NabooKeyboardDriverPlugin)




