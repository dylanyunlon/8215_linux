package com.hcn.media_dummy.view.video;

import static com.hcn.media_dummy.utils.CommonUtil.getActionBarHeight;
import static com.hcn.media_dummy.utils.CommonUtil.getStatusBarHeight;
import static com.hcn.media_dummy.utils.CommonUtil.hideNavKey;
import static com.hcn.media_dummy.utils.CommonUtil.hideSupportActionBar;
import static com.hcn.media_dummy.utils.CommonUtil.showNavKey;
import static com.hcn.media_dummy.utils.CommonUtil.showSupportActionBar;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.Point;
import android.os.Handler;
import android.os.Looper;
import android.transition.TransitionManager;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;

import com.hcn.common.misc.LogUtils;
import com.hcn.media_dummy.R;
import com.hcn.media_dummy.utils.CommonUtil;
import com.hcn.media_dummy.utils.OrientationOption;
import com.hcn.media_dummy.utils.OrientationUtils;
import com.hcn.media_dummy.view.base.FunMediaState;
import com.hcn.media_dummy.view.base.SmallVideoTouch;

import java.lang.reflect.Constructor;

/**
 * 处理全屏和小屏逻辑
 * @author 65821
 */
public abstract class FunBaseVideoPlayer extends FunVideoControlView {
    /** 保存系统状态 ui */
    protected int mSystemUiVisibility;

    /** 当前 item 框的屏幕位置 */
    protected int[] mListItemRect;

    /** 当前 item 的大小 */
    protected int[] mListItemSize;

    /** 是否需要在利用 window 实现全屏幕的时候隐藏 ActionBar */
    protected boolean mActionBar = false;

    /** 是否需要在利用 window 实现全屏幕的时候隐藏 StatusBar */
    protected boolean mStatusBar = false;

    /** 是否使用全屏动画效果 */
    protected boolean mShowFullAnimation = true;

    /** 是否自动旋转 */
    protected boolean mRotateViewAuto = true;

    /** 旋转使能后是否跟随系统设置 */
    protected boolean mRotateWithSystem = true;

    /** 当前全屏是否锁定全屏 */
    protected boolean mLockLand = false;

    /**
     * 是否根据视频尺寸，自动选择竖屏全屏或者横屏全屏，注意，这时候默认旋转无效
     * 这个标志为和 mLockLand 冲突，需要和 OrientationUtils  使用
     */
    protected boolean mAutoFullWithSize = false;

    /** 是否需要初始化内部 OrientationUtils */
    protected boolean mNeedOrientationUtils = true;

    /** 是否需要竖屏全屏的时候判断状态栏 */
    protected boolean isNeedAutoAdaptation = false;

    /** 全屏动画是否结束了 */
    protected boolean mFullAnimEnd = true;

    /** 小窗口关闭按键 */
    protected View mSmallClose;

    /** 旋转工具类 */
    protected OrientationUtils mOrientationUtils;

    private boolean mIsOnlyRotateLand = false;

    /** 全屏返回监听，如果设置了，默认返回无效 */
    protected View.OnClickListener mBackFromFullScreenListener;
    protected Handler mInnerHandler = new Handler(Looper.getMainLooper());

    public FunBaseVideoPlayer(Context context, Boolean fullFlag) {
        super(context, fullFlag);
    }

    public FunBaseVideoPlayer(Context context) {
        super(context);
    }

