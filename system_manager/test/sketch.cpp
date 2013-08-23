

#include <stdlib.h>
#include <stdio.h>

#ifdef BUILD_FOR_ARM
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <fcntl.h>
#endif

#include <QtGui/QtGui>

#include "onyx/screen/screen_proxy.h"

struct Sample
{
    int x;
    int y;
    int pressure;
};
static Sample samp;

// This routine is a workaround.  We'd like to do this in mod_init - but because
// tslib opens the fd after module initialization, we can't.  So we set a flag
// to indicate it hasn't been called, and call it on the first read.
// The unfortunate side effect of this is that we can't indicate that the module
// did not load properly if there was a problem.  So we just have to keep
// responding as 'failed' to the data requests.
static int setup_fd(int fd)
{
#ifdef BUILD_FOR_ARM
    struct termios tios;
    int bytes_read=0;
    int bytes_needed=0;
    int offset = 0;
    int stored_offset = 0;
    int tries = 0;
    char s[12];
    char buffer[12];

#ifdef DEBUG
    fprintf(stderr,"Initializing fd for idsv4\n");
#endif

    // Let's lock the port - just so nobody messes up our tablet configuration
    // while we are running
    // Always unlock the fd, as it may be called by calibration.
    // fprintf(stderr, "always unlock fd.\n");
    flock(fd, LOCK_UN);

    if (flock(fd, (LOCK_EX|LOCK_NB)) == -1)
    {
        fprintf(stderr,"Unable to lock port: %s\n",strerror(errno));
        // Can not just return -1, as it may happen during mouse calibration.
        return -1;
    }

    if (tcgetattr (fd, &tios))
    {
        fprintf(stderr,"Failed to get port params: %s",strerror(errno));
        return -1;
    }

    tios.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    tios.c_oflag &= ~OPOST;
    tios.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    tios.c_cflag &= ~(CSIZE|PARENB);
    tios.c_cflag |= CS8|CLOCAL;
    tios.c_cflag &= ~(CSTOPB); /* 1 stop bit */
    tios.c_cflag &= ~(CSIZE); /* 8 data bits */
    tios.c_cflag |= CS8;
    tios.c_cflag &= ~(PARENB); /* no parity */
    tios.c_iflag |= IXOFF;        /* flow control XOff */
    tios.c_cc[VMIN] = 1;        /* vmin value */
    tios.c_cc[VTIME] = 0;        /* vtime value */

    cfsetispeed(&tios, B19200);
    cfsetospeed(&tios, B19200);

    if (tcsetattr (fd, TCSANOW, &tios))
    {
        fprintf(stderr, "Failed to set port params: %s", strerror(errno));
        return -1;
    }

    // Tell the wacom to stop sampling
    // write(fd, "0", 1);
    // printf("Stop sampling\n.");

    // sleep for 200ms for wacom to relax.
    // usleep(200000);

    // query the tablet.  It seems to want to tell us it's capabilities.
    // if we don't ask it it tells us in the first packet - which is no good,
    // and confuses everything... so we ask it.
    // write(fd, "1", 1);
    // printf("Enable sampling now\n.");
#endif

    return 1;
}


static int idsv4_read_and_decode_packet(int fd)
{
#ifdef BUILD_FOR_ARM
    unsigned char event_buffer[9];
    unsigned char s[9];
    int stored_offset = 0;
    int offset = 0;
    int bytes_needed = 9;
    int bytes_read = 0;
    unsigned int buttons = 0;

    memset(s, 0, 9);

    while(bytes_needed > 0)
    {
        offset = 0;
        bytes_read = read(fd, event_buffer, bytes_needed);
        if (bytes_read <= 0)
        {
#ifdef DEBUG
            if (bytes_read == -1)
            {
                fprintf(stderr, "fd %d tablet read error: %s\n", fd, strerror(errno));
            }
#endif
            return 0;
        }
        else
        {
            if (stored_offset == 0)
            {
                while ( offset < bytes_read)
                {
                    /* 0x20 - docs say yes.  Tablet says no.  0xc0 is the only one that works.
                    *  if (!((event_buffer[offset] & 0x20) && (event_buffer[offset] & 0x80))) {
                    */
                    if (!((event_buffer[offset] & 0xc0) && (event_buffer[offset] & 0x80)))
                    {
                        offset++;
                    }
                    else
                    {
                        memcpy(s, event_buffer+offset, bytes_read-offset);
                        stored_offset = bytes_read-offset;
                        bytes_needed -= stored_offset;
                        offset = bytes_read;
                    }
                }
            }
            else
            {
                memcpy(s+stored_offset, event_buffer, bytes_read);
                stored_offset += bytes_read;
                bytes_needed -= bytes_read;
            }
        }
    }

    int prox = (s[0] & 0x20);

    samp.y     = (s[1]<<9) | (s[2]<<2) | ((s[6]>>5)&0x03);
    samp.x     = (s[3]<<9) | (s[4]<<2) | ((s[6]>>3)&0x03);
    samp.pressure         = ((s[6]&0x07)<<7) | s[5];
    buttons = s[0] & 0x07;
    samp.x = samp.x * 6 / 89;
    samp.y = 800 - samp.y * 8 / 120;
//#ifdef DEBUG
    fprintf(stderr, "raw data %dx%d  pressure %d\n",
           samp.x, samp.y,  samp.pressure);
//#endif

#endif
    return 1;
}



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#ifdef BUILD_FOR_ARM
    int fd = open("/dev/ttymxc1", O_RDONLY);
    if (fd < 0)
    {
        qDebug("Could not open /dev/ttymxc1");
        return -1;
    }

    setup_fd(fd);

    Sample prev = {-1, -1, 0};
    onyx::screen::ScreenCommand cmd;
    while (1)
    {

        idsv4_read_and_decode_packet(fd);
        {
            if (prev.x < 0)
            {
                prev = samp;
            }

            if (prev.x != samp.x && prev.y != samp.y)
            {
                cmd.x1 = prev.x;
                cmd.y1 = prev.y;
                cmd.x2 = samp.x;
                cmd.y2 = samp.y;
                cmd.color = 0;
            }
            qDebug("draw line and update screen %d %d %d %d.", cmd.x1, cmd.y1, cmd.x2, cmd.y2);
            onyx::screen::ScreenProxy::instance().drawLine(cmd.x1, cmd.y1, cmd.x2, cmd.y2, 0, 2);
            onyx::screen::ScreenProxy::instance().updateScreen(onyx::screen::ScreenProxy::DW, onyx::screen::WAIT_NONE);
            prev = samp;
        }
    }

#endif

    return 0;
}



