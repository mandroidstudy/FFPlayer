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
    const int MAX_CACHE_SIZE = 50;
    int stream_index;
    AVCodecContext * avCodecContext;

    SafeQueue<AVPacket*> packets;
    SafeQueue<AVFrame*> frames;

    AVRational timeBase;

public:
    BaseChannel(int index, AVCodecContext *codecContext):stream_index(index),avCodecContext(codecContext){
        packets.setReleaseCallback(ReleaseAVPacket);
        frames.setReleaseCallback(ReleaseAVFrame);
    }

    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();
    }

public:
    void setTimeBase(AVRational& avRational){
        timeBase = avRational;
    }

    void clearAllAVPacketAndAVFrame(){
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