    public FunBaseVideoPlayer(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public FunBaseVideoPlayer(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected void init(Context context) {
        super.init(context);
        mSmallClose = findViewById(R.id.small_close);
    }

    @Override
    public void onBackFullscreen() {
        clearFullscreenLayout();
    }

    /**
     * 小窗口
     **/
    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void setSmallVideoTextureView() {
        if (mProgressBar != null) {
            mProgressBar.setOnTouchListener(null);
            mProgressBar.setVisibility(INVISIBLE);
        }

        if (mFullscreenButton != null) {
            mFullscreenButton.setOnTouchListener(null);
            mFullscreenButton.setVisibility(INVISIBLE);
        }

        if (mCurrentTimeTextView != null) {
            mCurrentTimeTextView.setVisibility(INVISIBLE);
        }

        if (mTextureViewContainer != null) {
            mTextureViewContainer.setOnClickListener(null);
        }

        if (mSmallClose != null) {
            mSmallClose.setVisibility(VISIBLE);
            mSmallClose.setOnClickListener(v -> {
                hideSmallVideo();
                releaseVideos();
            });
        }
    }

    /**
     * 处理锁屏屏幕触摸逻辑
     */
    @Override
    protected void lockTouchLogic() {
        super.lockTouchLogic();

        if (!mLockCurScreen) {
            if (mOrientationUtils != null) {
                mOrientationUtils.setEnable(isRotateViewAuto());
            }
        } else {
            if (mOrientationUtils != null) {
                mOrientationUtils.setEnable(false);
            }
        }
    }

    @Override
    public void onPrepared() {
        super.onPrepared();

        // 确保开启竖屏检测的时候正常全屏
        checkAutoFullSizeWhenFull();
    }

    @Override
    public void onInfo(int what, int extra) {
        super.onInfo(what, extra);
        if (what == getFunMediaManager().getRotateInfoFlag()) {
            checkAutoFullSizeWhenFull();
        }
    }

    protected ViewGroup getViewGroup() {
        return (ViewGroup) (CommonUtil.scanForActivity(
                getContext())).findViewById(Window.ID_ANDROID_CONTENT);
    }

    /**
     * 移除没用的
     */
    private void removeVideo(ViewGroup vp, int id) {
        View old = vp.findViewById(id);
        if (old != null) {
            if (old.getParent() != null) {
                ViewGroup viewGroup = (ViewGroup) old.getParent();
                vp.removeView(viewGroup);
            }
        }
    }

    /**
     * 保存大小和状态
     *
     * @param context 上下文
     * @param statusBar 是否有状态栏
     * @param actionBar 是否有活动栏
     */
    private void saveLocationStatus(Context context, boolean statusBar, boolean actionBar) {
        getLocationOnScreen(mListItemRect);

        if (context instanceof Activity) {
            int statusBarH = getStatusBarHeight(context);
            int actionBerH = getActionBarHeight(CommonUtil.getActivityNestWrapper(context));
            boolean isTranslucent = ((WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS
                    & CommonUtil.getActivityNestWrapper(context).getWindow().getAttributes().flags)
                        == WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            LogUtils.v("*************isTranslucent*************** " + isTranslucent);

            if (statusBar && !isTranslucent) {
                mListItemRect[1] = mListItemRect[1] - statusBarH;
            }
            if (actionBar) {
                mListItemRect[1] = mListItemRect[1] - actionBerH;
            }
        }

        mListItemSize[0] = getWidth();
        mListItemSize[1] = getHeight();
    }

    /**
     * 克隆切换参数
     *
     * @param from 源
     * @param to 目标
     */
    protected void cloneParams(FunBaseVideoPlayer from, FunBaseVideoPlayer to) {
        to.mHadPlay = from.mHadPlay;
        to.mPlayTag = from.mPlayTag;
        to.mPlayPosition = from.mPlayPosition;
        to.mEffectFilter = from.mEffectFilter;
        to.mFullPauseBitmap = from.mFullPauseBitmap;
        to.mNeedShowWifiTip = from.mNeedShowWifiTip;
        to.mShrinkImageRes = from.mShrinkImageRes;
        to.mEnlargeImageRes = from.mEnlargeImageRes;
        to.mRotate = from.mRotate;
        to.mShowPauseCover = from.mShowPauseCover;
        to.mDismissControlTime = from.mDismissControlTime;
        to.mSeekRatio = from.mSeekRatio;
        to.mNetChanged = from.mNetChanged;
        to.mNetSate = from.mNetSate;
        to.mRotateViewAuto = from.mRotateViewAuto;
        to.mRotateWithSystem = from.mRotateWithSystem;
        to.mBackUpPlayingBufferState = from.mBackUpPlayingBufferState;
        to.mRenderer = from.mRenderer;
        to.mMode = from.mMode;
        to.mBackFromFullScreenListener = from.mBackFromFullScreenListener;
        to.mFunVideoProgressListener = from.mFunVideoProgressListener;
        to.mHadPrepared = from.mHadPrepared;
        to.mSurfaceErrorPlay = from.mSurfaceErrorPlay;
        to.mStartAfterPrepared = from.mStartAfterPrepared;
        to.mPauseBeforePrepared = from.mPauseBeforePrepared;
        to.mReleaseWhenLossAudio = from.mReleaseWhenLossAudio;
        to.mVideoAllCallBack = from.mVideoAllCallBack;
        to.mRotateViewAuto = from.mRotateViewAuto;
        to.mActionBar = from.mActionBar;
        to.mStatusBar = from.mStatusBar;
        to.mAutoFullWithSize = from.mAutoFullWithSize;
        to.mOverrideExtension = from.mOverrideExtension;
        to.mNeedOrientationUtils = from.mNeedOrientationUtils;
        to.onAudioFocusChangeListener = from.onAudioFocusChangeListener;

        if (from.mSetUpLazy) {
            to.setUpLazy(from.mOriginUrl, from.mCache, from.mCachePath, from.mMapHeadData, from.mTitle);
            to.mUrl = from.mUrl;
        } else {
            to.setUp(from.mOriginUrl, from.mCache, from.mCachePath, from.mMapHeadData, from.mTitle);
        }

        to.setLooping(from.isLooping());
        to.setIsTouchWidgetFull(from.mIsTouchWidgetFull);
        to.setSpeed(from.getSpeed(), from.mSoundTouch);
        to.setStateAndUi(from.mCurrentState);
    }

    /**
     * 全屏的暂停的时候返回页面不黑色
     */
    private void pauseFullCoverLogic() {
        if (mCurrentState == FunMediaState.CURRENT_STATE_PAUSE
                && mTextureView != null
                && (mFullPauseBitmap == null || mFullPauseBitmap.isRecycled())
                && mShowPauseCover) {
            try {
                initCover();
            } catch (Exception e) {
                e.printStackTrace();
                mFullPauseBitmap = null;
            }
        }
    }

    /**
     * 全屏的暂停返回的时候返回页面不黑色
     */
    private void pauseFullBackCoverLogic(FunBaseVideoPlayer funVideoPlayer) {
        // 如果是暂停状态
        if (funVideoPlayer.mCurrentState == FunMediaState.CURRENT_STATE_PAUSE
                && funVideoPlayer.mTextureView != null
                && mShowPauseCover) {
            // 全屏的位图还在，说明没播放，直接用原来的
            if (funVideoPlayer.mFullPauseBitmap != null
                    && !funVideoPlayer.mFullPauseBitmap.isRecycled() && mShowPauseCover) {
                mFullPauseBitmap = funVideoPlayer.mFullPauseBitmap;
            } else if (mShowPauseCover) {
                // 不在了说明已经播放过，还是暂停的话，我们拿回来就好
                try {
                    funVideoPlayer.initCover();
                } catch (Exception e) {
                    e.printStackTrace();
                    mFullPauseBitmap = null;
                }
            }
        }
    }

    /**
     * 全屏
     */
    protected void resolveFullVideoShow(Context context,
                                        final FunBaseVideoPlayer funVideoPlayer,
                                        final FrameLayout frameLayout) {
        LayoutParams lp = (LayoutParams) funVideoPlayer.getLayoutParams();
        lp.setMargins(0, 0, 0, 0);
        lp.height = ViewGroup.LayoutParams.MATCH_PARENT;
        lp.width = ViewGroup.LayoutParams.MATCH_PARENT;
        lp.gravity = Gravity.CENTER;
        funVideoPlayer.setLayoutParams(lp);
        funVideoPlayer.setIfCurrentIsFullscreen(true);

        if (mNeedOrientationUtils) {
            mOrientationUtils = new OrientationUtils((Activity) context, funVideoPlayer, getOrientationOption());
            mOrientationUtils.setEnable(isRotateViewAuto());
            mOrientationUtils.setRotateWithSystem(mRotateWithSystem);
            mOrientationUtils.setOnlyRotateLand(mIsOnlyRotateLand);
            funVideoPlayer.mOrientationUtils = mOrientationUtils;
        }

        final boolean isVertical = isVerticalFullByVideoSize();
        final boolean isLockLand = isLockLandByAutoFullSize();

        if (isShowFullAnimation()) {
            mInnerHandler.postDelayed(() -> {
                // autoFull 模式下，非横屏视频视频不横屏，并且不自动旋转
                if (!isVertical
                        && isLockLand
                        && mOrientationUtils != null
                        && mOrientationUtils.getIsLand() != 1) {
                    // ------- ！！！如果不需要旋转屏幕，可以不调用！！！-------
                    // 不需要屏幕旋转，还需要设置 setNeedOrientationUtils(false)
                    mOrientationUtils.resolveByClick();
                }

                funVideoPlayer.setVisibility(VISIBLE);
                frameLayout.setVisibility(VISIBLE);
            }, 300);
        } else {
            if (!isVertical
                    && isLockLand
                    && mOrientationUtils != null) {
                // ------- ！！！如果不需要旋转屏幕，可以不调用！！！-------
                // 不需要屏幕旋转，还需要设置 setNeedOrientationUtils(false)
                mOrientationUtils.resolveByClick();
            }

            funVideoPlayer.setVisibility(VISIBLE);
            frameLayout.setVisibility(VISIBLE);
        }

        if (mVideoAllCallBack != null) {
            LogUtils.w("onEnterFullscreen");
            mVideoAllCallBack.onEnterFullscreen(mOriginUrl, mTitle, funVideoPlayer);
        }

        mIfCurrentIsFullscreen = true;

        checkoutState();
        checkAutoFullWithSizeAndAdaptation(funVideoPlayer);
    }

    /**
     * 恢复正常视频显示
     * <p> 如果是全屏播放状态，则退出全屏状态；
     */
    protected void resolveNormalVideoShow(View oldF,
                                          ViewGroup vp,
                                          FunVideoPlayer funVideoPlayer) {
        if (oldF != null && oldF.getParent() != null) {
            ViewGroup viewGroup = (ViewGroup) oldF.getParent();
            vp.removeView(viewGroup);
        }

        mCurrentState = getFunMediaManager().getLastState();
        if (funVideoPlayer != null) {
            cloneParams(funVideoPlayer, this);
        }

        if ((mCurrentState != CURRENT_STATE_NORMAL)
                || (mCurrentState != CURRENT_STATE_AUTO_COMPLETE)) {
            createNetWorkState();
        }

        // 恢复监听对象
        getFunMediaManager().setListener(getFunMediaManager().lastListener());
        getFunMediaManager().setLastListener(null);
        setStateAndUi(mCurrentState);
        addTextureView();

        mSaveChangeViewTIme = System.currentTimeMillis();
        if (mVideoAllCallBack != null) {
            LogUtils.w("onQuitFullscreen");
            mVideoAllCallBack.onQuitFullscreen(mOriginUrl, mTitle, this);
        }

        mIfCurrentIsFullscreen = false;
        if (mHideKey) {
            showNavKey(mContext, mSystemUiVisibility);
        }

        showSupportActionBar(mContext, mActionBar, mStatusBar);
        if (getFullscreenButton() != null) {
            getFullscreenButton().setImageResource(getEnlargeImageRes());
        }
    }

    /**
     * 退出 window 层播放全屏效果
     */
    @SuppressWarnings("ResourceType")
    protected void clearFullscreenLayout() {
        if (!mFullAnimEnd) {
            return;
        }

        mIfCurrentIsFullscreen = false;
        int delay = 0;
        // ------- ！！！如果不需要旋转屏幕，可以不调用！！！-------
        // 不需要屏幕旋转，还需要设置 setNeedOrientationUtils(false)
        if (mOrientationUtils != null) {
            delay = mOrientationUtils.backToPortVideo();
            mOrientationUtils.setEnable(false);
            if (mOrientationUtils != null) {
                mOrientationUtils.releaseListener();
                mOrientationUtils = null;
            }
        }

        if (!mShowFullAnimation) {
            delay = 0;
        }

        final ViewGroup vp = getViewGroup();
        final View oldF = vp.findViewById(getFullId());
        if (oldF != null) {
            // 退出全屏的时候，虚拟按键问题
            FunVideoPlayer funVideoPlayer = (FunVideoPlayer) oldF;
            funVideoPlayer.mIfCurrentIsFullscreen = false;
        }

        mInnerHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                backToNormal();
            }
        }, delay);
    }

