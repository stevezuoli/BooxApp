/// Author John
/// Qtopia based Eink keyboard for AM300.

#ifndef EINK_KEYBOARD_PLUGIN_H_
#define EINK_KEYBOARD_PLUGIN_H_

#include <QKbdDriverPlugin>
#include <qkbd_qws.h>
#include <qsocketnotifier.h>
#include <QTimer>

class EinkKeyboardHandler;
class EinkKeyboardHandlerPrivate : public QObject
{
    Q_OBJECT
public:
    EinkKeyboardHandlerPrivate(EinkKeyboardHandler * parent, const QString & dev_name);
    ~EinkKeyboardHandlerPrivate();

private slots:
    void OnTimeout(void);

private:
    EinkKeyboardHandler *parent_;
    QTimer timer_;
    int device_;
    unsigned char keyboard_state_;
    char *map_;
};

/// Implement eink keyboard handler. Only valid for Qtopia core version.
class EinkKeyboardHandler : public QWSKeyboardHandler
{
public:
    EinkKeyboardHandler(const QString & device_name);
    ~EinkKeyboardHandler();

    void processKeyEvent(int unicode, int keycode,
                         Qt::KeyboardModifiers modifiers,
                         bool isPress, bool autoRepeat);

protected:
    int transformDirKey(int key);
    void beginAutoRepeat(int uni, int code, Qt::KeyboardModifiers mod);
    void endAutoRepeat();

private:
    EinkKeyboardHandlerPrivate * receiver_;
};


/// Implement eink keyboard driver plugin.
/// EinkKeyboardDriverPlugin -> EinkKeyboardHandler
class EinkKeyboardDriverPlugin : public QKbdDriverPlugin
{
public:
    explicit EinkKeyboardDriverPlugin(QObject *parent = 0);
    ~EinkKeyboardDriverPlugin();

public:
    QStringList keys() const;
    QWSKeyboardHandler* create(const QString& driver, const QString &device);

private:
    static const QString driver_name;
};

#endif  // EINK_KEYBOARD_PLUGIN_H_

