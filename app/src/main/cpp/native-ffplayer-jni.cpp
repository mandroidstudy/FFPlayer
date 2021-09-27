#include <jni.h>
#include <string>
#include "NativeFFPlayer.h"
#include "JNICallback.h"

using namespace std;

JavaVM *javaVm = nullptr;

static jclass g_cls = nullptr;
static jfieldID g_fileID = nullptr;

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