    /**
     * 回到正常效果
     */
    @SuppressWarnings("ResourceType")
    protected void backToNormal() {
        final ViewGroup vp = getViewGroup();
        final View oldF = vp.findViewById(getFullId());
        final FunVideoPlayer funVideoPlayer;

        if (oldF != null) {
            funVideoPlayer = (FunVideoPlayer) oldF;
            // 如果暂停了
            pauseFullBackCoverLogic(funVideoPlayer);
            if (mShowFullAnimation) {
                TransitionManager.beginDelayedTransition(vp);

                LayoutParams lp = (LayoutParams) funVideoPlayer.getLayoutParams();
                lp.setMargins(mListItemRect[0], mListItemRect[1], 0, 0);
                lp.width = mListItemSize[0];
                lp.height = mListItemSize[1];
                // 注意配置回来，不然动画效果会不对
                lp.gravity = Gravity.NO_GRAVITY;
                funVideoPlayer.setLayoutParams(lp);

                mInnerHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        resolveNormalVideoShow(oldF, vp, funVideoPlayer);
                    }
                }, 400);
            } else {
                resolveNormalVideoShow(oldF, vp, funVideoPlayer);
            }
        } else {
            resolveNormalVideoShow(null, vp, null);
        }
    }

    protected Runnable mCheckoutTask = () -> {
        FunVideoPlayer funVideoPlayer = getFullWindowPlayer();
        if (funVideoPlayer != null
                && funVideoPlayer.mCurrentState != mCurrentState) {
            if (funVideoPlayer.mCurrentState == CURRENT_STATE_PLAYING_BUFFERING_START
                    && mCurrentState != CURRENT_STATE_PREPAREING) {
                funVideoPlayer.setStateAndUi(mCurrentState);
            }
        }
    };

    /**
     * 检查状态
     */
    protected void checkoutState() {
        removeCallbacks(mCheckoutTask);
        mInnerHandler.postDelayed(mCheckoutTask, 500);
    }

    /**
     * 是否竖屏模式的竖屏
     * @return 是/否
     */
    protected boolean isVerticalVideo() {
        boolean isVertical = false;
        int videoHeight = getCurrentVideoHeight();
        int videoWidth = getCurrentVideoWidth();

        LogUtils.v("FunVideoBase isVerticalVideo  videoHeight "
                + videoHeight + " videoWidth " + videoWidth);
        LogUtils.v("FunVideoBase isVerticalVideo  mRotate " + mRotate);

        if (videoHeight > 0 && videoWidth > 0) {
            if (mRotate == 90 || mRotate == 270) {
                isVertical = videoWidth > videoHeight;
            } else {
                isVertical = videoHeight > videoWidth;
            }
        }

        return isVertical;
    }

    /**
     * 是否根据autoFullSize调整lockLand
     */
    protected boolean isLockLandByAutoFullSize() {
        boolean isLockLand = mLockLand;
        if (isAutoFullWithSize()) {
            isLockLand = isVerticalVideo();
        }
        return isLockLand;
    }

    /**
     * 确保开启竖屏检测的时候正常全屏
     */
    protected void checkAutoFullSizeWhenFull() {
        if (mIfCurrentIsFullscreen) {
            // 确保开启竖屏检测的时候正常全屏
            boolean isV = isVerticalFullByVideoSize();
            LogUtils.v("GSYVideoBase onPrepared isVerticalFullByVideoSize " + isV);

            if (isV) {
                // ------- ！！！如果不需要旋转屏幕，可以不调用！！！-------
                // 不需要屏幕旋转，还需要设置 setNeedOrientationUtils(false)
                if (mOrientationUtils != null) {
                    mOrientationUtils.backToPortVideo();
                    // 处理在未开始播放的时候点击全屏
                    checkAutoFullWithSizeAndAdaptation(this);
                }
            }
        }
    }

    protected abstract int getFullId();

    protected abstract int getSmallId();

    /************************* 开放接口 *************************/

    /**
     * 是否根据视频尺寸，自动选择竖屏全屏或者横屏全屏，注意，这时候默认旋转无效
     */
    public boolean isVerticalFullByVideoSize() {
        return isVerticalVideo() && isAutoFullWithSize();
    }

    /**
     * 旋转处理
     *
     * @param activity         页面
     * @param newConfig        配置
     * @param orientationUtils 旋转工具类
     */
    public void onConfigurationChanged(Activity activity,
                                       Configuration newConfig,
                                       OrientationUtils orientationUtils) {
        onConfigurationChanged(activity, newConfig,
                orientationUtils, true, true);

    }

    /**
     * 旋转处理
     *
     * @param activity 页面
     * @param newConfig 配置
     * @param orientationUtils 旋转工具类
     * @param hideActionBar 是否隐藏actionbar
     * @param hideStatusBar 是否隐藏statusbar
     */
    public void onConfigurationChanged(Activity activity,
                                       Configuration newConfig,
                                       OrientationUtils orientationUtils,
                                       boolean hideActionBar,
                                       boolean hideStatusBar) {
        super.onConfigurationChanged(newConfig);

        // 如果旋转了就全屏
        if (newConfig.orientation == ActivityInfo.SCREEN_ORIENTATION_USER) {
            if (!isIfCurrentIsFullscreen()) {
                startWindowFullscreen(activity, hideActionBar, hideStatusBar);
            }
        } else {
            // 新版本 isIfCurrentIsFullscreen 的标志位内部提前设置了，所以不会和手动点击冲突
            if (isIfCurrentIsFullscreen()
                    && !isVerticalFullByVideoSize()) {
                backFromFull(activity);
            }

            if (orientationUtils != null) {
                orientationUtils.setEnable(isRotateWithSystem());
            }
        }
    }

    /**
     * 可配置旋转 OrientationUtils
     */
    public OrientationOption getOrientationOption() {
        return null;
    }

    /**
     * 利用 window 层播放全屏效果
     *
     * @param context 上下文
     * @param actionBar 是否有 ActionBar，有的话需要隐藏
     * @param statusBar 是否有状态 Bar，有的话需要隐藏
     * @return 返回一个播放器视图
     */
    @SuppressWarnings("ResourceType, unchecked")
    public FunBaseVideoPlayer startWindowFullscreen(final Context context,
                                                    final boolean actionBar,
                                                    final boolean statusBar) {
        mSystemUiVisibility = CommonUtil.getActivityNestWrapper(context)
                .getWindow().getDecorView().getSystemUiVisibility();
        hideSupportActionBar(context, actionBar, statusBar);

        if (mHideKey) {
            hideNavKey(context);
        }

        this.mActionBar = actionBar;
        this.mStatusBar = statusBar;

        mListItemRect = new int[2];
        mListItemSize = new int[2];

        final ViewGroup vp = getViewGroup();
        removeVideo(vp, getFullId());

        // 处理暂停的逻辑
        pauseFullCoverLogic();
        if (mTextureViewContainer.getChildCount() > 0) {
            mTextureViewContainer.removeAllViews();
        }

        saveLocationStatus(context, statusBar, actionBar);

        // 切换时关闭非全屏定时器
        cancelProgressTimer();

        boolean hadNewConstructor = true;

        try {
            // 检测是否存在带指定参数的构造函数
            FunBaseVideoPlayer.this.getClass().getConstructor(Context.class, Boolean.class);
        } catch (Exception e) {
            hadNewConstructor = false;
        }

        try {
            // 通过被重载的不同构造器来选择
            Constructor<FunBaseVideoPlayer> constructor;
            final FunBaseVideoPlayer funVideoPlayer;
            if (!hadNewConstructor) {
                constructor = (Constructor<FunBaseVideoPlayer>)
                        FunBaseVideoPlayer.this.getClass().getConstructor(Context.class);
                funVideoPlayer = constructor.newInstance(mContext);
            } else {
                constructor = (Constructor<FunBaseVideoPlayer>)
                        FunBaseVideoPlayer.this.getClass().getConstructor(Context.class, Boolean.class);
                funVideoPlayer = constructor.newInstance(mContext, true);
            }

            funVideoPlayer.setId(getFullId());
            funVideoPlayer.setIfCurrentIsFullscreen(true);
            funVideoPlayer.setVideoAllCallBack(mVideoAllCallBack);

            // 克隆当前对象参数到新播放器视图
            cloneParams(this, funVideoPlayer);

            if (funVideoPlayer.getFullscreenButton() != null) {
                funVideoPlayer.getFullscreenButton().setImageResource(getShrinkImageRes());
                funVideoPlayer.getFullscreenButton().setOnClickListener(v -> {
                    if (mBackFromFullScreenListener == null) {
                        clearFullscreenLayout();
                    } else {
                        mBackFromFullScreenListener.onClick(v);
                    }
                });
            }

            if (funVideoPlayer.getBackButton() != null) {
                funVideoPlayer.getBackButton().setVisibility(VISIBLE);
                funVideoPlayer.getBackButton().setOnClickListener(v -> {
                    if (mBackFromFullScreenListener == null) {
                        clearFullscreenLayout();
                    } else {
                        mBackFromFullScreenListener.onClick(v);
                    }
                });
            }

            final LayoutParams lpParent = new LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
            final FrameLayout frameLayout = new FrameLayout(context);
            frameLayout.setBackgroundColor(Color.BLACK);

            if (mShowFullAnimation) {
                mFullAnimEnd = false;
                LayoutParams lp = new LayoutParams(getWidth(), getHeight());
                lp.setMargins(mListItemRect[0], mListItemRect[1], 0, 0);
                frameLayout.addView(funVideoPlayer, lp);
                vp.addView(frameLayout, lpParent);
                mInnerHandler.postDelayed(() -> {
                    TransitionManager.beginDelayedTransition(vp);
                    resolveFullVideoShow(context, funVideoPlayer, frameLayout);
                    mFullAnimEnd = true;
                }, 300);
            } else {
                LayoutParams lp = new LayoutParams(getWidth(), getHeight());
                frameLayout.addView(funVideoPlayer, lp);
                vp.addView(frameLayout, lpParent);
                funVideoPlayer.setVisibility(INVISIBLE);
                frameLayout.setVisibility(INVISIBLE);
                resolveFullVideoShow(context, funVideoPlayer, frameLayout);
            }

            funVideoPlayer.addTextureView();
            funVideoPlayer.startProgressTimer();

            // 切换到全屏（保存监听对象）
            getFunMediaManager().setLastListener(this);
            getFunMediaManager().setListener(funVideoPlayer);

            checkoutState();
            return funVideoPlayer;
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }


    /**
     * 显示小窗口
     * <p> 创建一个小窗口显示播放用；
     */
    @SuppressWarnings("ResourceType, unchecked")
    public FunBaseVideoPlayer showSmallVideo(Point size,
                                             final boolean actionBar,
                                             final boolean statusBar) {
        final ViewGroup vp = getViewGroup();
        removeVideo(vp, getSmallId());

        if (mTextureViewContainer.getChildCount() > 0) {
            mTextureViewContainer.removeAllViews();
        }

        try {
            Constructor<FunBaseVideoPlayer> constructor =
                    (Constructor<FunBaseVideoPlayer>) FunBaseVideoPlayer
                            .this.getClass().getConstructor(Context.class);
            FunBaseVideoPlayer funVideoPlayer = constructor.newInstance(getActivityContext());
            funVideoPlayer.setId(getSmallId());

            LayoutParams lpParent = new LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
            FrameLayout frameLayout = new FrameLayout(mContext);

            LayoutParams lp = new LayoutParams(size.x, size.y);
            int marginLeft = CommonUtil.getScreenWidth(mContext) - size.x;
            int marginTop = CommonUtil.getScreenHeight(mContext) - size.y;

            if (actionBar) {
                marginTop = marginTop - getActionBarHeight((Activity) mContext);
            }

            if (statusBar) {
                marginTop = marginTop - getStatusBarHeight(mContext);
            }

            lp.setMargins(marginLeft, marginTop, 0, 0);
            frameLayout.addView(funVideoPlayer, lp);

            vp.addView(frameLayout, lpParent);
            cloneParams(this, funVideoPlayer);

            // 小窗口不能点击
            funVideoPlayer.setIsTouchWidget(false);
            funVideoPlayer.addTextureView();

            // 隐藏掉所有的弹出状态哟
            funVideoPlayer.onClickUiToggle(null);
            funVideoPlayer.setVideoAllCallBack(mVideoAllCallBack);
            funVideoPlayer.setSmallVideoTextureView(
                    new SmallVideoTouch(funVideoPlayer, marginLeft, marginTop));

            getFunMediaManager().setLastListener(this);
            getFunMediaManager().setListener(funVideoPlayer);
            if (mVideoAllCallBack != null) {
                LogUtils.w("onEnterSmallWidget");
                mVideoAllCallBack.onEnterSmallWidget(mOriginUrl, mTitle, funVideoPlayer);
            }

            return funVideoPlayer;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * 隐藏小窗口
     */
    @SuppressWarnings("ResourceType")
    public void hideSmallVideo() {
        final ViewGroup vp = getViewGroup();
        FunVideoPlayer funVideoPlayer = (FunVideoPlayer) vp.findViewById(getSmallId());
        removeVideo(vp, getSmallId());
        mCurrentState = getFunMediaManager().getLastState();
        if (funVideoPlayer != null) {
            cloneParams(funVideoPlayer, this);
        }

        // 恢复监听对象
        getFunMediaManager().setListener(getFunMediaManager().lastListener());
        getFunMediaManager().setLastListener(null);

        setStateAndUi(mCurrentState);
        addTextureView();
        mSaveChangeViewTIme = System.currentTimeMillis();

        if (mVideoAllCallBack != null) {
            LogUtils.v("onQuitSmallWidget");
            mVideoAllCallBack.onQuitSmallWidget(mOriginUrl, mTitle, this);
        }
    }

    public boolean isShowFullAnimation() {
        return mShowFullAnimation;
    }

    /**
     * 全屏动画
     *
     * @param showFullAnimation 是否使用全屏动画效果
     */
    public void setShowFullAnimation(boolean showFullAnimation) {
        this.mShowFullAnimation = showFullAnimation;
    }

    public boolean isRotateViewAuto() {
        if (mAutoFullWithSize) {
            return false;
        }
        return mRotateViewAuto;
    }

    /**
     * 是否开启自动旋转
     */
    public void setRotateViewAuto(boolean rotateViewAuto) {
        this.mRotateViewAuto = rotateViewAuto;
        if (mOrientationUtils != null) {
            mOrientationUtils.setEnable(rotateViewAuto);
        }
    }

    public boolean isLockLand() {
        return mLockLand;
    }

    /**
     * 一全屏就锁屏横屏，默认false竖屏，可配合setRotateViewAuto使用
     */
    public void setLockLand(boolean lockLand) {
        this.mLockLand = lockLand;
    }


    public boolean isRotateWithSystem() {
        return mRotateWithSystem;
    }

    /**
     * 是否更新系统旋转，false的话，系统禁止旋转也会跟着旋转
     *
     * @param rotateWithSystem 默认true
     */
    public void setRotateWithSystem(boolean rotateWithSystem) {
        this.mRotateWithSystem = rotateWithSystem;
        if (mOrientationUtils != null) {
            mOrientationUtils.setRotateWithSystem(rotateWithSystem);
        }
    }

    /**
     * 获取全屏播放器对象
     *
     * @return FunVideoPlayer 如果没有则返回空。
     */
    @SuppressWarnings("ResourceType")
    public FunVideoPlayer getFullWindowPlayer() {
        Activity activity = CommonUtil.scanForActivity(getContext());
        if (activity == null) {
            return null;
        }

        ViewGroup vp = (ViewGroup) activity.findViewById(Window.ID_ANDROID_CONTENT);
        final View full = vp.findViewById(getFullId());
        FunVideoPlayer funVideoPlayer = null;
        if (full != null) {
            funVideoPlayer = (FunVideoPlayer) full;
        }
        return funVideoPlayer;
    }

    /**
     * 获取小窗口播放器对象
     *
     * @return FunVideoPlayer 如果没有则返回空。
     */
    @SuppressWarnings("ResourceType")
    public FunVideoPlayer getSmallWindowPlayer() {
        ViewGroup vp = (ViewGroup) (CommonUtil.scanForActivity(getContext()))
                .findViewById(Window.ID_ANDROID_CONTENT);
        final View small = vp.findViewById(getSmallId());
        FunVideoPlayer FunVideoPlayer = null;
        if (small != null) {
            FunVideoPlayer = (FunVideoPlayer) small;
        }

        return FunVideoPlayer;
    }

    /**
     * 获取当前长在播放的播放控件
     */
    public FunBaseVideoPlayer getCurrentPlayer() {
        if (getFullWindowPlayer() != null) {
            return getFullWindowPlayer();
        }
        if (getSmallWindowPlayer() != null) {
            return getSmallWindowPlayer();
        }
        return this;
    }

    /**
     * 全屏返回监听，如果设置了，默认返回动作无效
     * 包含返回键和全屏返回按键，前提是这两个按键存在
     */
    public void setBackFromFullScreenListener(OnClickListener backFromFullScreenListener) {
        this.mBackFromFullScreenListener = backFromFullScreenListener;
    }

    public void setFullHideActionBar(boolean actionBar) {
        this.mActionBar = actionBar;
    }

    public void setFullHideStatusBar(boolean statusBar) {
        this.mStatusBar = statusBar;
    }

    public boolean isFullHideActionBar() {
        return mActionBar;
    }

    public boolean isFullHideStatusBar() {
        return mStatusBar;
    }

    public int getSaveBeforeFullSystemUiVisibility() {
        return mSystemUiVisibility;
    }

    public void setSaveBeforeFullSystemUiVisibility(int systemUiVisibility) {
        this.mSystemUiVisibility = systemUiVisibility;
    }

    public boolean isAutoFullWithSize() {
        return mAutoFullWithSize;
    }

    /**
     * 是否根据视频尺寸，自动选择竖屏全屏或者横屏全屏，注意，这时候默认旋转无效
     *
     * @param autoFullWithSize 默认false
     */
    public void setAutoFullWithSize(boolean autoFullWithSize) {
        this.mAutoFullWithSize = autoFullWithSize;
    }

    public boolean isNeedAutoAdaptation() {
        return isNeedAutoAdaptation;
    }

    /**
     * 是否需要旋转的 OrientationUtils
     *
     * @param need 默认 true
     */
    public void setNeedOrientationUtils(boolean need) {
        this.mNeedOrientationUtils = need;
    }

    public boolean isNeedOrientationUtils() {
        return mNeedOrientationUtils;
    }

    /**
     * 是否需要适配在竖屏横屏时，由于刘海屏或者打孔屏占据空间，导致标题显示被遮盖的问题
     *
     * @param needAutoAdaptation 默认false
     */
    public void setNeedAutoAdaptation(boolean needAutoAdaptation) {
        isNeedAutoAdaptation = needAutoAdaptation;
    }

    public boolean isOnlyRotateLand() {
        return mIsOnlyRotateLand;
    }

    /**
     * 旋转时仅处理横屏
     */
    public void setOnlyRotateLand(boolean onlyRotateLand) {
        this.mIsOnlyRotateLand = onlyRotateLand;
        if (mOrientationUtils != null) {
            mOrientationUtils.setOnlyRotateLand(mIsOnlyRotateLand);
        }
    }

    /**
     * 检测是否根据视频尺寸，自动选择竖屏全屏或者横屏全屏；
     * 并且适配在竖屏横屏时，由于刘海屏或者打孔屏占据空间，导致标题显示被遮盖的问题
     *
     * @param funVideoPlayer 将要显示的播放器对象
     */
    protected void checkAutoFullWithSizeAndAdaptation(final FunBaseVideoPlayer funVideoPlayer) {
        if (funVideoPlayer != null) {
            // 判断是否自动选择
            // 判断是否是竖直的视频
            // 判断是否隐藏状态栏
            if (isNeedAutoAdaptation
                    && isAutoFullWithSize()
                    && isVerticalVideo()
                    && isFullHideStatusBar()) {
                mInnerHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        funVideoPlayer.getCurrentPlayer().autoAdaptation();
                    }
                }, 100);
            }
        }
    }

    /**
     * 自动适配在竖屏全屏时，
     * 由于刘海屏或者打孔屏占据空间带来的影响(某些机型在全屏时会自动将布局下移（或者添加 padding），
     * 例如三星 S10、小米8；但是也有一些机型在全屏时不会处理，此时，就为了兼容这部分机型)
     */
    protected void autoAdaptation() {
        Context context = getContext();
        if (isVerticalVideo()) {
            int[] location = new int[2];
            getLocationOnScreen(location);
            /*同时判断系统是否有自动将布局从statusbar下方开始显示，根据在屏幕中的位置判断*/
            //如果系统没有将布局下移，那么此时处理
            if (location[1] == 0) {
                setPadding(0, getStatusBarHeight(context), 0, 0);
                LogUtils.v("竖屏，系统未将布局下移");
            } else {
                LogUtils.v("竖屏，系统将布局下移；y:" + location[1]);
            }
        }
    }
}
