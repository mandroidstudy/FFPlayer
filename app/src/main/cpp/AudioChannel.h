//
// Created by 毛维义 on 2021/9/25.
//

#ifndef FFPLAYER_AUDIOCHANNEL_H
#define FFPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"

class AudioChannel : public BaseChannel{

public:
    AudioChannel(int index, AVCodecContext *pContext);
    ~AudioChannel();

    void start();
};


#endif //FFPLAYER_AUDIOCHANNEL_H
