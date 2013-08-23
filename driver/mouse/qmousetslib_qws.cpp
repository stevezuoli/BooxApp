/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. 
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmousetslib_qws.h"

#if !defined(QT_NO_QWS_MOUSE_TSLIB) || defined(QT_PLUGIN)

#include "qsocketnotifier.h"
#include "qscreen_qws.h"

// Replace the tslib to improve performance and reduce method call.
// It also make the whole release more simple as tslib contains lot of
// plugins which can be integrated in one file.
//#include <tslib.h>

#define INSTATUS_SYNC            0xA0       //A0 or 80 
#define INSTATUS_SYNC_HW         0xC0       //A0 or 80 
#define INSTATUS_SYNC_WA         0x80
#define D6                       0x40
#define D5                       0x20
#define D0                       0x01
#define INSTATUS_RESERVED        0x18
#define INWDATA_SYNC             0x8080
#define MAX_SAMPLES              4
#define BUFFER_SIZE              256



#include <cstdlib>
#include <ctime>
#include <cstdio>   /* Standard input/output definitions */
#include <cstring>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <errno.h>

QT_BEGIN_NAMESPACE

#ifndef QT_QWS_TP_JITTER_LIMIT
#define QT_QWS_TP_JITTER_LIMIT 3
#endif

// Global type declaration
static const int ARRAY_SIZE = 11;
static unsigned char data_arr[ARRAY_SIZE];


/// Open touch screen port.
static int ts_open(const char *dev)
{
    int fd;
    fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd == -1)
    {
        perror("\nopen_port: Unable to open %s", dev);
    }
    return fd;
}

/// Config the touch screen.
static void ts_config(int fd)
{
    struct termios options;
    tcgetattr(fd, &options);                // Get the current options for the port...
    cfsetispeed(&options, B19200);          // Set the baud rates to 19200...
    cfsetospeed(&options, B19200);
    options.c_cflag |= (CLOCAL | CREAD);    // Enable the receiver and set local mode...
    cfmakeraw( &options );

    /*
    options.c_cflag &= ~PARENB;             //Set Parity Checking to 8N1
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;            // Disable hardware flow control
    options.c_cc[VMIN]  = 0;                                //one second timeout
    options.c_cc[VTIME] = 10;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);    // Set Local Options to Raw
    */

    options.c_iflag &= ~(IXON | IXOFF | IXANY);            //Disable Software flow control
    tcsetattr(fd, TCSANOW, &options);                     //Set the new options for the port...
}

/// For wacom sensor. Borrowed from eink am300.
static bool enable_device(int fd, bool enable = true)
{
    int start = -1;
    if (enable)
    {
        start = write(fd,"1", 1);
    }
    else
    {
        start = write(fd,"0", 1);
    }

    if (start < 0)
    {
        perror("write() of 1 failed!\n");
    }
    else 
    {
        usleep(10000);
    }
    return (start >= 0);
}


// Gumstix only, maybe removed later.
static void gpio_setup(void)
{
    int i, j, filedes, filedes_1;

    filedes = open("/proc/gpio/GPIO46", O_RDWR);              //setting GPIO46 for output
    i = write(filedes,"AF2 in", 6);
    if (i < 0)
    {
        perror("write() of 6 bytes failed!\n");
    }

    filedes_1 = open("/proc/gpio/GPIO47", O_RDWR);           //setting GPIO47 for input
    j = write(filedes_1,"AF1 out", 7);
    if (j < 0)
    {
        perror("write() of 7 bytes failed!\n");
    }
}

static int read_first_byte(int fd)
{
    // set the read function to normal blocking if no chars are ready
    fcntl(fd, F_SETFL, FNDELAY);
    return read(fd,&data_arr[0], 1);
}

static bool evaluate_first_byte(const )
{
    return (data_arr[0]& INSTATUS_SYNC_HW);
}

static int evaluate_wacom_first_byte(void)
{
    return (data_arr[0]& INSTATUS_SYNC_WA);
}

static int read_tts_data(int fd)
{
    // Set the read function to return immediately if no chars are ready.
    fcntl(fd, F_SETFL, FNDELAY);
    return read(fd,&data_arr[1], 6);
}

static int read_full_packet(int fd)
{
    // Set the read function to return immediately if no chars are ready.
    fcntl(fd, F_SETFL, FNDELAY);
    return read(fd,&data_arr[1], 7);
}

