// Author: Hong Jiang <hjiang@dev-gems.com>
//
// Adapted from Petru Marginean's Dr. Dobb's Journal article "Logging
// in C++"

/// This file provides a thread-safe and portable logging utility.

#ifndef LOGGING_LOG_H__
#define LOGGING_LOG_H__

#include "stdlib.h"

#include <sstream>
#include <string>
#include <stdio.h>

inline std::string NowTime();

// In debug mode L_DFATAL terminates the program. In release mode, it is
// the same as L_ERROR.
enum LogLevel { L_DFATAL,
                L_ERROR,
                L_WARNING,
                L_INFO,
                L_DEBUG,
                L_DEBUG1,
                L_DEBUG2,
                L_DEBUG3,
                L_DEBUG4};

template <typename T>
class Log
{
  public:
    Log();
    virtual ~Log();
    std::ostringstream& Get(LogLevel level = L_INFO);
  public:
    static LogLevel& ReportingLevel();
    static std::string ToString(LogLevel level);
    static LogLevel FromString(const std::string& level);
  protected:
    std::ostringstream os_;
  private:
    Log(const Log&);
    Log& operator =(const Log&);
    LogLevel current_level_;
};

template <typename T>
Log<T>::Log()
        : os_(),
          current_level_(L_DEBUG)
{
}

template <typename T>
Log<T>::~Log()
{
    os_ << std::endl;
    T::Output(os_.str());
    if (L_DFATAL == current_level_) {
        exit(EXIT_FAILURE);
    }
}

template <typename T>
std::ostringstream& Log<T>::Get(LogLevel level)
{
    current_level_ = level;
    os_ << "- " << NowTime();
    os_ << " " << ToString(level) << ": ";
    os_ << std::string(level > L_DEBUG ? level - L_DEBUG : 0, '\t');
    return os_;
}

template <typename T>
LogLevel& Log<T>::ReportingLevel()
{
    static LogLevel reportingLevel = L_DEBUG4;
    return reportingLevel;
}

template <typename T>
std::string Log<T>::ToString(LogLevel level)
{
    static const char* const buffer[] =
        {"FATAL",
         "ERROR",
         "WARNING",
         "INFO",
         "DEBUG",
         "DEBUG1",
         "DEBUG2",
         "DEBUG3",
         "DEBUG4"};
    return buffer[level];
}

template <typename T>
LogLevel Log<T>::FromString(const std::string& level)
{
    if (level == "DEBUG4")
        return L_DEBUG4;
    if (level == "DEBUG3")
        return L_DEBUG3;
    if (level == "DEBUG2")
        return L_DEBUG2;
    if (level == "DEBUG1")
        return L_DEBUG1;
    if (level == "DEBUG")
        return L_DEBUG;
    if (level == "INFO")
        return L_INFO;
    if (level == "WARNING")
        return L_WARNING;
    if (level == "ERROR")
        return L_ERROR;
    if (level == "DFATAL")  // This should never match
        return L_DFATAL;
    Log<T>().Get(L_WARNING) << "Unknown logging level '"
                            << level << "'. Using INFO level as default.";
    return L_INFO;
}

class Output2FILE
{
  public:
    static FILE*& Stream();
    static void Output(const std::string& msg);
};

inline FILE*& Output2FILE::Stream()
{
    static FILE* pStream = stderr;
    return pStream;
}

inline void Output2FILE::Output(const std::string& msg)
{
    FILE* pStream = Stream();
    if (!pStream)
        return;
    fprintf(pStream, "%s", msg.c_str());
    fflush(pStream);
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   if defined (BUILDING_FILELOG_DLL)
#       define FILELOG_DECLSPEC   __declspec (dllexport)
#   elif defined (USING_FILELOG_DLL)
#       define FILELOG_DECLSPEC   __declspec (dllimport)
#   else
#       define FILELOG_DECLSPEC
#   endif // BUILDING_DBSIMPLE_DLL
#else
#   define FILELOG_DECLSPEC
#endif // _WIN32

class FILELOG_DECLSPEC FILELog : public Log<Output2FILE> {};
//typedef Log<Output2FILE> FILELog;

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL L_DEBUG4
#endif

#define LOG(level)                                                      \
    if (level > FILELOG_MAX_LEVEL) ;                                    \
    else if (level > FILELog::ReportingLevel() || !Output2FILE::Stream()) ; \
    else FILELog().Get(level)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include <windows.h>

inline std::string NowTime()
{
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0,
                       "HH':'mm':'ss", buffer, MAX_LEN) == 0)
        return "Error in NowTime()";

    char result[100] = {0};
    static DWORD first = GetTickCount();
    sprintf_s(result, "%s.%03ld", buffer,
        (long)(GetTickCount() - first) % 1000);
    return result;
}

#else

#include <sys/time.h>

inline std::string NowTime()
{
    char buffer[64];
    time_t t;
    time(&t);
    tm r;
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    sprintf(result, "%s.%03ld", buffer,
                 static_cast<long>(tv.tv_usec) / 1000);
    return result;
}

#endif  // WIN32

#endif  // LOGGING_LOG_H__
