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
#include <libavutil/time.h>
}

class NativeFFPlayer {

private:
    const int MAX_SIZE = 100;
    std::string data_source;
    bool isPlaying;
    pthread_mutex_t seek_mutex_t;

    RenderCallback renderCallback = nullptr;

    JNICallback *jni_callback         = nullptr;
    VideoChannel *video_channel       = nullptr;
    AudioChannel *audio_channel       = nullptr;
    AVFormatContext *avFormatContext  = nullptr;
    int duration;

    pthread_t prepare_tid;
    pthread_t start_tid;
    pthread_t stop_tid;
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

    int getDuration() const;

    void seek(jint progress);

    void doStop();

    void pause();

    bool isPause();

    void resume();
};


#endif //FFPLAYER_NATIVEFFPLAYER_H
