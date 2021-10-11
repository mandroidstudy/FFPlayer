#include <jni.h>
#include <string>
#include "NativeFFPlayer.h"
#include "JNICallback.h"
#include <android/native_window_jni.h>
#include <pthread.h>
using namespace std;

JavaVM *javaVm = nullptr;

static jclass g_cls = nullptr;
static jfieldID g_fileID = nullptr;

ANativeWindow* nativeWindow = nullptr;
pthread_mutex_t windowMutex = PTHREAD_MUTEX_INITIALIZER;

static string jstring2string(JNIEnv *env, jstring str) {
    if (str) {
        const char *cstr = env->GetStringUTFChars(str, nullptr);
        if (cstr) {
            string result(cstr);
            env->ReleaseStringUTFChars(str, cstr);
            return result;
        }
    }
    return "";
}

static jstring string2jstring(JNIEnv *env, const string &str) {
    return env->NewStringUTF(str.c_str());
}

static NativeFFPlayer *getNativeFFPlayer(JNIEnv *env, jobject obj) {
    jlong handle = env->GetLongField(obj, g_fileID);
    return reinterpret_cast<NativeFFPlayer *>(handle);
}

void renderCallback(uint8_t * src_data, int width, int height, int src_linesize){
    pthread_mutex_lock(&windowMutex);
    //render raw data to window
    if (!nativeWindow){
        pthread_mutex_unlock(&windowMutex);
        return;
    }
    ANativeWindow_setBuffersGeometry(nativeWindow,width,height,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer  windowBuffer;
    if (ANativeWindow_lock(nativeWindow,&windowBuffer,0)){
        ANativeWindow_release(nativeWindow);
        nativeWindow = nullptr;
        pthread_mutex_unlock(&windowMutex);
        return;
    }
    uint8_t * dst_data = static_cast<uint8_t *>(windowBuffer.bits);
    int32_t dst_linesize = windowBuffer.stride * 4;
    for (int i = 0; i < windowBuffer.height; ++i) {
        //windowBuffer 64字节对齐 但是为啥1928不行？
//        memcpy(dst_data + i * 1792,data + i * 1704,1792);
        memcpy(dst_data + i * dst_linesize,src_data + i * src_linesize,dst_linesize);
    }
    ANativeWindow_unlockAndPost(nativeWindow);
    pthread_mutex_unlock(&windowMutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffplayer_player_FFPlayer_nativeStart(JNIEnv *env, jobject thiz) {
    NativeFFPlayer * player = getNativeFFPlayer(env,thiz);
    if (player){
        player->start();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffplayer_player_FFPlayer_nativeStop(JNIEnv *env, jobject thiz) {
    NativeFFPlayer * player = getNativeFFPlayer(env,thiz);
    if (player){
        player->stop();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffplayer_player_FFPlayer_nativePrepare(JNIEnv *env, jobject thiz) {
    NativeFFPlayer * player = getNativeFFPlayer(env,thiz);
    if (player){
        player->setRenderCallback(renderCallback);
        player->prepare();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffplayer_player_FFPlayer_nativeRelease(JNIEnv *env, jobject thiz) {
    NativeFFPlayer * player = getNativeFFPlayer(env,thiz);
    if (player){
        player->release();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffplayer_player_FFPlayer_nativeSetDataSource(JNIEnv *env, jobject thiz,jstring jsource) {
    NativeFFPlayer * player = getNativeFFPlayer(env,thiz);
    if (player){
        string source = jstring2string(env,jsource);
        player->setDataSource(source);
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_ffplayer_player_FFPlayer_nativeCreate(JNIEnv *env, jobject thiz) {
    auto * jniCallback = new JNICallback(javaVm,env,thiz);
    auto * nativeFfPlayer = new NativeFFPlayer(jniCallback);
    return reinterpret_cast<jlong>(nativeFfPlayer);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffplayer_player_FFPlayer_setNativeSurface(JNIEnv *env, jobject thiz,jobject surface) {
    NativeFFPlayer * player = getNativeFFPlayer(env,thiz);
    if (player){
        pthread_mutex_lock(&windowMutex);
        if (nativeWindow){
            ANativeWindow_release(nativeWindow);
            nativeWindow = nullptr;
        }
        nativeWindow = ANativeWindow_fromSurface(env,surface);
        pthread_mutex_unlock(&windowMutex);
    }
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    javaVm = vm;
    static const char *clsFullName = "com/example/ffplayer/player/FFPlayer";
    jclass instance = env->FindClass(clsFullName);
    g_cls = reinterpret_cast<jclass>(env->NewGlobalRef(instance));
    g_fileID = env->GetFieldID(g_cls, "nativeHandle", "J");
    return JNI_VERSION_1_6;
}