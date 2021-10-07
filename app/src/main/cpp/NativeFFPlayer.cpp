//
// Created by maoweiyi on 2021/9/24.
//

#include "NativeFFPlayer.h"

NativeFFPlayer::NativeFFPlayer(JNICallback *jniCallback) {
    jni_callback = jniCallback;
}

void* prepareTask(void * arg){
    auto * player = static_cast<NativeFFPlayer *>(arg);
    player->doPrepare();
    return 0;
}

/**
 * step1:avformat_open_input 打开媒体文件
 * step2:avformat_find_stream_info 获取流信息
 * step3:avcodec_find_decoder 查找解码器
 * step4:avcodec_alloc_context3 创建解码器上下文
 * step5:avcodec_parameters_to_context 给解码器上下文设置参数
 * step6:avcodec_open2 打开解码器
 * step7:获取音频流，视频流
 */
void NativeFFPlayer::doPrepare() {
    avFormatContext = avformat_alloc_context();
    int res = avformat_open_input(&avFormatContext,data_source.c_str(), 0, 0);
    if (res){
        if (jni_callback != nullptr){
            jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_OPEN_INPUT,av_err2str(res));
        }
        return;
    }
    res = avformat_find_stream_info(avFormatContext,0);
    if (res < 0){
        if (jni_callback != nullptr){
            jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_FIND_STREAM_INFO,av_err2str(res));
        }
        return;
    }
    int N = avFormatContext->nb_streams;
    for (int i = 0; i < N; i++){
        AVStream * avStream = avFormatContext->streams[i];
        AVCodecParameters * codecParameters = avStream->codecpar;
        AVCodec * avCodec = avcodec_find_decoder(codecParameters->codec_id);
        AVCodecContext * avCodecContext = avcodec_alloc_context3(avCodec);
        res = avcodec_parameters_to_context(avCodecContext,codecParameters);
        if (res < 0){
            if (jni_callback != nullptr){
                jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_CODEC_PARAMS_TO_CONTEXT,av_err2str(res));
            }
            return;
        }
        res = avcodec_open2(avCodecContext,avCodec,0);
        if (res){
            if (jni_callback != nullptr){
                jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_CODEC_OPEN,av_err2str(res));
            }
            return;
        }
        if (codecParameters->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO){
            video_channel = new VideoChannel(i, avCodecContext);
        } else if (codecParameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO){
            audio_channel = new AudioChannel(i,avCodecContext);
        }
    } //for end

    if (video_channel == nullptr && audio_channel == nullptr){
        if (jni_callback != nullptr){
            jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_NOT_AUDIO_VIDEO_STREAM,av_err2str(res));
        }
        return;
    }
    if (jni_callback){
        jni_callback->onPrepared(ThreadType::THREAD_TYPE_CHILD);
    }
}

void NativeFFPlayer::prepare() {
    pthread_t t_id;
    pthread_create(&t_id,nullptr,prepareTask,this);
}

void NativeFFPlayer::setDataSource(std::string& source) {
    this->data_source = source;
}


void* startTask(void * arg){
    auto * player = static_cast<NativeFFPlayer *>(arg);
    player->doStart();
    return 0;
}

void NativeFFPlayer::doStart() {
    while (isPlaying){
        AVPacket *pkt = av_packet_alloc();
        int res = av_read_frame(avFormatContext,pkt);
        if (res){
            if (pkt->stream_index == audio_channel->stream_index){
                audio_channel->packets.push(pkt);
            } else if (pkt->stream_index == video_channel->stream_index){
                video_channel->packets.push(pkt);
            }
        } else if(res == AVERROR_EOF){
            //read packets end
        } else{
            //error
            break;
        }
    } // end whild
    isPlaying = false;
}

void NativeFFPlayer::start() {
    if (audio_channel != nullptr){
        audio_channel->start();
    }
    if (video_channel != nullptr){
        video_channel->start();
    }
    pthread_t start_tid;
    pthread_create(&start_tid,0,startTask,0);
}

void NativeFFPlayer::stop() {

}

void NativeFFPlayer::release() {

}

NativeFFPlayer::~NativeFFPlayer() {
    if (video_channel != nullptr){
        delete video_channel;
        video_channel = nullptr;
    }
    if (audio_channel != nullptr){
        delete audio_channel;
        audio_channel = nullptr;
    }
}

void NativeFFPlayer::setRenderCallback(RenderCallback renderCallback) {
    video_channel->setRenderCallback(renderCallback);
}

