//
// Created by maoweiyi on 2021/9/26.
//

#ifndef FFPLAYER_BASECHANNEL_H
#define FFPLAYER_BASECHANNEL_H
#include "SafeQueue.h"

extern "C"{
#include "libavformat/avformat.h"
}

class BaseChannel{
public:
    int stream_index;
    AVCodecContext * avCodecContext;

    SafeQueue<AVPacket*> packets;
    SafeQueue<AVFrame*> frames;

public:
    BaseChannel(int index, AVCodecContext *codecContext){
        this->stream_index = index;
        this->avCodecContext = codecContext;
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
