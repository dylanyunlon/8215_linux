package com.hcn.bluetooth.service;

import static com.hcn.bluetooth.api.Utils.BT_MUSIC_PACKAGE_NAME;

import android.app.Service;
import android.bluetooth.BluetoothA2dpSink;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothAvrcpController;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.media.MediaMetadata;
import android.media.MediaPlayer;
import android.media.session.MediaController;
import android.media.session.MediaSessionManager;
import android.media.session.PlaybackState;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.IBinder.DeathRecipient;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.sourceservice.SourceInfo;
import android.support.v4.media.session.MediaSessionCompat;
import android.util.Log;
import android.view.KeyEvent;

import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.LocalBTMusicManager;
import com.hcn.bluetooth.api.MusicPlayState;

import java.io.IOException;
import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.List;

public class BluetoothMusicService extends Service {
    public static final String TAG = "BluetoothMusicService";
    private static final String A2DP_MBS_TAG =  "A2dpMediaBrowserService";
    private static final String A2DP_MBS_TAG_Q = "BluetoothMediaBrowserService"; //兼容android Q的TAG

    //同行者指令：退出蓝牙音乐
    public static final String VOICE_EVENT_EXIT = "com.btmusic.extra.exit";
    //亿联申请使用a2dp音频通道,蓝牙收到广播需申请音频焦点
    public static final String EASYCONN_REQUEST_A2DP = "net.easyconn.a2dp.acquire";

    //对应IMusicCallback的回调方法
    private static final int CALL_METADATA_CHANGE = 0;
    private static final int CALL_PLAYSTATE_CHANGE = 1;
    private static final int CALL_AVRCP_CONNECT_STATE_CHANGE = 2;
    private static final int CALL_A2DP_CONNECT_STATE_CHANGE = 3;

    private Context mContext;
    private IBinder mMusicBinder = null;
    private MusicDeathRecipient mMusicDeathRecipient = null;
    private BluetoothAvrcpController mAvrcpService = null;
    private BluetoothA2dpSink mA2dpSinkService = null;
    private SourceInfo mSourceInfo=SourceInfo.getInstance();

    private int mAvrcpConnectState = BluetoothProfile.STATE_DISCONNECTED;
    private int mA2dpConnectState = BluetoothProfile.STATE_DISCONNECTED;

    private static BluetoothMusicService sInstance = null;
    private IBluetoothMusicService.Stub mBinder = null;

    private MediaMetadata mMediaMetadata;
    private MediaController mMediaController = null;
    private MediaController.Callback mMediaCtrlCallback = null;
    private MediaSessionManager mSessionManager;
    private MediaSessionManager.OnActiveSessionsChangedListener mSessionListener;
    private Handler mMainHandler;
    private RemoteCallbackList<IMusicCallback> mCallBackList;

    private final MusicPlayState mMusicPlayState = new MusicPlayState();
    private String[] mID3Array = new String[3];

    private AudioManager mAudioManager = null;
    protected ComponentName mComponentName;
    private MediaPlayer mPlayer;
    protected MediaSessionCompat mMediaSessionCompat;

