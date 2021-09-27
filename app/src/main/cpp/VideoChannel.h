//
// Created by maoweiyi on 2021/9/25.
//

#ifndef FFPLAYER_VIDEOCHANNEL_H
#define FFPLAYER_VIDEOCHANNEL_H


#include "BaseChannel.h"

class VideoChannel : public BaseChannel{

public:
    VideoChannel(int index, AVCodecContext *pContext);
    ~VideoChannel();
    void start();
};


#endif //FFPLAYER_VIDEOCHANNEL_H
