
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "eink_keyboard_plugin.h"

/// Hardware button values. [KEY_BEGIN, KEY_END)
/// Only four keys.
/// Borrowed from get_button.c
#define B1_ADDR 0x40E00008
#define B1_PIN 17
#define B2_ADDR 0x40E00008
#define B2_PIN 19
#define B3_ADDR 0x40E00008
#define B3_PIN 18
#define B4_ADDR 0x40E00000
#define B4_PIN 20

#define USE_PAGE_NUMBER_FOR_MMAP 1
#define MAP_SIZE 4096
#define MAP_MASK ( MAP_SIZE - 1 )

/// Call stack of keyboard plugin
/// QFactoryLoader::QFactoryLoader
/// QKbdDriverFactory::create
/// QWSServer::openKeyboard
/// The plugin search path could be
/// /usr/local/Qt/lib/plugins/kbddrivers/sharedlib.so
/// So far, it's /opt/onyx/plugins/kbddrivers/libeinkkeyboard.so

EinkKeyboardHandlerPrivate::EinkKeyboardHandlerPrivate(EinkKeyboardHandler *parent,
                                                       const QString & dev_name)
: parent_(parent)
, timer_(0)
, device_(0)
, keyboard_state_(0)
, map_(0)
{
    // The device can also be read from EinkKeyboardDriverPlugin::create
    // Change it later.
    // the dev_name must be /dev/mem
    qDebug("device name %s", qPrintable(dev_name));
    device_ = open(dev_name.toAscii(), O_RDWR | O_SYNC);
#ifdef USE_PAGE_NUMBER_FOR_MMAP
    map_ = (char*)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, device_, B1_ADDR / MAP_MASK);
#else
    map_ = (char*)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, device_, B1_ADDR & ~MAP_MASK);
#endif

    connect(&timer_, SIGNAL(timeout(void)), this, SLOT(OnTimeout(void)));

    // not sure yet.
    timer_.start(100);
}

EinkKeyboardHandlerPrivate::~EinkKeyboardHandlerPrivate()
{
    timer_.stop();
    munmap(0,MAP_SIZE);
    close(device_);
}


void EinkKeyboardHandlerPrivate::OnTimeout(void)
{
    char *regaddr;
    unsigned int val;
    regaddr = map_ + (B1_ADDR & MAP_MASK);
    val = *(unsigned int *) regaddr;
    keyboard_state_ += ((~val >> B1_PIN) & 0x01);

    regaddr = map_ + (B2_ADDR & MAP_MASK);
    val = *(unsigned int *) regaddr;
    keyboard_state_ += ((~val >> B2_PIN) & 0x01)<<1;

    regaddr = map_ + (B3_ADDR & MAP_MASK);
    val = *(unsigned int *) regaddr;
    keyboard_state_ += ((~val >> B3_PIN) & 0x01)<<2;

    regaddr = map_ + (B4_ADDR & MAP_MASK);
    val = *(unsigned int *) regaddr;
    keyboard_state_ += ((~val >> B4_PIN) & 0x01)<<3;

    // The possible value could be 1, 2, 4, 8
    if (keyboard_state_ != 0)
    {
        qDebug("Key pressed %d", keyboard_state_);
    }
    switch (keyboard_state_)
    {
    case 1:
        {
            parent_->processKeyEvent(0, Qt::Key_PageDown, Qt::NoModifier, true, false);
            parent_->processKeyEvent(0, Qt::Key_PageDown, Qt::NoModifier, false, false);
        }
        break;
    case 2:
        {
            parent_->processKeyEvent(0, Qt::Key_PageUp, Qt::NoModifier, true, false);
            parent_->processKeyEvent(0, Qt::Key_PageUp, Qt::NoModifier, false, false);
        }
        break;
    case 4:
        {
            parent_->processKeyEvent(0, Qt::Key_F10, Qt::NoModifier, true, false);
            parent_->processKeyEvent(0, Qt::Key_F10, Qt::NoModifier, false, false);
        }
        break;
    case 8:
        {
            parent_->processKeyEvent(0, Qt::Key_Escape, Qt::NoModifier, true, false);
            parent_->processKeyEvent(0, Qt::Key_Escape, Qt::NoModifier, false, false);
        }
        break;
    default:
        break;
    }
    keyboard_state_ = 0;
}

EinkKeyboardHandler::EinkKeyboardHandler(const QString & device_name)
: receiver_(0)
{
    qDebug("EinkKeyboardHandler::EinkKeyboardHandler");
    receiver_ = new EinkKeyboardHandlerPrivate(this, device_name);
}

EinkKeyboardHandler::~EinkKeyboardHandler()
{
    delete receiver_;
}


void EinkKeyboardHandler::processKeyEvent(int unicode,
                                          int keycode,
                                          Qt::KeyboardModifiers modifiers,
                                          bool isPress,
                                          bool autoRepeat)
{
    QWSKeyboardHandler::processKeyEvent(unicode, keycode, modifiers, isPress, autoRepeat);
}

int EinkKeyboardHandler::transformDirKey(int )
{
    // TODO, add direction keys.
    return 0;
}

void EinkKeyboardHandler::beginAutoRepeat(int, int , Qt::KeyboardModifiers )
{
}

void EinkKeyboardHandler::endAutoRepeat()
{
}


EinkKeyboardDriverPlugin::EinkKeyboardDriverPlugin(QObject *)
{
}

EinkKeyboardDriverPlugin::~EinkKeyboardDriverPlugin()
{
}

/// Should return all supported drivers list.
const QString EinkKeyboardDriverPlugin::driver_name = QLatin1String("onyxkeyboard");
QStringList EinkKeyboardDriverPlugin::keys() const
{
    QStringList lst(driver_name);
    return lst;
}

QWSKeyboardHandler* EinkKeyboardDriverPlugin::create(const QString& driver, const QString &device)
{
    // check driver name and device name. if matched create our own keyboard handler
    // otherwise should return 0.
    if (driver == driver_name);
    {
        qDebug("create the keyboard");
        return new EinkKeyboardHandler(device);
    }
    return 0;
}


Q_EXPORT_PLUGIN2(dev_Eink_keyboard_plugin, EinkKeyboardDriverPlugin)




