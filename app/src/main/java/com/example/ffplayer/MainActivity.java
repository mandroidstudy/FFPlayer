package com.example.ffplayer;

import androidx.appcompat.app.AppCompatActivity;
import androidx.databinding.BaseObservable;
import androidx.databinding.Bindable;
import androidx.databinding.DataBindingUtil;
import androidx.lifecycle.ViewModelProvider;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;

import com.example.ffplayer.databinding.ActivityMainBinding;
import com.example.ffplayer.listener.OnCompletedListener;
import com.example.ffplayer.listener.OnErrorListener;
import com.example.ffplayer.listener.OnPreparedListener;
import com.example.ffplayer.player.FFPlayer;

public class MainActivity extends AppCompatActivity {


    private FFPlayer ffPlayer;
    Status status;
    int i= 0;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ActivityMainBinding binding = DataBindingUtil.setContentView(this, R.layout.activity_main);
        binding.setLifecycleOwner(this);
        binding.setClickProxy(new ClickProxy());
        status = new Status();
        binding.setStatus(status);
        ffPlayer = new FFPlayer();
        String parh = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).getAbsoluteFile().getAbsolutePath();
        parh = parh + "/Camera/ffplay222.mp4";
//        parh = parh + "/Camera/VID20211006053718.mp4";
        Log.d("maoweiyi","path = "+ parh);
        ffPlayer.setDataSource(parh);
        ffPlayer.setSurfaceHolder(binding.surfaceView.getHolder());

        ffPlayer.setOnPreparedListener(() -> {
            status.setText("准备好了");
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
    }


    public class ClickProxy{

        public void onStart(){
            status.setText("init");
            ffPlayer.prepare();
        }
    }

    public class Status extends BaseObservable {
        public String text;

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