//
// Created by maoweiyi on 2021/9/26.
//

#ifndef FFPLAYER_BASECHANNEL_H
#define FFPLAYER_BASECHANNEL_H
#include "SafeQueue.h"
#include "LogM.h"
extern "C"{
#include "libavformat/avformat.h"
#include <libavutil/time.h>
}

class BaseChannel{
public:
    int stream_index;
    AVCodecContext * avCodecContext;

    SafeQueue<AVPacket*> packets;
    SafeQueue<AVFrame*> frames;

public:
    BaseChannel(int index, AVCodecContext *codecContext):stream_index(index),avCodecContext(codecContext){
        packets.setReleaseCallback(ReleaseAVPacket);
        frames.setReleaseCallback(ReleaseAVFrame);
    }

    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();
    }

    static void ReleaseAVPacket(AVPacket** packet){
        av_packet_free(packet);
        packet = nullptr;
    }
    static void ReleaseAVFrame(AVFrame** frame){
        av_frame_free(frame);
        frame = nullptr;
    }
};


#endif //FFPLAYER_BASECHANNEL_H
