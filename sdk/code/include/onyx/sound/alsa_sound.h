#ifndef ONYX_ALSA_SOUND_H_
#define ONYX_ALSA_SOUND_H_

#include <QtCore/QtCore>

#ifdef BUILD_WITH_TFT
#include "alsa/asoundlib.h"
#endif

class AlsaSound
{
public:
    AlsaSound();
    ~AlsaSound();

public:
    int volume();
    bool setVolume(int volume);

    /// Set sample size, usually it's 8 or 16.
    bool setBitsPerSample(int bps);

    /// set channels. Usually it's mono or stereo.
    bool setChannels(int channels);

    /// Set sample rate.
    bool setSamplingRate(int rate);

    bool setRec();

    /// Play the data. Before using this function, make sure
    /// all parameters of sound chip are correctly configured.
    bool play(unsigned char *data, int size);

    /// Check if the device is enabled or not.
    inline bool isEnabled() { return enable_; }
    inline void enable(bool enable = true) { enable_ = enable; }

    bool openMixer();
    void closeMixer();

    bool openPCMHandler();
    void closePCMHandler();

    bool updateParameters();

private:
   int mono2Stereo(const unsigned char* mono_buf, long unsigned int frames, unsigned char* stereo_buf);

   bool setParams(unsigned int bitspersample, unsigned int channels, unsigned int samplerate);
private:
#ifdef BUILD_WITH_TFT
    snd_mixer_t      *mixer_;
    snd_mixer_elem_t *pcm_element_;
    snd_pcm_t        *pcm_handle_;
#endif
    bool enable_;   ///< Soft flag to enable or disable the device.
    int  bps_;
    int  channels_;
    int  samplerate_;

    int           byte_per_frames_;
    int           audio_data_per_ms_;
    bool          conv2stereo_;
    int           stereo_buff_len_;
    unsigned char *stereo_buff_;
};


#endif // ONYX_ALSA_SOUND_H_
