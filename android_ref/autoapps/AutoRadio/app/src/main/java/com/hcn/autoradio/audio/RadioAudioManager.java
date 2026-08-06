package com.hcn.autoradio.audio;

import android.Configures.HConfig;
import android.content.ComponentName;
import android.content.Context;
import android.media.AudioAttributes;
import android.media.AudioDevicePort;
import android.media.AudioDevicePortConfig;
import android.media.AudioFocusRequest;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioPatch;
import android.media.AudioPort;
import android.media.AudioPortConfig;
import android.media.AudioRecord;
import android.media.AudioSystem;
import android.media.AudioTrack;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.Settings;
import android.radio.RadioPlayer;
import android.support.v4.media.session.MediaSessionCompat;
import android.util.Log;

import com.hcn.autoradio.RadioMediaButtonReceiver;
import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.util.RadioUtils;

import java.io.IOException;
import java.util.ArrayList;

public class RadioAudioManager {
    private static final String TAG = "RadioAudioManager";
    private static RadioAudioManager mRadioAudioManager = null;
    private Context mContext = null;
    private AudioManager mAudioManager = null;

    private AudioPatch mAudioPatch = null;

    AudioDevicePort mAudioSource = null;
    AudioDevicePort mAudioSink = null;
    private boolean mIsForbidCreateAudioPatch = false;

    private MediaPlayer mPlayer;
    private int mDurationHint = -1;
    private AudioAttributes mStreamAttributes;
    private AudioFocusRequest mFocusRequest;

    private ReInitAudioSinkHandler reInitAudioSinkHandler = null;

    /**
     * Media Button 接受者
     * <p> 接收处理外部按键事件；
     */
    protected ComponentName mMediaButtonReceiver;
    protected MediaSessionCompat mMediaSessionCompat;