static int read_wacom_data(int fd)
{
    int r = 0;

    // First find the sync byte.
    do
    {
        r = read( fd, &data_arr[0], 1 );
    }
    while ( !( r==1 && (data_arr[0] & INSTATUS_SYNC_WA)));

    // Sync byte found, read next 8.
    do
    {
        r += read( fd, &data_arr[r], 9-r );
    }
    while ( r < 9 );
    return r;
}

static int read_wacom_query_data(int fd)
{
    // Set the read function to return immediately if no chars are ready
    fcntl(fd, F_SETFL,FNDELAY);
    return read(fd,data_arr_ptr, 11); 

}

static int flush_data(int fd)
{
    return tcflush(fd, TCIFLUSH);
}

/*!
    \internal

    \class QWSTslibMouseHandler
    \ingroup qws

    \brief The QWSTslibMouseHandler class implements a mouse driver
    for the Universal Touch Screen Library, tslib.

    QWSTslibMouseHandler inherits the QWSCalibratedMouseHandler class,
    providing calibration and noise reduction functionality in
    addition to generating mouse events, for devices using the
    Universal Touch Screen Library.

    To be able to compile this mouse handler, \l{Qt for Embedded Linux}
    must be configured with the \c -qt-mouse-tslib option, see the
    \l{Pointer Handling} documentation for details. In addition, the tslib
    headers and library must be present in the build environment.  The
    tslib sources can be downloaded from \l
    {http://tslib.berlios.de/}.  Use the \c -L and \c -I options
    with \c configure to explicitly specify the location of the
    library and its headers:

    \snippet doc/src/snippets/code/src.gui.embedded.qmousetslib_qws.cpp 0

    In order to use this mouse handler, tslib must also be correctly
    installed on the target machine. This includes providing a \c
    ts.conf configuration file and setting the necessary environment
    variables, see the README file provided with tslib for details.

    The ts.conf file will usually contain the following two lines

    \snippet doc/src/snippets/code/src.gui.embedded.qmousetslib_qws.cpp 1

    To make \l{Qt for Embedded Linux} explicitly choose the tslib mouse
    handler, set the QWS_MOUSE_PROTO environment variable.

    \sa {Pointer Handling}, {Qt for Embedded Linux}
*/

class QWSTslibMouseHandlerPrivate : public QObject
{
    Q_OBJECT
public:
    QWSTslibMouseHandlerPrivate(QWSTslibMouseHandler *h,
                                const QString &device);
    ~QWSTslibMouseHandlerPrivate();

    void suspend();
    void resume();

    void calibrate(const QWSPointerCalibrationData *data);
    void clearCalibration();

private:
    QWSTslibMouseHandler *handler;
    struct tsdev *dev;
    QSocketNotifier *mouseNotifier;

    struct ts_sample lastSample;
    bool wasPressed;
    int lastdx;
    int lastdy;

    bool calibrated;
    QString devName;

    void open();
    void close();
    inline bool get_sample(struct ts_sample *sample);

private slots:
    void readMouseData();
};

QWSTslibMouseHandlerPrivate::QWSTslibMouseHandlerPrivate(QWSTslibMouseHandler *h,
                                                         const QString &device)
    : handler(h)
{
    devName = device;

    if (devName.isNull()) {
        const char *str = getenv("TSLIB_TSDEVICE");
        if (str)
            devName = QString::fromLocal8Bit(str);
    }
    if (devName.isNull())
        devName = QLatin1String("/dev/ts");

    open();
    calibrated = true;

    int fd = ts_fd(dev);
    mouseNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
    resume();
}

QWSTslibMouseHandlerPrivate::~QWSTslibMouseHandlerPrivate()
{
    close();
}

void QWSTslibMouseHandlerPrivate::open()
{
    dev = ts_open(devName.toLocal8Bit().constData(), 1);
    if (!dev) {
        qCritical("QWSTslibMouseHandlerPrivate: ts_open() failed"
                  " with error: '%s'", strerror(errno));
        qCritical("Please check your tslib installation!");
        return;
    }

    if (ts_config(dev)) {
        qCritical("QWSTslibMouseHandlerPrivate: ts_config() failed"
                  " with error: '%s'", strerror(errno));
        qCritical("Please check your tslib installation!");
        close();
        return;
    }
}

void QWSTslibMouseHandlerPrivate::close()
{
    if (dev)
        ts_close(dev);
}

