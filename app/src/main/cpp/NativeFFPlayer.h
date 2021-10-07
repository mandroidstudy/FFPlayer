//
// Created by maoweiyi on 2021/9/24.
//

#ifndef FFPLAYER_NATIVEFFPLAYER_H
#define FFPLAYER_NATIVEFFPLAYER_H

#include <string>
#include <pthread.h>
#include "VideoChannel.h"
#include "AudioChannel.h"
#include "JNICallback.h"
#include "util.h"

extern "C"{
    #include "libavformat/avformat.h"
}

class NativeFFPlayer {

private:
    std::string data_source;
    bool isPlaying;

    JNICallback *jni_callback         = nullptr;
    VideoChannel *video_channel       = nullptr;
    AudioChannel *audio_channel       = nullptr;
    AVFormatContext *avFormatContext  = nullptr;

public:

    NativeFFPlayer(JNICallback *jniCallback);
    void prepare();
    void setDataSource(std::string& source);
    void start();
    void stop();
    void release();

    void doPrepare();
    void doStart();
    ~NativeFFPlayer();

    void setRenderCallback(RenderCallback renderCallback);
};


#endif //FFPLAYER_NATIVEFFPLAYER_H
