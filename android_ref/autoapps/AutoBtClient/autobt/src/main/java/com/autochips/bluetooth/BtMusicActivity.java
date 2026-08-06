package com.autochips.bluetooth;

import android.bluetooth.BluetoothProfile;
import android.content.ComponentName;
import android.content.Intent;
import android.media.MediaMetadata;
import android.media.browse.MediaBrowser;
import android.media.session.MediaController;
import android.media.session.PlaybackState;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.autochips.bluetooth.manager.HBluetoothManager;
import com.autochips.bluetooth.manager.HStateBroadReceiver;
import com.autochips.bluetooth.util.HMarqueeUtil;
import com.autochips.bluetooth.util.HUtils;
import com.autochips.bluetooth.util.SkinUtils;
import com.autochips.bluetooth.view.HCircleSeekBar;
import com.hcn.bluetooth.api.ConnectionListener;
import com.hcn.bluetooth.api.LocalBTMusicManager;
import com.hcn.bluetooth.api.MusicPlayState;
import com.hcn.bluetooth.service.IMusicCallback;
import com.hcn.skin.support.app.SkinCompatActivity;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

public class BtMusicActivity extends SkinCompatActivity implements View.OnClickListener {
    private final String TAG = "BtMusicActivity";
    private HBluetoothManager mBluetoothManager = null;
    //
    private LogicHandler mHandler;
    private HMarqueeUtil mMarqueeUtil;
    //
    private TextView mTvMusicName;
    private TextView mTvMusicAlbum;
    private TextView mTvMusicArtist;
    private HCircleSeekBar mSeekbar;
    private View mLayoutControl;
    private TextView mTvA2dpState;
    private ImageButton mBtnPlay;
    //
    //a2dp/avrcp state
    private int mA2dpsinkstate = BluetoothProfile.STATE_DISCONNECTED;
    private int mAvrcpctstate = BluetoothProfile.STATE_DISCONNECTED;
    private boolean mLastPlayState = false;
    private boolean bFirstIn = true;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_btmusic);
        initView();
        connectMusicService();
        mHandler = new LogicHandler(this);
        logd("onCreate ");
    }

    @Override
    protected void onResume() {
        super.onResume();
        //保持界面布局不随状态栏变化
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        logd("onResume ");
        mAvrcpctstate = mBluetoothManager.isAvrcpConnected() ?
                BluetoothProfile.STATE_CONNECTED : BluetoothProfile.STATE_DISCONNECTED;
        mA2dpsinkstate = mBluetoothManager.isA2dpConnected() ?
                BluetoothProfile.STATE_CONNECTED : BluetoothProfile.STATE_DISCONNECTED;
        refreshConnectState();
        refreshPlayState(mBluetoothManager.isPlay());
        if(mMarqueeUtil != null) {
            mMarqueeUtil.startMarquee();
        }
        logd("onResume : mAvrcpctstate= " + mAvrcpctstate);
        if(mBluetoothManager.isAvrcpConnected()){
            //优化过快执行切源导致蓝牙无法设置控制回调,按键控制无效
            //过快的情况：蓝牙先设置回调，但是接下啦视频才销毁导致，回调置空。
            mHandler.postDelayed(()->{mBluetoothManager.requestA2dp();},300);
        }
        logd("onResume : isCanPlay= " + mBluetoothManager.isCanPlay());
        if(!mBluetoothManager.isPlay() && mBluetoothManager.isCanPlay()) {
            mHandler.sendEmptyMessageDelayed(LogicHandler.MSG_UPDATE_AUDIO_STATE, 2500);
        }

        mHandler.sendEmptyMessageDelayed(LogicHandler.MSG_UPDATE_MUSIC_INFO,200);
    }

    @Override
    protected void onStart() {
        super.onStart();
        logd("onStart");
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        logd("onRestart");
    }

    @Override
    protected void onPause() {
        super.onPause();
        logd("onPause");
        if(mMarqueeUtil != null) {
            mMarqueeUtil.stopMarquee();
        }
        if(mHandler != null && mHandler.hasMessages(LogicHandler.MSG_UPDATE_AUDIO_STATE)){
            mHandler.removeMessages(LogicHandler.MSG_UPDATE_AUDIO_STATE);
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        logd("onStop");
    }

    /**
     * ui
     */
    private void initView() {
        mTvMusicName = findViewById(SkinUtils.getId(R.id.id_music_name));
        mTvMusicArtist = findViewById(SkinUtils.getId(R.id.id_music_artist));
        mTvMusicAlbum = findViewById(SkinUtils.getId(R.id.id_music_album));
        //
        mMarqueeUtil = new HMarqueeUtil(new Handler());
        mMarqueeUtil.add(mTvMusicName);
        mMarqueeUtil.add(mTvMusicArtist);
        mMarqueeUtil.add(mTvMusicAlbum);
        mMarqueeUtil.startMarquee();
        //
        mSeekbar = findViewById(R.id.id_music_progress);
        mLayoutControl = findViewById(SkinUtils.getId(R.id.id_music_control_page));
        mTvA2dpState = findViewById(SkinUtils.getId(R.id.id_music_bt_state));
        findViewById(SkinUtils.getId(R.id.id_music_play_next)).setOnClickListener(this);
        findViewById(SkinUtils.getId(R.id.id_music_play_prev)).setOnClickListener(this);
        mBtnPlay = findViewById(SkinUtils.getId(R.id.id_music_play_pause));
        mBtnPlay.setOnClickListener(this);
        findViewById(SkinUtils.getId(R.id.id_music_eq)).setOnClickListener(this);
        findViewById(SkinUtils.getId(R.id.id_music_setting)).setOnClickListener(this);
    }

    private void refreshPlayId3(String title, String artist, String album) {
        logd("()->refreshPlayId3#  " + title + " , " + artist + " , " + album);
        boolean nNedRestart = false;
        if (!TextUtils.isEmpty(title)) {
            //过掉重复的刷新，主要是会影响跑马灯
            if(!title.equals(mTvMusicName.getText().toString())){
                mTvMusicName.setText(title);
                nNedRestart = true;
            }else{
                logd("refreshPlayId3 the same title : " + title);
            }
        } else {
            mTvMusicName.setText(getString(R.string.txt_music_deft_title_unknown));
        }
        if (!TextUtils.isEmpty(artist)) {
            if(!artist.equals(mTvMusicArtist.getText().toString())) {
                mTvMusicArtist.setText(artist);
                nNedRestart = true;
            }else{
                logd("refreshPlayId3 the same artist : " + artist);
            }
        } else {
            mTvMusicArtist.setText(getString(R.string.txt_music_deft_title_unknown));
        }
        if (!TextUtils.isEmpty(album)) {
            if(!album.equals(mTvMusicAlbum.getText().toString())) {
                mTvMusicAlbum.setText(album);
                nNedRestart = true;
            }else{
                logd("refreshPlayId3 the same album : " + album);
            }
        } else {
            mTvMusicAlbum.setText(getString(R.string.txt_music_deft_title_unknown));
        }
        if(mMarqueeUtil != null && nNedRestart){
            mMarqueeUtil.reStart();
        }
    }

    private void refreshPlayState(boolean isPlay) {
        if(mLastPlayState == isPlay){
            return;
        }
        logd("()->refreshPlayState#  mLastPlayState :" + mLastPlayState + " , isPlay : " + isPlay);
        mLastPlayState = isPlay;
        if (isPlay) {
            mBtnPlay.setImageResource(R.drawable.selector_music_btn_play);
            if(mHandler != null && mHandler.hasMessages(LogicHandler.MSG_UPDATE_AUDIO_STATE)) {
                mHandler.removeMessages(LogicHandler.MSG_UPDATE_AUDIO_STATE);
            }
        }else{
            mBtnPlay.setImageResource(R.drawable.selector_music_btn_pause);
        }
    }

    private void refreshPlayState(int pro, int max) {
        mSeekbar.setCurProcess(pro);
        mSeekbar.setMaxProcess(max);
    }

    private void refreshConnectState() {
        logw("()-> refreshConnectState # mA2dpsinkstate : " + mA2dpsinkstate);
        logw("()-> refreshConnectState # mAvrcpctstate : " + mAvrcpctstate);
        if(mA2dpsinkstate == BluetoothProfile.STATE_CONNECTED
            /*&& mAvrcpctstate == BluetoothProfile.STATE_CONNECTED*/){
            if(mLayoutControl.getVisibility() != View.VISIBLE){
                mLayoutControl.setVisibility(View.VISIBLE);
            }
            mTvA2dpState.setText(getString(R.string.txt_bt_state_connected));
            if(mHandler.hasMessages(LogicHandler.MSG_CONNECT_A2DP)){
                mHandler.removeMessages(LogicHandler.MSG_CONNECT_A2DP);
            }
        }else{
            if(mLayoutControl.getVisibility() == View.VISIBLE){
                mLayoutControl.setVisibility(View.GONE);
            }
            mTvA2dpState.setText(getString(R.string.txt_bt_state_disconnected));
            //概率出现hfp连接但是A2DP断开的情况
            if(!mHandler.hasMessages(LogicHandler.MSG_CONNECT_A2DP)){
                mHandler.sendEmptyMessageDelayed(LogicHandler.MSG_CONNECT_A2DP,3000);
            }
        }

        if(mAvrcpctstate == BluetoothProfile.STATE_CONNECTED && bFirstIn){
            bFirstIn = false;
            if(!mHandler.hasMessages(LogicHandler.MSG_UPDATE_AUDIO_STATE)){
                mHandler.sendEmptyMessageDelayed(LogicHandler.MSG_UPDATE_AUDIO_STATE,500);
            }
        }
    }

    private void logd(String msg) {
        Log.d(TAG, msg);
    }

    private void logw(String msg) {
        Log.w("BtMusic", msg);
    }

    /**
     * 连接
     */
    private void connectMusicService() {
        if(mBluetoothManager == null) {
            mBluetoothManager = BaseApplication.getInstance().getBluetoothAdapter();
            mBluetoothManager.registerMusicCallback(mBTMusicCallback);
        }

    }

    @Override
    public void onClick(View v) {
        switch(v.getId()){
            case R.id.id_music_eq:
                //sendBroadcast(new Intent("com.hcn.bt.ui.show.change"));
                HUtils.startApp(BtMusicActivity.this,"com.hcn.autosetting","com.hcn.autosetting.EqActivity");
                //HUtils.startApp(BtMusicActivity.this,"com.autochips.bluetooth","com.autochips.bluetooth.MainBluetoothActivity");
                this.overridePendingTransition(android.R.anim.fade_in,android.R.anim.fade_out);
                break;
            case R.id.id_music_play_pause:
                mBluetoothManager.musicPauseOrPlay();
                break;
            case R.id.id_music_play_prev:
                mBluetoothManager.musicPrev();
                break;
            case R.id.id_music_play_next:
                mBluetoothManager.musicNext();
                break;
            case R.id.id_music_setting:
                HUtils.startApp(BtMusicActivity.this,"com.autochips.bluetooth","com.autochips.bluetooth.MainBluetoothActivity");
                break;
            default:break;
        }
    }

    //TODO 更新
    class LogicHandler extends Handler {
        private final static int MSG_UPDATE_PLAY_ID3 = 1;
        private final static int MSG_UPDATE_PLAY_STATE = 2;
        private final static int MSG_UPDATE_PLAY_PRO = 3;
        private final static int MSG_UPDATE_CONN_STATE = 5;
        private final static int MSG_UPDATE_AUDIO_STATE = 6;
        private final static int MSG_UPDATE_MUSIC_INFO = 11;
        private final static int MSG_CONNECT_A2DP = 21;

        private WeakReference<BtMusicActivity> mWr;

        public LogicHandler(BtMusicActivity activity) {
            mWr = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            if (mWr != null && mWr.get() != null) {
                BtMusicActivity activity = mWr.get();
                switch (msg.what) {
                    case MSG_UPDATE_PLAY_ID3:
                        String[] obj = (String[]) msg.obj;
                        activity.refreshPlayId3(obj[0], obj[1], obj[2]);
                        break;
                    case MSG_UPDATE_PLAY_STATE:
                        activity.refreshPlayState((Boolean) msg.obj);
                        break;
                    case MSG_UPDATE_PLAY_PRO:
                        activity.refreshPlayState(msg.arg1, msg.arg2);
                        break;
                    case MSG_UPDATE_CONN_STATE:
                        activity.refreshConnectState();
                        break;
                    case MSG_UPDATE_AUDIO_STATE:
                        logd("MSG_UPDATE_AUDIO_STATE : isAvrcpConnected = " + mBluetoothManager.isAvrcpConnected());
                        if(mBluetoothManager != null && mBluetoothManager.isAvrcpConnected()){
                            logd("MSG_UPDATE_AUDIO_STATE");
                            if(!mBluetoothManager.isPlay()){
                                mBluetoothManager.requestA2dp();
                                mBluetoothManager.musicPlay();
                                sendEmptyMessageDelayed(MSG_UPDATE_AUDIO_STATE,3500);
                            }
                        }
                        break;
                    case MSG_UPDATE_MUSIC_INFO:
                        if(mBluetoothManager != null) {
                            String[] id3 = mBluetoothManager.getID3Info();
                            if (id3 != null) {
                                updateID3(id3);
                            }
                        }
                        break;
                    case MSG_CONNECT_A2DP:
                        boolean nHfpCon = mBluetoothManager.isBluetoothConnected();
                        String nConAddr = mBluetoothManager.getConnectAddress();
                        logd("MSG_CONNECT_A2DP # nHfpCon:"+nHfpCon + ",  nConAddr:"+nConAddr);
                        if(mBluetoothManager != null && nHfpCon && !TextUtils.isEmpty(nConAddr)){
                            mBluetoothManager.connectA2dp(nConAddr);
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        private void updateID3(String[] id3) {
            if (hasMessages(MSG_UPDATE_PLAY_ID3)) {
                removeMessages(MSG_UPDATE_PLAY_ID3);
            }
            obtainMessage(MSG_UPDATE_PLAY_ID3, id3).sendToTarget();
        }

        private void updateMusicState(boolean play) {
            if (hasMessages(MSG_UPDATE_PLAY_STATE)) {
                removeMessages(MSG_UPDATE_PLAY_STATE);
            }
            obtainMessage(MSG_UPDATE_PLAY_STATE, play).sendToTarget();
        }

        private void updateMusicProgress(int progress, int max) {
            if (hasMessages(MSG_UPDATE_PLAY_PRO)) {
                removeMessages(MSG_UPDATE_PLAY_PRO);
            }
            obtainMessage(MSG_UPDATE_PLAY_PRO, progress, max).sendToTarget();
        }

        private void updateBtState() {
            if (hasMessages(MSG_UPDATE_CONN_STATE)) {
                removeMessages(MSG_UPDATE_CONN_STATE);
            }
            obtainMessage(MSG_UPDATE_CONN_STATE).sendToTarget();
        }
    }

    private void requestId3(){
        if (mHandler != null) {
            String[] id3 = mBluetoothManager.getID3Info();
            if(id3 != null) {
                mHandler.updateID3(mBluetoothManager.getID3Info());
            }
        }
    }

    private final IMusicCallback mBTMusicCallback = new IMusicCallback.Stub() {

        @Override
        public void onA2dpConnectStateChanged(int state) throws RemoteException {
            logd("()->onA2dpConnectStateChanged : " + state);
            mA2dpsinkstate = state;
            if (mHandler != null) {
                mHandler.updateBtState();
            }
        }

        @Override
        public void onAvrcpConnectStateChanged(int state) throws RemoteException {
            logd("()->onAvrcpConnectStateChanged : " + state);
            mAvrcpctstate = state;
            if (mHandler != null) {
                mHandler.updateBtState();
            }
        }

        @Override
        public void onMetadataChanged(String title, String artist, String album)
                throws RemoteException {
            logd(" ()->onMetadataChanged : " + title + ", " + artist + " , " + album);
            if (mHandler != null) {
                mHandler.updateID3(new String[]{title, artist, album});
            }
        }

        @Override
        public void onPlayStatusChanged(MusicPlayState musicPlayState) throws RemoteException {
            if (mHandler != null && musicPlayState != null) {
                //logd("()_>onPlayStatusChanged pos : " + musicPlayState.getPosition());
                //logd("()_>onPlayStatusChanged dur : " + musicPlayState.getDuration());
                //logd("()_>onPlayStatusChanged state : " + musicPlayState.getState());
                int state = musicPlayState.getState();
                mHandler.updateMusicState(state == MusicPlayState.STATE_PLAYING);
                if (state != MusicPlayState.STATE_PAUSED) {
                    int pro = (int) (musicPlayState.getPosition() * 0.001f);
                    int dur = (int) (musicPlayState.getDuration() * 0.001f);
                    mHandler.updateMusicProgress(pro, dur);
                }
            }
        }
    };


    //test
    private MediaBrowser mMediaBrowser = null;
    private MediaController mMediaController = null;
    private int mTryConnectMediaSessionCnt = 0;
    private static final String BT_BROWSED_PACKAGE = "com.android.bluetooth";
    private static final String BT_BROWSED_SERVICE =
            "com.android.bluetooth.avrcpcontroller.BluetoothMediaBrowserService";

    private void connectMediaBrowser() {

        mMediaBrowser = new MediaBrowser(this,
                new ComponentName(BT_BROWSED_PACKAGE, BT_BROWSED_SERVICE),
                mConnectionCallback, null);
        mMediaBrowser.connect();
        mTryConnectMediaSessionCnt++;
    }

    private MediaBrowser.ConnectionCallback mConnectionCallback = new MediaBrowser.ConnectionCallback() {
        @Override
        public void onConnected() {
            Log.d(TAG, "onConnected: session token " + mMediaBrowser.getSessionToken());
            if (mMediaBrowser.getSessionToken() == null) {
                throw new IllegalArgumentException("No Session token");
            }
            //get mediaContoller
            mMediaController = new MediaController(
                    BtMusicActivity.this, mMediaBrowser.getSessionToken());

            mTryConnectMediaSessionCnt = 0;
            mHandler.sendEmptyMessageDelayed(99, 2000);
            mMediaController.registerCallback(new MediaController.Callback() {
                @Override
                public void onMetadataChanged(@Nullable MediaMetadata metadata) {
                    super.onMetadataChanged(metadata);
                    Log.d(TAG, "" +
                            metadata.getString(MediaMetadata.METADATA_KEY_ALBUM));
                }

                @Override
                public void onPlaybackStateChanged(@Nullable PlaybackState state) {
                    super.onPlaybackStateChanged(state);
                    Log.d(TAG, "" +
                            state.getState());
                }
            });

        }

        @Override
        public void onConnectionFailed() {
            Log.d(TAG, "onConnectionFailed");
            if (mTryConnectMediaSessionCnt <= 3) {
                connectMediaBrowser();
            }
        }

        @Override
        public void onConnectionSuspended() {
            Log.d(TAG, "onConnectionSuspended");
            mTryConnectMediaSessionCnt = 0;
        }
    };
}
