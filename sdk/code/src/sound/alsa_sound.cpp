#include <stdlib.h>
#include <stdio.h>

#ifndef _WINDOWS
#include <unistd.h>
#endif

#include "onyx/sound/alsa_sound.h"

AlsaSound::AlsaSound()
    :
#ifdef BUILD_WITH_TFT
    mixer_(0)
    , pcm_element_(0)
    , pcm_handle_(0),
#endif
    enable_(true)
    , bps_(8)
    , channels_(2)
    , samplerate_(0)
    , byte_per_frames_(0)
    , audio_data_per_ms_(1)
    , conv2stereo_(false)
    , stereo_buff_len_(0)
    , stereo_buff_(0)
{
#ifdef BUILD_WITH_TFT
    openPCMHandler();
    if (openMixer())
    {
        snd_mixer_selem_set_playback_volume_range(pcm_element_, 0, 100);
    }
#endif
}

AlsaSound::~AlsaSound()
{
    closePCMHandler();
    closeMixer();
}

int AlsaSound::volume()
{
#ifdef BUILD_WITH_TFT
    long ll = 0, lr = 0;
    snd_mixer_handle_events(mixer_);
    snd_mixer_selem_get_playback_volume(pcm_element_,
                                        SND_MIXER_SCHN_FRONT_LEFT, &ll);
    snd_mixer_selem_get_playback_volume(pcm_element_,
                                        SND_MIXER_SCHN_FRONT_RIGHT, &lr);
    return (ll + lr) >> 1;
#endif
    return 0;
}

void setPCMVolume(snd_mixer_elem_t *pcm_element, snd_mixer_selem_channel_id_t chn, int volume)
{
    if (snd_mixer_selem_has_playback_channel(pcm_element, chn))
    {
        snd_mixer_selem_set_playback_volume(pcm_element, chn, volume);
    }
}

bool AlsaSound::setVolume(int volume)
{
    if (volume < 0)
    {
        volume = 0;
    }

    static int limit = -1;
    if (limit <= 0)
    {
        limit = qgetenv("VOLUME_LIMIT").toInt();
        if (limit <= 0)
        {
            limit = 100;
        }
    }

    if (volume > limit)
    {
        volume = limit;
    }

#ifdef BUILD_WITH_TFT
    qDebug("AlsaSound set volume:%d", volume);
    //snd_mixer_selem_set_playback_volume_all(pcm_element_, volume);

    long min = 0, max = 100;
    snd_mixer_selem_get_playback_volume_range(pcm_element_, &min, &max);
    qDebug("Min vol:%d, Max vol:%d", min, max);

    setPCMVolume(pcm_element_, SND_MIXER_SCHN_FRONT_LEFT, volume);
    setPCMVolume(pcm_element_, SND_MIXER_SCHN_FRONT_RIGHT, volume);
    setPCMVolume(pcm_element_, SND_MIXER_SCHN_REAR_LEFT, volume);
    setPCMVolume(pcm_element_, SND_MIXER_SCHN_REAR_RIGHT, volume);
    setPCMVolume(pcm_element_, SND_MIXER_SCHN_FRONT_CENTER, volume);
    setPCMVolume(pcm_element_, SND_MIXER_SCHN_WOOFER, volume);
    setPCMVolume(pcm_element_, SND_MIXER_SCHN_SIDE_LEFT, volume);
    setPCMVolume(pcm_element_, SND_MIXER_SCHN_SIDE_RIGHT, volume);
    setPCMVolume(pcm_element_, SND_MIXER_SCHN_REAR_CENTER, volume);
    setPCMVolume(pcm_element_, SND_MIXER_SCHN_LAST, volume);
#endif
    return true;
}

bool AlsaSound::setBitsPerSample(int bps)
{
    bps_ = bps;
    return true;
}

bool AlsaSound::setChannels(int channels)
{
    channels_ = channels;
    return true;
}

bool AlsaSound::setSamplingRate(int rate)
{
    samplerate_ = rate;
    return true;
}

bool AlsaSound::updateParameters()
{
    return setParams(bps_, channels_, samplerate_);
}

bool AlsaSound::setRec()
{
    return false;
}

bool AlsaSound::setParams(unsigned int bitspersample, unsigned int channels, unsigned int samplerate)
{
#ifdef BUILD_WITH_TFT
    snd_pcm_format_t format;
    int rc;
    if (bitspersample == 8)
    {
        format = SND_PCM_FORMAT_S8;
    }
    else if (bitspersample == 16)
    {
        format = SND_PCM_FORMAT_S16;
    }
    else
    {
        return false;
    }

    if (snd_pcm_set_params (pcm_handle_, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                            channels, samplerate, 0, 500*1000) < 0)
    {
        qDebug("snd_pcm_set_params fails!!");
        if (channels == 1)
        {
            qDebug("channels = 1, reset params.");
            if ((rc = snd_pcm_set_params (pcm_handle_, format, SND_PCM_ACCESS_RW_INTERLEAVED, 2, samplerate, 0, 500*1000)) < 0)
            {
                fprintf(stderr, "Error setting PCM params %s.\n", snd_strerror(rc));
                return false;
            }
            conv2stereo_ = true;
        }
        else
        {
            return false;
        }
    }
    fprintf(stderr, "set parameters %d %d %d.\n", bitspersample, channels, samplerate);
    audio_data_per_ms_ = (samplerate * channels * bitspersample / 8) / 1000;
    byte_per_frames_ = channels * bitspersample / 8;
#endif
    return true;
}

