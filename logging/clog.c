#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "clog.h"

#ifdef ENABLE_LOG

// only log errors by default
static LogLevel g_log_level = LOG_ERROR;

static FILE* g_log_fp = NULL;

void log_init()
{
    const char* p = getenv("LOG_LEVEL");
    if (p)
    {
        g_log_level = atoi(p);
        
        if (g_log_level < LOG_ERROR)
        {
            g_log_level = LOG_ERROR;
        }
        else if (g_log_level > LOG_INFO)
        {
            g_log_level = LOG_INFO;
        }
    }

    // output to stdout by default
    g_log_fp = stdout;

    p = getenv("LOG_FILE");
    if (p)
    {
        FILE *fp = fopen(p, "a");
        if (fp)
        {
            // set file stream line buffered
            // the buffer will be flushed to file once and only it contains a newline indicator (\n)
            setlinebuf(fp);
            g_log_fp = fp;
        }
    }
}

void log_msg(LogLevel level, const char* src, int line, const char* fmt, ...)
{
    if (level <= g_log_level)
    {
        flockfile(g_log_fp);

        va_list args;
        va_start(args, fmt);

        fprintf(g_log_fp, "%s:%d ", src, line);
        vfprintf(g_log_fp, fmt, args);
        fprintf(g_log_fp, "\n");

        va_end(args);

        funlockfile(g_log_fp);
    }
}

#endif
