//
// Created by maoweiyi on 2021/9/24.
//

#include "NativeFFPlayer.h"

NativeFFPlayer::NativeFFPlayer(JNICallback *jniCallback) {
    pthread_mutex_init(&seek_mutex_t, nullptr);
    jni_callback = jniCallback;
}

void* prepareTask(void * arg){
    auto * player = static_cast<NativeFFPlayer *>(arg);
    if (player){
        player->doPrepare();
    }
    return nullptr;
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
    int res = avformat_open_input(&avFormatContext,data_source.c_str(), nullptr, nullptr);
    if (res){
        if (jni_callback != nullptr){
            jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_OPEN_INPUT,av_err2str(res));
        }
        return;
    }
    res = avformat_find_stream_info(avFormatContext, nullptr);
    if (res < 0){
        if (jni_callback != nullptr){
            jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_FIND_STREAM_INFO,av_err2str(res));
        }
        return;
    }

    int video_stream = av_find_best_stream(avFormatContext,AVMediaType::AVMEDIA_TYPE_VIDEO,-1,-1, nullptr,0);
    if (video_stream >= 0){
        AVCodecParameters * codecpar = avFormatContext->streams[video_stream]->codecpar;
        AVCodec * codec = avcodec_find_decoder(codecpar->codec_id);
        if (!codec) {
            if (jni_callback != nullptr){
                jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_CODEC_OPEN,"Can't find decoder");
            }
            return;
        }
        AVCodecContext *  avCodecContext = avcodec_alloc_context3(codec);
        if (!avCodecContext) {
            if (jni_callback != nullptr){
                jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_CODEC_OPEN,"Can't allocate decoder context");
            }
            return;
        }
        res = avcodec_parameters_to_context(avCodecContext,codecpar);
        if (res < 0){
            if (jni_callback){
                jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_CODEC_OPEN,"Can't copy decoder context");
            }
            return;
        }
        res = avcodec_open2(avCodecContext,codec, nullptr);
        if (res < 0){
            if (jni_callback){
                jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_CODEC_OPEN,av_err2str(res));
            }
            return;
        }
        AVStream * stream = avFormatContext->streams[video_stream];
        video_channel = new VideoChannel(video_stream, avCodecContext);
        video_channel->setTimeBase(stream->time_base);
        AVRational frame_rational = stream->avg_frame_rate;
        int fps = av_q2d(frame_rational);
        video_channel->setFps(fps);
        video_channel->setRenderCallback(renderCallback);
    }

    int audio_stream = av_find_best_stream(avFormatContext,AVMediaType::AVMEDIA_TYPE_AUDIO,-1,-1, nullptr,0);
    if (audio_stream >= 0){
        AVCodecParameters * codecpar = avFormatContext->streams[audio_stream]->codecpar;
        AVCodec * codec = avcodec_find_decoder(codecpar->codec_id);
        if (!codec) {
            if (jni_callback != nullptr){
                jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_CODEC_OPEN,"Can't find decoder");
            }
            return;
        }
        AVCodecContext *  avCodecContext = avcodec_alloc_context3(codec);
        if (!avCodecContext) {
            if (jni_callback != nullptr){
                jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_CODEC_OPEN,"Can't allocate decoder context");
            }
            return;
        }
        res = avcodec_parameters_to_context(avCodecContext,codecpar);
        if (res < 0){
            if (jni_callback){
                jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_CODEC_OPEN,"Can't copy decoder context");
            }
            return;
        }
        res = avcodec_open2(avCodecContext,codec, nullptr);
        if (res < 0){
            if (jni_callback){
                jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_CODEC_OPEN,av_err2str(res));
            }
            return;
        }
        audio_channel = new AudioChannel(audio_stream,avCodecContext);
        audio_channel->setTimeBase(avFormatContext->streams[audio_stream]->time_base);
    }

    if (video_channel == nullptr && audio_channel == nullptr){
        if (jni_callback != nullptr){
            jni_callback->onError(ThreadType::THREAD_TYPE_CHILD,FFMPEG_ERR_NOT_AUDIO_VIDEO_STREAM,"Can't find video stream and audio stream");
        }
        return;
    }
    duration = avFormatContext->duration / AV_TIME_BASE;
    if (audio_channel && duration>0){
        audio_channel->addJniCallback(jni_callback);
    }
    if (video_channel){
        video_channel -> setAudioChannel(audio_channel);
    }
    if (jni_callback){
        jni_callback->onPrepared(ThreadType::THREAD_TYPE_CHILD);
    }
}

void NativeFFPlayer::prepare() {
    pthread_create(&prepare_tid,nullptr,prepareTask,this);
}

void NativeFFPlayer::setDataSource(std::string& source) {
    this->data_source = source;
}


void* startTask(void * arg){
    auto * player = static_cast<NativeFFPlayer *>(arg);
    if (player){
        player->doStart();
    }
    return nullptr;
}

