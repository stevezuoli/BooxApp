

#include "cpu_monitor.h"
#include <QtCore/QtCore>


CPUMonitor::CPUMonitor()
: proc_fd_(0)
{
}

CPUMonitor::~CPUMonitor()
{
}

/// Record current state.
void CPUMonitor::sample()
{
    state(prev_user_, prev_nice_, prev_system_, prev_idle_);
}

/// Re-sample and check if it's idle during last period.
bool CPUMonitor::isIdle()
{
    int user, nice, system, idle;
    if (!state(user, nice, system, idle))
    {
        return false;
    }

    // qDebug("now %d %d %d %d", user, nice, system, idle);
    // qDebug("pre %d %d %d %d", prev_user_, prev_nice_, prev_system_, prev_idle_);

    int usage = (user - prev_user_) + (system - prev_system_) + (nice - prev_nice_);
    int total = usage + (idle - prev_idle_);
    int percentage = usage * 100 / total;

    // qDebug("usage %d", percentage);
    prev_user_ = user;
    prev_nice_ = nice;
    prev_system_ = system;
    prev_idle_ = idle;

    if ((percentage <= 0 && usage <= 0) && !isAudioBusy())
    {
        return true;
    }
    return false;
}

bool CPUMonitor::state(int & user, int & nice, int & system, int & idle)
{
    proc_fd_ = fopen("/proc/stat", "r");
    if (proc_fd_ == 0)
    {
        qWarning("Can't open /proc/stat!");
        return false;
    }

    rewind(proc_fd_);
    fscanf(proc_fd_, "%*s %d %d %d %d", &user, &nice, &system, &idle);
    fclose(proc_fd_);
    proc_fd_ = 0;
    return true;
}

bool CPUMonitor::isAudioBusy()
{
    FILE *audio_proc = fopen("/proc/driver/wm8711", "r");
    if (audio_proc == 0)
    {
        qWarning("Can't open /proc/driver/wm8711!");
        return false;
    }

    int busy = 0;
    char ignore[30] = {0};
    rewind(audio_proc);
    fscanf(audio_proc, "%s %s %d", &ignore[0], &ignore[0], &busy);
    fclose(audio_proc);
    if (busy)
    {
        qDebug("audio device is busy.");
        return true;
    }
    return false;
}




