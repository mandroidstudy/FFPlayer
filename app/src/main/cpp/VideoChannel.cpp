//
// Created by maoweiyi on 2021/9/25.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel(int index, AVCodecContext *pContext):BaseChannel(index,pContext) {}

void * decodeTask(void * arg){
    auto *videoChannel = static_cast<VideoChannel *>(arg);
    if (videoChannel){
        videoChannel->doDecodePacket();
    }
    return 0;
}

void * playTask(void * arg){
    auto *videoChannel = static_cast<VideoChannel *>(arg);
    if (videoChannel){
        videoChannel->doPlay();
    }
    return 0;
}

void VideoChannel::start() {
    isPlaying = true;
    packets.setWorkStatus(true);
    frames.setWorkStatus(true);
    pthread_t decode_tid;
    pthread_create(&decode_tid, nullptr,decodeTask, this);
    pthread_t play_tid;
    pthread_create(&play_tid, nullptr,playTask, this);
}

VideoChannel::~VideoChannel() {

}

void VideoChannel::doDecodePacket() {
    while (isPlaying){
        AVPacket * packet = nullptr;
        int res = packets.frontAndPop(packet);
        if (!isPlaying){
            ReleaseAVPacket(&packet);
            break;
        }
        if (res == 0){
            continue;
        }
        res= avcodec_send_packet(avCodecContext,packet);
        ReleaseAVPacket(&packet);
        if (res == 0){
            AVFrame * frame = av_frame_alloc();
            res = avcodec_receive_frame(avCodecContext,frame);
            if (res == 0){
                frames.push(frame);
            } else if (res == AVERROR(EAGAIN)){
                continue;
            } else{
                break;
            }
        } else{
            break;
        }
    }
}

void VideoChannel::doPlay() {
    uint8_t * dst_data[4];
    int dst_linesizes[4];
    av_image_alloc(dst_data,dst_linesizes,avCodecContext->width,avCodecContext->height,AVPixelFormat::AV_PIX_FMT_RGBA,1);
    SwsContext * swsContext = sws_getContext(
            //input
            avCodecContext->width,
            avCodecContext->height,
            avCodecContext->pix_fmt,
            //output
            avCodecContext->width,
            avCodecContext->height,
            AVPixelFormat::AV_PIX_FMT_RGBA,
            SWS_BILINEAR,
            nullptr, nullptr, nullptr
            );
    while (isPlaying) {
        AVFrame *frame = nullptr;
        int res = frames.frontAndPop(frame);
        if (!isPlaying) {
            ReleaseAVFrame(&frame);
            break;
        }
        if (res == 0) {
            continue;
        }
        sws_scale(swsContext, frame->data, frame->linesize, 0, avCodecContext->height,
                  dst_data,dst_linesizes);

        //success get dst_data, render it to window
    }
}
