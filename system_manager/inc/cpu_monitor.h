
#ifndef ONYX_SYSTEM_CPU_MONITOR_H_
#define ONYX_SYSTEM_CPU_MONITOR_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/// CPU usage monitor. It can report CPU is busy or not in specified period
/// during two sampling points.
class CPUMonitor
{
public:
    CPUMonitor();
    ~CPUMonitor();

public:
    void sample();
    bool isIdle();

private:
    bool open();
    void close();
    bool state(int & user, int & nice, int & system, int & idle);
    bool isAudioBusy();

private:
    FILE* proc_fd_;
    int prev_user_;
    int prev_nice_;
    int prev_system_;
    int prev_idle_;
};

#endif  // ONYX_SYSTEM_CPU_MONITOR_H_
