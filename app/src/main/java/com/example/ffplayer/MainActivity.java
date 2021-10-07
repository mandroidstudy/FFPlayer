package com.example.ffplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;

import com.example.ffplayer.databinding.ActivityMainBinding;
import com.example.ffplayer.listener.OnErrorListener;
import com.example.ffplayer.listener.OnPreparedListener;
import com.example.ffplayer.player.FFPlayer;

public class MainActivity extends AppCompatActivity {


    private ActivityMainBinding binding;
    private FFPlayer ffPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        ffPlayer = new FFPlayer();
        ffPlayer.setDataSource("");
        ffPlayer.setSurfaceHolder(binding.surfaceView.getHolder());
        binding.btnStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ffPlayer.prepare();
            }
        });

        ffPlayer.setOnPreparedListener(new OnPreparedListener(){
            @Override
            public void onPrepared() {
                ffPlayer.start();
            }
        });
        ffPlayer.setOnErrorListener(new OnErrorListener() {
            @Override
            public void onError(int code, String desc) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        binding.tvStatus.setText(desc);
                    }
                });
            }
        });
        setTextMarquee(binding.tvStatus);
    }

    public static void setTextMarquee(TextView textView) {
        if (textView != null) {
            textView.setSingleLine(true);
            textView.setEllipsize(TextUtils.TruncateAt.MARQUEE);
            textView.setSelected(true);
            textView.setFocusable(true);
            textView.setFocusableInTouchMode(true);
        }
    }
}