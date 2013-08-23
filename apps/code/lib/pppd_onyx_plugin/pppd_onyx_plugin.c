#include <pppd/pppd.h>

char pppd_version[] = VERSION;

static void ip_up_cb (void *data, int arg)
{
    system("/usr/bin/pppd_notify.sh ip-up");
}

static void exit_cb (void *data, int arg)
{
    char cmd[64];
    if (arg != EXIT_OK)
    {
        sprintf(cmd, "/usr/bin/pppd_notify.sh pppd-exit %d", arg);
        system(cmd);
    }
}

int plugin_init (void)
{
    add_notifier (&ip_up_notifier, ip_up_cb, NULL);
    add_notifier (&exitnotify, exit_cb, NULL);
    return 0;
}
