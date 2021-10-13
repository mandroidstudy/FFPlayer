//
// Created by maoweiyi on 2021/9/25.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel(int index, AVCodecContext *pContext):BaseChannel(index,pContext) {}

void * decodeVideoTask(void * arg){
    auto *videoChannel = static_cast<VideoChannel *>(arg);
    if (videoChannel){
        videoChannel->doDecodePacket();
    }
    return nullptr;
}

void * playVideoTask(void * arg){
    auto *videoChannel = static_cast<VideoChannel *>(arg);
    if (videoChannel){
        videoChannel->doPlay();
    }
    return nullptr;
}

void VideoChannel::start() {
    isPlaying = true;
    packets.setWorkStatus(true);
    frames.setWorkStatus(true);
    pthread_t decode_tid;
    pthread_create(&decode_tid, nullptr, decodeVideoTask, this);
    pthread_t play_tid;
    pthread_create(&play_tid, nullptr, playVideoTask, this);
}

VideoChannel::~VideoChannel() {

}

void VideoChannel::doDecodePacket() {
    AVPacket * packet = nullptr;
    while (isPlaying){
        if (frames.size() > MAX_CACHE_SIZE){
            av_usleep(10 * 1000);
            continue;
        }
        int res = packets.frontAndPop(packet);
        if (!isPlaying){
            break;
        }
        if (res == 0){
            continue;
        }
        //packet will be empty on read error/EOF
        res= avcodec_send_packet(avCodecContext,packet);
        if (res < 0){
            break;
        }
        av_packet_unref(packet);
        ReleaseAVPacket(&packet);
        while (true){
            AVFrame * frame = av_frame_alloc();
            res = avcodec_receive_frame(avCodecContext,frame);
            if (res == 0){
                frames.push(frame);
            }else if (res == AVERROR(EAGAIN)){
                if (frame){
                    av_frame_unref(frame);
                    ReleaseAVFrame(&frame);
                }
                break;
            } else {
                if (frame){
                    av_frame_unref(frame);
                    ReleaseAVFrame(&frame);
                }
                goto finish;
            }
        }
    }
    finish:
    isPlaying = false;
    av_packet_unref(packet); // 减1 = 0 释放成员指向的堆区
    ReleaseAVPacket(&packet); // 释放AVPacket * 本身的堆区空间
    packets.setWorkStatus(false);
    frames.setWorkStatus(false);
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
    AVFrame *frame = nullptr;
    while (isPlaying) {
        int res = frames.frontAndPop(frame);
        if (!isPlaying) {
            break;
        }
        if (res == 0) {
            continue;
        }
        sws_scale(swsContext, frame->data, frame->linesize, 0, avCodecContext->height,
                  dst_data,dst_linesizes);
        //success get dst_data, render it to windowav_image_fill_arrays
        if (_renderCallback){
            _renderCallback(dst_data[0],avCodecContext->width,avCodecContext->height,dst_linesizes[0]);
        }
        av_frame_unref(frame);
        ReleaseAVFrame(&frame);
    }
    isPlaying = false;
    av_frame_unref(frame);
    ReleaseAVFrame(&frame);
    packets.setWorkStatus(false);
    frames.setWorkStatus(false);
    av_free(&dst_data[0]);
    sws_freeContext(swsContext);
}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {
    this->_renderCallback = renderCallback;
}

void VideoChannel::stop() {
}
