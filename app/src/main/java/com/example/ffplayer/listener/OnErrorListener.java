package com.example.ffplayer.listener;

/**
 * @author maoweiyi
 * @time 2021/9/25
 * @describe
 */
@FunctionalInterface
public interface OnErrorListener {
    void onError(int code,String desc);
}
