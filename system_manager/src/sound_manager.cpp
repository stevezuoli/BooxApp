
#include "sound_manager.h"
#include "onyx/sys/sys_conf.h"

using namespace sys;

SoundManager::SoundManager()
{
    init();
}

void SoundManager::init()
{
    // Read conf from sys conf. Does not need to emit signal here
    // as all listeners are not created yet.
    SystemConfig conf;
    volume_ = conf.volume();
    volumes_ = conf.volumes();
    mute_ = conf.isMute();

    int i = index(volume_);
    if (i < 0)
    {
        volume_ = volumes_.at(1);
    }
}

int SoundManager::index(int volume)
{
    for(int i = 0; i < volumes_.size(); ++i)
    {
        if (volumes_.at(i) == volume)
        {
            return i;
        }
    }
    return -1;
}

SoundManager::~SoundManager()
{
    SystemConfig conf;
    conf.setVolume(volume_);
    conf.mute(mute_);
}

int SoundManager::volume()
{
    return volume_;
}

/// Should check it's mute or not.
bool SoundManager::setVolume(int volume)
{
    if (mute_ || volume == volume_)
    {
        return false;
    }

    // Broadcast signal.
    volume_ = volume;
    emit volumeChanged(volume_, false);
    return true;
}

bool SoundManager::increaseVolume()
{
    int now = index(volume_) + 1;
    if (now >= volumes_.size() || now < 0 || mute_)
    {
        return false;
    }

    volume_ = volumes_.at(now);
    emit volumeChanged(volume_, false);
    return true;
}

bool SoundManager::decreaseVolume()
{
    int now = index(volume_) - 1;
    if (now < 0 || now >= volumes_.size() || mute_)
    {
        return false;
    }

    volume_ = volumes_.at(now);
    emit volumeChanged(volume_, false);
    return true;
}

/// Mute the device, but does not change the volume stored in
/// the database, so that we can restore the volume later.
bool SoundManager::mute(bool m)
{
    if (mute_ == m)
    {
        return false;
    }

    mute_ = m;
    emit volumeChanged(volume_, mute_);
    return true;
}

/// Check if it's mute now.
bool SoundManager::isMute()
{
    return mute_;
}

