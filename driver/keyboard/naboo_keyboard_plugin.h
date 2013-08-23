/// Author John

#ifndef NABOO_KEYBOARD_PLUGIN_H_
#define NABOO_KEYBOARD_PLUGIN_H_

#include <QKbdDriverPlugin>
#include <qkbd_qws.h>
#include <qsocketnotifier.h>
#include <QtCore/QtCore>


struct KeyMapEntry
{
    int linux_key;
    Qt::Key qt_key;
    Qt::Key qt_long_press_key;
    int flags;
};


/// Implement the keyboard handler. Works for Qtopia core only.
/// It receives the keyboard event from the specified input device.
/// The raw event is translated into pre-defined key and sent to
/// qtopia message queue.
class NabooKeyboardHandler : public QObject
                           , public QWSKeyboardHandler
{
    Q_OBJECT
public:
    NabooKeyboardHandler(const QString & device_name);
    ~NabooKeyboardHandler();

    void processKeyEvent(int unicode, int keycode,
                         Qt::KeyboardModifiers modifiers,
                         bool isPress, bool autoRepeat);


private Q_SLOTS:
    void onReceivedKeyboardEvent(int);
    void onLongPressTimeout();
    void myAutoRepeat();

private:
    void startAutoRepeat();
    void stopAutoRepeat();

private:
    QSocketNotifier *notify_;
    int device_;
    KeyMapEntry current_key_;
    QTimer long_press_timer_;
    QTimer auto_repeat_timer_;
};


/// Implement the keyboard driver plugin a shared library.
/// NabooKeyboardDriverPlugin -> NabooKeyboardHandler
class NabooKeyboardDriverPlugin : public QKbdDriverPlugin
{
public:
    explicit NabooKeyboardDriverPlugin(QObject *parent = 0);
    ~NabooKeyboardDriverPlugin();

public:
    QStringList keys() const;
    QWSKeyboardHandler* create(const QString& driver, const QString &device);

private:
    static const QString driver_name;
};

#endif
