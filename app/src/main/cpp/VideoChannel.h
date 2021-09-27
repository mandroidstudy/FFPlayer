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

class VideoChannel : public BaseChannel{

public:
    bool isPlaying = false;
    VideoChannel(int index, AVCodecContext *pContext);
    ~VideoChannel();
    void start();

    void doDecodePacket();

    void doPlay();
};


#endif //FFPLAYER_VIDEOCHANNEL_H