bool AlsaSound::play(unsigned char *data, int size)
{
#ifdef BUILD_WITH_TFT
    static const unsigned int MAX_VOLUME_FOR_AK98 = 0xa186;
    bool ret;
    unsigned long frames = size / byte_per_frames_;

    if (conv2stereo_)
    {
        if (stereo_buff_len_ < size * 2)
        {
            /* enalarge stereo_buff */
            if (stereo_buff_)
            {
                free (stereo_buff_);
                stereo_buff_len_ = 0;
            }

            stereo_buff_ = (unsigned char *)malloc (size * 2);
            if (stereo_buff_ == NULL)
            {
                return false;
            }
            stereo_buff_len_ = size * 2;
        }

        if (mono2Stereo (data, frames, stereo_buff_) < 0)
        {
            return false;
        }
    }

    while (frames > 0)
    {
        int rc = snd_pcm_writei (pcm_handle_, data, frames);
        if (rc == -EAGAIN)
            continue;

        if (rc < 0)
        {
            if (snd_pcm_recover(pcm_handle_, rc, 0) < 0)
            {
                return false;
            }
            break;
        }

        if (conv2stereo_)
            data += rc * byte_per_frames_ * 2;
        else
            data += rc * byte_per_frames_;
        frames -= rc;
    }
#endif
    return true;
}

bool AlsaSound::openPCMHandler()
{
    if (pcm_handle_ != 0)
    {
        qDebug("PCM Handler has already been opened.");
        return true;
    }

    // open handler
    if (snd_pcm_open(&pcm_handle_, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        qDebug("Could not initial ALSA sound system.\n");
        return false;
    }
    return true;
}

void AlsaSound::closePCMHandler()
{
    snd_pcm_drain(pcm_handle_);
    snd_pcm_close(pcm_handle_);
    pcm_handle_  = 0;
    if (stereo_buff_)
    {
        free (stereo_buff_);
        stereo_buff_len_ = 0;
    }
}

bool AlsaSound::openMixer()
{
#ifdef BUILD_WITH_TFT
    int sts = snd_mixer_open(&mixer_, 0);
    if (sts != 0)
    {
        qDebug("snd_mixer_open failed; %d\n", sts);
        return false;
    }

    if (mixer_)
    {
        sts = snd_mixer_attach(mixer_, "default");
        if (sts != 0)
        {
            qDebug("snd_mixer_attach: failed; %d\n", sts);
            return false;
        }
        sts = snd_mixer_selem_register(mixer_, NULL, NULL);
        if (sts != 0)
        {
            qDebug("snd_mixer_selem_register: failed; %d\n", sts);
            return false;
        }
        sts = snd_mixer_load(mixer_);
        if (sts != 0)
        {
            qDebug("snd_mixer_selem_register: failed; %d\n", sts);
            return false;
        }

        pcm_element_ = snd_mixer_first_elem(mixer_);  
        while (pcm_element_ != 0)
        {
            QString element_name = snd_mixer_selem_get_name(pcm_element_);
            qDebug("Element Name:%s", qPrintable(element_name));
            if (element_name.startsWith("Headphone"))
            {
                //Anyka onyx supports headphone volume adjustment now, update this function later
                qDebug("Select element:%s", qPrintable(element_name));
                return true;
            }
	    pcm_element_ = snd_mixer_elem_next(pcm_element_);
        }
        qDebug("Cannot find Headphone");
    }
#endif
    return false;
}

void AlsaSound::closeMixer()
{
#ifdef BUILD_WITH_TFT
    snd_mixer_close(mixer_);
#endif
}

int AlsaSound::mono2Stereo(const unsigned char* mono_buf,
                           long unsigned int frames,
                           unsigned char* stereo_buf)
{
    int i = 0, j =0;

    switch (byte_per_frames_)
    {
    case 1:
        for (; frames > 0; frames--)
        {
            stereo_buf[i++] = mono_buf[j];
            stereo_buf[i++] = mono_buf[j];
            j++;
        }
        break;
    case 2:
        for (; frames > 0; frames--)
        {
            stereo_buf[i++] = mono_buf[j];
            stereo_buf[i++] = mono_buf[j+1];
            stereo_buf[i++] = mono_buf[j];
            stereo_buf[i++] = mono_buf[j+1];
            j += 2;
        }
        break;
    case 3:
        for (; frames > 0; frames--)
        {
            stereo_buf[i++] = mono_buf[j];
            stereo_buf[i++] = mono_buf[j+1];
            stereo_buf[i++] = mono_buf[j+2];
            stereo_buf[i++] = mono_buf[j];
            stereo_buf[i++] = mono_buf[j+1];
            stereo_buf[i++] = mono_buf[j+2];
            j += 3;
        }
        break;
    case 4:
        for (; frames > 0; frames--)
        {
            stereo_buf[i++] = mono_buf[j];
            stereo_buf[i++] = mono_buf[j+1];
            stereo_buf[i++] = mono_buf[j+2];
            stereo_buf[i++] = mono_buf[j+3];
            stereo_buf[i++] = mono_buf[j];
            stereo_buf[i++] = mono_buf[j+1];
            stereo_buf[i++] = mono_buf[j+2];
            stereo_buf[i++] = mono_buf[j+3];
            j += 4;
        }
        break;
    default:
        return -1;
        break;
    }
    return frames;
}
