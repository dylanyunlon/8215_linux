package com.hcn.media_model.player;

import static java.lang.Math.abs;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.SystemClock;
import android.text.TextUtils;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

import com.hcn.auto_compat.os.ProcessCompat;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_common.HMessage;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_model.MediaUtils;
import com.hcn.media_model.base.ILocalzModel;
import com.hcn.media_model.base.IPlayerModel;
import com.hcn.media_model.player.base.BasePlayer;
import com.hcn.media_model.player.base.IMediaPlayer;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.CommandExecution;
import com.hcn.media_base.HMediaConfig;
import com.hcn.media_theme.ThemeEx;

import java.io.File;
import java.sql.Time;
import java.util.Locale;
import java.util.Objects;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import io.vov.vitamio.MediaPlayer;
import io.vov.vitamio.MediaPlayer.OnBufferingUpdateListener;
import io.vov.vitamio.MediaPlayer.OnCompletionListener;
import io.vov.vitamio.MediaPlayer.OnErrorListener;
import io.vov.vitamio.MediaPlayer.OnInfoListener;
import io.vov.vitamio.MediaPlayer.OnPreparedListener;
import io.vov.vitamio.MediaPlayer.OnSeekCompleteListener;
import io.vov.vitamio.MediaPlayer.OnTimedTextListener;
import io.vov.vitamio.MediaPlayer.OnVideoSizeChangedListener;
import io.vov.vitamio.MediaPlayer.OnHWRenderFailedListener;
import io.vov.vitamio.utils.Log;

/**
 * 软解播放器
 * <p> 非平台关联类，禁止外部调用它；
 *
 * @author 65821
 */
