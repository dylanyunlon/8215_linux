package com.autochips.bluetooth;

import android.animation.ObjectAnimator;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemClock;
import android.text.TextUtils;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.LinearInterpolator;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.autochips.bluetooth.skin.SkinID;
import com.autochips.bluetooth.skin.SkinUtils;
import com.autochips.bluetooth.skin.ThemeUtilsEx;
import com.autochips.bluetooth.utils.Utility;
import com.autochips.bluetooth.utils.WallpaperUtil;
import com.hcn.auto.utils.HImageUtils;
import com.hcn.bluetooth.api.ConnectionListener;
import com.hcn.bluetooth.api.LocalBTMusicManager;
import com.hcn.bluetooth.api.MusicPlayState;
import com.hcn.bluetooth.api.Utils;
import com.hcn.bluetooth.service.IMusicCallback;
import com.hcn.skin.support.app.SkinCompatActivity;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.util.Objects;


public class BtMusicActivity extends SkinCompatActivity implements View.OnClickListener {

    private static final String TAG = "BTMusicActivity";
    //同行者指令：退出蓝牙音乐
    public static final String VOICE_EVENT_EXIT = "com.btmusic.extra.exit";
    public static final String NEED_STATUS_BAR_CHANGE = "need_status_bar_change";
    public static final int NO_STATUS_BAR_CHANGE = 0;
    public static final int STATUS_BAR_CHANGE = 1;

    private Button btnPause;
    private Button btnPlay;
    private ProgressBar mMusicPlayingProgressBar = null;
    private TextView a2dpsinkStateInfo;
    private TextView avrcpctStateInfo;
    private TextView mediaTitleInfo;
    private TextView mediaArtistInfo;
    private TextView mediaAlbumInfo;
    private TextView mediaPlayingPositionInfo;
    private TextView mediaLengthInfo;
    private ImageButton btnVolDown;
    private ImageButton btnVolUp;
    private ImageButton btnStartBt;
    private ImageView connectState;
    private Button btn_music_eq;
    private ImageView playingAnimation = null;
    private ObjectAnimator mRotateAnim = null;
    private ViewGroup llMain = null;

    //a2dp/avrcp state
    public int mA2dpsinkstate = BluetoothProfile.STATE_DISCONNECTED;
    public int mAvrcpctstate = BluetoothProfile.STATE_DISCONNECTED;

    LocalBTMusicManager mBluetoothMusicManager;
    private MusicPlayState mLastMusicPlayState;

    private String musicTitle = "";
    private String musicArtist = "";
    private String musicAlbum = "";

    private int musicDuration = 0;
    private int musicPosition = 0;


    private AudioManager mAudioManager = null;

    private MainHandler mMainHandler = null;
    public static final int MSG_UPDATE_METADATA = 0x01;
    public static final int MSG_A2DP_CONNECT_STATE_CHANGE = 0x02;
    public static final int MSG_AVRCP_CONNECT_STATE_CHANGE = 0x03;
    public static final int MSG_UPDATE_SEEKBAR = 0x04;
    public static final int MSG_UPDATE_PLAY_STATE = 0x05;

