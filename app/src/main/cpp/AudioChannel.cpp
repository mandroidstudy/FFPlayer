//
// Created by maoweiyi on 2021/9/25.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int index, AVCodecContext *pContext) :BaseChannel(index,pContext){

}

void AudioChannel::start() {
    packets.setWorkStatus(true);
    frames.setWorkStatus(true);
}

AudioChannel::~AudioChannel() {

}
