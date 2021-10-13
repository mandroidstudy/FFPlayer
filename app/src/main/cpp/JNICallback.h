//
// Created by maoweiyi on 2021/9/25.
//

#ifndef FFPLAYER_JNICALLBACK_H
#define FFPLAYER_JNICALLBACK_H

#include <jni.h>
#include "ThreadType.h"

class JNICallback {
private:
    JavaVM *javaVm = nullptr;
    JNIEnv *mainEnv = nullptr;
    jobject jobj;

    jmethodID onPreparedMid;
    jmethodID onCompletedMid;
    jmethodID onErrorMid;
public:
    JNICallback(JavaVM *javaVm, JNIEnv *env, jobject jobj);
    void onPrepared(ThreadType threadType);
    void onCompleted(ThreadType threadType);
    void onError(ThreadType threadType,jint code,const char * desc);
    ~JNICallback();
};


#endif //FFPLAYER_JNICALLBACK_H