    private class MainHandler extends Handler {
        public MainHandler(@NonNull Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_A2DP_CONNECT_STATE_CHANGE:
                    updateA2dpConnectStatus(mA2dpsinkstate);
                    break;
                case MSG_AVRCP_CONNECT_STATE_CHANGE:
                    updateAvrcpConnectStatus(mAvrcpctstate);
                    break;
                case MSG_UPDATE_METADATA:
                    updateMetadata(musicTitle, musicArtist, musicAlbum);
                    break;
                case MSG_UPDATE_SEEKBAR:
                    updateMusicPlayingProgress();
                    if (null != mLastMusicPlayState
                            && mLastMusicPlayState.getState() == MusicPlayState.STATE_PLAYING) {
                        sendEmptyMessageDelayed(MSG_UPDATE_SEEKBAR, 1000);
                    }
                    break;
                case MSG_UPDATE_PLAY_STATE:
                    if (null != mLastMusicPlayState) {
                        updatePlayPauseButton(mLastMusicPlayState.getState());
                    }
                    break;
                default:
                    break;
            }
        }
    }

    private void updateMusicPlayingProgress() {
        if (mLastMusicPlayState != null) {
            int play_state = mLastMusicPlayState.getState();
            switch (play_state) {
                case MusicPlayState.STATE_PLAYING:
                    musicPosition = (int) (mLastMusicPlayState.getPosition() * 0.001f);
                    int timeDelta = (int) ((SystemClock.elapsedRealtime()
                            - mLastMusicPlayState.getLastPositionUpdateTime()) * 0.001f);
                    musicPosition += timeDelta * mLastMusicPlayState.getPlaybackSpeed();
                    musicDuration = (int) (mLastMusicPlayState.getDuration() * 0.001f);
                    if (musicPosition > musicDuration) {
                        musicPosition = 0;
                    }
                    break;
                default:
                    musicDuration = (int) (mLastMusicPlayState.getDuration() * 0.001f);
                    musicPosition = (int) (mLastMusicPlayState.getPosition() * 0.001f);
                    if (musicPosition > musicDuration) {
                        musicPosition = 0;
                    }
                    break;
            }
        }

        String str_position = DateUtils.formatElapsedTime(musicPosition);
        String str_duration = DateUtils.formatElapsedTime(musicDuration);
        Log.d(TAG, "updateProgress: pos=" + str_position + " duration=" + str_duration);
        mediaLengthInfo.setText(str_duration);
        mediaPlayingPositionInfo.setText(str_position);
        mMusicPlayingProgressBar.setMax(musicDuration);
        mMusicPlayingProgressBar.setProgress(musicPosition);
    }

    private void updateMetadata(String title, String artist, String album) {
        if (!TextUtils.isEmpty(title)) {
            mediaTitleInfo.setText(title);
        } else {
            mediaTitleInfo.setText(SkinUtils.getString(R.string.str_music_title));
        }

        if (!TextUtils.isEmpty(artist)) {
            mediaArtistInfo.setText(artist);
        } else {
            mediaArtistInfo.setText(SkinUtils.getString(R.string.str_music_artist));
        }

        if (!TextUtils.isEmpty(album)) {
            mediaAlbumInfo.setText(album);
        } else {
            mediaAlbumInfo.setText(SkinUtils.getString(R.string.str_music_album));
        }
    }

    private void updatePlayPauseButton(int playState) {
        switch (playState) {
            case MusicPlayState.STATE_PLAYING:
                btnPause.setVisibility(View.VISIBLE);
                btnPlay.setVisibility(View.GONE);
                if (mRotateAnim != null) {
                    mRotateAnim.resume();
                }
                break;
            default:
                btnPause.setVisibility(View.GONE);
                btnPlay.setVisibility(View.VISIBLE);
                if (mRotateAnim != null) {
                    mRotateAnim.pause();
                }
                break;
        }
    }

    private void updateA2dpConnectStatus(int state) {
        if (state == BluetoothProfile.STATE_CONNECTED) {
            a2dpsinkStateInfo.setText(SkinUtils.getString(R.string.a2dpsink_status_connected_info));
            connectState.setBackground(SkinUtils.getDrawable(R.drawable.bt_connect));
        } else {
            a2dpsinkStateInfo.setText(SkinUtils.getString(R.string.a2dpsink_status_notconnected_info));
            //这里改用setBackground，原因是如果这个view是皮肤包里面的，那么他的context就是皮肤的context，
            //而setBackGroundResource(ID)这个方法里面是在当前这个view的context中找资源，当前context中不一定包含
            //对应ID的资源除非是本体的context，以下方式能保证不会报错。
            connectState.setBackground(SkinUtils.getDrawable(R.drawable.bt_unconnect));
        }
    }

    private void updateAvrcpConnectStatus(int state) {
        if (state == BluetoothProfile.STATE_CONNECTED) {
            avrcpctStateInfo.setText(SkinUtils.getString(R.string.avrcpct_status_connected_info));
        } else {
            avrcpctStateInfo.setText(SkinUtils.getString(R.string.avrcpct_status_notconnected_info));
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Window window = getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.TRANSPARENT);
        }
        super.onCreate(savedInstanceState);

        setContentView(R.layout.bt_music);

        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mMainHandler = new MainHandler(Looper.getMainLooper());
        mBluetoothMusicManager = LocalBTMusicManager.getInstance().init(this);
        mBluetoothMusicManager.addConnectListener(mConnectionListener);
        mBluetoothMusicManager.regMusicClientBinder(MyApplication.getInstance().getBinder());

        initView();

        IntentFilter filter = new IntentFilter();
        filter.addAction(VOICE_EVENT_EXIT);
        registerReceiver(mReceiver, filter);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Window window = getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            boolean isNight = isNight(getBaseContext().getResources().getConfiguration());
            //获取皮肤包里面的具体资源情况：判定该皮肤包是否需要状态栏进行背景颜色适配，0：不需要；1需要；
            boolean needStatusBarChange = needStatusBarChange();
            if (needStatusBarChange) {
                if (isNight) {
                    getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_VISIBLE);
                } else {
                    getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
                }
            } else {
                getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            }
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.TRANSPARENT);
        }
    }

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();
        updateWallpaperWithChangeMode(getResources().getConfiguration(), true);
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (SkinUtils.getInteger(R.integer.support_day_night_mode) == 1) {
            setContentView(R.layout.bt_music);
            initView();
        }
        updateWallpaperWithChangeMode(newConfig, false);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Window window = getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            //获取皮肤包里面的具体资源情况：判定该皮肤包是否需要状态栏进行背景颜色适配，0：不需要；1需要；
            boolean needStatusBarChange = needStatusBarChange();
            if (needStatusBarChange) {
                if (isNight(newConfig)) {
                    getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_VISIBLE);
                } else {
                    getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
                }
            } else {
                getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            }
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.TRANSPARENT);
        }
    }

    public boolean isNight(Configuration newConfig) {
        return (newConfig.uiMode & Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES;
    }

    public boolean needStatusBarChange() {
        int values = NO_STATUS_BAR_CHANGE;
        values = SkinCompatResources.getInstance().getInteger(NEED_STATUS_BAR_CHANGE);
        return STATUS_BAR_CHANGE == values;
    }

    private BroadcastReceiver receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            updateWallpaper();
        }
    };

    private void initView() {
        llMain = findViewById(SkinUtils.getId(R.id.ll_main));
        //a2dp state
        a2dpsinkStateInfo = findViewById(SkinUtils.getId(R.id.tv_A2DP_status));
        //avrcp state
        avrcpctStateInfo = findViewById(SkinUtils.getId(R.id.tv_AVRCP_status));

        connectState = findViewById(SkinUtils.getId(R.id.bt_connect_state));

        playingAnimation = findViewById(SkinUtils.getId(R.id.ivPlayingAnim));

//        btnVolDown = findViewById(R.id.btn_vol_down);
//        btnVolUp = findViewById(R.id.btn_vol_up);
//        btnStartBt = findViewById(R.id.btn_start_bt);

        if (btnVolDown != null) {
            btnVolDown.setOnClickListener(this);
        }
        if (btnVolUp != null) {
            btnVolUp.setOnClickListener(this);
        }
        if (btnStartBt != null) {
            btnStartBt.setOnClickListener(this);
        }

        //for play,pause,stop,next,pre buttons
        btnPause = findViewById(SkinUtils.getId(R.id.btn_music_pause));
        btnPlay = findViewById(SkinUtils.getId(R.id.btn_music_play));
        btn_music_eq = findViewById(SkinUtils.getId(R.id.btn_music_eq));
        if (btnPause != null) {
            btnPause.setOnClickListener(this);
        }
        if (btnPlay != null) {
            btnPlay.setOnClickListener(this);
        }
        if (btn_music_eq != null) {
            btn_music_eq.setOnClickListener(this);
        }

        findViewById(SkinUtils.getId(R.id.btn_music_prev)).setOnClickListener(this);
        findViewById(SkinUtils.getId(R.id.btn_music_next)).setOnClickListener(this);

        //for music title, artist, album, playingtime, totaltime information
        mediaTitleInfo = findViewById(SkinUtils.getId(R.id.tv_music_title));
        mediaArtistInfo = findViewById(SkinUtils.getId(R.id.tv_music_icon_artist));
        mediaAlbumInfo = findViewById(SkinUtils.getId(R.id.tv_music_icon_album));
        mediaPlayingPositionInfo = findViewById(SkinUtils.getId(R.id.tv_music_play_time));
        mediaLengthInfo = findViewById(SkinUtils.getId(R.id.tv_music_total_time));
        mMusicPlayingProgressBar = findViewById(SkinUtils.getId(R.id.tv_music_playing_progress));

        // 屏蔽mMusicPlayingProgressBar的触摸事件
        // 没有做进度条滑动功能--取消滑动
        if (!Objects.isNull(mMusicPlayingProgressBar)) {
            mMusicPlayingProgressBar.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    return true;
                }
            });
        }
        updateMusicPlayingProgress();
        updateMetadata(musicTitle, musicArtist, musicAlbum);
        updatePlayPauseButton(MusicPlayState.STATE_PAUSED);

        initAnimation();
    }

    /**
     * 通过配置更新壁纸
     * @param newConfig 新配置
     * @param isInit 是否初始化
     */
    private void updateWallpaperWithChangeMode(Configuration newConfig, boolean isInit){
        int flag = newConfig.uiMode & Configuration.UI_MODE_NIGHT_MASK;
        if (flag == Configuration.UI_MODE_NIGHT_YES || flag == Configuration.UI_MODE_NIGHT_NO) {
            updateWallpaper();
        }
        Log.i(TAG, "updateWallpaperWithChangeMode: " + isInit);
    }

    /**
     * 更新壁纸
     */
    public void updateWallpaper() {
        Drawable wallPaper = null;
        if (Utility.supportWallpaperCustomized()) {
            //加载用户设置的壁纸  /apd/appWallpaper/路径的壁纸
            String wallpaperPath = WallpaperUtil.getInstance(getApplicationContext())
                    .getShowWallpaperPath(getResources().getConfiguration());
            if (!TextUtils.isEmpty(wallpaperPath) && Utility.isValidPath(wallpaperPath)) {
                Bitmap bitmap = HImageUtils.getBitmap(wallpaperPath);
                if (!Objects.isNull(bitmap)) {
                    wallPaper = new BitmapDrawable(getResources(), bitmap);
                }
            }
        }

        if (Objects.isNull(wallPaper)) {
            wallPaper = ThemeUtilsEx.getAppShareBackground();
        }

        if (null != llMain && null != wallPaper) {
            llMain.setBackground(wallPaper);
        }
    }

    private void initAnimation() {
        if (null != playingAnimation) {
            mRotateAnim = ObjectAnimator.ofFloat(playingAnimation, "rotation", 0, 360);
            mRotateAnim.setDuration(5000);
            mRotateAnim.setInterpolator(new LinearInterpolator());
            mRotateAnim.setRepeatCount(ObjectAnimator.INFINITE);
            mRotateAnim.setRepeatMode(ObjectAnimator.RESTART);
            mRotateAnim.start();
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.i(TAG, "onStart");
        if (mBluetoothMusicManager.isReady()) {
            mLastMusicPlayState = null;
            mBluetoothMusicManager.registerBTMusicCallback(mBTMusicCallback);
        }
        IntentFilter filter = new IntentFilter("android.intent.action.ACTION_WALLPAPER_CHANGED");
        LocalBroadcastManager.getInstance(this).registerReceiver(receiver, filter);
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
        mBluetoothMusicManager.requestA2dp();
        boolean isAvrcpConnected = mBluetoothMusicManager.isAvrcpConnected();
        boolean isA2dpConnected = mBluetoothMusicManager.isA2dpConnected();
        if (isA2dpConnected) {
            mA2dpsinkstate = BluetoothProfile.STATE_CONNECTED;
        } else {
            mA2dpsinkstate = BluetoothProfile.STATE_DISCONNECTED;
        }
        if (isAvrcpConnected) {
            mAvrcpctstate = BluetoothProfile.STATE_CONNECTED;
        } else {
            mAvrcpctstate = BluetoothProfile.STATE_DISCONNECTED;
        }
        updateA2dpConnectStatus(mA2dpsinkstate);
        updateAvrcpConnectStatus(mAvrcpctstate);

        Log.i(TAG, "isAvrcpConnected=" + isAvrcpConnected + " isA2dpConnected=" + isA2dpConnected);
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.e(TAG, "unRegisterBTMusicCallback: " + mBTMusicCallback.hashCode());
        mBluetoothMusicManager.unRegisterBTMusicCallback(mBTMusicCallback);
        mMainHandler.removeMessages(MSG_UPDATE_SEEKBAR);
    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "onDestroy");
        super.onDestroy();
        if (null != mConnectionListener) {
            mBluetoothMusicManager.removeConnectListener(mConnectionListener);
        }
        if (null != mMainHandler) {
            mMainHandler.removeCallbacksAndMessages(null);
        }
        if (null != mReceiver) {
            unregisterReceiver(mReceiver);
        }
        LocalBroadcastManager.getInstance(this).unregisterReceiver(receiver);
    }


    @Override
    public void onClick(View v) {
        // TODO Auto-generated method stub
        int viewId = SkinUtils.getViewId(v);
        switch (viewId) {
            case R.id.btn_music_play:
                mBluetoothMusicManager.send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_PLAY);
                break;
            case R.id.btn_music_pause:
                mBluetoothMusicManager.send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_PAUSE);
                break;
            case R.id.btn_music_prev:
                mBluetoothMusicManager.send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_PREV);
                break;
            case R.id.btn_music_next:
                mBluetoothMusicManager.send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_NEXT);
                break;
            case R.id.btn_music_eq:
                //McuManager.getsInstance().injectKeyEventTimeout(McuConstant.K_EQ, 50);
                break;
