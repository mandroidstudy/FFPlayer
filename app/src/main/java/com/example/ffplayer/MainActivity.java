package com.example.ffplayer;

import androidx.appcompat.app.AppCompatActivity;
import androidx.databinding.BaseObservable;
import androidx.databinding.Bindable;
import androidx.databinding.DataBindingUtil;
import androidx.databinding.ViewDataBinding;
import androidx.lifecycle.ViewModelProvider;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.SeekBar;

import com.example.ffplayer.databinding.ActivityMainBinding;
import com.example.ffplayer.listener.OnCompletedListener;
import com.example.ffplayer.listener.OnErrorListener;
import com.example.ffplayer.listener.OnPreparedListener;
import com.example.ffplayer.listener.OnProgressListener;
import com.example.ffplayer.player.FFPlayer;

public class MainActivity extends AppCompatActivity {


    private FFPlayer ffPlayer;
    Status status;
    int i= 0;
    private ActivityMainBinding binding;
    private boolean isTrackingTouch;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = DataBindingUtil.setContentView(this, R.layout.activity_main);
        binding.setLifecycleOwner(this);
        binding.setClickProxy(new ClickProxy());
        status = new Status();
        binding.setStatus(status);
        initView();
        initPlayer();
    }

    private void initView() {
        binding.seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser){
                    int current = status.duration * progress/100;
                    binding.tvTime.setText(current+"/"+ status.duration);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isTrackingTouch = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                isTrackingTouch = false;
                int progress = seekBar.getProgress();
                int playProgress = status.duration * progress / 100;
                ffPlayer.seek(playProgress);
            }
        });
    }

    private void initPlayer() {
        ffPlayer = new FFPlayer();
        String parh = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).getAbsoluteFile().getAbsolutePath();
        parh = parh + "/Camera/ffplay222.mp4";
//        parh = parh + "/Camera/VID20211006053718.mp4";
//        parh = "rtmp://58.200.131.2:1935/livetv/cctv14";
        Log.d("maoweiyi","path = "+ parh);
        ffPlayer.setDataSource(parh);
        ffPlayer.setSurfaceHolder(binding.surfaceView.getHolder());

        ffPlayer.setOnPreparedListener(() -> {
            status.setText("准备好了");
            status.duration = ffPlayer.getDuration();
            if (status.duration  > 0){
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        binding.tvTime.setText("time = "+ status.duration);
                    }
                });
            }
            ffPlayer.start();
        });
        ffPlayer.setOnErrorListener((code, desc) -> runOnUiThread(new Runnable() {
            @Override
            public void run() {
                status.setText("出错了 code = "+code+ ", desc=" + desc);
            }
        }));
        ffPlayer.setOnCompletedListener(new OnCompletedListener() {
            @Override
            public void onCompleted() {
                status.setText("完成了");
            }
        });
        ffPlayer.setOnProgressListener(new OnProgressListener() {
            @Override
            public void OnProgress(int progress) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (!isTrackingTouch){
                            binding.tvTime.setText(progress+"/"+ status.duration);
                            binding.seekBar.setProgress(progress * 100 /  status.duration);
                        }
                    }
                });
            }
        });
    }


    public class ClickProxy{

        public void onStart(){
            status.setText("init");
            ffPlayer.prepare();
        }

        public void pauseAndResume(){
            if (ffPlayer.isPause()){
                ffPlayer.resume();
            }else {
                ffPlayer.pause();
            }
        }
    }

    public class Status extends BaseObservable {
        public String text;
        public int duration;

        @Bindable
        public String getText() {
            return text;
        }

        public void setText(String text) {
            this.text = text;
            notifyPropertyChanged(BR.text);
        }
    }
}