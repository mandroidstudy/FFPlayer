//
// Created by maoweiyi on 2021/9/25.
//

#include "JNICallback.h"

JNICallback::JNICallback(JavaVM *javaVm, JNIEnv *env, jobject jobj) {
    this->javaVm = javaVm;
    this->mainEnv = env;
    this->jobj = env->NewGlobalRef(jobj);
    jclass jclz = env->GetObjectClass(jobj);
    onPreparedMid = env->GetMethodID(jclz,"onPrepared","()V");
    onProgressMid = env->GetMethodID(jclz,"onProgress","(I)V");;
    onCompletedMid = env->GetMethodID(jclz,"onCompleted","()V");
    onErrorMid = env->GetMethodID(jclz,"onError","(ILjava/lang/String;)V");
}

void JNICallback::onPrepared(ThreadType threadType) {
    if (threadType ==ThreadType::THREAD_TYPE_MAIN){
        mainEnv->CallVoidMethod(jobj,onPreparedMid);
    } else{
        JNIEnv * env;
        javaVm->AttachCurrentThread(&env,0);
        env->CallVoidMethod(jobj,onPreparedMid);
        javaVm->DetachCurrentThread();
    }
}

void JNICallback::onProgress(ThreadType threadType,int progress) {
    if (threadType ==ThreadType::THREAD_TYPE_MAIN){
        mainEnv->CallVoidMethod(jobj,onProgressMid);
    } else{
        JNIEnv * env;
        javaVm->AttachCurrentThread(&env,0);
        env->CallVoidMethod(jobj,onProgressMid,progress);
        javaVm->DetachCurrentThread();
    }
}

void JNICallback::onCompleted(ThreadType threadType) {
    if (threadType ==ThreadType::THREAD_TYPE_MAIN){
        mainEnv->CallVoidMethod(jobj,onCompletedMid);
    } else{
        JNIEnv * env;
        javaVm->AttachCurrentThread(&env,0);
        env->CallVoidMethod(jobj,onCompletedMid);
        javaVm->DetachCurrentThread();
    }
}

void JNICallback::onError(ThreadType threadType,jint code,const char* desc) {
    if (threadType ==ThreadType::THREAD_TYPE_MAIN){
        mainEnv->CallVoidMethod(jobj,onErrorMid,code,mainEnv->NewStringUTF(desc));
    } else{
        JNIEnv * env;
        javaVm->AttachCurrentThread(&env,0);
        env->CallVoidMethod(jobj,onErrorMid,code,env->NewStringUTF(desc));
        javaVm->DetachCurrentThread();
    }
}

JNICallback::~JNICallback() {
    javaVm = nullptr;
    mainEnv->DeleteGlobalRef(jobj);
    mainEnv = nullptr;
    jobj = nullptr;
}
