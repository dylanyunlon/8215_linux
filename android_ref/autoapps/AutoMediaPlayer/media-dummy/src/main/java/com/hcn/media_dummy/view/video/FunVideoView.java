package com.hcn.media_dummy.view.video;

import static com.hcn.media_dummy.utils.CommonUtil.getTextSpeed;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.RectF;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.ConnectivityManager;
import android.os.Handler;
import android.os.Looper;
import android.util.AttributeSet;
import android.view.InflateException;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;

import androidx.annotation.AttrRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.common.misc.LogUtils;
import com.hcn.media_dummy.R;
import com.hcn.media_dummy.listener.FunMediaPlayerListener;
import com.hcn.media_dummy.view.base.FunMediaState;
import com.hcn.media_dummy.listener.VideoAllCallBack;
import com.hcn.media_dummy.utils.CommonUtil;
import com.hcn.media_dummy.utils.NetInfoModule;
import com.hcn.media_dummy.base.FunMediaViewBridge;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

/**
 * 视频显示视图
 * <p> 视频状态回调处理等相关层；
 * @author 65821
 */
public abstract class FunVideoView extends FunTextureRenderView
        implements FunMediaPlayerListener, FunMediaState {

    /** 避免切换时频繁 setup */
    public static final int CHANGE_DELAY_TIME = 2000;

    /** 当前的播放状态 */
    protected int mCurrentState = -1;

    /** 播放的 tag，防止错误，因为普通的 url 也可能重复  */
    protected int mPlayPosition = -22;

    /** 屏幕宽度  */
    protected int mScreenWidth;

    /** 屏幕高度 */
    protected int mScreenHeight;

    /** 缓存进度 */
    protected int mBufferPoint;

    /** 备份缓存前的播放状态 */
    protected int mBackUpPlayingBufferState = -1;

    /** 从哪个开始播放 */
    protected long mSeekOnStart = -1;

    /** 当前的播放位置 */
    protected long mCurrentPosition;

    /** 保存切换时的时间，避免频繁契合 */
    protected long mSaveChangeViewTIme = 0;

    /** 播放速度 */
    protected float mSpeed = 1;

    /** 是否播边边缓冲 */
    protected boolean mCache = false;

    /** 当前是否全屏 */
    protected boolean mIfCurrentIsFullscreen = false;

    /** 循环 */
    protected boolean mLooping = false;

    /** 是否播放过 */
    protected boolean mHadPlay = false;

    /** 是否发送了网络改变 */
    protected boolean mNetChanged = false;

    /** 是否不变调 */
    protected boolean mSoundTouch = false;

    /** 是否需要显示暂停锁定效果 */
    protected boolean mShowPauseCover = false;

    /** 是否准备完成前调用了暂停 */
    protected boolean mPauseBeforePrepared = false;

    /** Prepared 之后是否自动开始播放 */
    protected boolean mStartAfterPrepared = true;

    /** Prepared */
    protected boolean mHadPrepared = false;

    /** 是否播放器当失去音频焦点 */
    protected boolean mReleaseWhenLossAudio = true;

    /** 音频焦点的监听 */
    protected AudioManager mAudioManager;

    /** 播放的 tag，防止错误，因为普通的 url 也可能重复 */
    protected String mPlayTag = "";

    /** 上下文 */
    protected Context mContext;

    /** 原来的 url */
    protected String mOriginUrl;

    /** 转化后的 URL */
    protected String mUrl;

    /** 标题 */
    protected String mTitle;

    /** 网络状态 */
    protected String mNetSate = "NORMAL";

    /** 是否需要覆盖拓展类型 */
    protected String mOverrideExtension;

    /** 缓存路径，可不设置 */
    protected File mCachePath;

    /** 视频回调 */
    protected VideoAllCallBack mVideoAllCallBack;

    /** http request header */
    protected Map<String, String> mMapHeadData = new HashMap<>();

    /** 网络监听 */
    protected NetInfoModule mNetInfoModule;

    public FunVideoView(@NonNull Context context) {
        super(context);
        init(context);
    }

    public FunVideoView(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public FunVideoView(@NonNull Context context, @Nullable AttributeSet attrs, @AttrRes int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context);
    }

    public FunVideoView(Context context, Boolean fullFlag) {
        super(context);
        mIfCurrentIsFullscreen = fullFlag;
        init(context);
    }

    @Override
    protected void showPauseCover() {
        if (mCurrentState == CURRENT_STATE_PAUSE
                && mFullPauseBitmap != null
                && !mFullPauseBitmap.isRecycled()
                && mShowPauseCover
                && mSurface != null
                && mSurface.isValid()) {
            if (getFunMediaManager().isSurfaceSupportLockCanvas()) {
                try {
                    RectF rectF = new RectF(0, 0,
                            mTextureView.getWidth(), mTextureView.getHeight());
                    Canvas canvas;
                    canvas = mSurface.lockCanvas(new Rect(
                            0, 0, mTextureView.getWidth(), mTextureView.getHeight()));
                    if (canvas != null) {
                        canvas.drawBitmap(mFullPauseBitmap, null, rectF, null);
                        mSurface.unlockCanvasAndPost(canvas);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    @Override
    protected void releasePauseCover() {
        try {
            if (mCurrentState != CURRENT_STATE_PAUSE
                    && mFullPauseBitmap != null
                    && !mFullPauseBitmap.isRecycled()
                    && mShowPauseCover) {
                mFullPauseBitmap.recycle();
                mFullPauseBitmap = null;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public int getCurrentVideoWidth() {
        if (getFunMediaManager() != null) {
            return getFunMediaManager().getVideoWidth();
        }
        return 0;
    }

    @Override
    public int getCurrentVideoHeight() {
        if (getFunMediaManager() != null) {
            return getFunMediaManager().getVideoHeight();
        }
        return 0;
    }

    protected void updatePauseCover() {
        if ((mFullPauseBitmap == null || mFullPauseBitmap.isRecycled()) && mShowPauseCover) {
            try {
                initCover();
            } catch (Exception e) {
                e.printStackTrace();
                mFullPauseBitmap = null;
            }
        }
    }

    protected Context getActivityContext() {
        return CommonUtil.getActivityContext(getContext());
    }

    protected void init(Context context) {
        if (getActivityContext() != null) {
            this.mContext = getActivityContext();
        } else {
            this.mContext = context;
        }

        initInflate(mContext);
        mTextureViewContainer = (ViewGroup) findViewById(R.id.surface_container);
        if (isInEditMode()) {
            return;
        }

        mScreenWidth = mContext.getResources()
                .getDisplayMetrics().widthPixels;
        mScreenHeight = mContext.getResources()
                .getDisplayMetrics().heightPixels;
        mAudioManager = (AudioManager) mContext.getApplicationContext()
                .getSystemService(Context.AUDIO_SERVICE);
    }

    protected void initInflate(Context context) {
        try {
            View.inflate(context, getLayoutId(), this);
        } catch (InflateException e) {
            if (e.toString().contains("FunImageCover")) {
                LogUtils.w("********************\n" +
                        "*****   注意   *****" +
                        "********************\n" +
                        "*该版本需要清除布局文件中的 FunImageCover\n" +
                        "****  Attention  ***\n" +
                        "*Please remove FunImageCover from Layout in this Version\n" +
                        "********************\n");
                e.printStackTrace();
                throw new InflateException("该版本需要清除布局文件中的 " +
                        "FunImageCover, please remove FunImageCover from your layout.");
            } else {
                e.printStackTrace();
            }
        }
    }

    /**
     * 开始播放逻辑
     * <p>
     */
    protected void startButtonLogic() {
        if (mVideoAllCallBack != null
                && (mCurrentState == CURRENT_STATE_NORMAL
                    || mCurrentState == CURRENT_STATE_AUTO_COMPLETE)) {
            LogUtils.v("onClickStartIcon");
            mVideoAllCallBack.onClickStartIcon(mOriginUrl, mTitle, this);
        } else if (mVideoAllCallBack != null) {
            LogUtils.v("onClickStartError");
            mVideoAllCallBack.onClickStartError(mOriginUrl, mTitle, this);
        }

        prepareVideo();
    }

    /**
     * 开始状态视频播放
     */
    protected void prepareVideo() {
        startPrepare();
    }

    protected void startPrepare() {
        if (getFunMediaManager().listener() != null) {
            getFunMediaManager().listener().onCompletion();
        }

        if (mVideoAllCallBack != null) {
            LogUtils.v("onStartPrepared");
            mVideoAllCallBack.onStartPrepared(mOriginUrl, mTitle, this);
        }

        getFunMediaManager().setListener(this);
        getFunMediaManager().setPlayTag(mPlayTag);
        getFunMediaManager().setPlayPosition(mPlayPosition);

        mAudioManager.requestAudioFocus(
                onAudioFocusChangeListener,
                AudioManager.STREAM_MUSIC,
                AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);

        try {
            if (mContext instanceof Activity) {
                ((Activity) mContext).getWindow().addFlags(
                        WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        mBackUpPlayingBufferState = -1;
        getFunMediaManager().prepare(mUrl,
                (mMapHeadData == null) ? new HashMap<>() : mMapHeadData,
                mLooping, mSpeed, mCache, mCachePath, mOverrideExtension);
        setStateAndUi(CURRENT_STATE_PREPAREING);
    }

    /**
     * 监听是否有外部其他多媒体开始播放
     */
    protected AudioManager.OnAudioFocusChangeListener
            onAudioFocusChangeListener = focusChange -> {
        switch (focusChange) {
            case AudioManager.AUDIOFOCUS_GAIN:
                onGainAudio();
                break;
            case AudioManager.AUDIOFOCUS_LOSS:
                onLossAudio();
                break;
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                onLossTransientAudio();
                break;
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                onLossTransientCanDuck();
                break;
            default:
                break;
        }
    };

    /**
     * 获得了 Audio Focus
     */
    protected void onGainAudio() {
    }

    /**
     * 失去了 Audio Focus，并将会持续很长的时间
     */
    protected void onLossAudio() {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                if (FunVideoView.this.mReleaseWhenLossAudio) {
                    FunVideoView.this.releaseVideos();
                } else {
                    getFunMediaManager().listener().onMediaPause();
                }
            }
        });
    }

    /**
     * 暂时失去 Audio Focus，并会很快再次获得
     */
    protected void onLossTransientAudio() {
        try {
            getFunMediaManager().listener().onMediaPause();
        } catch (Exception var2) {
            var2.printStackTrace();
        }
    }

    /**
     * 暂时失去AudioFocus，但是可以继续播放，不过要在降低音量
     */
    protected void onLossTransientCanDuck() {
    }

    /**
     * 设置播放URL
     *
     * @param url           播放url
     * @param cacheWithPlay 是否边播边缓存
     * @param title         title
     * @return
     */
    public boolean setUp(String url, boolean cacheWithPlay, String title) {
        return setUp(url, cacheWithPlay, ((File) null), title);
    }

    /**
     * 设置播放 URL
     *
     * @param url 播放 url
     * @param cacheWithPlay 是否边播边缓存
     * @param cachePath 缓存路径，如果是 M3U8 或者 HLS，请设置为 false
     * @param mapHeadData 头部信息
     * @param title title
     * @return 成功/失败
     */
    public boolean setUp(String url,
                         boolean cacheWithPlay,
                         File cachePath,
                         Map<String, String> mapHeadData,
                         String title) {
        if (setUp(url, cacheWithPlay, cachePath, title)) {
            if (this.mMapHeadData != null) {
                this.mMapHeadData.clear();
            } else {
                this.mMapHeadData = new HashMap<>();
            }

            if (mapHeadData != null) {
                this.mMapHeadData.putAll(mapHeadData);
            }

            return true;
        }

        return false;
    }

    /**
     * 设置播放 URL
     *
     * @param url 播放 url
     * @param cacheWithPlay 是否边播边缓存
     * @param cachePath 缓存路径，如果是 M3U8 或者 HLS，请设置为 false
     * @param title title
     * @return 成功/失败
     */
    public boolean setUp(String url,
                         boolean cacheWithPlay,
                         File cachePath,
                         String title) {
        return setUp(url, cacheWithPlay, cachePath, title, true);
    }

    /**
     * 设置播放 URL
     *
     * @param url  播放url
     * @param cacheWithPlay 是否边播边缓存
     * @param cachePath 缓存路径，如果是 M3U8 或者 HLS，请设置为 false
     * @param title title
     * @param changeState 是否修改状态
     * @return 成功/失败
     */
    protected boolean setUp(String url,
                            boolean cacheWithPlay,
                            File cachePath,
                            String title,
                            boolean changeState) {
        mCache = cacheWithPlay;
        mCachePath = cachePath;
        mOriginUrl = url;
        if (isCurrentMediaListener() &&
                (System.currentTimeMillis() - mSaveChangeViewTIme) < CHANGE_DELAY_TIME) {
            return false;
        }

        mCurrentState = CURRENT_STATE_NORMAL;
        this.mUrl = url;
        this.mTitle = title;
        if (changeState) {
            setStateAndUi(CURRENT_STATE_NORMAL);
        }

        return true;
    }

    /**
     * 重置
     */
    public void onVideoReset() {
        setStateAndUi(CURRENT_STATE_NORMAL);
    }

    /**
     * 暂停状态
     */
    @Override
    public void onMediaPause() {
        if (mCurrentState == CURRENT_STATE_PREPAREING) {
            mPauseBeforePrepared = true;
        }

        try {
            if (getFunMediaManager() != null &&
                    getFunMediaManager().isPlaying()) {
                setStateAndUi(CURRENT_STATE_PAUSE);
                mCurrentPosition = getFunMediaManager().getCurrentPosition();
                if (getFunMediaManager() != null) {
                    getFunMediaManager().pause();
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 恢复暂停状态
     */
    @Override
    public void onMediaResume() {
        onMediaResume(true);
    }

    /**
     * 恢复暂停状态
     * @param seek 是否产生seek动作
     */
    @Override
    public void onMediaResume(boolean seek) {
        mPauseBeforePrepared = false;
        if (mCurrentState == CURRENT_STATE_PAUSE) {
            try {
                if (mCurrentPosition >= 0 && getFunMediaManager() != null) {
                    if (seek) {
                        getFunMediaManager().seekTo(mCurrentPosition);
                    }

                    getFunMediaManager().start();
                    setStateAndUi(CURRENT_STATE_PLAYING);
                    if (mAudioManager != null && !mReleaseWhenLossAudio) {
                        mAudioManager.requestAudioFocus(
                                onAudioFocusChangeListener,
                                AudioManager.STREAM_MUSIC,
                                AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
                    }

                    mCurrentPosition = 0;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 处理因切换网络而导致的问题
     */
    protected void netWorkErrorLogic() {
        final long currentPosition = getCurrentPositionWhenPlaying();
        LogUtils.w("******* Net State Changed. renew player to connect ******* " + currentPosition);

        getFunMediaManager().releaseMediaPlayer();
        postDelayed(() -> {
            setSeekOnStart(currentPosition);
            startPlayLogic();
        }, 500);
    }

    /**
     * 播放错误的时候，删除缓存文件
     */
    protected void deleteCacheFileWhenError() {
        clearCurrentCache();
        LogUtils.w("Link Or mCache Error, Please Try Again " + mOriginUrl);
        if (mCache) {
            LogUtils.w("mCache Link：" + mUrl);
        }
        mUrl = mOriginUrl;
    }

    @Override
    public void onPrepared() {
        if (mCurrentState != CURRENT_STATE_PREPAREING) {
            return;
        }

        mHadPrepared = true;

        if (mVideoAllCallBack != null && isCurrentMediaListener()) {
            LogUtils.v("onPrepared");
            mVideoAllCallBack.onPrepared(mOriginUrl, mTitle, this);
        }

        if (!mStartAfterPrepared) {
            setStateAndUi(CURRENT_STATE_PAUSE);
            onMediaPause();
            return;
        }

        startAfterPrepared();
    }

    @Override
    public void onAutoCompletion() {
        setStateAndUi(CURRENT_STATE_AUTO_COMPLETE);
        mSaveChangeViewTIme = 0;
        mCurrentPosition = 0;

        if (mTextureViewContainer.getChildCount() > 0) {
            mTextureViewContainer.removeAllViews();
        }

        if (!mIfCurrentIsFullscreen) {
            getFunMediaManager().setLastListener(null);
        }

        mAudioManager.abandonAudioFocus(onAudioFocusChangeListener);
        if (mContext instanceof Activity) {
            try {
                ((Activity) mContext).getWindow().clearFlags(
                        WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        releaseNetWorkState();
        if (mVideoAllCallBack != null && isCurrentMediaListener()) {
            LogUtils.v("onAutoComplete");
            mVideoAllCallBack.onAutoComplete(mOriginUrl, mTitle, this);
        }

        mHadPlay = false;
    }

    @Override
    public void onCompletion() {
        //make me normal first
        setStateAndUi(CURRENT_STATE_NORMAL);

        mSaveChangeViewTIme = 0;
        mCurrentPosition = 0;

        if (mTextureViewContainer.getChildCount() > 0) {
            mTextureViewContainer.removeAllViews();
        }

        if (!mIfCurrentIsFullscreen) {
            getFunMediaManager().setListener(null);
            getFunMediaManager().setLastListener(null);
        }

        getFunMediaManager().setCurrentVideoHeight(0);
        getFunMediaManager().setCurrentVideoWidth(0);

        mAudioManager.abandonAudioFocus(onAudioFocusChangeListener);
        if (mContext instanceof Activity) {
            try {
                ((Activity) mContext).getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        releaseNetWorkState();

        if (mVideoAllCallBack != null) {
            LogUtils.v("onComplete");
            mVideoAllCallBack.onComplete(mOriginUrl, mTitle, this);
        }

        mHadPlay = false;
    }

    @Override
    public void onSeekComplete() {
        LogUtils.v("onSeekComplete");
    }

    @Override
    public void onError(int what, int extra) {
        if (mNetChanged) {
            mNetChanged = false;
            netWorkErrorLogic();
            if (mVideoAllCallBack != null) {
                mVideoAllCallBack.onPlayError(mOriginUrl, mTitle, this);
            }

            return;
        }

        if (what != 38 && what != -38) {
            setStateAndUi(CURRENT_STATE_ERROR);
            deleteCacheFileWhenError();
            if (mVideoAllCallBack != null) {
                mVideoAllCallBack.onPlayError(mOriginUrl, mTitle, this, what, extra);
            }
        }
    }

    @Override
    public void onInfo(int what, int extra) {
        switch (what) {
            case MediaPlayer.MEDIA_INFO_BUFFERING_START: {
                mBackUpPlayingBufferState = mCurrentState;
                // 避免在 onPrepared 之前就进入了 buffering，导致一直 loading
                if (mHadPlay &&
                        mCurrentState > 0 &&
                        mCurrentState != CURRENT_STATE_PREPAREING) {
                    setStateAndUi(CURRENT_STATE_PLAYING_BUFFERING_START);
                }
                break;
            }
            case MediaPlayer.MEDIA_INFO_BUFFERING_END: {
                if (mBackUpPlayingBufferState != -1) {
                    if (mBackUpPlayingBufferState == CURRENT_STATE_PLAYING_BUFFERING_START) {
                        mBackUpPlayingBufferState = CURRENT_STATE_PLAYING;
                    }
                    if (mHadPlay &&
                            mCurrentState > 0 &&
                            mCurrentState != CURRENT_STATE_PREPAREING) {
                        setStateAndUi(mBackUpPlayingBufferState);
                    }
                    mBackUpPlayingBufferState = -1;
                }
                break;
            }
            default: {
                if (what == getFunMediaManager().getRotateInfoFlag()) {
                    mRotate = extra;
                    LogUtils.d("Video Rotate Info: " + extra);

                    if (mTextureView != null) {
                        mTextureView.setRotation(mRotate);
                    }
                }
                break;
            }
        }
    }

    @Override
    public void onVideoSizeChanged() {
        int mVideoWidth = getFunMediaManager().getCurrentVideoWidth();
        int mVideoHeight = getFunMediaManager().getCurrentVideoHeight();
        if (mVideoWidth != 0 && mVideoHeight != 0 && mTextureView != null) {
            mTextureView.requestLayout();
        }
    }

    @Override
    protected void setDisplay(Surface surface) {
        getFunMediaManager().setDisplay(surface);
    }

    @Override
    protected void releaseSurface(Surface surface) {
        getFunMediaManager().releaseSurface(surface);
    }

    /**
     * 清除当前缓存
     */
    public void clearCurrentCache() {
        if (getFunMediaManager().isCacheFile() && mCache) {
            // 是否为缓存文件
            LogUtils.d("Play Error: " + mUrl);
            mUrl = mOriginUrl;
            getFunMediaManager().clearCache(mContext, mCachePath, mOriginUrl);
        } else if (mUrl != null && mUrl.contains("127.0.0.1")) {
            getFunMediaManager().clearCache(getContext(), mCachePath, mOriginUrl);
        }
    }

    /**
     * 获取当前播放进度
     * <p> 非播放状态返回 0，播放状态直接找播放内核要；
     */
    public long getCurrentPositionWhenPlaying() {
        long position = 0;
        if (mCurrentState == CURRENT_STATE_PLAYING
                || mCurrentState == CURRENT_STATE_PAUSE) {
            try {
                position = getFunMediaManager().getCurrentPosition();
            } catch (Exception e) {
                e.printStackTrace();
                return position;
            }
        }
        if (position == 0 && mCurrentPosition > 0) {
            return mCurrentPosition;
        }
        return position;
    }

    /**
     * 获取当前总时长
     */
    public long getDuration() {
        long duration = 0;
        try {
            duration = getFunMediaManager().getDuration();
        } catch (Exception e) {
            e.printStackTrace();
            return duration;
        }
        return duration;
    }

    /**
     * 释放
     */
    public void release() {
        mSaveChangeViewTIme = 0;
        if (isCurrentMediaListener() &&
                (System.currentTimeMillis() - mSaveChangeViewTIme) > CHANGE_DELAY_TIME) {
            releaseVideos();
        }
    }

    /**
     * prepared 成功之后会开始播放
     */
    public void startAfterPrepared() {
        if (!mHadPrepared) {
            prepareVideo();
        }

        try {
            if (getFunMediaManager() != null) {
                getFunMediaManager().start();
            }

            setStateAndUi(CURRENT_STATE_PLAYING);
            if (getFunMediaManager() != null && mSeekOnStart > 0) {
                getFunMediaManager().seekTo(mSeekOnStart);
                mSeekOnStart = 0;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        addTextureView();
        createNetWorkState();
        listenerNetWorkState();

        mHadPlay = true;
        if (mTextureView != null) {
            mTextureView.onResume();
        }

        if (mPauseBeforePrepared) {
            onMediaPause();
            mPauseBeforePrepared = false;
        }
    }

    protected boolean isCurrentMediaListener() {
        return getFunMediaManager().listener() != null
                && getFunMediaManager().listener() == this;
    }

    /**
     * 创建网络监听
     * <p> {@link ConnectivityManager#CONNECTIVITY_ACTION}
     */
    protected void createNetWorkState() {
        if (mNetInfoModule == null) {
            mNetInfoModule = new NetInfoModule(
                    mContext.getApplicationContext(),
                    state -> {
                        if (!mNetSate.equals(state)) {
                            LogUtils.v("******* change network state ******* " + state);
                            mNetChanged = true;
                        }
                        mNetSate = state;
                    });
            mNetSate = mNetInfoModule.getCurrentConnectionType();
        }
    }

    /**
     * 监听网络状态
     */
    protected void listenerNetWorkState() {
        if (mNetInfoModule != null) {
            mNetInfoModule.onHostResume();
        }
    }

    /**
     * 取消网络监听
     */
    protected void unListenerNetWorkState() {
        if (mNetInfoModule != null) {
            mNetInfoModule.onHostPause();
        }
    }

    /**
     * 释放网络监听
     */
    protected void releaseNetWorkState() {
        if (mNetInfoModule != null) {
            mNetInfoModule.onHostPause();
            mNetInfoModule = null;
        }
    }

    /************************* 需要继承处理部分 *************************/

    /**
     * 退出全屏
     *
     * @return 是否在全屏界面
     */
    protected abstract boolean backFromFull(Context context);

    /**
     * 释放播放器
     */
    protected abstract void releaseVideos();

    /**
     * 设置播放显示状态
     *
     * @param state 状态（ui 显示）
     */
    protected abstract void setStateAndUi(int state);

    /**
     * 获取管理器桥接的实现
     * <p> 播放器引擎和播放器视图之间的桥梁；
     */
    public abstract FunMediaViewBridge getFunMediaManager();

    /**
     * 当前 UI 布局资源
     * <p> 返回当前播放器视图绑定的资源布局；
     */
    public abstract int getLayoutId();

    /**
     * 开始播放逻辑
     * <p> 主要是初始化播放参数，并调用 prepareVideo 接口；
     */
    public abstract void startPlayLogic();

    /************************* 公开接口 *************************/

    /**
     * 获取当前播放状态
     * <p> {@link FunMediaState}
     */
    public int getCurrentState() {
        return mCurrentState;
    }

    /**
     * 根据状态判断是否播放中
     * <p> 我们认为在播放初始化、缓存中、播放中、暂停状态都认为是在播放中；
     */
    public boolean isInPlayingState() {
        return (mCurrentState >= 0
                && mCurrentState != CURRENT_STATE_NORMAL
                && mCurrentState != CURRENT_STATE_AUTO_COMPLETE
                && mCurrentState != CURRENT_STATE_ERROR);
    }

    /**
     * 播放tag防止错误，因为普通的url也可能重复
     */
    public String getPlayTag() {
        return mPlayTag;
    }

    /**
     * 播放 tag 防止错误，因为普通的 url 也可能重复
     *
     * @param playTag 保证不重复就好
     */
    public void setPlayTag(String playTag) {
        this.mPlayTag = playTag;
    }


    public int getPlayPosition() {
        return mPlayPosition;
    }

    /**
     * 设置播放位置防止错位
     */
    public void setPlayPosition(int playPosition) {
        this.mPlayPosition = playPosition;
    }

    /**
     * 网络速度
     * 注意，这里如果是开启了缓存，因为读取本地代理，缓存成功后还是存在速度的
     * 再打开已经缓存的本地文件，网络速度才会回 0. 因为是播放本地文件了
     */
    public long getNetSpeed() {
        return getFunMediaManager().getNetSpeed();
    }

    /**
     * 网络速度
     * 注意，这里如果是开启了缓存，因为读取本地代理，缓存成功后还是存在速度的
     * 再打开已经缓存的本地文件，网络速度才会回0.因为是播放本地文件了
     */
    public String getNetSpeedText() {
        long speed = getNetSpeed();
        return getTextSpeed(speed);
    }

    public long getSeekOnStart() {
        return mSeekOnStart;
    }

    /**
     * 从哪里开始播放
     * 目前有时候前几秒有跳动问题，毫秒
     * 需要在 startPlayLogic 之前，即播放开始之前
     */
    public void setSeekOnStart(long seekOnStart) {
        this.mSeekOnStart = seekOnStart;
    }

    /**
     * 缓冲进度/缓存进度
     */
    public int getBufferPoint() {
        return mBufferPoint;
    }

    /**
     * 是否全屏
     */
    public boolean isIfCurrentIsFullscreen() {
        return mIfCurrentIsFullscreen;
    }

    public void setIfCurrentIsFullscreen(boolean ifCurrentIsFullscreen) {
        this.mIfCurrentIsFullscreen = ifCurrentIsFullscreen;
    }

    public boolean isLooping() {
        return mLooping;
    }

    /**
     * 设置循环
     * @param looping
     */
    public void setLooping(boolean looping) {
        this.mLooping = looping;
    }

    /**
     * 设置播放过程中的回调
     *
     * @param mVideoAllCallBack
     */
    public void setVideoAllCallBack(VideoAllCallBack mVideoAllCallBack) {
        this.mVideoAllCallBack = mVideoAllCallBack;
    }

    public float getSpeed() {
        return mSpeed;
    }

    /**
     * 播放速度
     * @param speed 速度
     */
    public void setSpeed(float speed) {
        setSpeed(speed, false);
    }

    /**
     * 播放速度
     *
     * @param speed 速度
     * @param soundTouch 是否对 6.0 下开启变速不变调
     */
    public void setSpeed(float speed, boolean soundTouch) {
        this.mSpeed = speed;
        this.mSoundTouch = soundTouch;
        if (getFunMediaManager() != null) {
            getFunMediaManager().setSpeed(speed, soundTouch);
        }
    }

    /**
     * 播放中生效的播放数据
     *
     * @param speed
     * @param soundTouch
     */
    public void setSpeedPlaying(float speed, boolean soundTouch) {
        setSpeed(speed, soundTouch);
        getFunMediaManager().setSpeedPlaying(speed, soundTouch);
    }

    public boolean isShowPauseCover() {
        return mShowPauseCover;
    }

    /**
     * 是否需要加载显示暂停的 cover 图片
     * <pre>
     *    打开状态下，暂停退到后台，再回到前台不会显示黑屏，但可以对某些机型有概率出现 OOM
     *    关闭情况下，暂停退到后台，再回到前台显示黑屏
     *    目前某些特定情况可能会出现切换视频时黑屏：
     * </pre>
     *
     * @param showPauseCover 默认 true
     */
    public void setShowPauseCover(boolean showPauseCover) {
        this.mShowPauseCover = showPauseCover;
    }

    /**
     * 找到你想要的位置
     * @param position 位置
     */
    public void seekTo(long position) {
        try {
            if (getFunMediaManager() != null && position > 0) {
                getFunMediaManager().seekTo(position);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public boolean isStartAfterPrepared() {
        return mStartAfterPrepared;
    }

    /**
     * 准备成功之后立即播放
     *
     * @param startAfterPrepared 默认true，false的时候需要在prepared后调用startAfterPrepared()
     */
    public void setStartAfterPrepared(boolean startAfterPrepared) {
        this.mStartAfterPrepared = startAfterPrepared;
    }

    public boolean isReleaseWhenLossAudio() {
        return mReleaseWhenLossAudio;
    }

    /**
     * 长时间失去音频焦点，暂停播放器
     *
     * @param releaseWhenLossAudio 默认true，false的时候只会暂停
     */
    public void setReleaseWhenLossAudio(boolean releaseWhenLossAudio) {
        this.mReleaseWhenLossAudio = releaseWhenLossAudio;
    }

    public Map<String, String> getMapHeadData() {
        return mMapHeadData;
    }

    /**
     * 单独设置 mapHeader
     *
     * @param headData
     */
    public void setMapHeadData(Map<String, String> headData) {
        if (headData != null) {
            this.mMapHeadData = headData;
        }
    }

    public String getOverrideExtension() {
        return mOverrideExtension;
    }

    /**
     * 是否需要覆盖拓展类型，目前只针对exoPlayer内核模式有效
     *
     * @param overrideExtension 比如传入 m3u8,mp4,avi 等类型
     */
    public void setOverrideExtension(String overrideExtension) {
        this.mOverrideExtension = overrideExtension;
    }
}
