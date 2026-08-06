package com.hcn.media_model.player;

import static com.hcn.auto_compat.PlatformUtils.SM6225;
import static java.lang.Math.abs;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Point;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnBufferingUpdateListener;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnInfoListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.media.MediaPlayer.OnSeekCompleteListener;
import android.media.MediaPlayer.OnTimedTextListener;
import android.media.MediaPlayer.OnVideoSizeChangedListener;
import android.media.TimedText;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

import com.hcn.auto_compat.PlatformUtils;
import com.hcn.media_common.HMessage;
import com.hcn.media_model.player.base.IMediaPlayer;
import com.hcn.media_theme.Argument;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.IState;
import com.hcn.media_common.utils.MediaID3Util;
import com.hcn.media_model.MediaUtils;
import com.hcn.media_model.base.ILocalzModel;
import com.hcn.media_model.base.IPlayerModel;
import com.hcn.media_model.player.base.BasePlayer;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_base.HMediaConfig;
import com.hcn.media_theme.ThemeEx;
import com.orhanobut.logger.Logger;

import java.io.File;
import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Locale;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * 硬解码播放器
 * <p> 平台关联类，禁止外部调用它；
 *
 * @author 65821
 */
class HCorePlayer extends BasePlayer
        implements IMediaPlayer,
        OnCompletionListener,
        OnPreparedListener,
        OnErrorListener,
        OnInfoListener,
        OnSeekCompleteListener,
        OnVideoSizeChangedListener,
        OnBufferingUpdateListener,
        OnTimedTextListener {

    private static final String TAG = "HCorePlayer";
    private static final String SPEC_VIDEO_SUFFIX = ".avi.";

    private MediaPlayer mMediaPlayer = null;
    private SurfaceHolder mCurrSurHolder = null;
    private SurfaceHolder mCurrRearSurHolder = null;
    private boolean mIsNullDisplay = true;

    private volatile int mMPlayerState = IState.END;
    private volatile boolean mIsVideoRenderingStart = false;
    private boolean mOnPreparedTimeout = false;

    private MusicInfo mMediaInfo = null;
    private static final HDVideoObject mHDVideoObject = new HDVideoObject();
    private ExecutorService mAsyncTaskThreadPool = null;

    /**
     * 当前高清视频对象
     * <pre>
     *    用来记录当前不能正常播放的高清视频信息；
     *    用于跳转到软解播放组件后，提供给其查询是否需要做特殊处理；
     * </pre>
     */
    private static final class HDVideoObject {
        MusicInfo mediaInfo = null;
        boolean suggestLoseQuality = false;

        public void set(MusicInfo info, boolean loseQuality) {
            mediaInfo = info;
            suggestLoseQuality = loseQuality;
        }

        public void reset() {
            mediaInfo = null;
            suggestLoseQuality = false;
        }
    }

    /**
     * 当前播放对象是否需要丢失画面效果播放
     * <pre>
     *    平台组件播放失败后，提供给软解组件查询初始化使用；
     *    注意这个接口是一次性的，查询后该状态将会被重置掉；
     * </pre>
     *
     * @param info 目标对象
     * @return 需要丢失质量/不需要丢失质量
     */
    public static boolean isNeedLoseQuality(@NonNull MusicInfo info) {
        MusicInfo sourceInfo = mHDVideoObject.mediaInfo;
        if (Objects.isNull(info)
                || Objects.isNull(sourceInfo)
                || TextUtils.isEmpty(sourceInfo.mFilePath)) {
            LogUtil.v(TAG, "isNeedLoseQuality: " + sourceInfo);
            return false;
        }

        // 是否需要丢失部分效果播放
        boolean needLoseQuality =
                mHDVideoObject.suggestLoseQuality
                        && sourceInfo.mFilePath.equals(info.mFilePath);
        mHDVideoObject.reset();
        return needLoseQuality;
    }

    /** 异步任务处理器 **/
    private class AsyncTaskRunnable implements Runnable {
        private String mFilePath = null;
        private final Reference<MusicInfo> mMusicInfoRef;

        // [同步更新 ID3 信息]
        private static final int WM_UPDATE_ID3_INFO = 1;
        private final Handler mHandler = new TaskHandler(Looper.getMainLooper());

        @SuppressLint("HandlerLeak")
        private class TaskHandler extends Handler {
            public TaskHandler(@NonNull Looper looper) {
                super(looper);
            }

            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);

                switch (msg.what) {
                    case WM_UPDATE_ID3_INFO: {
                        if (!(msg.obj instanceof MusicInfo)) {
                            return;
                        }

                        MusicInfo info = (MusicInfo) msg.obj;
                        MusicInfo targetInfo = mMusicInfoRef.get();

                        // 无效对象返回
                        if (null == targetInfo) {
                            return;
                        }

                        // 相同文件才需要更新同步
                        if (MiscUtils.reverseEquals(info.mFilePath, targetInfo.mFilePath)) {
                            // 如果 ID3 已经被解析过了, 不需要同步
                            if (MusicInfo.ID3_TYPE_NONE != targetInfo.mID3Type) {
                                return;
                            }

                            targetInfo.mID3Type = info.mID3Type;
                            targetInfo.mTitle = info.mTitle;
                            targetInfo.mAlbum = info.mAlbum;
                            targetInfo.mArtist = info.mArtist;
                            targetInfo.mTotalTime = info.mTotalSize;

                            // 通知更新 UI 显示
                            onSendMessage(IMediaEvent.EVENT_UPDATE_MUSIC_ID3, null);
                        }
                        break;
                    }

                    case -1:
                    default:
                        break;
                }
            }
        }

        public AsyncTaskRunnable(MusicInfo info) {
            if (null != info) {
                mFilePath = info.mFilePath;
            }

            mMusicInfoRef = new WeakReference<>(info);
        }

        @Override
        public void run() {
            MusicInfo info = new MusicInfo(mFilePath);
            MediaID3Util.retrieveTargetID3Info(info);

            // ID3 有效才需要更新同步
            if (MusicInfo.ID3_TYPE_EXTRACTED == info.mID3Type) {
                Message msg = mHandler.obtainMessage();
                msg.what = WM_UPDATE_ID3_INFO;
                msg.obj = info;
                mHandler.sendMessage(msg);
            }
        }
    }

    /** [播放服务消息处理器] **/
    private static final int MSG_SET_SESSION_ID = 1;
    private static final int MSG_UNSUPPORT_SEEKABLE = 2; // [不支持拖动]
    private static final int MSG_UNSUPPORT_DECODE = 3; // [通知走软解]
    private static final int MSG_HANDLE_PLAYER_ERROR = 4; // [播放出错]
    private static final int MSG_VOLUME_FADE_DOWN = 5;
    private static final int MSG_VOLUME_FADE_UP = 6;
    private static final int MSG_WRITE_MEDIA_TIME = 7;
    private static final int MSG_HIDE_BLACK_PAGE = 8; // [隐藏视频黑屏遮罩, 提高切曲体验]
    private static final int MSG_PLAYING_STATE_REPORT = 9; // [延时上报播放中状态, 避免马上硬解失败]
    private static final int MSG_WAIT_PREPARED_TIMEOUT = 10; // [onPrepared 反馈超时]
    private static final int MSG_VIDEO_SIZE_CHANGED_TIMEOUT = 11; // [函数 onVideoSizeChanged 回调超时]

    /**
     * 处理播放事件
     * <p> 播放逻辑相关消息任务；
     */
    private final MediaPlayerHandler mPlayerHandler = new MediaPlayerHandler(Looper.getMainLooper());

    @SuppressLint("HandlerLeak")
    private class MediaPlayerHandler extends Handler {
        public float mTargetVolume = 1.0f;

        public float mCurrentVolume = 1.0f;

        public MediaPlayerHandler(Looper looper) {
            super(looper);
        }

        public void fadeUp() {
            removeFade();

            // [有的歌曲开头有个 Pop 音, 这里适当掩盖他, 延时时间长了也不好]
            sendEmptyMessageDelayed(MSG_VOLUME_FADE_UP, 800);
        }

        public void fadeDown() {
            removeFade();
            sendEmptyMessageDelayed(MSG_VOLUME_FADE_DOWN, 10);
        }

        public void removeFade() {
            removeMessages(MSG_VOLUME_FADE_UP);
            removeMessages(MSG_VOLUME_FADE_DOWN);
        }

        // 强制设置当前音量值，不会改变 mTargetVolume 的值
        public void setCurrentVolume(float volume) {
            removeFade();

            mCurrentVolume = volume;
            setVolume(volume, volume);
        }

        // 根据当前状态，渐变到请求的目标音量值
        public void requestSetVolume(float volume) {
            mTargetVolume = volume;
            float delta = mTargetVolume - mCurrentVolume;

            // [如果相等返回]
            if (abs(delta) < 1E-7) {
                removeFade();
                return;
            }

            if (mTargetVolume > mCurrentVolume) {
                fadeUp();
            } else {
                fadeDown();
            }
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_WRITE_MEDIA_TIME: {
                    writeCurrentMediaTime();

                    // [播放状态触发后, 5S 更新一次]
                    if (isPrepared()) {
                        if (!mMediaPlayer.isPlaying()) {
                            return;
                        }

                        mPlayerHandler.sendEmptyMessageDelayed(MSG_WRITE_MEDIA_TIME, 2000);
                    }
                    break;
                }

                case MSG_UNSUPPORT_SEEKABLE: {
                    onSendMessage(IMediaEvent.EVENT_UNSUPPORT_SEEKABLE, null);
                    break;
                }

                case MSG_UNSUPPORT_DECODE: {
                    if (mMediaInfo != null) {
                        onSendMessage(IMediaEvent.EVENT_CODE_UNSUPPORT, mMediaInfo);
                    }
                    break;
                }

                case MSG_SET_SESSION_ID: {
                    LogUtil.d(TAG, "setAudioSessionId");

                    if (null != mMediaPlayer) {
                        setAudioSessionId(mMediaPlayer.getAudioSessionId());
                    }
                    break;
                }

                case MSG_VOLUME_FADE_DOWN: {
                    mCurrentVolume -= .05f; // 20 * 20 = 400ms

                    if (mCurrentVolume > mTargetVolume) {
                        sendEmptyMessageDelayed(MSG_VOLUME_FADE_DOWN, 20);
                    } else {
                        mCurrentVolume = mTargetVolume;
                    }

                    setVolume(mCurrentVolume, mCurrentVolume);
                    break;
                }

                case MSG_VOLUME_FADE_UP: {
                    mCurrentVolume += .0125f; // 80 * 25 = 2000ms

                    if (mCurrentVolume < mTargetVolume) {
                        sendEmptyMessageDelayed(MSG_VOLUME_FADE_UP, 25);
                    } else {
                        mCurrentVolume = mTargetVolume;
                    }

                    setVolume(mCurrentVolume, mCurrentVolume);
                    break;
                }

                case MSG_HANDLE_PLAYER_ERROR: {
                    onHandleError();
                    break;
                }

                case MSG_HIDE_BLACK_PAGE: {
                    onSendMessage(IMediaEvent.EVENT_VIDEO_HIDE_BLACK_PAGE, null);
                    break;
                }

                case MSG_PLAYING_STATE_REPORT: {
                    if (isPrepared()) {
                        // 非播放状态, 直接返回
                        if (!mMediaPlayer.isPlaying()) {
                            return;
                        } else {
                            // [视频模式如果是后台播放状态]
                            if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
                                boolean isBackgroundMode = Argument.isBackgroundPlayMode();
                                if (null == mCurrSurHolder
                                        && isBackgroundMode
                                        && mSeekToFlag
                                        && !mIsVideoRenderingStart) {
                                    mSeekToFlag = false;
                                    seekToTime(mSeekTime);
                                }
                            }
                        }

                        onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY, null);
                    }
                    break;
                }

                case MSG_WAIT_PREPARED_TIMEOUT: {
                    // onPrepared() 函数超时
                    if (isInited() && IState.PREPARING == mMPlayerState) {
                        mOnPreparedTimeout = true;
                        // 如果超时怎么办: ?
                        // ANR: notifyToPlayNext(-404);
                    }
                    break;
                }

                case MSG_VIDEO_SIZE_CHANGED_TIMEOUT: {
                    // onVideoSizeChanged(...) 函数超时
                    // [支持软解情况下 Video 不能播放需要切换软解码]
                    if (Argument.isSupportVitamio()) {
                        onHandleError(70001, true);
                    }
                    break;
                }

                default: {
                    break;
                }
            }
        }
    }

    /** 禁止构造无参对象 **/
    private HCorePlayer() {
        super(null, null, null);
        throw new RuntimeException(
                "Prohibit the construction of parameterless objects");
    }

    /**
     * HCorePlayer 默认构造函数
     *
     * @param context 应用上下文环境
     * @param localzModel {@link ILocalzModel} 业务模型
     * @param playerModel {@link IPlayerModel} 播放模型
     */
    public HCorePlayer(
            @NonNull Context context,
            @NonNull ILocalzModel localzModel,
            @NonNull IPlayerModel playerModel) {
        super(context, localzModel, playerModel);
    }

    /** [当前 MediaPlayer 状态] **/
    private boolean isMediaPlayerState(final int state) {
        return state == mMPlayerState;
    }

    /** 检查目标是否需要及时更新 ID3 信息 **/
    public void checkTargetID3Info(MusicInfo info) {
        if (null == info) {
            return;
        }

        // 非音乐不处理
        if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
            if (mAsyncTaskThreadPool != null) {
                mAsyncTaskThreadPool.shutdown();

                try {
                    boolean terminated = mAsyncTaskThreadPool
                            .awaitTermination(20, TimeUnit.MILLISECONDS);
                    LogUtil.d(TAG, "checkTargetID3Info:" +
                            " AsyncTaskThreadPool/awaitTermination = " + terminated);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                } finally {
                    mAsyncTaskThreadPool = null;
                }
            }

            return;
        }

        // 已经解析过，不处理
        if (MusicInfo.ID3_TYPE_NONE == info.mID3Type) {
            // 无效路径不处理
            boolean isMounted = mLocalzModel.targetStorageMounted(info.mFilePath);
            if (!isMounted) {
                return;
            }

            try {
                if (null == mAsyncTaskThreadPool) {
                    mAsyncTaskThreadPool = new ThreadPoolExecutor(1, 2,
                            10L, TimeUnit.SECONDS, new LinkedBlockingQueue<Runnable>());
                }

                mAsyncTaskThreadPool.execute(new AsyncTaskRunnable(info));
            } catch (RejectedExecutionException | NullPointerException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 回调消息事件到上层 View
     *
     * @param eventId 事件 ID
     * @param info 媒体信息
     */
    private void onSendMessage(int eventId, MusicInfo info) {
        onSendMessage(eventId, info, null);
    }

    private void onSendMessage(int eventId, Object wParam, Object lParam) {
        // 及时同步状态
        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY:
                mAppData.mMediaPlayState = IMusicState.E_PLAY_STATE_PLAY;
                break;
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_PAUSE:
                mAppData.mMediaPlayState = IMusicState.E_PLAY_STATE_PAUSE;
                break;
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_STOP:
                mAppData.mMediaPlayState = IMusicState.E_PLAY_STATE_STOP;
                break;
            default:
                break;
        }

        mPlayerModel.onMediaEvent(eventId, wParam, lParam);
    }

    @Override
    public boolean isPlayState() {
        if (isPrepared()) {
            return mMediaPlayer.isPlaying();
        }

        return false;
    }

    @Override
    public boolean isInited() {
        return mMediaPlayer != null;
    }

    @Override
    public int getVideoWidth() {
        int width = -1;

        if (isPrepared()) {
            width = mMediaPlayer.getVideoWidth();
        }

        return width;
    }

    @Override
    public int getVideoHeight() {
        int height = -1;

        if (isPrepared()) {
            height = mMediaPlayer.getVideoHeight();
        }

        return height;
    }

    @SuppressLint("ObsoleteSdkInt")
    private void onCreatePlayer(MusicInfo info) {
        if (null != mMediaPlayer) {
            onStopPlayer();
        }

        LogUtil.d(TAG, ">>> onCreatePlayer.");

        reset();
        mAppData.mCurrentMediaInfo = info;
        mMediaPlayer = new MediaPlayer();

        mMediaPlayer.reset();
        mMPlayerState = IState.IDLE;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            mMediaPlayer.setAudioAttributes(new AudioAttributes
                    .Builder()
                    .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                    .build());
        } else {
            mMediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
        }

        mPlayerHandler.removeMessages(MSG_SET_SESSION_ID);
        setAudioSessionId(mMediaPlayer.getAudioSessionId());

        mMediaPlayer.setOnCompletionListener(this);
        mMediaPlayer.setOnPreparedListener(this);
        mMediaPlayer.setOnErrorListener(this);
        mMediaPlayer.setOnInfoListener(this);
        mMediaPlayer.setOnSeekCompleteListener(this);
        mMediaPlayer.setOnVideoSizeChangedListener(this);
        mMediaPlayer.setOnBufferingUpdateListener(this);
        mMediaPlayer.setOnTimedTextListener(this);

        updateSurfaceHolder(true);
    }

    private void setAudioSessionId(int Id) {
        mLocalzModel.setAudioSessionId(Id);
    }

    /** 存储播放信息 **/
    private void writeCurrentMediaTime() {
        if (null != mAppData.mCurrentMediaInfo) {
            mLocalzModel.writeMediaTime(
                    mAppData.mLastMediaType,
                    mAppData.mCurrentMediaInfo.mFilePath,
                    mAppData.mPlayTimeInfo.mCurrentTime,
                    107);
        }
    }

    private void reset() {
        mVideoWidth = -1;
        mVideoHeight = -1;
        mVideoSizeChanged = false;

        mMPlayerState = IState.END;
        mIsVideoRenderingStart = false;
        mOnPreparedTimeout = false;

        mSeekTime = 0;
        mSeekToFlag = false;

        mCurrSurHolder = null;
        mCurrRearSurHolder = null;
        mIsNullDisplay = true;

        mAppData.mIsMediaPlayerLocked = false;
    }

    private void onStopPlayer() {
        onStopPlayer(0);
    }

    private void onStopPlayer(int reason) {
        LogUtil.d(TAG, ">>> onStopPlayer: prepared = " + isPrepared() + ", reason = " + reason);

        // [还不能把它 mAppData.mCurrentMediaInfo 重置为 null, 因为播放完成后的切
        //  曲流程 EVENT_MEDIA_COMPLETION 会去重置播放进度, 否则单曲循环会出问题. ]
        // mAppData.mCurrentMediaInfo = null;

        mPlayerHandler.removeMessages(MSG_WRITE_MEDIA_TIME);
        mPlayerHandler.removeMessages(MSG_WAIT_PREPARED_TIMEOUT);

        // 清除播放相关的消息
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            mPlayerHandler.removeMessages(MSG_HIDE_BLACK_PAGE);
            mPlayerHandler.removeMessages(MSG_PLAYING_STATE_REPORT);
        }

        // [释放资源，大部分 ANR 基本都是这里发生的]
        if (null != mMediaPlayer) {
            try {
                // stop
                if (isPrepared()) {
                    if (mMediaPlayer.isPlaying()) {
                        mPlayerHandler.requestSetVolume(.0f);
                        mMediaPlayer.stop();
                        mMPlayerState = IState.STOPPED;
                    }
                }

                // reset
                mMediaPlayer.reset();
                mMPlayerState = IState.IDLE;

                if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
                    mMediaPlayer.setDisplay(null);
                }

                // release
                mMediaPlayer.release();
                mMediaPlayer = null;
                mMPlayerState = IState.END;
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                reset();
            }
        }
    }

    private void onPlayEvent(int reason) {
        LogUtil.d(TAG, ">>> onPlayEvent: prepared = " + isPrepared() + ", reason = " + reason);
        mLocalzModel.registerMediaButton();

        if (isPrepared()) {
            try {
                boolean isPlaying = mMediaPlayer.isPlaying();
                LogUtil.d(TAG, "isPlaying: " + isPlaying);

                if (!isPlaying) {
                    updateSurfaceHolder(false, true);

                    mPlayerHandler.setCurrentVolume(.0f);
                    mMediaPlayer.start();
                    mMPlayerState = IState.STARTED;
                    mPlayerHandler.requestSetVolume(1.0f);

                    // [播放: 触发存储记忆]
                    mPlayerHandler.removeMessages(MSG_WRITE_MEDIA_TIME);
                    mPlayerHandler.sendEmptyMessageDelayed(MSG_WRITE_MEDIA_TIME, 500);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void onPauseEvent() {
        LogUtil.d(TAG, ">>> onPauseEvent: " + isPrepared());

        if (isPrepared()) {
            mPlayerHandler.removeMessages(MSG_WRITE_MEDIA_TIME);

            try {
                if (mMediaPlayer.isPlaying()) {
                    mMediaPlayer.pause();
                    mMPlayerState = IState.PAUSED;
                    mPlayerHandler.requestSetVolume(.0f);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private boolean onPlayPauseEvent() {
        LogUtil.d(TAG, ">>> onPlayPauseEvent: " + isPrepared());

        if (isPrepared()) {
            try {
                mPlayerHandler.removeMessages(MSG_WRITE_MEDIA_TIME);

                if (mMediaPlayer.isPlaying()) {
                    mMediaPlayer.pause();
                    mMPlayerState = IState.PAUSED;
                    mPlayerHandler.requestSetVolume(.0f);
                } else {
                    updateSurfaceHolder(false, true);

                    mPlayerHandler.setCurrentVolume(.0f);
                    mMediaPlayer.start();
                    mMPlayerState = IState.STARTED;
                    mPlayerHandler.requestSetVolume(1.0f);

                    mPlayerHandler.sendEmptyMessageDelayed(MSG_WRITE_MEDIA_TIME, 500);

                    return true;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return false;
    }

    @Override
    public int getCurrentPosition() {
        if (isPrepared()) {
            try {
                return mMediaPlayer.getCurrentPosition();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return 0;
    }

    @Override
    public int getTotalTime() {
        if (isPrepared()) {
            try {
                return mMediaPlayer.getDuration();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return 0;
    }

    @Override
    public void seekToTime(int time) {
        LogUtil.d(TAG, ">>> seekToTime," +
                " isPrepared = " + isPrepared() + " nTime: " + time);

        if (isPrepared()) {
            try {
                // [方法是异步的]
                mMediaPlayer.seekTo(time);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 一般在切换曲目和播放完成调用
     * <p> 只是移除存储状态消息事件，实际没什么用；
     */
    @Override
    public void onSetSeekTimeZero() {
        LogUtil.d(TAG, ">>> setSeekTimeToZero");
        mPlayerHandler.removeMessages(MSG_WRITE_MEDIA_TIME);
    }

    public void setVolume(float leftVolume, float rightVolume) {
        if (isInited()) {
            mMediaPlayer.setVolume(leftVolume, rightVolume);
        } else {
            mPlayerHandler.removeFade();
        }
    }

    /**
     * 调解当前 AudioTrack 音量大小
     * <p> 确保在主线程调用 [非现线程安全]
     *
     * @param volume [0.0F ~ 1.0F]
     */
    @Override
    public void requestSetVolume(float volume) {
        mPlayerHandler.requestSetVolume(volume);
    }

    @Override
    public void onPlayControlEvent(int command) {
        onPlayControlEvent(command, 0);
    }

    @Override
    public void onPlayControlEvent(int command, int reason) {
        switch (command) {
            case IMusicState.PLAY_CMD_PLAY: {
                onPlayEvent(reason);

                // [视频需要特殊处理下: onPrepared() 会触发播放, 但是视频可能解码失败.]
                if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
                    mPlayerHandler.removeMessages(MSG_PLAYING_STATE_REPORT);

                    // [-1: 第一次 <onPrepared> 触发播放]
                    if (-1 == reason) {
                        // [过早通知 Service 播放状态, 会触发定时任务去检索刷新当前播放进度条]
                        mPlayerHandler.sendEmptyMessageDelayed(MSG_PLAYING_STATE_REPORT, 300);

                        // [通知 Service 更新状态值，仅仅如此：避免出现 300ms 状态未及时更新的情况下，出现记忆问题]
                        onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY, null, "only-state");
                    } else {
                        onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY, null);
                    }
                } else {
                    onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY, null);
                }
                break;
            }

            case IMusicState.PLAY_CMD_PAUSE: {
                onPauseEvent();

                mPlayerHandler.removeMessages(MSG_PLAYING_STATE_REPORT);
                onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PAUSE, null);
                break;
            }

            case IMusicState.PLAY_CMD_STOP: {
                onStopPlayer();

                mPlayerHandler.removeMessages(MSG_PLAYING_STATE_REPORT);
                onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_STOP, null);
                break;
            }

            case IMusicState.PLAY_CMD_PLAY_PAUSE: {
                boolean isPlayEvent = onPlayPauseEvent();

                if (isPlayEvent) {
                    // [视频播放状态不要着急触发]
                    if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
                        if (!mPlayerHandler.hasMessages(MSG_PLAYING_STATE_REPORT)) {
                            mPlayerHandler.sendEmptyMessageDelayed(MSG_PLAYING_STATE_REPORT, 300);
                        }
                    } else {
                        onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY, null);
                    }
                } else {
                    mPlayerHandler.removeMessages(MSG_PLAYING_STATE_REPORT);
                    onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PAUSE, null);
                }
                break;
            }

            default: {
                break;
            }
        }
    }

    @Override
    public void updateSurfaceHolder(boolean init) {
        updateSurfaceHolder(init, false);
    }

    public void updateSurfaceHolder(boolean init, boolean start) {
        // 只有视频才有 Surface 需要设置
        if (IMusicState.MEDIA_TYPE_VIDEO != mAppData.mMediaType) {
            return;
        }

        mCurrSurHolder = mAppData.mFrontSurfaceHolder;
        mCurrRearSurHolder = mAppData.mRearSurfaceHolder;

        if (isPrepared()) {
            LogUtil.d(TAG, ">>> updateSurfaceHolder isFrontVideo =  " + mAppData.isFrontVideo);

            if (mAppData.isFrontVideo) {
                if (null == mCurrSurHolder) {
                    LogUtil.d(TAG, "mCurrSurHolder: null.");
                } else {
                    if (!mCurrSurHolder.getSurface().isValid()) {
                        LogUtil.d(TAG, ">>> updateSurfaceHolder surface invalid.");
                    }
                }

                // SurfaceView 销毁 和 触发播放才需要设置显示
                mMediaPlayer.setDisplay(mCurrSurHolder);

                // [如果 setDisplay(null) 后再次恢复前台，需要设置尺寸，否则存在问题.]
                if (mIsNullDisplay && null != mCurrSurHolder) {
                    if (mVideoSizeChanged) {
                        if (mVideoWidth > 0 && mVideoHeight > 0) {
                            computeAndUpdateVideoSize();
                            mCurrSurHolder.setFixedSize(mVideoWidth, mVideoHeight);

                            mPlayerModel.onMediaEvent(
                                    IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_SIZE,
                                    mVideoWidth, mVideoHeight);
                        }
                    }
                }

                // 存储当前 SurfaceHolder 是否有效
                mIsNullDisplay = (null == mCurrSurHolder);

                // 必须当前视频源存在有效的显示帧，才通知隐藏
                if (!init && mIsVideoRenderingStart) {
                    onSendMessage(IMediaEvent.EVENT_VIDEO_HIDE_BLACK_PAGE, null);
                }
            } else {
                if (null == mCurrRearSurHolder) {
                    LogUtil.d(TAG, "mCurrRearSurHolder: " + null);
                }

                mCurrSurHolder = mCurrRearSurHolder;
                mMediaPlayer.setDisplay(mCurrSurHolder);
                mIsNullDisplay = (null == mCurrSurHolder);
            }
        }
    }


    /**
     * 当前播放文件是否是指定的文件后缀
     * <p> 文件后缀参数格式必须是 ".avi"、".mp4"、".mkv" 等；
     *
     * @param suffix 文件后缀
     * @return 是/否
     */
    private boolean isPlayFileSuffix(String suffix) {
        // 参数有效性检查
        if (Objects.isNull(mMediaInfo)
                || TextUtils.isEmpty(mMediaInfo.mFilePath)
                || TextUtils.isEmpty(suffix)) {
            return false;
        }

        String filePath = mMediaInfo.mFilePath;
        int pos = filePath.lastIndexOf('.');
        if (pos != -1) {
            String strSuffix = filePath.substring(pos);
            strSuffix = strSuffix.toLowerCase(Locale.getDefault());
            return suffix.toLowerCase().equals(strSuffix);
        }
        return false;
    }

    /** 强制软解码 */
    public boolean forceSoftDecoding(String suffix) {
        // 默认软解码的文件
        if (HMediaConfig.VIDEO_SOFT_DECODER_SUFFIX.contains(suffix)) {
            return true;
        }
        // 特许情况特殊处理，平台体验考虑；
        return PlatformUtils.isHardware(SM6225) && ".ts.".equals(suffix);
    }

    @Override
    public void onSetDataSourceEvent(MusicInfo info) {
        if (null == info) {
            return;
        }

        // [ 如果在 IState.PREPARING 不支持下一曲 ]
        if (isMediaPlayerState(IState.PREPARING)) {
            LogUtil.d(TAG, ">>>> [Reject]onSetDataSourceEvent, IState.PREPARING!");
            return;
        }

        // [ 经过实测发现, 需要等待软解码异步任务释放干净,
        //   否则 MediaPlayer 也无法进入 PREPARED 状态。]
        IMediaPlayer player = mPlayerModel.vitamioPlayer();
        try {
            int timeout = 0;
            while (player != null && player.isAsyncReleasing()) {
                if (timeout++ > 99) {
                    break;
                }

                Thread.sleep(10);
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            // 资源没有释放干净, 不处理新任务
            if (player != null && player.isAsyncReleasing()) {
                LogUtil.vitamio_d(TAG, ">>>> [Reject]onSetDataSourceEvent, isAsyncReleasing!");
                return;
            }
        }

        // 触发新任务
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            mPlayerHandler.removeMessages(MSG_HIDE_BLACK_PAGE);
            mPlayerHandler.removeMessages(MSG_PLAYING_STATE_REPORT);
        }

        // 如果触发了新任务，移除软解跳转 和 错误处理
        mPlayerHandler.removeMessages(MSG_UNSUPPORT_DECODE);
        mPlayerHandler.removeMessages(MSG_HANDLE_PLAYER_ERROR);
        mPlayerHandler.removeMessages(MSG_UNSUPPORT_SEEKABLE);

        // 记录并标记播放对象
        mMediaInfo = info;
        mHDVideoObject.reset();

        String filePath = info.mFilePath;
        File file = new File(filePath);
        LogUtil.d(TAG, ">>>> onSetDataSourceEvent, filePath: " + filePath);

        if (!file.exists()) {
            onFileNotExistError();
            return; // [文件不存在，直接中断当前流程]
        } else {
            mAppData.mFileNotExistCount = 0;
        }

        // [小于 10K 的文件调过，理论上视频要单独区分]
        if (file.length() < 10 * 1024) {
            onSendMessage(IMediaEvent.EVENT_ERROR_FILE_IS_TOO_SMALL, null);
            notifyToPlayNext(-1);
            return;
        }

        // 特定后缀直接使用软解，例如[rm 格式不反馈信息，直接没有视频显示]
        int pos = filePath.lastIndexOf('.');
        if (pos != -1) {
            String strSuffix = filePath.substring(pos) + ".";
            strSuffix = strSuffix.toLowerCase(Locale.getDefault());

            if (forceSoftDecoding(strSuffix)) {
                LogUtil.d(TAG, ">>>> onSetDataSourceEvent, Jump directly to the soft decoder!");

                mMediaInfo = null;
                onStopPlayer(-1);

                onSendMessage(IMediaEvent.EVENT_CODE_UNSUPPORT, info);
                return;
            }
        }

        // [重置 MediaPlayer]
        onCreatePlayer(info);

        // 视频切换过渡（避免闪烁）
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            onSendMessage(IMediaEvent.EVENT_VIDEO_SHOW_BLACK_PAGE, null);
        }

        try {
            mFilePath = filePath;
            mAppData.mIsMediaPlayerLocked = true;
            mMediaPlayer.setDataSource(filePath);
            mMPlayerState = IState.INITIALIZED;
        } catch (Exception ex) {
            ex.printStackTrace();

            // [解锁: 否则无法下一曲]
            mAppData.mIsMediaPlayerLocked = false;
            onHandleError();
            return;
        }

        try {
            // 异步加载资源，加载完成后会触发[onPrepared]
            mMediaPlayer.prepareAsync();
            mMPlayerState = IState.PREPARING;
            LogUtil.d(TAG, ">>> prepareAsync.");

            // 通知播放器进入 PREPARING 状态
            dispatchMediaEvent(
                    HMessage.obtain(
                            mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)?
                                    IMediaEvent.EVENT_MUSIC_PLAYER_PREPARING:
                                    IMediaEvent.EVENT_VIDEO_PLAYER_PREPARING,
                            info));

            // [特定的设备状态下 onPrepared(..) 函数可能不会那么及时返回
            //  PREPARING 状态下 reset() 和 release() 都会存在 ANR 问题]
            mPlayerHandler.removeMessages(MSG_WAIT_PREPARED_TIMEOUT);
            mPlayerHandler.sendEmptyMessageDelayed(MSG_WAIT_PREPARED_TIMEOUT, 10 * 1000);
        } catch (Exception e) {
            e.printStackTrace();

            // [解锁: 否则无法下一曲]
            mAppData.mIsMediaPlayerLocked = false;
            onHandleError();
        }
    }

    private void onFileNotExistError() {
        LogUtil.e(TAG, "onFileNotExistError!");

        onSendMessage(IMediaEvent.EVENT_ERROR_FILE_NOT_EXIST, mMediaInfo);
        notifyToPlayNext(-2);
    }

    // [通知播放下一个]
    private void notifyToPlayNext(int reason) {
        notifyToPlayNext(reason, false);
    }

    private void notifyToPlayNext(int reason, boolean completion) {
        // [如果先捕获到 PlayError 后收到 onCompletion 事件]
        if (completion) {
            if (mPlayerHandler.hasMessages(MSG_HANDLE_PLAYER_ERROR)
                    || mPlayerHandler.hasMessages(MSG_UNSUPPORT_DECODE)) {
                // 走软解，无须直接切换下一个媒体文件。
                return;
            }
        } else {
            mPlayerHandler.removeMessages(MSG_HANDLE_PLAYER_ERROR);
            mPlayerHandler.removeMessages(MSG_UNSUPPORT_DECODE);
        }

        // 释放解码资源
        onStopPlayer(reason);

        // [通知播放下一个媒体文件: 如果进程第一次启动第一个文件就不支持,
        //  需要重置 mAppData.mIsMediaPlayerLocked, 否则无法播放下一曲]
        mAppData.mIsMediaPlayerLocked = false;
        onSendMessage(IMediaEvent.EVENT_MEDIA_COMPLETION, null);
    }

    /** 处理播放错误信息 **/
    private void onHandleError() {
        onHandleError(-1, true);
    }

    private void onHandleError(int reason, boolean direct) {
        onHandleError(reason, -1, direct);
    }

    private void onHandleError(int reason, int extra, boolean direct) {
        LogUtil.e(TAG, "onHandlerError:"
                + " reason = " + reason + "[" + extra + "], direct = " + direct);

        // 已经处理过，无须重复处理
        if (null == mMediaPlayer) {
            return;
        }

        // 移除功能重复消息
        mPlayerHandler.removeMessages(MSG_HANDLE_PLAYER_ERROR);

        // 特殊原因特殊处理
        switch (reason) {
            case MediaPlayer.MEDIA_ERROR_SERVER_DIED: {
                mAppData.mIsMediaPlayerLocked = false;

                switch (extra) {
                    case MEDIASERVER_PROCESS_DEATH:
                    case MEDIAEXTRACTOR_PROCESS_DEATH: {
                        // 多媒体提取信息进程挂了[一般出现在播放过程中存储设备被移除]
                        // 这个时候不能随意去跳转软解, 因为可能目标文件已经移除, 如果
                        // 再去使用软解播放器播放, 会导致系统卸载失败，最终导致被闪退。
                        if (checkSpecialMediaErrorInfo()) {
                            LogUtil.e(TAG, "checkSpecialMediaErrorInfo: skip soft decoding!");
                            return; // 如果错误被处理, 直接返回。
                        }

                        // [不跳转软解码, 发现 checkSpecialMediaErrorInfo 不能 100% 拦截]
                        LogUtil.e(TAG, "MediaPlayerService Error: skip soft decoding!");
                        return;
                    }

                    default: {
                        break;
                    }
                }
                break;
            }

            case MediaPlayer.MEDIA_ERROR_UNKNOWN:
            default: {
                break;
            }
        }

        // [转发出去由独立的消息处理, 主要是为了测试移除盘符效果]
        if (!direct) {
            mPlayerHandler.sendEmptyMessageDelayed(MSG_HANDLE_PLAYER_ERROR, 10);
            return;
        }

        onStopPlayer();

        // 硬解码不支持，软解码可能支持，不支持拖动消息可以移除，避免提示误解。
        mPlayerHandler.removeMessages(MSG_UNSUPPORT_SEEKABLE);

        // 不要马上就切换，资源释放可能需要时间，避免并行太多任务
        mPlayerHandler.removeMessages(MSG_UNSUPPORT_DECODE);
        mPlayerHandler.sendEmptyMessageDelayed(MSG_UNSUPPORT_DECODE, 1000);
    }

    // [检查特殊的媒体异常信息, 并单独处理]
    private boolean checkSpecialMediaErrorInfo() {
        if (null == mMediaInfo) {
            // 目标信息无效
            return false;
        }

        String targetPath = mMediaInfo.mFilePath;
        if (TextUtils.isEmpty(targetPath)) {
            // 目标文件无效
            return false;
        }

        // 系统移除 USB 后, USB 设备可能还在 Mounted 状态。
        boolean isMounted = mLocalzModel.targetStorageMounted(targetPath);
        if (isMounted) {
            LogUtil.e(TAG, "checkSpecialMediaErrorInfo: isMounted!");
            // 目标存储有效, 无须特殊处理
            return false;
        }

        // 可以添加消息通知系统刷新, 提高 UI 刷新体验
        // ...

        return true;
    }

    /**
     * 是指定文件类型
     *
     * @param type 歌曲后缀类型(.mp3/.flac/.wav...)
     * @return 是/否
     */
    private boolean isFileType(String type) {
        if (mMediaInfo != null) {
            String filePath = mMediaInfo.mFilePath;
            if (!TextUtils.isEmpty(filePath) && !TextUtils.isEmpty(type)) {
                int pos = filePath.lastIndexOf('.');
                if (pos != -1) {
                    String strSuffix = filePath.substring(pos);
                    if (type.equalsIgnoreCase(strSuffix)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    @Override
    public void onBufferingUpdate(MediaPlayer mp, int percent) {
        LogUtil.i(TAG, "onBufferingUpdate, percent: " + percent);
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        LogUtil.i(TAG, "PlayService_OnCompletionListener.");

        // 只处理音乐的情况(个别歌曲播放过程中突然中断)
        // 案例：测试歌曲(Jihan Audy - Goyang Pak Eko.mp3)
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC
                && !isMediaPlayerState(IState.ERROR) && !isMediaPlayerState(IState.END)
                && !mPlayerHandler.hasMessages(MSG_HANDLE_PLAYER_ERROR)
                && !mPlayerHandler.hasMessages(MSG_UNSUPPORT_DECODE)
                && mMediaPlayer != null && mMediaPlayer == mp && isFileType(".mp3")) {
           int duration = mp.getDuration() / 1000;
           int current = (mp.getCurrentPosition() + 500) / 1000;

           if (current + 10 < duration) {
               seekToTime((current + 1) * 1000 + 500);
               onPlayControlEvent(IMusicState.PLAY_CMD_PLAY, -2);
               Logger.t(TAG).d("MP3/Unable to resync. Signalling end of stream?");
               return;
           }
        }

        mMPlayerState = IState.PLAYBACK_COMPLETED;
        notifyToPlayNext(301, true);
    }

    // MediaPlayer.MEDIA_ERROR_SERVER_DIED extra info
    private static final int MEDIASERVER_PROCESS_DEATH = 0;
    private static final int MEDIAEXTRACTOR_PROCESS_DEATH = 1;
    private static final int MEDIACODEC_PROCESS_DEATH = 2;
    private static final int AUDIO_PROCESS_DEATH = 3;
    private static final int CAMERA_PROCESS_DEATH = 4;

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        LogUtil.i(TAG, "PlayService_OnErrorListener, what: " + what + " extra: " + extra);

        /**
         * -------------------------------------
         * @param extra:
         *   MEDIA_ERROR_IO          = -1004;
         *   MEDIA_ERROR_MALFORMED   = -1007;
         *   MEDIA_ERROR_UNSUPPORTED = -1010;
         *   MEDIA_ERROR_TIMED_OUT   = -110;
         *   MEDIAEXTRACTOR_PROCESS_DEATH = 1;   // media.extractor
         * -------------------------------------
         */

        switch (what) {
            case MediaPlayer.MEDIA_ERROR_UNKNOWN:
                LogUtil.i(TAG, "PlayService_OnErrorListener: MEDIA_ERROR_UNKNOWN.");
                break;

            case MediaPlayer.MEDIA_ERROR_SERVER_DIED:
                LogUtil.i(TAG, "PlayService_OnErrorListener: MEDIA_ERROR_SERVER_DIED.");
                break;

            case -38:
                // 可能的情况: [SurfaceHolder Invalid, 原因 SurfaceView 被占用]
                break;

            default:
                break;
        }

        mMPlayerState = IState.ERROR;
        onHandleError(what, extra, false);
        return false;
    }

    @Override
    public boolean onInfo(MediaPlayer mp, int what, int extra) {
        LogUtil.i(TAG, "onInfo, what: " + what);

        switch (what) {
            case MediaPlayer.MEDIA_INFO_BAD_INTERLEAVING:
                break;

            case MediaPlayer.MEDIA_INFO_METADATA_UPDATE:
                break;

            case MediaPlayer.MEDIA_INFO_AUDIO_NOT_PLAYING: {
                switch (mAppData.mMediaType) {
                    case IMusicState.MEDIA_TYPE_MUSIC:
                        onHandleError(what, false);
                        break;

                    case IMusicState.MEDIA_TYPE_VIDEO:
                        // [支持软解情况下 Audio 不能播放需要切换软解码]
                        if (Argument.isSupportVitamio()) {
                            if (is1080PVideoSource()
                                    && isPlayFileSuffix(".mkv")) {
                                // 8321 平台特殊处理
                                if (PlatformUtils.isHardware(PlatformUtils.MT8321)) {
                                    return false;
                                }

                                // 标记可以牺牲视频质量
                                mHDVideoObject.set(mMediaInfo, true);
                            }

                            onHandleError(what, false);
                        }
                        break;

                    default:
                        break;
                }
                break;
            }

            case MediaPlayer.MEDIA_INFO_VIDEO_NOT_PLAYING: {
                // [播放视频时才处理]
                // [发现如果 setDisplay 不清空, 如果 SurfaceHolder 无效了，它也会上报]
                if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
                    onHandleError(what, false);
                }
                break;
            }

            case MediaPlayer.MEDIA_INFO_VIDEO_TRACK_LAGGING:
            // MediaPlayer.MEDIA_INFO_VIDEO_NOT_SUPPORTED:
            case 860:
                onSendMessage(IMediaEvent.EVENT_UNSUPPORT_VIDEO_CODE, mMediaInfo);
                break;

            // MediaPlayer.MEDIA_INFO_AUDIO_NOT_SUPPORTED
            case 862:
                onSendMessage(IMediaEvent.EVENT_UNSUPPORT_AUDIO_CODE, null);
                break;

            case MediaPlayer.MEDIA_INFO_NOT_SEEKABLE:
                // 1S 后再通知UI提示，快速切曲的时候，体验会比较好
                mPlayerHandler.removeMessages(MSG_UNSUPPORT_SEEKABLE);
                mPlayerHandler.sendEmptyMessageDelayed(MSG_UNSUPPORT_SEEKABLE, 1000);
                break;

            case MediaPlayer.MEDIA_INFO_VIDEO_RENDERING_START: {
                if (mSeekToFlag) {
                    mSeekToFlag = false;
                    seekToTime(mSeekTime);
                }

                // 存储播放信息
                mLocalzModel.writeMediaTime(
                        mAppData.mMediaType, mFilePath, mSeekTime, 108);

                // 第一帧图像准备好， 隐藏 UI 黑色遮罩
                mIsVideoRenderingStart = true;
                mPlayerHandler.removeMessages(MSG_HIDE_BLACK_PAGE);

                if (isPrepared()) {
                    // [主要是为了避免拉起进程后闪烁视频第一帧显示]
                    mPlayerHandler.sendEmptyMessageDelayed(MSG_HIDE_BLACK_PAGE, 200);
                } else {
                    // [代码覆盖率打印]理论上不应该发生
                    LogUtil.i(TAG, "MEDIA_INFO_BUFFERING_END: <false == isPrepared()>.");
                }
                break;
            }

            case MediaPlayer.MEDIA_INFO_UNKNOWN: {
                LogUtil.i(TAG, "handleError3");
                onHandleError(what, false);
                break;
            }

            default:
                break;
        }

        return false;
    }

    /** 是 Prepared 状态 **/
    public boolean isPrepared() {
        if (null == mMediaPlayer) {
            return false;
        }

        switch (mMPlayerState) {
            case IState.PREPARED:
            case IState.STARTED:
            case IState.PAUSED:
            case IState.PLAYBACK_COMPLETED:
            case IState.ERROR:
                return true;
            default:
                break;
        }

        return false;
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        LogUtil.i(TAG, ">>> onPrepared.");

        mMPlayerState = IState.PREPARED;
        mOnPreparedTimeout = false;
        mPlayerHandler.removeMessages(MSG_WAIT_PREPARED_TIMEOUT);

        // 解锁说明可以播放了 [非解锁状态, release() 会出现 ANR]
        mAppData.mIsMediaPlayerLocked = false;

        // [过滤不匹配的回调事件]
        if (null == mMediaPlayer) {
            return;
        }

        // 如果是视频需要调整 Surface 的尺寸
        if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
            if (mVideoSizeChanged) {
                updateSurfaceHolderSize(mp);
            } else {
                // 硬解码部分，这里如果视频还没有大小，基本没可能再有 onVideoSizeChanged(...) 回调了；
                mPlayerHandler.removeMessages(MSG_VIDEO_SIZE_CHANGED_TIMEOUT);
                mPlayerHandler.sendEmptyMessageDelayed(MSG_VIDEO_SIZE_CHANGED_TIMEOUT, 200);
            }
        }

        mSeekTime = 0;
        mSeekToFlag = false;
        int duration = mMediaPlayer.getDuration();
        mAppData.mPlayTimeInfo.mTotalTime = duration;
        LogUtil.d(TAG, "duration: " + duration);

        // 断电记忆播放
        if (!TextUtils.isEmpty(mFilePath)
                && mAppData.mMediaType != IMusicState.MEDIA_TYPE_IDLE) {
            int nTime = mLocalzModel.readMediaTime(mAppData.mMediaType, mFilePath);

            if (nTime > 0) {
                switch (mAppData.mMediaType) {
                    case IMusicState.MEDIA_TYPE_MUSIC:
                        mSeekTime = nTime;
                        seekToTime(nTime);
                        break;
                    case IMusicState.MEDIA_TYPE_VIDEO:
                        mSeekToFlag = true;
                        mSeekTime = nTime;
                        break;
                    default:
                        break;
                }
            } else if (nTime == -1) {
                mSeekTime = 0;
                mLocalzModel.writeMediaTime(
                        mAppData.mMediaType, mFilePath, 0, 109);
            }
        }

        // [检查状态: 比较乱，需要梳理原因]
        if (IMusicState.MEDIA_TYPE_IDLE == mAppData.mMediaType) {
            LogUtil.d(TAG, " -- MEDIA_TYPE_NULL");
            mAppData.mAllowResumePlay = true;
        } else if (mLocalzModel.existsHighPriorityEvent()) {
            mAppData.mAllowResumePlay = true;
            LogUtil.d(TAG, " -- existsHighPriorityEvent: true.");
        } else if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType
                && !mLocalzModel.isCanPlayVideo()) {
            mAppData.mAllowResumePlay = true;
            LogUtil.d(TAG, " -- isCanPlayVideo: false.");
        } else {
            onPlayControlEvent(IMusicState.PLAY_CMD_PLAY, -1);
        }

        // 检测 ID3 信息
        checkTargetID3Info(mMediaInfo);
    }

    @Override
    public void onSeekComplete(MediaPlayer mp) {
        LogUtil.i(TAG, ">>> onSeekComplete...");

        if (isPrepared()) {
            // [MediaPlayer.seekTo() 是异步方法]
            mAppData.mPlayTimeInfo.setCurrentTime(
                    getCurrentPosition(), true, "core");
            onSendMessage(IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME, mMediaInfo);

            // [拖动完成: 存储记忆]
            mPlayerHandler.removeMessages(MSG_WRITE_MEDIA_TIME);
            mPlayerHandler.sendEmptyMessageDelayed(MSG_WRITE_MEDIA_TIME, 1000);

            // [拖动完成: 通知播放界面，是否触发播放]
            onSendMessage(IMediaEvent.EVENT_SEEK_TO_COMPLETE, mMediaInfo);
        }
    }

    @Override
    public void onTimedText(MediaPlayer mp, TimedText text) {
        LogUtil.i(TAG, ">>> onTimedText ");
    }

    /**
     * 注意调用顺序:
     * <pre>
     *     void NuPlayer::StreamingSource::prepareAsync() {
     *        ....
     *        notifyVideoSizeChanged();
     *        notifyPrepared();
     *     }
     * </pre>
     *
     * @param mp MediaPlayer 对象
     * @param width 视频宽度
     * @param height 视频高度
     */
    @Override
    public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
        if (mp != mMediaPlayer) {
            LogUtil.i(TAG, ">>> onVideoSizeChanged, [mp != mMediaPlayer]...");
        }

        mVideoSizeChanged = true;
        mPlayerHandler.removeMessages(MSG_VIDEO_SIZE_CHANGED_TIMEOUT);

        // 过滤打印
        if (mVideoWidth != width || mVideoHeight != height) {
            LogUtil.i(TAG, ">>> onVideoSizeChanged, width: " + width + " height: " + height);

            if (is1080PVideoSource()) {
                mHDVideoObject.set(mMediaInfo, false);
            }
        }

        updateSurfaceHolderSize(mp);
    }

    // 计算视频尺寸
    private void computeAndUpdateVideoSize() {
        // 无效状态不计算
        if (!isPrepared()) {
            return;
        }

        // 获取当前视频源高宽
        mVideoWidth = mMediaPlayer.getVideoWidth();
        mVideoHeight = mMediaPlayer.getVideoHeight();

        // 检查是否是无效的视频源
        if (mVideoWidth <= 0 || mVideoHeight <= 0) {
            return;
        }

        // [横屏需要计算视频尺寸, 主要是为了不动出货状态]
        Context context = mContextRef.get();
        assert context != null;
        if (ThemeEx.isHorizontalScreenDeviceCompat(context)) {
            Point point = MediaUtils.computeAndUpdateVideoSize(mVideoWidth, mVideoHeight);
            mVideoWidth = point.x;
            mVideoHeight = point.y;
        }
    }

    /**
     * 是否是 1080P 视频源
     * <pre>
     *    由 onVideoSizeChanged(MediaPlayer, int, int) 触发；
     *    简单判定，如果视频源的分辨率大于 1440x960 我们就认为是 1080P；
     * </pre>
     *
     * @return 是/否
     */
    @Override
    public boolean is1080PVideoSource() {
        if (Objects.isNull(mMediaPlayer)) {
            return false;
        }

        return mVideoSizeChanged
                && mMediaPlayer.getVideoWidth() > 1440
                && mMediaPlayer.getVideoHeight() > 960;
    }

    private void updateSurfaceHolderSize(MediaPlayer mp) {
        if (null == mp) {
            LogUtil.e(TAG, ">>> updateSurfaceHolderSize, [null == mp]...");
            return;
        }

        mVideoWidth = mp.getVideoWidth();
        mVideoHeight = mp.getVideoHeight();

        // [如果没有视频: getVideoWidth/Height 是有可能返回 0 的，所以涉及计算需要判定]
        if (0 != mVideoWidth && 0 != mVideoHeight) {
            computeAndUpdateVideoSize();

            if (null != mCurrSurHolder) {
                mCurrSurHolder.setFixedSize(mVideoWidth, mVideoHeight);
            }

            if (null != mCurrRearSurHolder) {
                mCurrRearSurHolder.setFixedSize(mVideoWidth, mVideoHeight);
            }

            mPlayerModel.onMediaEvent(
                    IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_SIZE,
                    mVideoWidth,
                    mVideoHeight);
        }
    }
}
