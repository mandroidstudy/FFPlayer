//
// Created by maoweiyi on 2021/9/25.
//

#ifndef FFPLAYER_VIDEOCHANNEL_H
#define FFPLAYER_VIDEOCHANNEL_H


#include "BaseChannel.h"
extern "C"{
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

typedef void(*RenderCallback) (uint8_t *, int, int, int);

class VideoChannel : public BaseChannel{

public:

    RenderCallback _renderCallback = 0;

    bool isPlaying = false;

    VideoChannel(int index, AVCodecContext *pContext);

    void start();

    void doDecodePacket();

    void doPlay();

    void setRenderCallback(RenderCallback renderCallback);

    ~VideoChannel();

    void stop();
};


#endif //FFPLAYER_VIDEOCHANNEL_H