void NativeFFPlayer::doStart() {
    while (isPlaying){
        if ((video_channel && video_channel->packets.size() > MAX_SIZE)
            || (audio_channel && audio_channel->packets.size() > MAX_SIZE)) {
            av_usleep(10 * 1000); //10毫秒
            continue;
        }

        AVPacket *pkt = av_packet_alloc();
        int res = av_read_frame(avFormatContext,pkt);
        if (res == 0){
            if (audio_channel && pkt->stream_index == audio_channel->stream_index){
                audio_channel->packets.push(pkt);
            } else if (video_channel && pkt->stream_index == video_channel->stream_index){
                video_channel->packets.push(pkt);
            }
        } else if(res == AVERROR_EOF){
            //read packets end
            if (audio_channel->packets.empty() && audio_channel->frames.empty() &&
                    video_channel->packets.empty() && video_channel->frames.empty()){
                if (jni_callback){
                    jni_callback->onCompleted(ThreadType::THREAD_TYPE_CHILD);
                }
                break;
            } else {
                av_usleep(10 * 1000); //10毫秒
            }
        } else{
            //error
            break;
        }
    } // end whild
}

void NativeFFPlayer::start() {
    isPlaying = true;
    if (audio_channel){
        audio_channel->start();
    }
    if (video_channel){
        video_channel->start();
    }
    pthread_create(&start_tid,nullptr,startTask,this);
}

void* stopTask(void * arg){
    auto * player = static_cast<NativeFFPlayer *>(arg);
    if (player){
        player->doStop();
    }
    return nullptr;
}

void NativeFFPlayer::stop() {
    isPlaying = false;
    jni_callback = nullptr;
    if (audio_channel){
        audio_channel->jniCallback = nullptr;
    }
    pthread_create(&stop_tid, nullptr,stopTask,this);
}

void NativeFFPlayer::doStop() {
    pthread_join(prepare_tid, nullptr);
    pthread_join(start_tid, nullptr);
    if (audio_channel){
        audio_channel->stop();
        delete audio_channel;
        audio_channel = nullptr;
    }
    if (video_channel){
        video_channel->stop();
        delete video_channel;
        video_channel = nullptr;
    }
    if (avFormatContext){
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
        avFormatContext = nullptr;
    }
}

void NativeFFPlayer::release() {

}

NativeFFPlayer::~NativeFFPlayer() {
    pthread_mutex_destroy(&seek_mutex_t);
    if (video_channel != nullptr){
        delete video_channel;
        video_channel = nullptr;
    }
    if (audio_channel != nullptr){
        delete audio_channel;
        audio_channel = nullptr;
    }
}

void NativeFFPlayer::setRenderCallback(RenderCallback _renderCallback) {
    this->renderCallback = _renderCallback;
}

int NativeFFPlayer::getDuration() const {
    return duration;
}

void NativeFFPlayer::seek(int progress) {
    if (!isPlaying) return;
    if (progress < 0 || progress > duration) return;
    if (!avFormatContext) return;
    if (!video_channel && !audio_channel) return;
    pthread_mutex_lock(&seek_mutex_t);
    int res = av_seek_frame(avFormatContext,-1,progress * AV_TIME_BASE,AVSEEK_FLAG_BACKWARD|AVSEEK_FLAG_FRAME);
    if (res >= 0){
        if (video_channel){
            video_channel->packets.setWorkStatus(false);
            video_channel->frames.setWorkStatus(false);
        }
        if (audio_channel){
            audio_channel->packets.setWorkStatus(false);
            audio_channel->frames.setWorkStatus(false);
        }
        if (video_channel){
            video_channel->clearAllAVPacketAndAVFrame();
        }
        if (audio_channel){
            audio_channel->clearAllAVPacketAndAVFrame();
        }
        if (video_channel){
            video_channel->packets.setWorkStatus(true);
            video_channel->frames.setWorkStatus(true);
        }
        if (audio_channel){
            audio_channel->packets.setWorkStatus(true);
            audio_channel->frames.setWorkStatus(true);
        }
    }
    pthread_mutex_unlock(&seek_mutex_t);
}

void NativeFFPlayer::pause() {
    if (audio_channel){
        audio_channel->frames.setWorkStatus(false);
        audio_channel->packets.setWorkStatus(false);
    }
    if (video_channel){
        video_channel->frames.setWorkStatus(false);
        video_channel->packets.setWorkStatus(false);
    }
}

bool NativeFFPlayer::isPause() {
    return isPlaying
            && !audio_channel->frames.isWorking
            && !video_channel->frames.isWorking;

}

void NativeFFPlayer::resume() {
    if (!isPlaying) return;
    if (audio_channel){
        LOGD("resume")
        audio_channel->frames.setWorkStatus(true);
        audio_channel->packets.setWorkStatus(true);
    }
    if (video_channel){
        video_channel->frames.setWorkStatus(true);
        video_channel->packets.setWorkStatus(true);
    }
}