class VitamioPlayer extends BasePlayer
        implements IMediaPlayer,
        OnCompletionListener,
        OnPreparedListener,
        OnErrorListener,
        OnInfoListener,
        OnSeekCompleteListener,
        OnVideoSizeChangedListener,
        OnBufferingUpdateListener,
        OnTimedTextListener,
        OnHWRenderFailedListener {

    private static final String TAG = VitamioPlayer.class.getSimpleName();
    private final static String SPEC_VIDEO_SUFFIX = ".avi.mp4.";

    /** BUFFERING STATUS | 默认值 **/
    public static final int BUFFERING_NONE = 0;

    /** BUFFERING STATUS | 初始加载缓存 **/
    public static final int BUFFERING_START = 1;

    /** BUFFERING STATUS | 播放中触发缓存 **/
    public static final int BUFFERING_RESTART = 2;

    /** BUFFERING STATUS | 加载缓存结束 **/
    public static final int BUFFERING_END = 3;

    /** LITTLE VIDEO DURATION LIMIT | 小视频时长判定限值 **/
    public static final int LITTLE_VIDEO_DURATION_LIMIT = 4000;

    /** LITTLE VIDEO DURATION LIMIT | 小视频初始定位时间 **/
    public static final int LITTLE_VIDEO_INITIAL_SEEK_TIME = 200;

    /** 异步任务处理器 */
    private final AsyncHandler mAsyncHandler;

    /** 播放解码组件 **/
    private MediaPlayer mMediaPlayer = null;
    private volatile boolean mIsPrepared = false;
    private volatile boolean mIsVideoRenderingStart = false;
    private MusicInfo mMediaInfo = null;
    private final Object mMediaPlayerLockObj = new Object();

    private SurfaceHolder mCurrSurHolder = null;
    private SurfaceHolder mCurrRearSurHolder = null;
    private boolean mIsNullDisplay = true;

    private int mBufferingStatus = BUFFERING_NONE;

    /** 日志 no frame! 事件计数 **/
    private int mNoFrameLogCount = 0;

    /** MediaPlayerHandler 消息定义 **/
    private interface MsgEx {
        /** 不支持的解码 **/
        int MSG_UNSUPPORT_DECODE = 2;

        /** 处理播放错误 **/
        int MSG_HANDLE_PLAYER_ERROR = 3;

        /** 存储播放状态 **/
        int MSG_WRITE_MEDIA_TIME = 4;

        /** 声音淡出处理 **/
        int MSG_VOLUME_FADE_DOWN = 5;

        /** 声音淡入处理 **/
        int MSG_VOLUME_FADE_UP = 6;

        /** 隐藏视频显示层黑色遮罩 **/
        int MSG_HIDE_BLACK_PAGE = 7;

        /** BUFFERING_START/END 事件接收超时 **/
        int MSG_BUFFERING_TIMEOUT = 8;

        /** 第一个 BUFFERING_END 事件接收超时 **/
        int MSG_FIRST_BUFFERING_END_TIMEOUT = 9;

        /** 视频大小从有效变无效 **/
        int MSG_VIDEO_SIZE_VALID_2_INVALID = 10;

        /** 检测视频流畅度 **/
        int MSG_MONITOR_PLAY_FLUENCY = 11;

        /** 避免部分小视频(短时长)不自动播放跳曲 **/
        int MSG_LITTLE_VIDEO_PLAY_TIMEOUT = 12;

        /** 抓到打印日志，提示没有帧数据。 **/
        int MSG_FFMPEG_NO_FRAME_LOG = 13;
    }

    private final MediaPlayerHandler mPlayerHandler = new MediaPlayerHandler(Looper.getMainLooper());

    @SuppressLint("HandlerLeak")
    private class MediaPlayerHandler extends Handler {
        public float mTargetVolume = 1.0f;
        public float mCurrentVolume = 1.0f;

        public MediaPlayerHandler(@NonNull Looper looper) {
            super(looper);
        }

        public void fadeUp() {
            removeFade();
            sendEmptyMessageDelayed(MsgEx.MSG_VOLUME_FADE_UP, 800);
        }

        public void fadeDown() {
            removeFade();
            sendEmptyMessageDelayed(MsgEx.MSG_VOLUME_FADE_DOWN, 10);
        }

        public void removeFade() {
            removeMessages(MsgEx.MSG_VOLUME_FADE_UP);
            removeMessages(MsgEx.MSG_VOLUME_FADE_DOWN);
        }

        // 强制改变当前音量值，不会改变 mTargetVolume 的值
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
                case MsgEx.MSG_UNSUPPORT_DECODE: {
                    if (mMediaInfo != null) {
                        onSendMessage(IMediaEvent.EVENT_VITAMIO_CODE_UNSUPPORT, mMediaInfo);
                    }
                    break;
                }

                case MsgEx.MSG_HANDLE_PLAYER_ERROR: {
                    onHandleError();
                    break;
                }

                case MsgEx.MSG_WRITE_MEDIA_TIME: {
                    writeCurrentMediaTime();

                    // [播放状态触发后, 5S 更新一次]
                    if (mMediaPlayer != null && mIsPrepared) {
                        if (!mMediaPlayer.isPlaying()) {
                            return;
                        }

                        sendEmptyMessageDelayed(MsgEx.MSG_WRITE_MEDIA_TIME, 2000);
                    }
                    break;
                }

                case MsgEx.MSG_VOLUME_FADE_DOWN: {
                    mCurrentVolume -= .05f;

                    if (mCurrentVolume > mTargetVolume) {
                        removeMessages(MsgEx.MSG_VOLUME_FADE_DOWN);
                        sendEmptyMessageDelayed(MsgEx.MSG_VOLUME_FADE_DOWN, 20);
                    } else {
                        mCurrentVolume = mTargetVolume;
                    }

                    setVolume(mCurrentVolume, mCurrentVolume);
                    break;
                }

                case MsgEx.MSG_VOLUME_FADE_UP: {
                    mCurrentVolume += .025f;

                    if (mCurrentVolume < mTargetVolume) {
                        removeMessages(MsgEx.MSG_VOLUME_FADE_UP);
                        sendEmptyMessageDelayed(MsgEx.MSG_VOLUME_FADE_UP, 25);
                    } else {
                        mCurrentVolume = mTargetVolume;
                    }

                    setVolume(mCurrentVolume, mCurrentVolume);
                    break;
                }

                case MsgEx.MSG_HIDE_BLACK_PAGE: {
                    onMsgHideBlackPage();
                    break;
                }

                case MsgEx.MSG_BUFFERING_TIMEOUT: {
                    // [INT 非常难遇到，如果遇到不处理，就是黑屏显示]
                    if (mIsPrepared && mVideoSizeChanged) {
                        if (!hasMessages(MsgEx.MSG_HIDE_BLACK_PAGE)) {
                            sendEmptyMessageDelayed(MsgEx.MSG_HIDE_BLACK_PAGE, 200);
                        }
                    }
                    break;
                }

                case MsgEx.MSG_FIRST_BUFFERING_END_TIMEOUT: {
                    // [有些视频格式的文件没有视频帧, 但是有音频帧]
                    if (mIsPrepared && !mVideoSizeChanged) {
                        onSendMessage(IMediaEvent.EVENT_UNSUPPORT_VIDEO_PROMPT_SHOW, null);
                    }
                    break;
                }

                case MsgEx.MSG_VIDEO_SIZE_VALID_2_INVALID: {
                    notifyToPlayNext(501, false);
                    break;
                }

                case MsgEx.MSG_MONITOR_PLAY_FLUENCY: {
                    monitorVideoPlayFluency(true);
                    break;
                }

                case MsgEx.MSG_LITTLE_VIDEO_PLAY_TIMEOUT: {
                    // 小视频播放超时了, 强制跳曲
                    notifyToPlayNext(601, false);
                    break;
                }

                case MsgEx.MSG_FFMPEG_NO_FRAME_LOG: {
                    if (mMediaPlayer != null && mIsPrepared) {
                        if (mNoFrameLogCount++ == 5) {
                            notifyToPlayNext(701, false);
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        }

        /** 处理 MSG_HIDE_BLACK_PAGE 消息 **/
        private void onMsgHideBlackPage() {
            // [检查当前目标视频文件是否存在有效视频显示帧]
            if (!mVideoSizeChanged || !mIsVideoRenderingStart) {
                onSendMessage(IMediaEvent.EVENT_VIDEO_HIDE_BLACK_PAGE, null);
            }

            // [如果播放组件状态正常，可检查解析一帧缩略图]
            if (isInited() && !TextUtils.isEmpty(mFilePath)) {
                boolean specialVideoSuffix = false;
                boolean needExtractThumbnail = true;

                // [获取视频文件后缀]
                int pos = mFilePath.lastIndexOf('.');
                if (pos != -1) {
                    String strSuffix = mFilePath.substring(pos) + ".";
                    strSuffix = strSuffix.toLowerCase(Locale.getDefault());

                    // [是否是特殊视频后缀]
                    if (HMediaConfig.SPECIAL_VIDEO_FRAME_SUFFIX.contains(strSuffix)) {
                        specialVideoSuffix = true;
                    }
                }

                // [是否存在缓存缩略图]
                if (specialVideoSuffix) {
                    boolean exist =
                            BitmapCache.getInstance().
                                    existDiskCacheThumbnail(mFilePath);
                    needExtractThumbnail = !exist;
                }

                // [是否需要提取缩略图]
                if (needExtractThumbnail) {
                    mAsyncHandler.removeMessages(
                            AsyncHandler.MSG_EXTRACT_VIDEO_THUMBNAIL);

                    Message message = mAsyncHandler.obtainMessage();
                    message.what = AsyncHandler.MSG_EXTRACT_VIDEO_THUMBNAIL;
                    message.obj = mFilePath;
                    mAsyncHandler.sendMessageDelayed(message, 200);
                }
            }

            onSendMessage(IMediaEvent.EVENT_VIDEO_HIDE_BLACK_PAGE, null);
        }
    }

    /**
     * 监视视频播放流畅度
     * <pre>
     *    检查打印视频是否播放卡顿，简单的说就是现实 1 秒播放不了 1 秒的进度；
     *    一般出现这种情况表示系统性能不够解析播放该视频，或者是视频本身的总时长信息错误；
     * </pre>
     */
    private int mCurrentDuration = 0;
    private int mMonitorFluencyCount = 0;

    /** 禁止构造无参对象 **/
    private VitamioPlayer() {
        super(null, null, null);
        throw new RuntimeException(
                "Prohibit the construction of parameterless objects");
    }

    /**
     * VitamioPlayer 默认构造函数
     *
     * @param context 应用上下文环境
     * @param localzModel {@link ILocalzModel} 业务模型
     * @param playerModel {@link IPlayerModel} 播放模型
     */
    public VitamioPlayer(
            @NonNull Context context,
            @NonNull ILocalzModel localzModel,
            @NonNull IPlayerModel playerModel) {
        super(context, localzModel, playerModel);

        // 处理异步任务的 Handler
        HandlerThread handlerThread = new HandlerThread("H-VitamioPlayer");
        handlerThread.start();
        mAsyncHandler = new AsyncHandler(handlerThread.getLooper());

        // 错误日志铺货过滤器
        new Thread() {
            @Override
            public void run() {
                super.run();

                String command = "logcat -s Vitamio[5.2.3][Player]:E";
                CommandExecution.execCommand(command, false, info -> {
                    if (mMediaPlayer == null) {
                        return;
                    }

                    if (info != null && info.contains("] no frame!")) {
                        mPlayerHandler.sendEmptyMessage(MsgEx.MSG_FFMPEG_NO_FRAME_LOG);
                    }
                });

                Log.d(TAG, "logcat filter thread exit!");
            }
        }.start();
    }

    /** 监视视频播放流畅度 **/
    private void monitorVideoPlayFluency(boolean start) {
        if (start) {
            // 视频才需要监视
            if (IMusicState.MEDIA_TYPE_VIDEO != mAppData.mMediaType) {
                monitorVideoPlayFluency(false);
                return;
            }

            // 有效才需要监视
            if (null == mMediaPlayer || !mIsPrepared) {
                monitorVideoPlayFluency(false);
                return;
            }

            // 播放状态才需要监视
            if (!mMediaPlayer.isPlaying()) {
                monitorVideoPlayFluency(false);
                return;
            }

            // 记忆当前播放位置
            if (0 == mMonitorFluencyCount) {
                mCurrentDuration = getCurrentPosition();
                mCurrentDuration = mCurrentDuration / 1000;
            }

            // 如果总时长有效, 就开始跳秒。
            int totalTime = mAppData.mPlayTimeInfo.mTotalTime;
            if (totalTime > 0) {
                mMonitorFluencyCount++;

                if (mMonitorFluencyCount + mCurrentDuration > totalTime) {
                    LogUtil.vitamio_d(TAG, " -- monitorVideoPlayFluency. Found Exception[0]!");
                    monitorVideoPlayFluency(false);
                    return;
                }
            } else {
                // 时长无效
                monitorVideoPlayFluency(false);
                return;
            }

            // 1S 检查一次播放状态
            mPlayerHandler.removeMessages(MsgEx.MSG_MONITOR_PLAY_FLUENCY);
            mPlayerHandler.sendEmptyMessageDelayed(
                    MsgEx.MSG_MONITOR_PLAY_FLUENCY, 1000);
        } else {
            // 停止检查视频播放流畅度
            mPlayerHandler.removeMessages(MsgEx.MSG_MONITOR_PLAY_FLUENCY);

            mCurrentDuration = 0;
            mMonitorFluencyCount = 0;
        }
    }

    private void writeCurrentMediaTime() {
        if (null != mAppData.mCurrentMediaInfo) {
            mLocalzModel.writeMediaTime(
                    mAppData.mLastMediaType,
                    mAppData.mCurrentMediaInfo.mFilePath,
                    mAppData.mPlayTimeInfo.mCurrentTime,
                    110);
        }
    }

    /** 异步 Handler 处理器 **/
    private class AsyncHandler extends Handler {
        // [异步事件定义]
        public static final int MSG_RELEASE_MEDIAPLAYER = 1;
        public static final int MSG_EXTRACT_VIDEO_THUMBNAIL = 2;

        public AsyncHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_RELEASE_MEDIAPLAYER:
                    onMsgReleaseMediaPlayer(msg);
                    break;
                case MSG_EXTRACT_VIDEO_THUMBNAIL:
                    onMsgExtractVideoThumbnail(msg);
                    break;
                default:
                    break;
            }
        }

        /**
         * 释放软解码资源
         * <p> 只能在子线程中触发调用，禁止主线程调用这个方法；
         */
        private void releaseMediaPlayer() {
            synchronized (mMediaPlayerLockObj) {
                // 是空的直接返回
                if (Objects.isNull(mMediaPlayer)) {
                    mIsAsyncReleasing.set(false);
                    return;
                }

                // 超时任务（release() 可能会阻塞）
                Runnable timeoutTask = () -> {
                    if (Objects.isNull(mMediaPlayer)) {
                        return;
                    }

                    // 释放异常直接退出进程
                    mPlayerModel.onMediaEvent(
                            IMediaEvent.EVENT_EXIT_PROCESS, -9999, null);
                };

                // 避免释放异常
                mPlayerHandler.postDelayed(timeoutTask, 5000);

                // mMediaPlayer.reset();
                mMediaPlayer.release();
                mMediaPlayer = null;

                // 移除超时任务（超时任务会退出进程）
                mPlayerHandler.removeCallbacks(timeoutTask);

                // 重置与之关联的播放器变量（主线程）
                mPlayerHandler.post(() -> {
                    // 是空才可重置 prepareAsync() 关联状态
                    if (Objects.isNull(mMediaPlayer)) {
                        mIsPrepared = false;
                        mAppData.mIsMediaPlayerLocked = false;
                    }
                });

                // 异步执行释放任务完成
                mIsAsyncReleasing.set(false);
            }
        }

        /** MSG_RELEASE_MEDIAPLAYER **/
        private void onMsgReleaseMediaPlayer(Message msg) {
            LogUtil.vitamio_d(TAG, ">> [Enter]AsyncHandler: release.");

            final int[] timeout = {0};
            if (null != mMediaPlayer) {
                if (!(msg.obj instanceof Boolean)) {
                    throw new RuntimeException("MSG_RELEASE_MEDIAPLAYER/Illegal message payload!");
                }

                // 需要等待 Vitamio 进入 Prepared 状态后再释放
                boolean needWaitVitamio = (boolean) msg.obj;
                if (needWaitVitamio) {
                    // 构建一个定时器任务
                    Timer timer = new Timer();
                    TimerTask timerTask = new TimerTask() {
                        @Override
                        public void run() {
                            if (mIsPrepared && !mAppData.mIsMediaPlayerLocked) {
                                releaseMediaPlayer();
                                timer.cancel();

                                LogUtil.vitamio_d(TAG,
                                        ">> [Leave]AsyncHandler: release/timeout: " + timeout[0]);
                            } else {
                                timeout[0] += 25;
                                final int threshold = 2000;
                                if (timeout[0] > threshold) {
                                    releaseMediaPlayer();
                                    timer.cancel();

                                    LogUtil.vitamio_d(TAG,
                                            ">> [Leave]AsyncHandler: release/timeout.");
                                }
                            }
                        }
                    };

                    timer.schedule(timerTask, 0, 25);
                } else {
                    // 直接释放掉资源
                    releaseMediaPlayer();
                    LogUtil.vitamio_d(TAG, ">> [Leave]AsyncHandler: release.");
                }
            }
        }

        /** MSG_EXTRACT_VIDEO_THUMBNAIL **/
        private void onMsgExtractVideoThumbnail(Message msg) {
            String filePath;
            if (msg.obj instanceof String) {
                filePath = (String) msg.obj;
            } else {
                return;
            }

            // [非法参数]
            if (!isInited() || TextUtils.isEmpty(filePath)) {
                return;
            } else {
                if (!filePath.equals(mFilePath)) {
                    return;
                }
            }

            // [开始提取缩略图]
            Bitmap target = null;
            Bitmap bitmap = null;

            synchronized (mMediaPlayerLockObj) {
                if (mMediaPlayer != null) {
                    bitmap = mMediaPlayer.getCurrentFrame();
                } else {
                    return; // 无效返回
                }
            }

            if (null != bitmap) {
                // [压缩帧到缩略图]
                if (bitmap.getWidth() > 150 || bitmap.getHeight() > 150) {
                    target = Bitmap.createBitmap(150, 150,
                            bitmap.getConfig());

                    Canvas canvas = new Canvas(target);
                    canvas.drawBitmap(bitmap, null,
                            new Rect(0,
                                    0,
                                    target.getWidth(),
                                    target.getHeight()),
                            null);
                }

                if (null != target) {
                    bitmap.recycle();
                } else {
                    target = bitmap;
                }

                // [添加到磁盘缓存]
                BitmapCache.getInstance().addBitmapToDiskCache(filePath, target);

                // [添加到内存缓存]
                BitmapCache.HBmpPackage objValue = new BitmapCache.HBmpPackage();
                objValue.mBitmap = target;
                objValue.mExtracted = true;
                BitmapCache.getInstance().addHBmpPackageToMemoryCache(filePath, objValue);
            }
        }
    }

    private void onSendMessage(int eventId, MusicInfo info) {
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

        mPlayerModel.onMediaEvent(eventId, info, null);
    }

    /** [释放中不属于初始化状态] **/
    @Override
    public boolean isInited() {
        return (mMediaPlayer != null) && !isAsyncReleasing();
    }

    @Override
    public boolean isPrepared() {
        return isInited() && mIsPrepared;
    }

    @Override
    public int getVideoWidth() {
        int width = -1;

        if (null != mMediaPlayer) {
            if (isPrepared()) {
                width = mMediaPlayer.getVideoWidth();
            }
        }

        return width;
    }

    @Override
    public int getVideoHeight() {
        int height = -1;

        if (null != mMediaPlayer) {
            if (isPrepared()) {
                height = mMediaPlayer.getVideoHeight();
            }
        }

        return height;
    }

    private void onCreatePlayer(MusicInfo info) {
        if (null != mMediaPlayer) {
            onStopPlayer(-1);
        }

        LogUtil.vitamio_d(TAG, ">>> onCreatePlayer.");

        reset(false);
        mAppData.mCurrentMediaInfo = info;
        mMediaPlayer = new MediaPlayer(mContextRef.get());

        mMediaPlayer.setOnCompletionListener(this);
        mMediaPlayer.setOnPreparedListener(this);
        mMediaPlayer.setOnErrorListener(this);
        mMediaPlayer.setOnInfoListener(this);
        mMediaPlayer.setOnSeekCompleteListener(this);
        mMediaPlayer.setOnVideoSizeChangedListener(this);
        mMediaPlayer.setOnBufferingUpdateListener(this);
        mMediaPlayer.setOnTimedTextListener(this);
        mMediaPlayer.setOnHWRenderFailedListener(this);

        updateSurfaceHolder(true, 1);

        // 是否可以牺牲画质（放慢播放速度和增加缓存）
        if (HCorePlayer.isNeedLoseQuality(info)) {
            LogUtil.v(TAG, ">>> HD Video/Need lose quality!");
            mMediaPlayer.setVideoQuality(MediaPlayer.VIDEOQUALITY_LOW);
            mMediaPlayer.setPlaybackSpeed(0.9f);
            mMediaPlayer.setBufferSize(30 * 1024 * 1024);

            // 是在前台播放，那么提高进程优先级；
            Context context = mContextRef.get();
            assert context != null;
            if (mAppData.mVideoUiShow) {
                ProcessCompat.setThreadPriority(
                        context, Process.THREAD_PRIORITY_AUDIO);
            }
        }
    }

    private void setAudioSessionId(int Id) {
        mLocalzModel.setAudioSessionId(Id);
    }

    public void setVolume(float leftVolume, float rightVolume) {
        if (mMediaPlayer != null) {
            if (isAsyncReleasing()) {
                LogUtil.vitamio_d(TAG, ">>>> [Reject]setVolume, isAsyncReleasing!");
                return;
            }

            mMediaPlayer.setVolume(leftVolume, rightVolume);
        } else {
            mPlayerHandler.removeFade();
        }
    }

    // [非现线程安全] 确保在主线程调用
    @Override
    public void requestSetVolume(float volume) {
        mPlayerHandler.requestSetVolume(volume);
    }

    /**
     * 重置播放器参数
     * @param asyncReleasing 是否是异步释放资源中
     */
    private void reset(boolean asyncReleasing) {
        mVideoWidth = -1;
        mVideoHeight = -1;

        mNoFrameLogCount = 0;

        mSeekTime = 0;
        mSeekToFlag = false;

        mVideoSizeChanged = false;
        mIsVideoRenderingStart = false;

        mCurrSurHolder = null;
        mCurrRearSurHolder = null;
        mIsNullDisplay = true;

        mBufferingStatus = BUFFERING_NONE;
        mOnBufferingUpdatePercent = -1000;
        mOnBufferingUpdateCount = 0;
        mBufferingUpdateZeroTime = -1;

        if (!asyncReleasing) {
            mIsPrepared = false;
            mAppData.mIsMediaPlayerLocked = false;
        }
    }

    private void onStopPlayer() {
        onStopPlayer(0);
    }

    // 是资源释放中
    public volatile AtomicBoolean mIsAsyncReleasing = new AtomicBoolean(false);

    @Override
    public boolean isAsyncReleasing() {
        return mIsAsyncReleasing.get();
    }

    // [移除因视频播放触发的相关消息]
    private void removeVideoTriggerMessages() {
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            mPlayerHandler.removeMessages(MsgEx.MSG_HIDE_BLACK_PAGE);
            mPlayerHandler.removeMessages(MsgEx.MSG_BUFFERING_TIMEOUT);
            mPlayerHandler.removeMessages(MsgEx.MSG_FIRST_BUFFERING_END_TIMEOUT);
            mPlayerHandler.removeMessages(MsgEx.MSG_VIDEO_SIZE_VALID_2_INVALID);
            mPlayerHandler.removeMessages(MsgEx.MSG_LITTLE_VIDEO_PLAY_TIMEOUT);
        }
    }

    private void onStopPlayer(int reason) {
        if (isAsyncReleasing()) {
            LogUtil.vitamio_d(TAG, ">>>> [Reject]onStopPlayer, isAsyncReleasing!");
            return;
        }

        LogUtil.vitamio_d(TAG, ">>> onStopPlayer:" +
                " prepared = " + mIsPrepared + " reason = " + reason);

        // [还不能把它 mAppData.mCurrentMediaInfo 重置为 null, 因为播放完成后的切
        //  曲流程 EVENT_MEDIA_COMPLETION 会去重置播放进度, 否则单曲循环会出问题. ]
        // mAppData.mCurrentMediaInfo = null;
        mPlayerHandler.removeMessages(MsgEx.MSG_WRITE_MEDIA_TIME);

        // 清除播放相关的消息
        removeVideoTriggerMessages();

        // 停止监视视频流畅度
        monitorVideoPlayFluency(false);

        // [调节为正常优先级]
        ProcessCompat.setThreadPriority(null,
                ProcessCompat.THREAD_PRIORITY_TOP_APP_BOOST);

        if (Objects.isNull(mMediaPlayer)) {
            return;
        }

        // 是否播放相关资源
        try {
            if (mIsPrepared) {
                if (mMediaPlayer.isPlaying()) {
                    mPlayerHandler.setCurrentVolume(.01f);
                    mMediaPlayer.stop();
                }
            }

            // [不用设置 null]
            if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
                mMediaPlayer.setDisplay(null);
            }

            // [mMediaPlayer.reset() 会导致ARN, 它会执行 demux pthread_join 需要等待线程退出
            //  看 Vitamio 打印, mMediaPlayer.release() 包含了 mMediaPlayer.reset() 的工作.]
            // mMediaPlayer.reset();

            // [MediaPlayer.release() 有可能执行超时导致 ANR, 原因可能存在多种，需要具体分析;
            //  例如: 当前播放的文件存在损坏, 验证方法: 文件夹拷贝该文件到内置存储, 会拷贝不动.]

            if (HMediaConfig.VITAMIO_ASYNC_RELEASE) {
                synchronized (mMediaPlayerLockObj) {
                    mIsAsyncReleasing.set(true);

                    // 不要在非 Prepared 状态释放资源
                    boolean needWaitVitamio = mAppData.mIsMediaPlayerLocked && !mIsPrepared;
                    int releaseMsg = AsyncHandler.MSG_RELEASE_MEDIAPLAYER;
                    if (!mAsyncHandler.hasMessages(releaseMsg)) {
                        Message msg = mAsyncHandler.obtainMessage();
                        msg.what = releaseMsg;
                        msg.obj = needWaitVitamio;
                        mAsyncHandler.sendMessage(msg);
                    }
                }
            } else {
                synchronized (mMediaPlayerLockObj) {
                    mMediaPlayer.reset();
                    mMediaPlayer.release();
                    mMediaPlayer = null;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            reset(mIsAsyncReleasing.get());
        }
    }

    private void onPlayEvent(int reason) {
        if (isAsyncReleasing()) {
            LogUtil.vitamio_d(TAG, ">>> [Reject]onPlayEvent, isAsyncReleasing!");
            return;
        }

        LogUtil.vitamio_d(TAG,
                ">>> onPlayEvent: prepared = " + mIsPrepared + ", reason = " + reason);
        mLocalzModel.registerMediaButton();

        if (mMediaPlayer != null && mIsPrepared) {
            try {
                if (!mMediaPlayer.isPlaying()) {
                    updateSurfaceHolder(false, 2);

                    mPlayerHandler.setCurrentVolume(.01f);
                    mMediaPlayer.start();
                    mPlayerHandler.fadeUp();
                    monitorVideoPlayFluency(true);
                    checkLittleVideoPlayTimeout(true);

                    // [播放: 触发存储记忆]
                    mPlayerHandler.removeMessages(MsgEx.MSG_WRITE_MEDIA_TIME);
                    mPlayerHandler.sendEmptyMessageDelayed(
                            MsgEx.MSG_WRITE_MEDIA_TIME, 500);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void onPauseEvent() {
        if (isAsyncReleasing()) {
            LogUtil.vitamio_d(TAG, ">>> [Reject]onPauseEvent, isAsyncReleasing!");
            return;
        }

        LogUtil.vitamio_d(TAG, ">>> onPauseEvent.");

        if (mMediaPlayer != null && mIsPrepared) {
            mPlayerHandler.removeMessages(MsgEx.MSG_WRITE_MEDIA_TIME);

            try {
                if (mMediaPlayer.isPlaying()) {
                    mMediaPlayer.pause();
                    mPlayerHandler.setCurrentVolume(.01f);
                    monitorVideoPlayFluency(false);
                    checkLittleVideoPlayTimeout(false);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private boolean onPlayPauseEvent() {
        if (isAsyncReleasing()) {
            LogUtil.vitamio_d(TAG, ">>> [Reject]onPlayPauseEvent, isAsyncReleasing!");
            return false;
        }

        LogUtil.vitamio_d(TAG, ">>> onPlayPauseEvent.");

        if (mMediaPlayer != null && mIsPrepared) {
            mPlayerHandler.removeMessages(MsgEx.MSG_WRITE_MEDIA_TIME);

            try {
                if (mMediaPlayer.isPlaying()) {
                    mMediaPlayer.pause();
                    mPlayerHandler.setCurrentVolume(.01f);
                    monitorVideoPlayFluency(false);
                    checkLittleVideoPlayTimeout(false);
                } else {
                    updateSurfaceHolder(false, 3);

                    mPlayerHandler.setCurrentVolume(.01f);
                    mMediaPlayer.start();
                    mPlayerHandler.fadeUp();
                    monitorVideoPlayFluency(true);
                    checkLittleVideoPlayTimeout(true);

                    // [播放: 触发存储记忆]
                    mPlayerHandler.sendEmptyMessageDelayed(
                            MsgEx.MSG_WRITE_MEDIA_TIME, 500);

                    return true;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return false;
    }

    @Override
    public boolean isPlayState() {
        if (isAsyncReleasing()) {
            LogUtil.vitamio_d(TAG, ">>>> [Reject]isPlayStatus, isAsyncReleasing!");
            return false;
        }

        if (mMediaPlayer != null && mIsPrepared) {
            return mMediaPlayer.isPlaying();
        }

        return false;
    }

    // [一般在切换曲目和播放完成调用]
    @Override
    public void onSetSeekTimeZero() {
        mPlayerHandler.removeMessages(MsgEx.MSG_WRITE_MEDIA_TIME);
    }

    @Override
    public void seekToTime(int nTime) {
        if (isAsyncReleasing()) {
            LogUtil.vitamio_d(TAG, ">>>> [Reject]seekToTime, isAsyncReleasing!");
            return;
        }

        LogUtil.vitamio_d(TAG, ">>> seekToTime," +
                " isPrepared = " + mIsPrepared + " nTime: " + nTime);

        if (mMediaPlayer != null && mIsPrepared) {
            try {
                // [方法是异步的]
                mMediaPlayer.seekTo(nTime);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public int getCurrentPosition() {
        if (isAsyncReleasing()) {
            LogUtil.vitamio_d(TAG, ">>>> [Reject]getCurrentPosition, isAsyncReleasing!");
            return 0;
        }

        if (mMediaPlayer != null && mIsPrepared) {
            try {
                return (int) mMediaPlayer.getCurrentPosition();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return 0;
    }

    @Override
    public int getTotalTime() {
        if (isAsyncReleasing()) {
            LogUtil.vitamio_d(TAG, ">>>> [Reject]getTime, isAsyncReleasing!");
            return 0;
        }

        LogUtil.vitamio_d(TAG, ">>> getTime");
        if (mMediaPlayer != null && mIsPrepared) {
            try {
                return (int) mMediaPlayer.getDuration();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return 0;
    }

    @Override
    public void onPlayControlEvent(int nCommand) {
        onPlayControlEvent(nCommand, 0);
    }

    @Override
    public void onPlayControlEvent(int nCommand, int reason) {
        if (isAsyncReleasing()) {
            LogUtil.vitamio_d(TAG, ">>>> [Reject]onPlayControlEvent, isAsyncReleasing!");
            return;
        }

        switch (nCommand) {
            case IMusicState.PLAY_CMD_PLAY: {
                onPlayEvent(reason);
                onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY, null);
                break;
            }

            case IMusicState.PLAY_CMD_PAUSE: {
                onPauseEvent();
                onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PAUSE, null);
                break;
            }

            case IMusicState.PLAY_CMD_STOP: {
                onStopPlayer(reason);
                onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_STOP, null);
                break;
            }

            case IMusicState.PLAY_CMD_PLAY_PAUSE: {
                if (onPlayPauseEvent()) {
                    onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY, null);
                } else {
                    onSendMessage(IMediaEvent.EVENT_CHANGE_PLAY_STATE_PAUSE, null);
                }
                break;
            }

            default:
                break;
        }
    }

    @Override
    public void updateSurfaceHolder(boolean init) {
        updateSurfaceHolder(init, 0);
    }

    public void updateSurfaceHolder(boolean init, int reason) {
        // 非视频播放不处理
        if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_VIDEO) {
            return;
        }

        // 资源没有释放干净, 不处理新任务
        if (isAsyncReleasing()) {
            LogUtil.vitamio_d(TAG,
                    ">>>> [Reject]updateSurfaceHolder, isAsyncReleasing! reason: " + reason);
            return;
        }

        mCurrSurHolder = mAppData.mFrontSurfaceHolderEx;
        mCurrRearSurHolder = mAppData.mRearSurfaceHolder;

        if (mMediaPlayer != null && mIsPrepared) {
            LogUtil.vitamio_d(TAG,
                    ">>> updateSurfaceHolder isFrontVideo =  " + mAppData.isFrontVideo);

            if (mAppData.isFrontVideo) {
                if (null == mCurrSurHolder) {
                    LogUtil.vitamio_d(TAG, "mCurrSurHolder: null");
                } else {
                    if (!mCurrSurHolder.getSurface().isValid()) {
                        LogUtil.vitamio_d(TAG, ">>> updateSurfaceHolder surface invalid.");
                    }
                }

                // [抓取 SurfaceHolder 对象改变的场景]
                SurfaceHolder holder = mMediaPlayer.surfaceHolder();
                if (holder != null && mCurrSurHolder != holder) {
                    LogUtil.vitamio_d(TAG, ">>> updateSurfaceHolder surfaceHolder change.");
                }

                // [没有底层源码，所以重复设置效果未知]
                mMediaPlayer.setDisplay(mCurrSurHolder);

                // [如果 setDisplay(null) 后再次恢复前台，需要设置尺寸，否则存在问题.]
                if (mIsNullDisplay && null != mCurrSurHolder) {
                    if (mVideoSizeChanged) {
                        // [问题场景: 全屏模式或者画中画模式, 进入 RecentApps 后直接回主界
                        //  面, 再进入前台，然后再进入画中画，出现闪烁一下主界面背景的问题.]
                        if (mVideoWidth > 0 && mVideoHeight > 0) {
                            // [分屏切换, 需要重新计算]
                            computeAndUpdateVideoSize();
                            mCurrSurHolder.setFixedSize(mVideoWidth, mVideoHeight);

                            // 通知 Surface 大小更新
                            mPlayerModel.onMediaEvent(
                                    IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_SIZE,
                                    mVideoWidth, mVideoHeight);

                        }
                    } else {
                        // [问题场景: 画中画模式, 进入 RecentApps 后直接回主界面, 再次回到前台,
                        //  如果当前视频没有视频数据, 需要恢复 <不支持的视频编码> 提示信息显示.]
                        switch (mBufferingStatus) {
                            case BUFFERING_START:
                            case BUFFERING_END:
                            case BUFFERING_RESTART: {
                                // 接收到了 MediaPlayer.MEDIA_INFO_BUFFERING_START/END 还是没有视频数据。
                                mPlayerHandler.removeMessages(MsgEx.MSG_FIRST_BUFFERING_END_TIMEOUT);
                                mPlayerHandler.sendEmptyMessageDelayed(
                                        MsgEx.MSG_FIRST_BUFFERING_END_TIMEOUT, 1000);
                                break;
                            }

                            default: {
                                break;
                            }
                        }
                    }
                }

                // 存储当前 SurfaceHolder 是否有效
                mIsNullDisplay = (null == mCurrSurHolder);

                // 必须当前源存在有效显示帧
                if (!init && mVideoSizeChanged && mIsVideoRenderingStart) {
                    // [例如场景: 暂停播放，触发 Home 回到后台会进入这里]
                    LogUtil.vitamio_d(TAG, ">>> updateSurfaceHolder EVENT_VIDEO_HIDE_BLACK_PAGE.");
                    onSendMessage(IMediaEvent.EVENT_VIDEO_HIDE_BLACK_PAGE, null);
                }
            } else {
                if (null == mCurrRearSurHolder) {
                    LogUtil.vitamio_d(TAG, "mCurrRearSurHolder: " + null);
                }

                mCurrSurHolder = mCurrRearSurHolder;
                mMediaPlayer.setDisplay(mCurrSurHolder);
                mIsNullDisplay = (null == mCurrSurHolder);
            }
        }
    }

    @Override
    public void onSetDataSourceEvent(MusicInfo info) {
        if (null == info) {
            return;
        }

        // [特殊情况下给到最多 1S 的释放等待时间]
        try {
            int timeout = 0;
            while (isAsyncReleasing()) {
                if (timeout++ > 99) {
                    break;
                }

                Thread.sleep(10);
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        // 资源没有释放干净, 不处理新任务
        if (isAsyncReleasing()) {
            // 不释放干净，就不能处理下一个任务，将等待超时退出；
            LogUtil.vitamio_d(TAG, ">>> [Reject]onSetDataSourceEvent, isAsyncReleasing!");
            return;
        }

        // 停止监视视频播放流畅度
        monitorVideoPlayFluency(false);

        // 触发新任务, 移除播放相关消息
        removeVideoTriggerMessages();

        // 如果触发了新任务，移除软解跳转
        mPlayerHandler.removeMessages(MsgEx.MSG_UNSUPPORT_DECODE);
        mPlayerHandler.removeMessages(MsgEx.MSG_HANDLE_PLAYER_ERROR);

        mMediaInfo = info;
        String filePath = info.mFilePath;
        File file = new File(filePath);
        LogUtil.vitamio_d(TAG, ">>> [Enter]onSetDataSourceEvent, file: " + filePath);

        if (!file.exists()) {
            onFileNotExistError();
            return; // [文件不存在，直接中断当前流程]
        } else {
            mAppData.mFileNotExistCount = 0;
        }

        // [小于 100K 的文件调过，理论上视频要单独区分]
        if (file.length() < 100 * 1024) {
            onSendMessage(IMediaEvent.EVENT_ERROR_FILE_IS_TOO_SMALL, null);
            notifyToPlayNext(-1);
            return;
        }

        // [重置 MediaPlayer]
        onCreatePlayer(info);

        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            onSendMessage(IMediaEvent.EVENT_VIDEO_SHOW_BLACK_PAGE, null);
        }

        try {
            mFilePath = filePath;
            mAppData.mIsMediaPlayerLocked = true;
            mMediaPlayer.setDataSource(filePath);
        } catch (Exception ex) {
            ex.printStackTrace();

            // [解锁: 否则无法下一曲]
            mAppData.mIsMediaPlayerLocked = false;
            onHandleError();
            return;
        }

        try {
            // 非阻塞的准备模式
            mMediaPlayer.prepareAsync();

            // 通知播放器进入 PREPARING 状态
            if (mAppData.isMediaType(IMusicState.MEDIA_TYPE_VIDEO)) {
                dispatchMediaEvent(
                        HMessage.obtain(
                                IMediaEvent.EVENT_VIDEO_PLAYER_PREPARING,
                                info));
            }
        } catch (Exception e) {
            e.printStackTrace();

            // [解锁: 否则无法下一曲]
            mAppData.mIsMediaPlayerLocked = false;
            onHandleError();
        }

        LogUtil.vitamio_d(TAG, ">>> [Leave]onSetDataSourceEvent...");
    }

    private void onFileNotExistError() {
        // 文件不存在，提示后直接跳下一曲
        onSendMessage(IMediaEvent.EVENT_ERROR_FILE_NOT_EXIST, mMediaInfo);
        notifyToPlayNext(-2);
    }

    // 处理播放错误信息
    private void onHandleError() {
        onHandleError(true);
    }

    private void onHandleError(boolean direct) {
        LogUtil.e(TAG, "onHandlerError: direct = " + direct);

        // 已经处理过，无须重复处理
        if (null == mMediaPlayer) {
            return;
        }

        // 移除功能重复消息
        mPlayerHandler.removeMessages(MsgEx.MSG_HANDLE_PLAYER_ERROR);

        // [转发出去由独立的消息处理, 主要是为了测试移除盘符效果]
        if (!direct) {
            mPlayerHandler.sendEmptyMessageDelayed(
                    MsgEx.MSG_HANDLE_PLAYER_ERROR, 10);
            return;
        }

        onStopPlayer(102);

        // 不要马上就切换，资源释放可能需要时间，避免并行太多任务
        mPlayerHandler.removeMessages(MsgEx.MSG_UNSUPPORT_DECODE);
        mPlayerHandler.sendEmptyMessageDelayed(
                MsgEx.MSG_UNSUPPORT_DECODE, 1000);
    }

    // [通知播放下一个]
    private void notifyToPlayNext(int reason) {
        notifyToPlayNext(reason, false);
    }

    private void notifyToPlayNext(int reason, boolean completion) {
        // [如果先捕获到 PlayError 后收到 onCompletion 事件]
        if (completion) {
            if (mPlayerHandler.hasMessages(MsgEx.MSG_HANDLE_PLAYER_ERROR)
                    || mPlayerHandler.hasMessages(MsgEx.MSG_UNSUPPORT_DECODE)) {
                // 无须重复处理
                return;
            }
        } else {
            mPlayerHandler.removeMessages(MsgEx.MSG_HANDLE_PLAYER_ERROR);
            mPlayerHandler.removeMessages(MsgEx.MSG_UNSUPPORT_DECODE);
        }

        // 释放解码资源
        onStopPlayer(reason);

        // [通知播放下一个媒体文件: 如果进程第一次启动第一个文件就不支持,
        //  需要重置 mAppData.mIsMediaPlayerLocked, 否则无法播放下一曲]
        mAppData.mIsMediaPlayerLocked = false;
        onSendMessage(IMediaEvent.EVENT_MEDIA_COMPLETION, null);
    }

    // OnBufferingUpdateListener: onBufferingUpdate(... percent ...)
    public static int mOnBufferingUpdatePercent = -1000;
    public static int mOnBufferingUpdateCount = 0;
    public static long mBufferingUpdateZeroTime = -1;

    @Override
    public void onBufferingUpdate(MediaPlayer mp, int percent) {
        if (mOnBufferingUpdatePercent != percent) {
            mOnBufferingUpdatePercent = percent;
            mOnBufferingUpdateCount = 0;

            if (0 == percent) {
                mBufferingUpdateZeroTime = SystemClock.elapsedRealtime();
            }

            LogUtil.low_i(TAG, "onBufferingUpdate, percent: " + percent);
        } else {
            mOnBufferingUpdateCount++;

            // 过滤打印
            if (0 == mOnBufferingUpdateCount % 10) {
                // 清楚计数，无须太大的计数值
                if (0 == mOnBufferingUpdateCount % 60) {
                    mOnBufferingUpdateCount = 0;
                }

                LogUtil.low_i(TAG, "onBufferingUpdate, percent: " + percent);
            }
        }

        // [跳曲不过情况检查]
        if (0 == percent) {
            switch (mBufferingStatus) {
                case BUFFERING_START:
                case BUFFERING_RESTART: {
                    // 检查是否存在加载超时现象
                    long currentTime = SystemClock.elapsedRealtime();
                    long deltaTime = (currentTime - mBufferingUpdateZeroTime) / 1000;
                    int timeout = (BUFFERING_START == mBufferingStatus) ? 3 : 1;

                    // 超过3S加载不动，直接跳下一曲
                    if (deltaTime > timeout) {
                        mBufferingUpdateZeroTime = currentTime;
                        notifyToPlayNext(201);
                    }
                    break;
                }

                case BUFFERING_END: {
                    // 缓冲结束还连续5次上报无法加载, 说明视频播放存在问题了。
                    if (mOnBufferingUpdateCount >= 4) {
                        mOnBufferingUpdateCount = -1;

                        notifyToPlayNext(202);
                    }
                    break;
                }

                case BUFFERING_NONE:
                    // 暂不处理，还未遇到不触发加载且不报错情况。
                    LogUtil.vitamio_i(TAG, "onBufferingUpdate, BUFFERING_NONE!");
                    break;

                default:
                    break;
            }
        }
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        LogUtil.vitamio_i(TAG, "VitamioPlayer_OnCompletionListener.");

        // [可能存在: Error 捕获事件先于 onCompletion 回调]
        if (null == mMediaPlayer) {
            return; // 如果已经处理过了，不要再重复处理 Error
        }

        // 正常播放完成移除播放超时消息
        checkLittleVideoPlayTimeout(false);

        // 播放完成, 触发播放下一个流程
        notifyToPlayNext(301, true);
    }

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        LogUtil.vitamio_i(TAG,
                "VitamioPlayer_OnErrorListener, what: " + what + " extra: " + extra);

        onHandleError(false);

        return false; // 返回 false 将引发 OnCompletionListener 被调用。
    }

    // OnInfoListener: onInfo(... what ...)
    public static int mOnInfoListenerWhat = -1000;
    public static int mOnInfoListenerCount = 0;

    @Override
    public boolean onInfo(MediaPlayer mp, int what, int extra) {
        if (mOnInfoListenerWhat != what) {
            mOnInfoListenerWhat = what;
            mOnInfoListenerCount = 0;

            LogUtil.vitamio_i(TAG, "onInfo, what: " + what);
        } else {
            mOnInfoListenerCount++;

            if (0 == mOnInfoListenerCount % 60) {
                mOnInfoListenerCount = 0;
                LogUtil.vitamio_i(TAG, "onInfo, what: " + what);
            }
        }

        switch (what) {
            case MediaPlayer.MEDIA_INFO_VIDEO_TRACK_LAGGING: {
                if (0 == mOnInfoListenerCount) {
                    LogUtil.vitamio_i(TAG, "视频显示帧过于复杂，显示可能会卡顿，可能只有音频可以正常播放.");
                }
                break;
            }

            case 860: { // MediaPlayer.MEDIA_INFO_VIDEO_NOT_SUPPORTED:
                onSendMessage(IMediaEvent.EVENT_UNSUPPORT_VIDEO_CODE2, null);
                break;
            }

            case 862: { // MediaPlayer.MEDIA_INFO_AUDIO_NOT_SUPPORTED
                onSendMessage(IMediaEvent.EVENT_UNSUPPORT_AUDIO_CODE, null);
                break;
            }

            case MediaPlayer.MEDIA_INFO_NOT_SEEKABLE: {
                onSendMessage(IMediaEvent.EVENT_UNSUPPORT_SEEKABLE, null);
                break;
            }

            // 开始缓冲数据到缓冲区
            case MediaPlayer.MEDIA_INFO_BUFFERING_START: {
                onMediaInfoBufferingStart();
                break;
            }

            // 缓冲区填充完成[正常需要在这里开始触发播放才合理]
            // [注意: 如果有 MEDIA_INFO_BUFFERING_START 消息，没有 MEDIA_INFO_BUFFERING_END 消息，处理缺失]
            case MediaPlayer.MEDIA_INFO_BUFFERING_END: {
                onMediaInfoBufferingEnd();
                break;
            }

            // [猜测: 缓冲区无数据, 无法缓冲?]
            case 705: {
                // [例如：<序号5-ST-1 1920x1080 .... DOLBY_AC3.ts> 文件播放卡住一段时间后上报 705]
                notifyToPlayNext(401);
                break;
            }

            case MediaPlayer.MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK: {
                // 保留
                break;
            }

            case MediaPlayer.MEDIA_ERROR_TIMED_OUT:
            case MediaPlayer.MEDIA_ERROR_UNSUPPORTED:
            case MediaPlayer.MEDIA_ERROR_UNKNOWN: {
                onHandleError(false);
                break;
            }

            case MediaPlayer.MEDIA_INFO_DOWNLOAD_RATE_CHANGED: {
                if (0 == mOnInfoListenerCount) {
                    LogUtil.vitamio_i(TAG, "av_read_frame rate: " + extra);
                }
                break;
            }

            default:
                break;
        }

        return false;
    }

    /**
     * 处理缓冲开始事件（也可以理解为开始卡顿）
     * <pre>
     *    处理 MediaPlayer.MEDIA_INFO_BUFFERING_START 事件；
     *    所有软解码的视频加载解析阶段都会触发缓存开始事件，反之异常；
     * </pre>
     */
    private void onMediaInfoBufferingStart() {
        if (mSeekToFlag) {
            // [必定: mSeekTime > 0]
            seekToTime(mSeekTime);
        }

        // 标记缓存状态
        switch (mBufferingStatus) {
            case BUFFERING_END:
            case BUFFERING_RESTART:
                mBufferingStatus = BUFFERING_RESTART;
                break;

            case BUFFERING_NONE:
                mBufferingStatus = BUFFERING_START;
                // [有些视频不上报 MediaPlayer. MEDIA_INFO_BUFFERING_END 消息]
                mPlayerHandler.removeMessages(MsgEx.MSG_FIRST_BUFFERING_END_TIMEOUT);
                mPlayerHandler.sendEmptyMessageDelayed(
                        MsgEx.MSG_FIRST_BUFFERING_END_TIMEOUT, 1000);
                break;

            default:
                mBufferingStatus = BUFFERING_START;
                break;
        }

        // 移除缓冲状态上报超时消息
        mPlayerHandler.removeMessages(MsgEx.MSG_BUFFERING_TIMEOUT);
    }

    /**
     * 处理缓冲结束事件（也可以理解为卡顿结束）
     * <pre>
     *    处理 MediaPlayer.MEDIA_INFO_BUFFERING_END 事件；
     *    [注意: 如果有 MEDIA_INFO_BUFFERING_START 消息，没有 MEDIA_INFO_BUFFERING_END 消息，处理缺失]
     * </pre>
     */
    private void onMediaInfoBufferingEnd() {
        mBufferingStatus = BUFFERING_END;

        // [可以开始渲染]避免残留前一个视频的显示帧
        if (!mIsVideoRenderingStart) {
            mIsVideoRenderingStart = true;

            // [ 对于小于 3S 的视频，可能不会自动播放，且无播放结束消息上报
            //   例如测试文件: DD+_CUT.ts 硬解码正常，但是软解码不自动播放 ]
            int totalTime = mAppData.mPlayTimeInfo.mTotalTime;
            boolean littleVideo = (totalTime < LITTLE_VIDEO_DURATION_LIMIT);
            if (littleVideo) {
                // [条件约束: 0 == mSeekTime]
                if (0 == mSeekTime) {
                    // [主要是为了触发播放，所以需要: Seek 200ms]
                    int seekTime = LITTLE_VIDEO_INITIAL_SEEK_TIME;
                    if (totalTime < seekTime) {
                        seekTime = totalTime;
                    }

                    seekToTime(seekTime);
                }
            }
        }

        // 移除缓冲状态上报超时消息
        mPlayerHandler.removeMessages(MsgEx.MSG_BUFFERING_TIMEOUT);
        mPlayerHandler.removeMessages(MsgEx.MSG_FIRST_BUFFERING_END_TIMEOUT);

        // 如已经获取到视频尺寸信息
        if (mVideoSizeChanged) {
            mPlayerHandler.removeMessages(MsgEx.MSG_HIDE_BLACK_PAGE);

            if (mIsPrepared) {
                int delayMillis = 300;
                int totalTime = mAppData.mPlayTimeInfo.mTotalTime;
                boolean littleVideo = (totalTime < LITTLE_VIDEO_DURATION_LIMIT);

                // 对总时长很短的视频，遮罩隐藏时间适当缩短
                if (littleVideo) {
                    delayMillis = (int) (mAppData.mPlayTimeInfo.mTotalTime * 0.1);
                }

                mPlayerHandler.sendEmptyMessageDelayed(
                        MsgEx.MSG_HIDE_BLACK_PAGE, delayMillis);
            } else {
                // [代码覆盖率打印]理论上不应该发生
                LogUtil.vitamio_i(TAG, "MEDIA_INFO_BUFFERING_END: <false == mIsPrepared>.");
            }
        } else {
            // [缓冲结束还未取到视频的大小信息，可以判定无视频数据]
            onSendMessage(IMediaEvent.EVENT_UNSUPPORT_VIDEO_PROMPT_SHOW, null);
        }
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        LogUtil.vitamio_i(TAG, ">>> onPrepared.");

        mIsPrepared = true;
        mAppData.mIsMediaPlayerLocked = false;

        if (isAsyncReleasing()) {
            LogUtil.vitamio_i(TAG, " -- [Reject]onPrepared, isAsyncReleasing!");
            return;
        }

        if (null == mMediaPlayer) {
            LogUtil.vitamio_i(TAG, " -- [Reject]onPrepared, <null == mMediaPlayer>.");
            return;
        }

        // 如果是视频需要调整 Surface 的尺寸
        if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
            updateSurfaceHolderSize(mp);
        }

        // 获取文件后缀
        String strSuffix = "";
        int pos = mFilePath.lastIndexOf('.');
        if (pos != -1) {
            strSuffix = mFilePath.substring(pos) + ".";
        }

        mSeekTime = 0;
        mSeekToFlag = false;
        int duration = (int) mMediaPlayer.getDuration();
        mAppData.mPlayTimeInfo.mTotalTime = duration;
        LogUtil.vitamio_i(TAG, "  -- duration: " + duration);

        if (!TextUtils.isEmpty(mFilePath) &&
                IMusicState.MEDIA_TYPE_IDLE != mAppData.mMediaType) {
            int nTime = mLocalzModel.readMediaTime(mAppData.mMediaType, mFilePath);

            if (nTime > 0) {
                mLocalzModel.writeMediaTime(
                        mAppData.mMediaType, mFilePath, 0, 111);

                // [特殊格式需要在缓冲开始后跳转，具体原因?]
                String suffix = strSuffix.toLowerCase(Locale.getDefault());
                if (SPEC_VIDEO_SUFFIX.contains(suffix)) {
                    LogUtil.vitamio_i(TAG, " -- SPEC_VIDEO_SUFFIX: " + mFilePath);

                    mSeekToFlag = true;
                    mSeekTime = nTime;
                } else {
                    seekToTime(nTime);
                }
            } else if (-1 == nTime) {
                mLocalzModel.writeMediaTime(
                        mAppData.mMediaType, mFilePath, 0, 112);
            }
        }

        // [检查状态: 比较乱，需要梳理原因]
        if (IMusicState.MEDIA_TYPE_IDLE == mAppData.mMediaType) {
            mAppData.mAllowResumePlay = true;
        } else if (mLocalzModel.existsHighPriorityEvent()) {
            mAppData.mAllowResumePlay = true;
            LogUtil.vitamio_i(TAG, " -- existsHighPriorityEvent: true.");
        } else if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType
                && !mLocalzModel.isCanPlayVideo()) {
            mAppData.mAllowResumePlay = true;
            LogUtil.vitamio_i(TAG, " -- isCanPlayVideo: false.");
        } else {
            onPlayControlEvent(IMusicState.PLAY_CMD_PLAY);
        }
    }

    @Override
    public void onSeekComplete(MediaPlayer mp) {
        LogUtil.vitamio_i(TAG, ">>> onSeekComplete...");

        if (mMediaPlayer != null && mIsPrepared) {
            // [MediaPlayer.seekTo() 是异步方法]
            mAppData.mPlayTimeInfo.setCurrentTime(
                    getCurrentPosition(), true, "vitamio");
            onSendMessage(IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME, mMediaInfo);

            // [拖动完成: 存储记忆]
            mPlayerHandler.removeMessages(MsgEx.MSG_WRITE_MEDIA_TIME);
            mPlayerHandler.sendEmptyMessageDelayed(
                    MsgEx.MSG_WRITE_MEDIA_TIME, 1000);

            // [拖动完成: 通知播放界面，是否触发播放]
            onSendMessage(IMediaEvent.EVENT_SEEK_TO_COMPLETE, mMediaInfo);

            // [对播放时长小的视频特殊处理]
            checkLittleVideoPlayTimeout(true);
        }
    }

    // [检查小视频播放超时]
    private void checkLittleVideoPlayTimeout(boolean checkEnable) {
        if (IMusicState.MEDIA_TYPE_VIDEO != mAppData.mMediaType) {
            return;
        }

        if (checkEnable) {
            int totalTime = mAppData.mPlayTimeInfo.mTotalTime;
            int currentTime = mAppData.mPlayTimeInfo.mCurrentTime;
            int timeDelta = totalTime - currentTime;
            boolean littleVideo = (totalTime < LITTLE_VIDEO_DURATION_LIMIT);
            boolean playTimeValid = (currentTime >= 0 && timeDelta > 0);

            // 小视频第一次拖动定位后, 不一定会触发播放, 导致不主动跳曲
            // 例如: 视频在后台播放的情况, 没有设置 Surface, 就有可能不触发播放。
            if (littleVideo && playTimeValid && mMediaPlayer.isPlaying()) {
                LogUtil.vitamio_i(TAG, ">>> checkLittleVideoPlayTimeout...");

                // 超时总时长 2S 以上, 判断播放结束。
                mPlayerHandler.removeMessages(MsgEx.MSG_LITTLE_VIDEO_PLAY_TIMEOUT);
                mPlayerHandler.sendEmptyMessageDelayed(
                        MsgEx.MSG_LITTLE_VIDEO_PLAY_TIMEOUT, timeDelta + 2000);
            }
        } else {
            mPlayerHandler.removeMessages(MsgEx.MSG_LITTLE_VIDEO_PLAY_TIMEOUT);
        }
    }

    @Override
    public void onTimedText(String text) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onTimedTextUpdate(byte[] pixels, int width, int height) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onFailed() {
        LogUtil.e(TAG, ">>> HWRenderFailed...");
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

    @Override
    public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
        // 过滤打印
        if (mVideoWidth != width || mVideoHeight != height) {
            LogUtil.vitamio_i(TAG,
                    "VitamioPlayer_OnVideoSizeChangedListener, width: " + width + " height: "
                            + height);
        }

        // [有些视频播放或者拖动到最后几秒, 不再继续播放, 而是返回 0 x 0,
        //  导致 SeekTo 播放时候不跳曲, 我们可以通过下列条件让其跳下一曲.]
        // 例如: HCN-DQA-Video-Test-Resource <73-65560-1920-1080.m2ts>
        if (mVideoSizeChanged && mBufferingStatus != BUFFERING_NONE) {
            if (mVideoWidth != 0 && mVideoHeight != 0) {
                mPlayerHandler.removeMessages(MsgEx.MSG_VIDEO_SIZE_VALID_2_INVALID);

                // SeekTo 后可能视频尺寸由之前播放时的有效值变成无效值(0x0)
                if (0 == width && 0 == height) {
                    // 发现直接执行 release() 会 ANR。
                    mPlayerHandler.sendEmptyMessageDelayed(
                            MsgEx.MSG_VIDEO_SIZE_VALID_2_INVALID, 2000);
                    return;
                } else {
                    // 预留一个 2000ms 等待时间, 有些视频可以恢复。
                }
            }
        }

        // [规律]如果软解的视频不进入这个回调，说明: 当前文件显示帧无法解析
        mVideoSizeChanged = true;

        // 检查视频缓冲状态
        switch (mBufferingStatus) {
            case BUFFERING_NONE: {
                // [没有上报缓冲状态信息吗，确实遇到了这样的情况]
                if (mIsPrepared) {
                    LogUtil.vitamio_i(TAG,
                            "VitamioPlayer_OnVideoSizeChangedListener, BUFFERING_NONE!");

                    // 有视频 size 信息, 移除 MSG_FIRST_BUFFERING_END_TIMEOUT 消息
                    mPlayerHandler.removeMessages(MsgEx.MSG_FIRST_BUFFERING_END_TIMEOUT);

                    // [可能不上报 MediaPlayer.MEDIA_INFO_BUFFERING_END]
                    mPlayerHandler.removeMessages(MsgEx.MSG_BUFFERING_TIMEOUT);
                    mPlayerHandler.sendEmptyMessageDelayed(
                            MsgEx.MSG_BUFFERING_TIMEOUT, 1000);
                }
                break;
            }

            case BUFFERING_START:
            case BUFFERING_RESTART:
            case BUFFERING_END:
            default: {
                // [通知 UI，这个视频文件是存在视频数据显示帧的]
                onSendMessage(IMediaEvent.EVENT_UNSUPPORT_VIDEO_PROMPT_HIDE, null);
                break;
            }
        }

        updateSurfaceHolderSize(mp);
    }

    private void computeAndUpdateVideoSize() {
        // 无效状态不计算
        if (null == mMediaPlayer || !mIsPrepared) {
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
                    IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_SIZE, mVideoWidth, mVideoHeight);
        } else {
            // 没有视频图像[暂时还没遇到这样的文件返回(0, 0), 遇到类似场景后再处理]
        }
    }
}