//            case R.id.btn_vol_down:
//                onVolDown();
//                break;
//            case R.id.btn_vol_up:
//                onVolUp();
//                break;
            default:
                break;
        }
    }

    private void onVolUp() {
        mAudioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_RAISE,
                AudioManager.FLAG_SHOW_UI);
    }

    private void onVolDown() {
        mAudioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_LOWER,
                AudioManager.FLAG_SHOW_UI);
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(VOICE_EVENT_EXIT)) {
                exit();
            }
        }

    };

    private void exit() {
        String str_media_exit = Utils.getSystemProperty("persist.sys.media_exit", "1");
        if (str_media_exit.equals("1")) {
            try {
                mBluetoothMusicManager.unRegMusicClientBinder();
            } catch (RemoteException e) {

            }
        }
        finish();
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        exit();
    }

    private final IMusicCallback mBTMusicCallback = new IMusicCallback.Stub() {

        @Override
        public void onA2dpConnectStateChanged(int state) throws RemoteException {
            Log.d(TAG, "onA2dpConnectStateChanged:" + state);
            mA2dpsinkstate = state;
            mMainHandler.sendEmptyMessage(MSG_A2DP_CONNECT_STATE_CHANGE);
        }

        @Override
        public void onAvrcpConnectStateChanged(int state) throws RemoteException {
            Log.d(TAG, "onAvrcpConnectStateChanged:" + state);
            mAvrcpctstate = state;
            mMainHandler.sendEmptyMessage(MSG_AVRCP_CONNECT_STATE_CHANGE);
        }

        @Override
        public void onMetadataChanged(String title, String artist, String album)
                throws RemoteException {
            Log.d(TAG,
                    "onMetadataChanged: title=" + title + " artist=" + artist + " album=" + album);
            musicTitle = title;
            musicArtist = artist;
            musicAlbum = album;
            mMainHandler.sendEmptyMessage(MSG_UPDATE_METADATA);
        }

        @Override
        public void onPlayStatusChanged(MusicPlayState musicPlayState) throws RemoteException {
            if (null == musicPlayState) {
                Log.e(TAG, "onPlayStatusChanged: null!!");
                return;
            }
            Log.d(TAG, "onPlayStatusChanged:" + musicPlayState.toString());
            int state = musicPlayState.getState();
            if (null != mLastMusicPlayState && state == MusicPlayState.STATE_PAUSED) {
                mLastMusicPlayState = musicPlayState;
                mMainHandler.removeMessages(MSG_UPDATE_SEEKBAR);

                mMainHandler.removeMessages(MSG_UPDATE_PLAY_STATE);
                mMainHandler.sendEmptyMessage(MSG_UPDATE_PLAY_STATE);
            } else {
                mLastMusicPlayState = musicPlayState;
                mMainHandler.removeMessages(MSG_UPDATE_PLAY_STATE);
                mMainHandler.sendEmptyMessage(MSG_UPDATE_PLAY_STATE);

                mMainHandler.removeMessages(MSG_UPDATE_SEEKBAR);
                mMainHandler.sendEmptyMessage(MSG_UPDATE_SEEKBAR);
            }
        }
    };

    ConnectionListener mConnectionListener = new ConnectionListener() {
        @Override
        public void onServiceConnected() {
            Log.d(TAG, "BluetoothMusic onServiceConnected");
            mBluetoothMusicManager.regMusicClientBinder(MyApplication.getInstance().getBinder());
            mBluetoothMusicManager.registerBTMusicCallback(mBTMusicCallback);
            mBluetoothMusicManager.requestA2dp();

            boolean isAvrcpConnected = mBluetoothMusicManager.isAvrcpConnected();
            boolean isA2dpConnected = mBluetoothMusicManager.isA2dpConnected();
            if (isA2dpConnected) {
                mA2dpsinkstate = BluetoothProfile.STATE_CONNECTED;
            } else {
                mA2dpsinkstate = BluetoothProfile.STATE_DISCONNECTED;
            }
            if (isAvrcpConnected) {
                mAvrcpctstate = BluetoothProfile.STATE_CONNECTED;
            } else {
                mAvrcpctstate = BluetoothProfile.STATE_DISCONNECTED;
            }
            updateA2dpConnectStatus(mA2dpsinkstate);
            updateAvrcpConnectStatus(mAvrcpctstate);
        }

        @Override
        public void onServiceDisconnected() {
            Log.d(TAG, "BluetoothMusic onServiceDisconnected");
        }
    };

}