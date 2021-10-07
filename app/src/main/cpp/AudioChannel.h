//
// Created by 毛维义 on 2021/9/25.
//

#ifndef FFPLAYER_AUDIOCHANNEL_H
#define FFPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
//重采样
extern "C"{
#include <libswresample/swresample.h>
}

class AudioChannel : public BaseChannel{
private:
    bool isPlaying;
    // engine
    SLObjectItf engineObject = 0;
    SLEngineItf engineEngine = 0;

    // output mix
    const SLEnvironmentalReverbSettings reverbSettings =
            SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    SLObjectItf outputMixObject = 0;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = 0;

    SLObjectItf playerObject = 0;
    SLPlayItf player = 0;

    //bufferQueueItf
    SLAndroidSimpleBufferQueueItf  bufferQueueItf = 0;

public:
    int out_nb_channels;
    int out_bytes_per_sample;
    int out_sample_rate;
    int out_buffer_size;
    u_int8_t *out_buffer = 0;
    SwrContext *swr_ctx = 0;
public:
    AudioChannel(int index, AVCodecContext *pContext);
    ~AudioChannel();

    void start();

    void doDecodePacket();

    void doPlay();

    int reSampleAndObtainPCM();
};


#endif //FFPLAYER_AUDIOCHANNEL_H