void QWSTslibMouseHandlerPrivate::suspend()
{
    mouseNotifier->setEnabled(false);
}

void QWSTslibMouseHandlerPrivate::resume()
{
    memset(&lastSample, 0, sizeof(lastSample));
    wasPressed = false;
    lastdx = 0;
    lastdy = 0;
    mouseNotifier->setEnabled(true);
}

bool QWSTslibMouseHandlerPrivate::get_sample(struct ts_sample *sample)
{
    if (!calibrated)
    {
        return (ts_read_raw(dev, sample, 1) == 1);
    }
    return (ts_read(dev, sample, 1) == 1);
}

void QWSTslibMouseHandlerPrivate::readMouseData()
{
    if(!qt_screen)
        return;

    for(;;) {
        struct ts_sample sample = lastSample;
        bool pressed = wasPressed;

        // Fast return if there's no events.
        if (!get_sample(&sample))
        {
            return;
        }
        pressed = (sample.pressure > 0);

        // Only return last sample unless there's a press/release event.
        while (pressed == wasPressed) {
            if (!get_sample(&sample))
                break;
            pressed = (sample.pressure > 0);
        }

        // work around missing coordinates on mouse release in raw mode
        if (!calibrated && !pressed && sample.x == 0 && sample.y == 0) {
            sample.x = lastSample.x;
            sample.y = lastSample.y;
        }

        int dx = sample.x - lastSample.x;
        int dy = sample.y - lastSample.y;

        // Remove small movements in oppsite direction
        if (dx * lastdx < 0 && qAbs(dx) < QT_QWS_TP_JITTER_LIMIT) {
            sample.x = lastSample.x;
            dx = 0;
        }
        if (dy * lastdy < 0 && qAbs(dy) < QT_QWS_TP_JITTER_LIMIT) {
            sample.y = lastSample.y;
            dy = 0;
        }

        if (wasPressed == pressed && dx == 0 && dy == 0)
        {
            printf("no moved!\n");
            return;
        }

#ifdef TSLIBMOUSEHANDLER_DEBUG
        qDebug() << "last" << QPoint(lastSample.x, lastSample.y)
                 << "curr" << QPoint(sample.x, sample.y)
                 << "dx,dy" << QPoint(dx, dy)
                 << "ddx,ddy" << QPoint(dx*lastdx, dy*lastdy)
                 << "pressed" << wasPressed << pressed;
#endif
        printf("last %d %d\n", lastSample.x, lastSample.y);
        printf("curr %d %d\n", sample.x, sample.y);
        printf("dx dy %d %d\n", dx, dy);
        printf("ddx ddy %d %d\n", dx*lastdx, dy*lastdy);
        printf("pressed %d %d\n", wasPressed, pressed);



        lastSample = sample;
        wasPressed = pressed;
        if (dx != 0)
            lastdx = dx;
        if (dy != 0)
            lastdy = dy;

        // tslib should do all the translation and filtering, so we send a
        // "raw" mouse event
        handler->QWSMouseHandler::mouseChanged(QPoint(sample.x, sample.y),
                                               pressed);
    }
}

void QWSTslibMouseHandlerPrivate::clearCalibration()
{
    suspend();
    close();
    handler->QWSCalibratedMouseHandler::clearCalibration();
    calibrated = false;
    open();
    resume();
}

void QWSTslibMouseHandlerPrivate::calibrate(const QWSPointerCalibrationData *data)
{
    suspend();
    close();
    // default implementation writes to /etc/pointercal
    // using the same format as the tslib linear module.
    handler->QWSCalibratedMouseHandler::calibrate(data);
    calibrated = true;
    open();
    resume();
}

/*!
    \internal
*/
QWSTslibMouseHandler::QWSTslibMouseHandler(const QString &driver,
                                           const QString &device)
    : QWSCalibratedMouseHandler(driver, device)
{
    d = new QWSTslibMouseHandlerPrivate(this, device);
}

/*!
    \internal
*/
QWSTslibMouseHandler::~QWSTslibMouseHandler()
{
    delete d;
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::suspend()
{
    d->suspend();
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::resume()
{
    d->resume();
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::clearCalibration()
{
    d->clearCalibration();
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::calibrate(const QWSPointerCalibrationData *data)
{
    d->calibrate(data);
}

QT_END_NAMESPACE

#include "qmousetslib_qws.moc"

#endif //QT_NO_QWS_MOUSE_TSLIB