    private RadioAudioManager(Context context) {
        mContext = context;
        mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        mStreamAttributes = new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_MEDIA)
                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                .build();
        reInitAudioSinkHandler = new ReInitAudioSinkHandler(Looper.getMainLooper());
    }

    public static void init(Context context) {
        if (mRadioAudioManager == null) {
            mRadioAudioManager = new RadioAudioManager(context);
        }
    }

    public static RadioAudioManager getInstance() {
        if (mRadioAudioManager == null) {
            throw new RuntimeException("The 'init(Context context)' function was not called!");
        }

        return mRadioAudioManager;
    }

    public int getStreamVolume() {
        if (mAudioManager == null) {
            return -1;
        }
        return mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
    }

    public void setMute(boolean mute) {
        if (RadioUtils.getRadioModel().equalsIgnoreCase(RadioUtils.RADIO_INSIDE)) {
            if (null != mAudioTrack) {
                if (mute) {
                    //海外部分导航混音，需要mute，接口待添加
                    mAudioTrack.setVolume(0f);
                } else {
                    mAudioTrack.setVolume(1f);
                }
            }
        } else {
            RadioPlayer.getRadioPlayer().setMute(mute);
        }
    }

    public void setVolume(float gain) {
        if (!RadioUtils.getRadioModel().equalsIgnoreCase(RadioUtils.RADIO_INSIDE)) {
            return;
        }

        if (null != mAudioTrack) {
            mAudioTrack.setVolume(gain);
        }
    }

    /**
     * @param focusType  AudioManager.AUDIOFOCUS_GAIN AUDIOFOCUS_GAIN_TRANSIENT
     *                   AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK AUDIOFOCUS_GAIN_TRANSIENT_EXCLUSIVE
     * @param usage      AudioAttributes.USAGE_UNKNOWN USAGE_MEDIA USAGE_NOTIFICATION
     * @param streamType AudioAttributes.CONTENT_TYPE_MUSIC CONTENT_TYPE_SONIFICATION
     * @return AUDIOFOCUS_REQUEST_FAILED, AUDIOFOCUS_REQUEST_GRANTED or AUDIOFOCUS_REQUEST_DELAYED
     */
    public int requestAudioFocus(int focusType, int usage, int streamType) {
        if (mAudioManager == null) {
            return AudioManager.AUDIOFOCUS_REQUEST_FAILED;
        }
        mDurationHint = focusType;
        mStreamAttributes = new AudioAttributes.Builder()
                .setUsage(usage)
                .setContentType(streamType)
                .build();
        mFocusRequest = new AudioFocusRequest.Builder(focusType)
                .setAudioAttributes(mStreamAttributes)
                .setOnAudioFocusChangeListener(mAudioFocusChange)
                .setAcceptsDelayedFocusGain(true)
                .setWillPauseWhenDucked(true)
                .build();
        int ret = mAudioManager.requestAudioFocus(mFocusRequest);
        if (ret != AudioManager.AUDIOFOCUS_REQUEST_FAILED) {
            setMute(false);
            RadioUtils.setInputPathVolume(0x00,100);
        }
        return ret;
    }

    /**
     * @return 0:FAILED   1:GRANTED
     */
    public int releaseAudioFocus() {
        if (mAudioManager == null || mFocusRequest == null) {
            return AudioManager.AUDIOFOCUS_REQUEST_FAILED;
        }

        // 释放当前应用音频焦点
        mDurationHint = -1;
        return mAudioManager.abandonAudioFocusRequest(mFocusRequest);
    }

    /**
     * 注册媒体按键事件
     * <pre>
     *    监听外部 MediaButton 事件；
     *    注意：不请求音频焦点就不要去注册 MediaButton 事件，否则其它模块会注册失败；
     * </pre>
     */
    public void registerMediaButtonEvent() {
        // Audio Service
        if (null == mAudioManager) {
            mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        }

        // 高版本不再使用过时的接口
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            if (mMediaSessionCompat == null) {
                mMediaSessionCompat = new MediaSessionCompat(mContext, TAG);
                mMediaSessionCompat.setMediaButtonReceiver(null);
                mMediaSessionCompat.setCallback(new MediaSessionCallback(mContext));
            }
            mMediaSessionCompat.setActive(true);
        } else {
            if (mMediaButtonReceiver == null) {
                mMediaButtonReceiver = new ComponentName(
                        mContext.getPackageName(), RadioMediaButtonReceiver.class.getName());
            }
            mAudioManager.registerMediaButtonEventReceiver(mMediaButtonReceiver);
        }
        playNullFile();
    }

    /**
     * 取消媒体按键事件监听
     * <p> 如果不取消，其它模式如果注册将注册请求失败；
     *
     * @see #registerMediaButtonEvent()
     */
    public void unregisterMediaButtonEvent() {
        // 高版本不再使用过时的接口
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            if (mMediaSessionCompat != null) {
                mMediaSessionCompat.release();
                mMediaSessionCompat = null;
            }
        } else {
            if (mMediaButtonReceiver != null) {
                mAudioManager.unregisterMediaButtonEventReceiver(mMediaButtonReceiver);
                mMediaButtonReceiver = null;
            }
        }

        stopNullFile();
    }

    public int getAudioFocusType() {
        return mDurationHint;
    }

    /**
     * 音频焦点处理
     * <p> 注意系统音频焦点存在一定时序上的 BUG；
     */
    private AudioManager.OnAudioFocusChangeListener mAudioFocusChange =
            new AudioManager.OnAudioFocusChangeListener() {
                private int mFocusChange;

                @Override
                public void onAudioFocusChange(int focusChange) {
                    switch (focusChange) {
                        case AudioManager.AUDIOFOCUS_LOSS:
                            Log.d(TAG, "AUDIOFOCUS_LOSS");
                            mMainHandler.removeMessages(MSG_FOCUS_GAIN_DELAY_UNMUTE);
                            setMute(true);
                            releaseAudioFocus();
                            unregisterMediaButtonEvent();
                            break;
                        case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                            Log.d(TAG, "AUDIOFOCUS_LOSS_TRANSIENT");
                            mMainHandler.removeMessages(MSG_FOCUS_GAIN_DELAY_UNMUTE);
                            setMute(true);
                            break;
                        case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                            //短暂性丢失焦点并作降音处理
                            Log.d(TAG, "AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK");

                            // [如果系统有混音比例定义开关]
                            int audioBackgroundVolume = Settings.System.getInt(
                                    mContext.getContentResolver(), HConfig.audio_background_volume, 86);
                            if (audioBackgroundVolume >= 0 && audioBackgroundVolume <= 100) {
                                // [如果存在配置，则在 90% 的基准上做混音]
                                setVolume(0.7f * audioBackgroundVolume / 100);
                                //外挂收音模块音量
                                RadioUtils.setInputPathVolume(0x00,audioBackgroundVolume);
                            }
                            break;
                        case AudioManager.AUDIOFOCUS_GAIN:
                            Log.d(TAG, "AUDIOFOCUS_GAIN");
                            registerMediaButtonEvent();
                            mMainHandler.removeMessages(MSG_FOCUS_GAIN_DELAY_UNMUTE);
                            mMainHandler.sendEmptyMessageDelayed(MSG_FOCUS_GAIN_DELAY_UNMUTE,500);
                            break;
                        default:
                            break;
                    }
                    mFocusChange = focusChange;
                }
            };

    private final int MSG_FOCUS_GAIN_DELAY_UNMUTE = 1;

    private final Handler mMainHandler = new Handler(Looper.getMainLooper()){

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_FOCUS_GAIN_DELAY_UNMUTE:
                    setMute(false);
                    RadioUtils.setInputPathVolume(0x00,100);
                    break;
                default:
                    break;
            }
        }
    };

    public void removeCallbacksAndMessages() {
        if (null != mMainHandler) {
            mMainHandler.removeCallbacksAndMessages(null);
        }
    }

    /**
     * 播放音频，才能接收到 MediaButton；
     */
    private void playNullFile() {
        stopNullFile();
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
    }

    public void stopNullFile() {
        if (mPlayer != null) {
            if (mPlayer.isPlaying()) {
                mPlayer.stop();
            }

            mPlayer.reset();
            mPlayer.release();
            mPlayer = null;
        }
    }

    //-----------------------------Inside Radio begin------------------------------------
    private static final int AUDIO_FRAMES_TO_IGNORE_COUNT = 3;
    private final Object mRenderLock = new Object();
    private final Object mRenderingLock = new Object();

    private Thread mRenderThread = null;
    private AudioRecord mAudioRecord = null;
    private AudioTrack mAudioTrack = null;
    private final int mTrackSampleRate = 44100;
    private static final int SAMPLE_RATE = 44100;
    private static final int CHANNEL_CONFIG = AudioFormat.CHANNEL_CONFIGURATION_STEREO;
    private static final int AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;
    private static final int RECORD_BUF_SIZE = AudioRecord.getMinBufferSize(
            SAMPLE_RATE, CHANNEL_CONFIG, AUDIO_FORMAT);
    private boolean mIsRender = false;

    private static final int BUFFER_TARGET_MODE_STREAM_MS = 100;
    private static final int MILLIS_PER_SECOND = 1000;
    private static final int AUDIO_TRACK_SIZE = BUFFER_TARGET_MODE_STREAM_MS * 2 * 2 * SAMPLE_RATE / MILLIS_PER_SECOND;

    private synchronized boolean initAudioRecordSink() {
        Log.i(TAG, "initAudioRecordSink:" + FMDataControl.getPlatform());

        mAudioRecord = new AudioRecord(1998,//MediaRecorder.AudioSource.RADIO_TUNER
                SAMPLE_RATE, CHANNEL_CONFIG, AUDIO_FORMAT, RECORD_BUF_SIZE);
        if ("AC8227L".equals(FMDataControl.getPlatform())) {
            mAudioTrack = new AudioTrack(mStreamAttributes, new AudioFormat.Builder()
                    .setChannelMask(CHANNEL_CONFIG)
                    .setEncoding(AUDIO_FORMAT)
                    .setSampleRate(mTrackSampleRate)
                    .build(),
                    RECORD_BUF_SIZE, AudioTrack.MODE_STREAM,AudioManager.AUDIO_SESSION_ID_GENERATE);
        } else {
            mAudioTrack = new AudioTrack(mStreamAttributes, new AudioFormat.Builder()
                    .setChannelMask(CHANNEL_CONFIG)
                    .setEncoding(AUDIO_FORMAT)
                    .setSampleRate(mTrackSampleRate)
                    .build(),
                    AUDIO_TRACK_SIZE, AudioTrack.MODE_STREAM,AudioManager.AUDIO_SESSION_ID_GENERATE);
        }
        Log.i(TAG, "mAudioRecord state = " + mAudioRecord.getState() + " mAudioTrack state=" + mAudioTrack.getState());
        return mAudioRecord.getState() == AudioRecord.STATE_INITIALIZED && mAudioTrack.getState() == AudioTrack.STATE_INITIALIZED;
    }

    // 对策8321出现的问题[#JIRA MT8321-267]
    private class ReInitAudioSinkHandler extends Handler {
        public static final String TAG = "ReInitAudioHandler";
        public static final int INIT_AUDIO_RECORD_SINK = 1001;

        public ReInitAudioSinkHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message message) {
            super.handleMessage(message);

            switch (message.what) {
                case INIT_AUDIO_RECORD_SINK:
                    if (mAudioRecord != null && mAudioRecord.getState() == AudioRecord.STATE_UNINITIALIZED) {
                        mAudioRecord.release();
                        mAudioRecord = null;
                        mAudioRecord = new AudioRecord(1998,//MediaRecorder.AudioSource.RADIO_TUNER
                                SAMPLE_RATE, CHANNEL_CONFIG, AUDIO_FORMAT, RECORD_BUF_SIZE);
                        Log.i(TAG, "mAudioRecord state = " + mAudioRecord.getState());
                    }

                    if (mAudioTrack != null && mAudioTrack.getState() == AudioTrack.STATE_UNINITIALIZED) {
                        mAudioTrack.release();
                        mAudioTrack = null;
                        if ("AC8227L".equals(FMDataControl.getPlatform())) {
                            mAudioTrack = new AudioTrack(mStreamAttributes, new AudioFormat.Builder()
                                    .setChannelMask(CHANNEL_CONFIG)
                                    .setEncoding(AUDIO_FORMAT)
                                    .setSampleRate(mTrackSampleRate)
                                    .build(),
                                    RECORD_BUF_SIZE, AudioTrack.MODE_STREAM,AudioManager.AUDIO_SESSION_ID_GENERATE);
                        } else {
                            mAudioTrack = new AudioTrack(mStreamAttributes, new AudioFormat.Builder()
                                    .setChannelMask(CHANNEL_CONFIG)
                                    .setEncoding(AUDIO_FORMAT)
                                    .setSampleRate(mTrackSampleRate)
                                    .build(),
                                    AUDIO_TRACK_SIZE, AudioTrack.MODE_STREAM,AudioManager.AUDIO_SESSION_ID_GENERATE);
                        }
                        Log.i(TAG, "mAudioTrack state=" + mAudioTrack.getState());
                    }

                    if (mAudioRecord != null && mAudioRecord.getState() == AudioRecord.STATE_INITIALIZED &&
                            mAudioTrack != null && mAudioTrack.getState() == AudioTrack.STATE_INITIALIZED) {
                        mIsRender = true;
                        synchronized (mRenderLock) {
                            Log.i(TAG, "AudioRecordSink init success! notifying for mRenderLock!");
                            mRenderLock.notify();
                        }
                    } else {
                        Log.e(TAG, "AudioRecordSink init fail!");
                        reInitAudioSinkHandler.removeMessages(ReInitAudioSinkHandler.INIT_AUDIO_RECORD_SINK);
                        reInitAudioSinkHandler.sendEmptyMessageDelayed(ReInitAudioSinkHandler.INIT_AUDIO_RECORD_SINK, 1000);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    public synchronized void startRender() {
        if (!RadioUtils.getRadioModel().equalsIgnoreCase(RadioUtils.RADIO_INSIDE)) {
            return;
        }

        if ("AC8257".equals(FMDataControl.getPlatform())
                || RadioUtils.getProp("ro.mediatek.fm.audio_patch", "0").equals("1")) {
            createAudioPatch();
            return;
        }

        if (isRendering()) {
            return;
        }

        // need to create new audio record and audio play back track,
        // because input/output device may be changed.
        try {
            if (mAudioRecord != null
                    && mAudioRecord.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING) {
                mAudioRecord.stop();
                mAudioRecord.release();
                mAudioRecord = null;
                Log.i(TAG, "mAudioRecord = null when startRender");
            }
        } catch (IllegalStateException e) {
            Log.d(TAG, "startRender, IllegalStateException");
        } catch (NullPointerException e) {
            Log.d(TAG, "startRender, NullPointerException");
        }

        if (mAudioTrack != null) {
            stopAudioTrack();
        }

        if (initAudioRecordSink()) {
            Log.i(TAG, "initAudioRecordSink: success");
            mIsRender = true;
            synchronized (mRenderLock) {
                Log.i(TAG, "startRender: notifying for mRenderLock");
                mRenderLock.notify();
            }
        } else {
            Log.e(TAG, "initAudioRecordSink: fail");
            reInitAudioSinkHandler.removeMessages(ReInitAudioSinkHandler.INIT_AUDIO_RECORD_SINK);
            reInitAudioSinkHandler.sendEmptyMessageDelayed(ReInitAudioSinkHandler.INIT_AUDIO_RECORD_SINK, 1000);
        }
    }

    public synchronized void stopRender() {
        if (!RadioUtils.getRadioModel().equalsIgnoreCase(RadioUtils.RADIO_INSIDE)) {
            return;
        }

        if ("AC8257".equals(FMDataControl.getPlatform())
                || RadioUtils.getProp("ro.mediatek.fm.audio_patch", "0").equals("1")) {
            releaseAudioPatch();
            return;
        }

        synchronized (mRenderingLock) {
            Log.i(TAG, "stopRender_processing, mIsRender = " + isRendering());
            boolean localRender = isRendering();

            mIsRender = false;
            if (localRender) {
                try {
                    long wait = 200;
                    Log.i(TAG, "stopRender: waiting for mRenderingLock");
                    mRenderingLock.wait(wait);
                    if (mAudioRecord != null) {
                        mAudioRecord.stop();
                        mAudioRecord.release();
                        mAudioRecord = null;
                        Log.i(TAG, "mAudioRecord = null when stopRender");
                    }
                    if (mAudioTrack != null) {
                        stopAudioTrack();
                    }
                } catch (InterruptedException e) {
                    Log.w(TAG, "stopRender, thread is interrupted");
                } catch (IllegalStateException e) {
                    Log.d(TAG, "stopRender, IllegalStateException");
                } catch (NullPointerException e) {
                    Log.d(TAG, "stopRender, NullPointerException");
                }
            }
        }
    }

    public synchronized void createRenderThread() {
        if (!RadioUtils.getRadioModel().equalsIgnoreCase(RadioUtils.RADIO_INSIDE)) {
            return;
        }

        if (!"AC8257".equals(FMDataControl.getPlatform())
                && RadioUtils.getProp("ro.mediatek.fm.audio_patch", "0").equals("0")) {
            Log.d(TAG, "createRenderThread");
            if (mRenderThread == null) {
                mRenderThread = new RenderThread();
                mRenderThread.start();
            }
        }
    }

    public synchronized void exitRenderThread() {
        if (!RadioUtils.getRadioModel().equalsIgnoreCase(RadioUtils.RADIO_INSIDE)) {
            return;
        }
        stopRender();

        Log.d(TAG, "exitRenderThread");
        if (mRenderThread != null) {
            mRenderThread.interrupt();
        }
        mRenderThread = null;
    }

    private boolean isRendering() {
        return mIsRender;
    }

    private void startAudioTrack() {
        Log.d(TAG, "startAudioTrack, mAudioTrack = " + mAudioTrack);
        if (mAudioTrack == null) {
            return;
        }

        try {
            if (mAudioTrack.getState() == AudioTrack.STATE_INITIALIZED
                    && mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_STOPPED) {
                mAudioTrack.play();
            }
        } catch (IllegalStateException e) {
            Log.d(TAG, "startAudioTrack, IllegalStateException");
        } catch (NullPointerException e) {
            Log.d(TAG, "startAudioTrack, NullPointerException");
        }
    }

    private void stopAudioTrack() {
        Log.d(TAG, "stopAudioTrack, mAudioTrack = " + mAudioTrack);
        if (mAudioTrack == null) {
            return;
        }
        try {
            if (mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING) {
                mAudioTrack.stop();
                mAudioTrack.release();
                mAudioTrack = null;
                Log.i(TAG, "mAudioTrack release & null");
            }
        } catch (IllegalStateException e) {
            Log.d(TAG, "stopAudioTrack, IllegalStateException");
        } catch (NullPointerException e) {
            Log.d(TAG, "stopAudioTrack, NullPointerException");
        }
    }

    class RenderThread extends Thread {
        private int mCurrentFrame = 0;

        private boolean isAudioFrameNeedIgnore() {
            return mCurrentFrame < AUDIO_FRAMES_TO_IGNORE_COUNT;
        }

        @Override
        public void run() {
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_AUDIO);
            try {
                byte[] buffer = new byte[RECORD_BUF_SIZE];
                int size = 0;

                Log.e(TAG, "RenderThread, interrupted = " + Thread.interrupted());
                while (!Thread.interrupted()) {
                    if (isRendering()) {
                        // Speaker mode or BT a2dp mode will come here and keep
                        // reading and writing.
                        // If we want FM sound output from speaker or BT a2dp,
                        // we must record data
                        // to AudioRecrd and write data to AudioTrack.
                        try {
                            if (mAudioRecord != null
                                    && mAudioRecord.getState() == AudioRecord.STATE_INITIALIZED
                                    && mAudioRecord.getRecordingState()
                                    == AudioRecord.RECORDSTATE_STOPPED) {
                                mAudioRecord.startRecording();
                                Log.i(TAG, "mAudioRecord startRecording speaker mode");
                            }
                            if (mAudioTrack != null
                                    && mAudioTrack.getState() == AudioTrack.STATE_INITIALIZED
                                    && mAudioTrack.getPlayState() == AudioTrack
                                    .PLAYSTATE_STOPPED) {
                                mAudioTrack.play();
                                Log.i(TAG, "mAudioTrack play speaker mode");
                            }
                            if (mAudioRecord != null) {
                                size = mAudioRecord.read(buffer, 0,
                                        RECORD_BUF_SIZE);
                            }
                        } catch (IllegalStateException e) {
                            Log.d(TAG, "RenderThread, IllegalStateException");
                        } catch (NullPointerException e) {
                            Log.d(TAG, "RenderThread, NullPointerException");
                        }

                        // check whether need to ignore first 3 frames audio
                        // data from AudioRecord
                        // to avoid pop noise.
                        if (isAudioFrameNeedIgnore()) {
                            mCurrentFrame += 1;
                            synchronized (mRenderingLock) {
                                Log.i(TAG,
                                        "RenderThread: notifying for mRenderingLock 1");
                                mRenderingLock.notify();
                            }
                            continue;
                        }
                        if (size <= 0) {
                            Log.e(TAG, "RenderThread read data from AudioRecord "
                                    + "error size: " + size);
                            sleep(100);
                            synchronized (mRenderingLock) {
                                Log.i(TAG,
                                        "RenderThread: notifying for mRenderingLock 2");
                                mRenderingLock.notify();
                            }
                            continue;
                        }
                        byte[] tmpBuf = new byte[size];
                        System.arraycopy(buffer, 0, tmpBuf, 0, size);
                        // Check again to avoid noises, because mIsRender may be
                        // changed
                        // while AudioRecord is reading.
                        try {
                            if (isRendering() && (mAudioTrack != null)) {
                                mAudioTrack.write(tmpBuf, 0, tmpBuf.length);
                            }
                        } catch (IllegalStateException e) {
                            Log.d(TAG, "RenderThread, IllegalStateException");
                        } catch (NullPointerException e) {
                            Log.d(TAG, "RenderThread, NullPointerException");
                        }
                        synchronized (mRenderingLock) {
                            // Log.i(TAG,
                            // "RenderThread: notifying for mRenderingLock");
                            mRenderingLock.notify();
                        }
                    } else {
                        // Earphone mode will come here and wait.
                        mCurrentFrame = 0;
                        try {
                            if (mAudioTrack != null
                                    && mAudioTrack.getPlayState() == AudioTrack
                                    .PLAYSTATE_PLAYING) {
                                mAudioTrack.stop();
                                Log.i(TAG, "mAudioTrack stop earphone mode");
                            }
                            if (mAudioRecord != null
                                    && mAudioRecord.getRecordingState()
                                    == AudioRecord.RECORDSTATE_RECORDING) {
                                mAudioRecord.stop();
                                Log.i(TAG, "mAudioRecord stop earphone mode");
                            }
                        } catch (IllegalStateException e) {
                            Log.e(TAG, "RenderThread, IllegalStateException");
                        } catch (NullPointerException e) {
                            Log.d(TAG, "RenderThread, NullPointerException");
                        } finally {
                            synchronized (mRenderLock) {
                                Log.i(TAG,
                                        "RenderThread: waiting for mRenderLock");
                                mRenderLock.wait();
                            }
                        }
                    }
                }
            } catch (InterruptedException e) {
                Log.d(TAG, "RenderThread.run, thread is interrupted, need exit thread");
            } finally {
                try {
                    if (mAudioRecord != null
                            && (mAudioRecord.getRecordingState()
                            == AudioRecord.RECORDSTATE_RECORDING)) {
                        mAudioRecord.stop();
                        Log.i(TAG, "mAudioRecord stop finally");
                    }
                    if (mAudioTrack != null
                            && (mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING)) {
                        mAudioTrack.stop();
                        Log.i(TAG, "mAudioTrack stop finally");
                    }
                } catch (IllegalStateException e) {
                    Log.d(TAG, "RenderThread, IllegalStateException");
                } catch (NullPointerException e) {
                    Log.d(TAG, "RenderThread, NullPointerException");
                }
            }
        }
    }

    //-----------------------------Inside Radio end------------------------------------
    //-----------------------------Inside Radio with AudioPatch start------------------
    private int getDeviceForStream() {
        Log.d(TAG, "getDeviceForStream");
        return mAudioManager.getDevicesForStream(AudioManager.STREAM_MUSIC);
    }

    public synchronized void createAudioPatch() {
        Log.d(TAG, "createAudioPatch, mAudioPatch = " + mAudioPatch);
        if (mIsForbidCreateAudioPatch) {
            releaseAudioPatch();
            return;
        }

        if (mAudioPatch == null) {
            ArrayList<AudioPatch> patches = new ArrayList<AudioPatch>();
            mAudioManager.listAudioPatches(patches);
            int deviceForStream = getDeviceForStream();
            if (deviceForStream == AudioManager.DEVICE_OUT_WIRED_HEADSET
                    || deviceForStream == AudioManager.DEVICE_OUT_WIRED_HEADPHONE) {
                Log.d(TAG, "createAudioPatch earphone");
                createAudioPatchByEarphone();
            } else if (deviceForStream == AudioManager.DEVICE_OUT_SPEAKER) {
                Log.d(TAG, "createAudioPatch speaker");
                createAudioPatchBySpeaker();
            } else if ((deviceForStream ==
                    (AudioManager.DEVICE_OUT_WIRED_HEADSET | AudioManager.DEVICE_OUT_SPEAKER))
                    || (deviceForStream ==
                    (AudioManager.DEVICE_OUT_WIRED_HEADPHONE | AudioManager.DEVICE_OUT_SPEAKER))) {
                Log.d(TAG, "createAudioPatch earphone+speaker");
                createAudioPatchBySpeakerAndEarphone();
            }
        }

        Log.d(TAG, "createAudioPatch, mAudioPatch created = " + mAudioPatch);
    }

    public synchronized void releaseAudioPatch() {
        Log.d(TAG, "releaseAudioPatch, mAudioPatch = " + mAudioPatch);
        if (mAudioPatch != null) {
            mAudioManager.releaseAudioPatch(mAudioPatch);
            mAudioPatch = null;
        }
        mAudioSource = null;
        mAudioSink = null;
    }

    public synchronized void createAudioPatchByEarphone() {
        Log.d(TAG, "createAudioPatchByEarphone " + mIsForbidCreateAudioPatch);
        if (mIsForbidCreateAudioPatch) {
            releaseAudioPatch();
            return;
        }
        if (mAudioPatch != null) {
            Log.d(TAG, "createAudioPatch, mAudioPatch is not null, return");
            return;
        }

        mAudioSource = null;
        mAudioSink = null;
        ArrayList<AudioPort> ports = new ArrayList<AudioPort>();
        mAudioManager.listAudioPorts(ports);
        for (AudioPort port : ports) {
            if (port instanceof AudioDevicePort) {
                int type = ((AudioDevicePort) port).type();
                String name = AudioSystem.getOutputDeviceName(type);
                if (type == AudioSystem.DEVICE_IN_FM_TUNER) {
                    mAudioSource = (AudioDevicePort) port;
                } else if (type == AudioSystem.DEVICE_OUT_WIRED_HEADSET ||
                        type == AudioSystem.DEVICE_OUT_WIRED_HEADPHONE) {
                    mAudioSink = (AudioDevicePort) port;
                }
            }
        }
        if (mAudioSource != null && mAudioSink != null) {
            AudioDevicePortConfig sourceConfig = (AudioDevicePortConfig) mAudioSource
                    .activeConfig();
            AudioDevicePortConfig sinkConfig = (AudioDevicePortConfig) mAudioSink.activeConfig();
            AudioPatch[] audioPatchArray = new AudioPatch[]{null};
            int res = mAudioManager.createAudioPatch(audioPatchArray,
                    new AudioPortConfig[]{sourceConfig},
                    new AudioPortConfig[]{sinkConfig});
            if (res == AudioManager.ERROR_INVALID_OPERATION) {
                mIsForbidCreateAudioPatch = true;
            }
            mAudioPatch = audioPatchArray[0];
        }
    }

    public synchronized void createAudioPatchBySpeaker() {
        Log.d(TAG, "createAudioPatchBySpeaker");
        if (mIsForbidCreateAudioPatch) {
            releaseAudioPatch();
            return;
        }
        if (mAudioPatch != null) {
            Log.d(TAG, "createAudioPatch, mAudioPatch is not null, return");
            return;
        }

        mAudioSource = null;
        mAudioSink = null;
        ArrayList<AudioPort> ports = new ArrayList<AudioPort>();
        mAudioManager.listAudioPorts(ports);
        for (AudioPort port : ports) {
            if (port instanceof AudioDevicePort) {
                int type = ((AudioDevicePort) port).type();
                String name = AudioSystem.getOutputDeviceName(type);
                if (type == AudioSystem.DEVICE_IN_FM_TUNER) {
                    mAudioSource = (AudioDevicePort) port;
                } else if (type == AudioSystem.DEVICE_OUT_SPEAKER) {
                    mAudioSink = (AudioDevicePort) port;
                }
            }
        }
        if (mAudioSource != null && mAudioSink != null) {
            AudioDevicePortConfig sourceConfig = (AudioDevicePortConfig) mAudioSource
                    .activeConfig();
            AudioDevicePortConfig sinkConfig = (AudioDevicePortConfig) mAudioSink.activeConfig();
            AudioPatch[] audioPatchArray = new AudioPatch[]{null};
            int res = mAudioManager.createAudioPatch(audioPatchArray,
                    new AudioPortConfig[]{sourceConfig},
                    new AudioPortConfig[]{sinkConfig});
            if (res == AudioManager.ERROR_INVALID_OPERATION) {
                mIsForbidCreateAudioPatch = true;
            }
            mAudioPatch = audioPatchArray[0];
        }
    }

    private synchronized void createAudioPatchBySpeakerAndEarphone() {
        Log.d(TAG, "createAudioPatchBySpeakerAndEarphone");
        if (mIsForbidCreateAudioPatch) {
            releaseAudioPatch();
            return;
        }
        if (mAudioPatch != null) {
            Log.d(TAG, "createAudioPatchBySpeakerAndEarphone, mAudioPatch is not null, return");
            return;
        }

        mAudioSource = null;
        AudioDevicePort speakerSink = null;
        AudioDevicePort earphoneSink = null;
        ArrayList<AudioPort> ports = new ArrayList<AudioPort>();
        mAudioManager.listAudioPorts(ports);
        for (AudioPort port : ports) {
            if (port instanceof AudioDevicePort) {
                int type = ((AudioDevicePort) port).type();
                String name = AudioSystem.getOutputDeviceName(type);
                if (type == AudioSystem.DEVICE_IN_FM_TUNER) {
                    mAudioSource = (AudioDevicePort) port;
                } else if (type == AudioSystem.DEVICE_OUT_SPEAKER) {
                    speakerSink = (AudioDevicePort) port;
                } else if (type == AudioSystem.DEVICE_OUT_WIRED_HEADSET ||
                        type == AudioSystem.DEVICE_OUT_WIRED_HEADPHONE) {
                    earphoneSink = (AudioDevicePort) port;
                }
            }
        }
        if (mAudioSource != null && speakerSink != null && earphoneSink != null) {
            AudioDevicePortConfig sourceConfig = (AudioDevicePortConfig) mAudioSource
                    .activeConfig();
            AudioDevicePortConfig speakerSinkConfig = (AudioDevicePortConfig) speakerSink
                    .activeConfig();
            AudioDevicePortConfig earphoneSinkConfig = (AudioDevicePortConfig) earphoneSink
                    .activeConfig();
            AudioPatch[] audioPatchArray = new AudioPatch[]{null};
            int res = mAudioManager.createAudioPatch(audioPatchArray,
                    new AudioPortConfig[]{sourceConfig},
                    new AudioPortConfig[]{speakerSinkConfig, earphoneSinkConfig});
            if (res == AudioManager.ERROR_INVALID_OPERATION) {
                mIsForbidCreateAudioPatch = true;
            }
            mAudioPatch = audioPatchArray[0];
        }
    }
    //-----------------------------Inside Radio with AudioPatch end------------------------------------
}