    private static final int MSG_FILTER_PREV_NEXT_CMD = 0;
    private static final int MSG_FILTER_FORWARD_REWIND = 1;
    private static final int MSG_REQUEST_AUDIO_FOCUS = 10;
    private static final int MSG_ABANDON_AUDIO_FOCUS = 11;

    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_FILTER_PREV_NEXT_CMD:
                    //用于过滤上下曲指令,避免发送过快,此处无需处理
                    break;
                case MSG_FILTER_FORWARD_REWIND:
                    //用于过滤快进快退指令,避免发送过快,此处无需处理
                    break;
                case MSG_REQUEST_AUDIO_FOCUS:
                    if (!isGainAudioFocus()) {
                        Log.d(TAG, "handleMessage: MSG_REQUEST_AUDIO_FOCUS requestAudioFocus");
                        requestAudioFocus();
                        //Acc on后A2dpSinkStateMachine对象会重新创建，期间申请焦点会失败，所以循环申请
                        mMainHandler.sendEmptyMessageDelayed(MSG_REQUEST_AUDIO_FOCUS, 1000);
                    } else {
                        Log.d(TAG, "handleMessage: MSG_REQUEST_AUDIO_FOCUS Gain!!");
                    }
                    break;
                case MSG_ABANDON_AUDIO_FOCUS:
                    Log.d(TAG, "handleMessage: MSG_ABANDON_AUDIO_FOCUS");
                    abandonAudioFocus();
                    unregisterMediaButtonEvent();
                    break;
                default:
                    break;
            }
        }
    }

    public static synchronized BluetoothMusicService getInstance() {
        return sInstance;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        init();
        initSessionManager();
        super.onCreate();
    }

    public void init() {
        Log.i(TAG, "init");
        sInstance = this;
        mContext = getApplicationContext();
        mCallBackList = new RemoteCallbackList<IMusicCallback>();
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mMainHandler = new MainHandler(Looper.getMainLooper());
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (adapter != null) {
            adapter.getProfileProxy(mContext, mA2dpServiceListener,
                    BluetoothProfile.A2DP_SINK);
            adapter.getProfileProxy(mContext, mAvrcpServiceListener,
                    BluetoothProfile.AVRCP_CONTROLLER);
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(LocalBTMusicManager.ACTION_TRACK_EVENT);
        filter.addAction(VOICE_EVENT_EXIT);
        filter.addAction(BluetoothA2dpSink.ACTION_PLAYING_STATE_CHANGED);
        filter.addAction(BluetoothA2dpSink.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(BluetoothAvrcpController.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(BluetoothA2dpSink.A2DP_SINK_AUDIO_FOCUS_CHANGE);
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(EASYCONN_REQUEST_A2DP);
        mContext.registerReceiver(mReceiver, filter);

    }

    private void initSessionManager() {
        mSessionManager = (MediaSessionManager) getSystemService(Context.MEDIA_SESSION_SERVICE);

        mMediaCtrlCallback = new MediaControllerCallback();
        mSessionListener = new SessionChangeListener();
        // Listen on Active MediaSession changes, so we can get the active session's MediaController
        if (mSessionManager != null) {
            mSessionManager.addOnActiveSessionsChangedListener(mSessionListener, null,
                    mMainHandler);
            List<MediaController> controllers = mSessionManager.getActiveSessions(null);
            Log.d(TAG, " Num Sessions " + controllers.size());
            for (int i = 0; i < controllers.size(); i++) {
                Log.d(TAG, "Active session : " + i + (controllers.get(
                        i)).getPackageName() + (controllers.get(i)).getTag());
                MediaController controller = controllers.get(i);
                if ((controller.getTag().contains(A2DP_MBS_TAG)) || (controller.getTag().contains(A2DP_MBS_TAG_Q))) {
                    Log.d(TAG, "onCreate:contains A2DP_MBS_TAG");
                    setCurrentMediaController(controller);
                    PlaybackState state = controller.getPlaybackState();
                    if (null != state) {
                        mMusicPlayState.setState(state.getState());
                        mMusicPlayState.setPlaybackSpeed(state.getPlaybackSpeed());
                        mMusicPlayState.setBufferedPosition(state.getBufferedPosition());
                        mMusicPlayState.setPosition(state.getPosition());
                        mMusicPlayState.setLastPositionUpdateTime(
                                state.getLastPositionUpdateTime());
                    }
                }
            }
        }
    }

    private void unInitSessionManager() {
        if (null != mMediaController && null != mMediaCtrlCallback) {
            mMediaController.unregisterCallback(mMediaCtrlCallback);
        }
        if (null != mSessionManager && null != mSessionListener) {
            mSessionManager.removeOnActiveSessionsChangedListener(mSessionListener);
        }
    }

    private void unInit() {
        mContext.unregisterReceiver(mReceiver);
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (adapter != null) {
            adapter.closeProfileProxy(BluetoothProfile.A2DP_SINK, mA2dpSinkService);
            adapter.closeProfileProxy(BluetoothProfile.AVRCP_CONTROLLER, mAvrcpService);
        }
        unInitSessionManager();
        if (mBinder != null) {
            mBinder = null;
        }

    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        unInit();
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent arg0) {
        Log.d(TAG, "onBind");
        if (mBinder == null) {
            mBinder = new BluetoothMusicBinder();
        }
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "onUnbind");
        return super.onUnbind(intent);
    }

    private final BluetoothProfile.ServiceListener mAvrcpServiceListener =
            new BluetoothProfile.ServiceListener() {

                public void onServiceConnected(int profile, BluetoothProfile proxy) {
                    Log.d(TAG, "BluetoothAvrcp service connected");
                    mAvrcpService = (BluetoothAvrcpController) proxy;
                    mAvrcpConnectState = isAvrcpConnected() ? BluetoothProfile.STATE_CONNECTED
                            : BluetoothProfile.STATE_DISCONNECTED;
                }

                public void onServiceDisconnected(int profile) {
                    Log.d(TAG, "BluetoothAvrcp service disconnected");
                    mAvrcpService = null;
                    mAvrcpConnectState = BluetoothProfile.STATE_DISCONNECTED;
                }
            };

    private final BluetoothProfile.ServiceListener mA2dpServiceListener =
            new BluetoothProfile.ServiceListener() {

                public void onServiceConnected(int profile, BluetoothProfile proxy) {
                    Log.d(TAG, "BluetoothA2dpSink service connected");
                    mA2dpSinkService = (BluetoothA2dpSink) proxy;
                    mA2dpConnectState = isA2dpConnected() ? BluetoothProfile.STATE_CONNECTED
                            : BluetoothProfile.STATE_DISCONNECTED;
                }

                public void onServiceDisconnected(int profile) {
                    Log.d(TAG, "BluetoothA2dpSink service disconnected");
                    mA2dpSinkService = null;
                    mA2dpConnectState = BluetoothProfile.STATE_DISCONNECTED;
                }
            };

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(BluetoothA2dpSink.ACTION_PLAYING_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothA2dpSink.STATE_NOT_PLAYING);

            } else if (action.equals(BluetoothA2dpSink.ACTION_CONNECTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothA2dpSink.STATE_DISCONNECTED);
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                handleA2dpStateChanged(state, device);
            } else if (action.equals(BluetoothAvrcpController.ACTION_CONNECTION_STATE_CHANGED)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothProfile.STATE_DISCONNECTED);
                handleAvrcpStateChanged(state, device);
            } else if (action.equals(LocalBTMusicManager.ACTION_TRACK_EVENT)) {
//                PlaybackState pbb = intent.getParcelableExtra(EXTRA_PLAYBACK);
//                MediaMetadata mmd = intent.getParcelableExtra(EXTRA_METADATA);
            } else if (action.equals(VOICE_EVENT_EXIT)) {
                //通行者指令：退出蓝牙音乐    客户端同时接受此指令，结束activity
                Log.d(TAG, "onReceive: VOICE_EVENT_EXIT");
                send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_STOP);
            } else if (action.equals(BluetoothA2dpSink.A2DP_SINK_AUDIO_FOCUS_CHANGE)) {
                int state = intent.getIntExtra(BluetoothA2dpSink.EXTRA_AUDIO_STATE,
                        AudioManager.AUDIOFOCUS_LOSS);
                if (state == AudioManager.AUDIOFOCUS_GAIN) {
                    Log.d(TAG, "onReceive: AUDIOFOCUS_GAIN");
                    mMainHandler.removeMessages(MSG_REQUEST_AUDIO_FOCUS);
                    registerMediaButtonEvent();
                } else if (state == AudioManager.AUDIOFOCUS_LOSS) {
                    Log.d(TAG, "onReceive: AUDIOFOCUS_LOSS");
                    mMainHandler.removeMessages(MSG_REQUEST_AUDIO_FOCUS);
                    unregisterMediaButtonEvent();
                }
            } else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
                        BluetoothAdapter.STATE_OFF);
                if (state == BluetoothAdapter.STATE_OFF
                        || state == BluetoothAdapter.STATE_TURNING_OFF) {
                    if (mA2dpConnectState != BluetoothProfile.STATE_DISCONNECTED) {
                        handleA2dpStateChanged(BluetoothProfile.STATE_DISCONNECTED, null);
                    }
                    if (mAvrcpConnectState != BluetoothProfile.STATE_DISCONNECTED) {
                        handleAvrcpStateChanged(BluetoothProfile.STATE_DISCONNECTED, null);
                    }
                }
            } else if (action.equals(EASYCONN_REQUEST_A2DP)) {
                Log.d(TAG, "onReceive: EASYCONN_REQUEST_A2DP requestA2dp");
                requestA2dp();
            }
        }
    };

    private class MediaControllerCallback extends MediaController.Callback {
        @Override
        public void onPlaybackStateChanged(PlaybackState state) {
            if (state == null) {
                Log.e(TAG, " onPlaybackStateChanged...state==null");
                return;
            }

            if (mMediaMetadata == null) {
                mMediaMetadata = mMediaController.getMetadata();
                if (mMediaMetadata != null) {
                    handleMetadataChange();
                }
            }
            mMusicPlayState.setState(state.getState());
            mMusicPlayState.setPlaybackSpeed(state.getPlaybackSpeed());
            mMusicPlayState.setBufferedPosition(state.getBufferedPosition());
            mMusicPlayState.setPosition(state.getPosition());
            mMusicPlayState.setLastPositionUpdateTime(state.getLastPositionUpdateTime());

            if (mMusicPlayState.getPosition() <= mMusicPlayState.getDuration()) {
                callListener(CALL_PLAYSTATE_CHANGE);
            }
        }

        @Override
        public void onMetadataChanged(MediaMetadata metadata) {
            Log.d(TAG, " onMetadataChanged ");
            mMediaMetadata = metadata;
            handleMetadataChange();
        }
    }

    class SessionChangeListener implements MediaSessionManager.OnActiveSessionsChangedListener {
        /**
         * On the Phone side, it listens to the BluetoothSL4AAudioSrcMBS (that the SL4A app runs)
         * becoming active. On the Car side, it listens to the A2dpMediaBrowserService (associated
         * with the Bluetooth Audio App) becoming active. The idea is to get a handle to the
         * MediaController appropriate for the device, so that we can send and receive Media
         * commands.
         */
        @Override
        public void onActiveSessionsChanged(List<MediaController> controllers) {
            Log.d(TAG, " onActiveSessionsChanged : " + controllers.size());
            for (int i = 0; i < controllers.size(); i++) {
                MediaController controller = controllers.get(i);
                Log.d(TAG, "onActiveSessionsChanged : " + i + " " +controller.getPackageName()
                        + " Tag=" + controller.getTag());
            }
            for (int i = 0; i < controllers.size(); i++) {
                MediaController controller = controllers.get(i);
                if ((controller.getTag().contains(A2DP_MBS_TAG)) || (controller.getTag().contains(A2DP_MBS_TAG_Q))) {
                    Log.d(TAG, "onActiveSessionsChanged:contains A2DP_MBS_TAG");
                    setCurrentMediaController(controller);
                    return;
                }
            }
        }
    }

    private void setCurrentMediaController(MediaController controller) {
        Log.d(TAG, "setCurrentMediaController mMediaController="+mMediaController + " controller="+controller );
        if (controller == null) {
            Log.e(TAG, "setCurrentMediaController null!!!!");
        }

        if (mMediaController == null && controller != null) {
            Log.d(TAG, " Set MediaController " + controller.getTag());
            mMediaController = controller;
            mMediaController.registerCallback(mMediaCtrlCallback);
        } else if (mMediaController != null && controller != null) {
            // We have a new MediaController that we have to update to.
            if (!controller.getSessionToken().equals(mMediaController.getSessionToken())) {
                Log.d(TAG, " update a new MediaController " + controller.getTag());
                mMediaController.unregisterCallback(mMediaCtrlCallback);
                mMediaController = controller;
                mMediaController.registerCallback(mMediaCtrlCallback, mMainHandler);
            }
        } else if (mMediaController != null && controller == null) {
            // Clearing the current MediaController
            Log.d(TAG, " Clear MediaController " + mMediaController.getTag());
            mMediaController.unregisterCallback(mMediaCtrlCallback);
            mMediaController = controller;
        }
    }

    private void handleMetadataChange() {
        if (mMediaMetadata != null) {
            try {
                mID3Array[0] = mMediaMetadata.getString(MediaMetadata.METADATA_KEY_TITLE).trim();
            } catch (Exception e) {
                mID3Array[0] = "";
            }
            try {
                mID3Array[1] = mMediaMetadata.getString(MediaMetadata.METADATA_KEY_ARTIST).trim();
            } catch (Exception e) {
                mID3Array[1] = "";
            }
            try {
                mID3Array[2] = mMediaMetadata.getString(MediaMetadata.METADATA_KEY_ALBUM).trim();
            } catch (Exception e) {
                mID3Array[2] = "";
            }
            if (mMediaMetadata.containsKey(MediaMetadata.METADATA_KEY_DURATION)) {
                mMusicPlayState.setDuration(mMediaMetadata.getLong(
                        MediaMetadata.METADATA_KEY_DURATION));
            }
//            if (mMediaMetadata.containsKey(MediaMetadata.METADATA_KEY_GENRE)) {
//            }

//            if (mMediaMetadata.containsKey(MediaMetadata.METADATA_KEY_TRACK_NUMBER)) {
//            }
            callListener(CALL_METADATA_CHANGE);
        }
    }

    /**
     * 申请音频焦点，为了进入蓝牙音乐时停止其他源播放 {AudioManager.AUDIOFOCUS_GAIN, AudioManager
     * .AUDIOFOCUS_GAIN_TRANSIENT, AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK, AudioManager
     * .AUDIOFOCUS_GAIN_TRANSIENT_EXCLUSIVE}
     *
     * @return 0:FAILED   1:GRANTED
     */
    public boolean requestAudioFocus() {
        if (null == mA2dpSinkService) {
            return false;
        }
        mA2dpSinkService.controlAudioFocus(true);
        return true;
    }

    public boolean abandonAudioFocus() {
        if (null == mA2dpSinkService) {
            return false;
        }
        mA2dpSinkService.controlAudioFocus(false);
        return true;
    }

    public boolean isGainAudioFocus() {

        if (null == mA2dpSinkService) {
            return false;
        }
        return mA2dpSinkService.isGainAudioFocus();
    }

    public void onMediaKeyEvent(Intent intent) {
        KeyEvent event = intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
        if (event == null) {
            return;
        }
        int keycode = event.getKeyCode();
        int action = event.getAction();
        Log.d(TAG, "onMediaKeyEvent: keycode=" + keycode + " action=" + action);
        if (action == KeyEvent.ACTION_UP) {
            switch (keycode) {
                case KeyEvent.KEYCODE_MEDIA_STOP:
                    send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_STOP);
                    break;
                case KeyEvent.KEYCODE_MEDIA_PLAY:
                    send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_PLAY);
                    break;
                case KeyEvent.KEYCODE_MEDIA_PAUSE:
                    send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_PAUSE);
                    break;
                case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
                    send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_PLAY_PAUSE);
                    break;
                case KeyEvent.KEYCODE_MEDIA_NEXT:
                    send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_NEXT);
                    break;
                case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
                    send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_PREV);
                    break;
                default:
                    break;
            }
        } else if (action == KeyEvent.ACTION_DOWN) {
            switch (keycode) {
                case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
                    send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_FAST_FORWARD);
                    break;
                case KeyEvent.KEYCODE_MEDIA_REWIND:
                    send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_REWIND);
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * Media Session 回调
     * 处理媒体回调事件（媒体按键事件、媒体状态等）
     *
     * @author simon
     */
    private class MediaSessionCallback extends MediaSessionCompat.Callback {
        /**
         * 上下文环境引用
         * <p> 弱应用，避免未知场景内存泄露；
         */
        private Reference<Context> mContextRef;

        public MediaSessionCallback(Context context) {
            super();
            mContextRef = new WeakReference<>(context);
        }

        @Override
        public boolean onMediaButtonEvent(Intent mediaButtonEvent) {
            boolean result = super.onMediaButtonEvent(mediaButtonEvent);
            if (!result) {
                onMediaKeyEvent(mediaButtonEvent);
            }
            return result;
        }
    }

    /**
     * 获取媒体按键需播放一个空的音乐
     *
     * @return
     */
    private boolean playZeroMusic() {
        stopZeroMusic();
        Log.d(TAG, "playZeroMusic");
        mPlayer = new MediaPlayer();
        try {
            String uri = "system/media/audio/ringtones/zero.mp3";
            mPlayer.setDataSource(uri);
            mPlayer.prepare();
            mPlayer.start();
        } catch (IllegalArgumentException | SecurityException
                | IllegalStateException | IOException e) {
            e.printStackTrace();
        }

        return true;
    }

    /**
     * 停止播放
     *
     * @return
     */
    public boolean stopZeroMusic() {
        Log.d(TAG, "stopZeroMusic.");
        if (mPlayer != null) {
            if (mPlayer.isPlaying()) {
                mPlayer.stop();
            }
            mPlayer.reset();
            mPlayer.release();
            mPlayer = null;
        }
        return true;
    }

    void registerBTMusicCallback(IMusicCallback callback) {
        if (null == callback) {
            return;
        }
        mCallBackList.register(callback);
        Log.d(TAG, "registerBTMusicCallback: size=" + mCallBackList.getRegisteredCallbackCount());
        //注册后先回调一次
        try {
            callback.onMetadataChanged(mID3Array[0], mID3Array[1], mID3Array[2]);
            callback.onPlayStatusChanged(mMusicPlayState);
        } catch (RemoteException e) {
            Log.e(TAG, "registerBTMusicCallback: call back error!!!");
        }
    }

    public void unregisterBTMusicCallback(IMusicCallback callback) {
        if (null == callback) {
            return;
        }
        mCallBackList.unregister(callback);
        Log.d(TAG, "unregisterBTMusicCallback: size=" + mCallBackList.getRegisteredCallbackCount());
    }

    private synchronized void callListener(final int method) {
        int count = mCallBackList.beginBroadcast();
        try {
            for (int i = 0; i < count; i++) {
                IMusicCallback c = mCallBackList.getBroadcastItem(i);
                switch (method) {
                    case CALL_METADATA_CHANGE:
                        c.onMetadataChanged(mID3Array[0], mID3Array[1], mID3Array[2]);
                        break;
                    case CALL_PLAYSTATE_CHANGE:
                        c.onPlayStatusChanged(mMusicPlayState);
                        break;
                    case CALL_AVRCP_CONNECT_STATE_CHANGE:
                        c.onAvrcpConnectStateChanged(mAvrcpConnectState);
                        break;
                    case CALL_A2DP_CONNECT_STATE_CHANGE:
                        c.onA2dpConnectStateChanged(mA2dpConnectState);
                        break;
                    default:
                        break;
                }
            }
        } catch (RemoteException e) {

        } finally {
            mCallBackList.finishBroadcast();
        }
    }

    private void handleA2dpStateChanged(int state, BluetoothDevice device) {
        Log.d(TAG, "A2dpStateChanged:" + state);
        mA2dpConnectState = state;
        callListener(CALL_A2DP_CONNECT_STATE_CHANGE);
        if (state == BluetoothProfile.STATE_DISCONNECTED) {
            mMusicPlayState.reset();
            callListener(CALL_PLAYSTATE_CHANGE);
            mID3Array[0] = "";
            mID3Array[1] = "";
            mID3Array[2] = "";
            callListener(CALL_METADATA_CHANGE);
        } else if (state == BluetoothProfile.STATE_CONNECTED) {
            //a2dp连接后检测是蓝牙音乐源则申请焦点
            String sourcePackage = SystemProperties.get("sys.sourcing.package", "");
            if (sourcePackage.equals(BT_MUSIC_PACKAGE_NAME)) {
                Log.d(TAG, "A2dpStateChanged:STATE_CONNECTED requestA2dp");
                requestA2dp();
            }
        }

    }

    private void handleAvrcpStateChanged(int state, BluetoothDevice device) {
        Log.d(TAG, "AvrcpStateChanged:" + state);
        mAvrcpConnectState = state;
        callListener(CALL_AVRCP_CONNECT_STATE_CHANGE);
    }

    public synchronized boolean isA2dpConnected() {
        if (null == mA2dpSinkService) {
            return false;
        }
        List<BluetoothDevice> devices = mA2dpSinkService.getConnectedDevices();
        if (null == devices) {
            return false;
        }
        return devices.size() > 0;
    }

    public synchronized boolean isAvrcpConnected() {
        if (null == mAvrcpService) {
            return false;
        }
        List<BluetoothDevice> devices = mAvrcpService.getConnectedDevices();
        if (null == devices) {
            return false;
        }
        return devices.size() > 0;
    }

    public synchronized boolean isA2dpPlaying() {
        return mMusicPlayState.getState() == MusicPlayState.STATE_PLAYING;
    }

    public synchronized BluetoothDeviceInfo getConnectDevice() {
        if (null == mA2dpSinkService) {
            return null;
        }
        List<BluetoothDevice> devices = mA2dpSinkService.getConnectedDevices();
        if (devices.isEmpty()) {
            return null;
        } else {
            return new BluetoothDeviceInfo(devices.get(0));
        }
    }

    public synchronized void send_Avrcp_Cmd(int avrcp_cmd) {
        if (null == mMediaController) {
            Log.e(TAG, "send_Avrcp_Cmd: mMediaController null");
            return;
        }
        if (mA2dpConnectState != BluetoothProfile.STATE_CONNECTED) {
            Log.e(TAG, "send_Avrcp_Cmd: A2dp is not connected");
            return;
        }
        MediaController.TransportControls media_control = mMediaController.getTransportControls();

        if (null == media_control) {
            Log.e(TAG, "send_Avrcp_Cmd: TransportControls null");
            return;
        }
        Log.d(TAG, "send_Avrcp_Cmd: " + avrcp_cmd);
        switch (avrcp_cmd) {
            case LocalBTMusicManager.CMD_AVRCP_PLAY:
                media_control.play();
                break;
            case LocalBTMusicManager.CMD_AVRCP_PAUSE:
                media_control.pause();
                break;
            case LocalBTMusicManager.CMD_AVRCP_PLAY_PAUSE:
                if (mMusicPlayState.getState() == MusicPlayState.STATE_PLAYING) {
                    media_control.pause();
                } else {
                    media_control.play();
                }
                break;
            case LocalBTMusicManager.CMD_AVRCP_STOP:
                media_control.stop();
                break;
            case LocalBTMusicManager.CMD_AVRCP_NEXT:
                if (mMainHandler.hasMessages(MSG_FILTER_PREV_NEXT_CMD)) {
                    break;
                }
                mMainHandler.sendEmptyMessageDelayed(MSG_FILTER_PREV_NEXT_CMD, 400);
                media_control.skipToNext();
                break;
            case LocalBTMusicManager.CMD_AVRCP_PREV:
                if (mMainHandler.hasMessages(MSG_FILTER_PREV_NEXT_CMD)) {
                    break;
                }
                mMainHandler.sendEmptyMessageDelayed(MSG_FILTER_PREV_NEXT_CMD, 400);
                media_control.skipToPrevious();
                break;
            case LocalBTMusicManager.CMD_AVRCP_FAST_FORWARD:
                if (mMainHandler.hasMessages(MSG_FILTER_FORWARD_REWIND)) {
                    break;
                }
                mMainHandler.sendEmptyMessageDelayed(MSG_FILTER_FORWARD_REWIND, 400);
                media_control.fastForward();
                break;
            case LocalBTMusicManager.CMD_AVRCP_REWIND:
                if (mMainHandler.hasMessages(MSG_FILTER_FORWARD_REWIND)) {
                    break;
                }
                mMainHandler.sendEmptyMessageDelayed(MSG_FILTER_FORWARD_REWIND, 400);
                media_control.rewind();
                break;
            default:
                break;
        }
    }

    public synchronized void requestA2dp() {
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (null == adapter || adapter.getState() != BluetoothAdapter.STATE_ON) {
            Log.d(TAG, "requestA2dp return!!");
            return;
        }
        Log.d(TAG, "requestA2dp");
        //申请焦点,暂停其它音源
        mMainHandler.removeMessages(MSG_REQUEST_AUDIO_FOCUS);
        mMainHandler.sendEmptyMessage(MSG_REQUEST_AUDIO_FOCUS);
    }

    public synchronized void releaseA2dp() {
        Log.d(TAG, "releaseA2dp");
        mMainHandler.removeMessages(MSG_REQUEST_AUDIO_FOCUS);
        mMainHandler.removeMessages(MSG_ABANDON_AUDIO_FOCUS);
        mMainHandler.sendEmptyMessage(MSG_ABANDON_AUDIO_FOCUS);
    }

    private void registerMediaButtonEvent() {
        // 高版本不再使用过时的接口
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            if (mMediaSessionCompat == null) {
                mMediaSessionCompat = new MediaSessionCompat(mContext, TAG);
                mMediaSessionCompat.setMediaButtonReceiver(null);
                mMediaSessionCompat.setCallback(new MediaSessionCallback(mContext));
            }
            mMediaSessionCompat.setActive(true);
        } else {
            //注册媒体按键
            if (mComponentName == null) {
                mComponentName = new ComponentName(getPackageName(), BootReceiver.class.getName());
            }
            mAudioManager.registerMediaButtonEventReceiver(mComponentName);
        }
        playZeroMusic();
    }

    private void unregisterMediaButtonEvent() {
        // 高版本不再使用过时的接口
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            if (mMediaSessionCompat != null) {
                mMediaSessionCompat.release();
                mMediaSessionCompat = null;
            }
        } else {
            if (null != mComponentName) {
                mAudioManager.unregisterMediaButtonEventReceiver(mComponentName);
                mComponentName = null;
            }
        }
        stopZeroMusic();
    }

    public synchronized void regMusicClientBinder(IBinder cBinder) throws RemoteException {
        Log.i(TAG, "regMusicClientBinder");
        mMusicBinder = cBinder;
        if (null != mMusicBinder) {
            if (null == mMusicDeathRecipient) {
                mMusicDeathRecipient = new MusicDeathRecipient();
            }
            mMusicBinder.linkToDeath(mMusicDeathRecipient, 0);
        }
    }

    public synchronized void unRegMusicClientBinder() {
        Log.i(TAG, "unRegMusicClientBinder");
        if (null != mMusicBinder && null != mMusicDeathRecipient) {
            mMusicBinder.unlinkToDeath(mMusicDeathRecipient, 0);
        }
        mMusicDeathRecipient = null;
        mMusicBinder = null;
        if (mMusicPlayState.getState() == MusicPlayState.STATE_PLAYING) {
            send_Avrcp_Cmd(LocalBTMusicManager.CMD_AVRCP_STOP);
        }
        releaseA2dp();
    }

    private String[] getID3Info() {
        return mID3Array;
    }

    class MusicDeathRecipient implements DeathRecipient {

        @Override
        public void binderDied() {
            // TODO Auto-generated method stub
            unRegMusicClientBinder();
            Log.i(TAG, "MusicDeathRecipient binderDied");
        }

    }

    private final class BluetoothMusicBinder extends IBluetoothMusicService.Stub {

        @Override
        public boolean isA2dpConnected() {
            return BluetoothMusicService.this.isA2dpConnected();
        }

        @Override
        public boolean isAvrcpConnected() {
            // TODO Auto-generated method stub
            return BluetoothMusicService.this.isAvrcpConnected();
        }

        @Override
        public boolean isA2dpPlaying() {
            return BluetoothMusicService.this.isA2dpPlaying();
        }

        @Override
        public BluetoothDeviceInfo getConnectDevice() {
            return BluetoothMusicService.this.getConnectDevice();
        }

        @Override
        public void send_Avrcp_Cmd(int avrcp_cmd) {
            BluetoothMusicService.this.send_Avrcp_Cmd(avrcp_cmd);
        }

        @Override
        public void requestA2dp() {
            //传递蓝牙客户端包名，通知carservice切音源，用于第三方调用时，比如launcher
            mSourceInfo.onRequestPlayAudio("com.autochips.bluetooth/.BtMusicActivity");
            BluetoothMusicService.this.requestA2dp();
        }

        @Override
        public void releaseA2dp() {
            BluetoothMusicService.this.releaseA2dp();
        }

        @Override
        public void regMusicClientBinder(IBinder cBinder) throws RemoteException {
            BluetoothMusicService.this.regMusicClientBinder(cBinder);
        }

        @Override
        public void unRegMusicClientBinder() {
            BluetoothMusicService.this.unRegMusicClientBinder();
        }

        @Override
        public void registerBTMusicCallback(IMusicCallback callback) {
            BluetoothMusicService.this.registerBTMusicCallback(callback);
        }

        @Override
        public void unregisterBTMusicCallback(IMusicCallback callback) {
            BluetoothMusicService.this.unregisterBTMusicCallback(callback);
        }

        @Override
        public String[] getID3Info() {
            return BluetoothMusicService.this.getID3Info();
        }
    }
}
