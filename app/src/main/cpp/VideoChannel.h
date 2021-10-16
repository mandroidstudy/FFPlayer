//
// Created by maoweiyi on 2021/9/25.
//

#ifndef FFPLAYER_VIDEOCHANNEL_H
#define FFPLAYER_VIDEOCHANNEL_H


#include "BaseChannel.h"
#include "AudioChannel.h"
#include <queue>

extern "C"{
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

typedef void(*RenderCallback) (uint8_t *, int, int, int);

class VideoChannel : public BaseChannel{

public:

    AudioChannel *audio_channel;

    RenderCallback _renderCallback = 0;

    bool isPlaying = false;

    int fps;

    VideoChannel(int index, AVCodecContext *pContext);

    void start();

    void doDecodePacket();

    void doPlay();

    void setRenderCallback(RenderCallback renderCallback);

    void setFps(int fps);

    ~VideoChannel();

    void stop();

    void setAudioChannel(AudioChannel *audio_channel);

    static void DropAvFrame(std::queue<AVFrame*> &queue){
        if (!queue.empty()){
            AVFrame* avFrame = queue.front();
            queue.pop();
            av_frame_unref(avFrame);
            ReleaseAVFrame(&avFrame);
        }
    }

    static void DropAvPacket(std::queue<AVPacket *> &queue){
        while (!queue.empty()){
            AVPacket* avPacket = queue.front();
            if (avPacket->flags != AV_PKT_FLAG_KEY){
                queue.pop();
                av_packet_unref(avPacket);
                ReleaseAVPacket(&avPacket);
            } else{
                break;
            }
        }
    }
};


#endif //FFPLAYER_VIDEOCHANNEL_H
