#ifndef _LOG_H
#define _LOG_H

// this is the global switch for logging
// define ENABLE_LOG in your Makefile to enable logging
#ifdef ENABLE_LOG

#define LOGMSG(level, fmt, ...) \
    log_msg(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum
{
    LOG_ERROR,   // 0
    LOG_WARNING, // 1
    LOG_INFO,    // 2
} LogLevel;

void log_init();
void log_msg(LogLevel level, const char* src, int line, const char* fmt, ...);

#else
#define LOGMSG(level, fmt, ...)
#endif // ENABLE_LOG

#endif // _LOG_H
