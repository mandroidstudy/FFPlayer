//
// Created by maoweiyi on 2021/9/25.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel(int index, AVCodecContext *pContext):BaseChannel(index,pContext) {

}

void VideoChannel::start() {
    packets.setWorkStatus(true);
    frames.setWorkStatus(true);
}

VideoChannel::~VideoChannel() {

}
