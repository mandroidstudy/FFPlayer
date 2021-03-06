package com.example.ffplayer.player;

import android.view.Surface;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

import com.example.ffplayer.listener.ListenerInfo;
import com.example.ffplayer.listener.OnCompletedListener;
import com.example.ffplayer.listener.OnErrorListener;
import com.example.ffplayer.listener.OnPreparedListener;
import com.example.ffplayer.listener.OnProgressListener;

/**
 * @author maoweiyi
 * @time 2021/9/24
 * @describe
 */
public class FFPlayer implements SurfaceHolder.Callback {

    public static final int ERR_OPEN_INPUT = 1;
    public static final int ERR_FIND_STREAM_INFO = 2;
    public static final int ERR_CODEC_PARAMS_TO_CONTEXT = 3;
    public static final int ERR_NOT_AUDIO_VIDEO_STREAM = 4;

    private long nativeHandle;

    static {
        System.loadLibrary("ffplayer");
    }

    private ListenerInfo mListenerInfo;

    private SurfaceHolder mSurfaceHolder;

    public FFPlayer(){
        nativeHandle = nativeCreate();
    }

    public void prepare(){
        nativePrepare();
    }

    public void start(){
        nativeStart();
    }

    public void setDataSource(String source){
        nativeSetDataSource(source);
    }

    public void stop(){
        nativeStop();
    }

    public void release(){
        nativeRelease();
    }

    public void setSurfaceHolder(SurfaceHolder holder) {
        if (mSurfaceHolder != null){
            mSurfaceHolder.removeCallback(this);
        }
        mSurfaceHolder = holder;
        mSurfaceHolder.addCallback(this);
    }

    /**
     * @return seconds
     */
    public int getDuration() {
        return nativeGetDuration();
    }

    public void seek(int progress) {
        nativeSeek(progress);
    }

    public void pause() {
        nativePause();
    }

    public boolean isPause() {
        return nativeIsPause();
    }

    public void resume() {
        nativeResume();
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        setNativeSurface(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }

    ListenerInfo getListenerInfo() {
        if (mListenerInfo != null) {
            return mListenerInfo;
        }
        mListenerInfo = new ListenerInfo();
        return mListenerInfo;
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        getListenerInfo().mOnPreparedListener = onPreparedListener;
    }

    public void setOnCompletedListener(OnCompletedListener onCompletedListener) {
        getListenerInfo().mOnCompletedListener = onCompletedListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        getListenerInfo().mOnProgressListener = onProgressListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        getListenerInfo().mOnErrorListener = onErrorListener;
    }


    /******************** invoke by jni ********************/
    private void onPrepared(){
        if (getListenerInfo().mOnPreparedListener != null){
            getListenerInfo().mOnPreparedListener.onPrepared();
        }
    }

    private void onCompleted(){
        if (getListenerInfo().mOnCompletedListener != null){
            getListenerInfo().mOnCompletedListener.onCompleted();
        }
    }

    private void onProgress(int progress){
        if (getListenerInfo().mOnProgressListener != null){
            getListenerInfo().mOnProgressListener.OnProgress(progress);
        }
    }

    private void onError(int code,String desc){
        if (getListenerInfo().mOnPreparedListener != null){
            getListenerInfo().mOnErrorListener.onError(code,desc);
        }
    }


    /******************** jni method  ********************/
    private native long nativeCreate();

    private native void nativeStart();

    private native void nativePrepare();

    private native void nativeStop();

    private native void nativeSetDataSource(String source);

    private native void nativeRelease();

    private native void setNativeSurface(Surface surface);

    private native int nativeGetDuration();

    private native void nativeSeek(int progress);

    private native void nativePause();

    private native void nativeResume();

    private native boolean nativeIsPause();
}
