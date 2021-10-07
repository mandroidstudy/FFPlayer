//
// Created by maoweiyi on 2021/9/25.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int index, AVCodecContext *pContext) :BaseChannel(index,pContext){
    out_nb_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_bytes_per_sample = av_get_bytes_per_sample(AVSampleFormat::AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;
    out_buffer_size = out_sample_rate * out_nb_channels * out_bytes_per_sample;
    out_buffer = static_cast<u_int8_t *>(malloc(out_buffer_size));

    //ffmpeg 音频重采样参数设置
    swr_ctx = swr_alloc_set_opts(0,
                                 //output
                                 AV_CH_LAYOUT_STEREO,AVSampleFormat::AV_SAMPLE_FMT_S16,out_sample_rate,
                                 //input
                                             avCodecContext->channel_layout,avCodecContext->sample_fmt,avCodecContext->sample_rate,
                                             0,0
                                             );
    if (swr_ctx){
        if (swr_init(swr_ctx)){
            swr_free(&swr_ctx);
            swr_ctx = 0;
        }
    }
}

void * decodeAudioTask(void * arg){
    auto *audioChannel = static_cast<AudioChannel *>(arg);
    if (audioChannel){
        audioChannel->doDecodePacket();
    }
    return 0;
}

void * playAudioTask(void * arg){
    auto *audioChannel = static_cast<AudioChannel *>(arg);
    if (audioChannel){
        audioChannel->doPlay();
    }
    return 0;
}

void AudioChannel::start() {
    isPlaying = true;
    packets.setWorkStatus(true);
    frames.setWorkStatus(true);
    pthread_t decode_tid;
    pthread_create(&decode_tid, nullptr,decodeAudioTask, this);
    pthread_t play_tid;
    pthread_create(&play_tid, nullptr, playAudioTask, this);
}

void AudioChannel::doDecodePacket() {
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
    isPlaying = false;
    packets.setWorkStatus(false);
    frames.setWorkStatus(false);
}

//每次缓存队列的数据播放完就会回调这个函数
void bufferQueueCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext){
    AudioChannel* audioChannel = static_cast<AudioChannel *>(pContext);
    int pcm_size = audioChannel->reSampleAndObtainPCM();
    if (pcm_size <= 0){
        return;
    }
    (*caller)->Enqueue(caller,
                       audioChannel->out_buffer,//pcm data
                       pcm_size);//pcm data size
}

void AudioChannel::doPlay() {
    SLresult sLresult;
    //1、获取引擎接口
    sLresult = slCreateEngine(&engineObject,0, 0, 0, 0, 0);
    if (sLresult != SL_RESULT_SUCCESS){
        //fail
        return;
    }
    sLresult = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (sLresult != SL_RESULT_SUCCESS){
        //fail
        return;
    }
    sLresult = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (sLresult != SL_RESULT_SUCCESS || !engineEngine){
        //fail
        return;
    }

    //2、创建输出混音器
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    sLresult = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    if (sLresult != SL_RESULT_SUCCESS){
        //fail
        return;
    }
    sLresult = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (sLresult != SL_RESULT_SUCCESS){
        //fail
        return;
    }
    sLresult=(*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (sLresult != SL_RESULT_SUCCESS || !outputMixEnvironmentalReverb){
        //fail
        return;
    }
    sLresult = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
            outputMixEnvironmentalReverb, &reverbSettings);
    if (sLresult != SL_RESULT_SUCCESS){
        //fail
        return;
    }

    //3、创建播放器
    //创建播放器需要为其指定Data Source 和 Data Sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq={
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            2
    };
    //Data Source
    SLDataFormat_PCM format_pcm={
            SL_DATAFORMAT_PCM,//是pcm格式的
            2,//两声道
            SL_SAMPLINGRATE_44_1,//每秒的采样率
            SL_PCMSAMPLEFORMAT_FIXED_16,//每个采样的位数
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一样就行
            SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT,//立体声(左前和右前)
            SL_BYTEORDER_LITTLEENDIAN,//播放结束的标志
    };
    SLDataSource dataSource={&loc_bufq,&format_pcm};
    //Data Sink
    SLDataLocator_OutputMix loc_outmix={SL_DATALOCATOR_OUTPUTMIX,outputMixObject};
    SLDataSink dataSink={&loc_outmix,NULL};


    //create audio player:
    const SLInterfaceID iids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean ireq[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    sLresult = (*engineEngine)->CreateAudioPlayer(engineEngine,&playerObject,
                                       &dataSource,&dataSink,
                                       3,iids,ireq);

    if (sLresult != SL_RESULT_SUCCESS){
        //fail
        return;
    }
    sLresult = (*playerObject)->Realize(playerObject,SL_BOOLEAN_FALSE);
    if (sLresult != SL_RESULT_SUCCESS){
        //fail
        return;
    }

    //获取播放接口
    sLresult = (*playerObject)->GetInterface(playerObject,SL_IID_PLAY,&player);
    if (sLresult != SL_RESULT_SUCCESS || !player){
        //fail
        return;
    }

    //4、获取缓冲队列接口、给缓冲队列注册回调函数
    sLresult = (*playerObject)->GetInterface(playerObject,SL_IID_BUFFERQUEUE,&bufferQueueItf);
    if (sLresult != SL_RESULT_SUCCESS){
        //fail
        return;
    }
    sLresult = (*bufferQueueItf)->RegisterCallback(bufferQueueItf, bufferQueueCallback, this);
    if (sLresult != SL_RESULT_SUCCESS){
        //fail
        return;
    }
    //5、设置播放状态、主动调用一次回调使缓存队列接口工作
    (*player)->SetPlayState(player,SL_PLAYSTATE_PLAYING);
    bufferQueueCallback(bufferQueueItf, this);
}

AudioChannel::~AudioChannel() {
    if (swr_ctx){
        swr_free(&swr_ctx);
        swr_ctx = 0;
    }
}

int AudioChannel::reSampleAndObtainPCM() {
    int pcm_size = 0;
    while (isPlaying && swr_ctx) {
        AVFrame *frame = nullptr;
        int res = frames.frontAndPop(frame);
        if (!isPlaying) {
            ReleaseAVFrame(&frame);
            break;
        }
        if (res == 0) {
            continue;
        }
        int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx,frame->sample_rate) + frame->nb_samples,
                                            out_sample_rate,
                                            frame->sample_rate,
                                            AV_ROUND_UP);
        //重采样
        int number_samples_per_channel = swr_convert(swr_ctx,
                    &out_buffer,dst_nb_samples,
                    (const uint8_t **)frame->data,frame->nb_samples);
        pcm_size = number_samples_per_channel * out_nb_channels * out_bytes_per_sample;
        break;
    } //end while
    return pcm_size;
}
