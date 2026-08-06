package com.hcn.media.video.common;

import static com.hcn.config.Feature.BIT.REMOTE_CONTROL_FOCUS;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Point;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.auto_compat.PlatformUtils;
import com.hcn.auto_compat.app.WindowConfiguration;
import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.common.widget.HViewUtils;
import com.hcn.config.Feature;
import com.hcn.media.base.Preferences;
import com.hcn.media.extend.base.IExtend;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.auto_compat.os.ProcessCompat;
import com.hcn.media_common.utils.ViewUtilsEx;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_theme.Argument;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.media_theme.ThemeEx;
import com.hcn.media_theme.ThemeX;
import com.hcn.media_base.IMediaEvent;
import com.hcn.mediaservice.data.MediaTimeInfo;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media.local.utils.HFuncUtils;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_view.HTextView;
import com.hcn.media_base.HMediaConfig;
import com.hcn.plugin.ApkClassLoaderEx;

import java.util.List;
import java.util.Locale;
import java.util.Objects;

@SuppressLint({"ClickableViewAccessibility", "ValidFragment"})
public class VideoInfoFragment extends MediaFragment
        implements OnClickListener, OnTouchListener {
    private static final String TAG = VideoInfoFragment.class.getSimpleName();
    private static final String FRAGMENT_NAME = "video-info";

    /**
     * 当前 SurfaceView 类型
     * <pre>
     *    硬解码：E_GROUP_SHOW_SURFACE
     *    软解码：E_GROUP_SHOW_SURFACE_EX
     *    软解和硬解渲染时不使用同一个 SurfaceView；
     * </pre>
     */
    public static final int E_GROUP_SHOW_NULL = -1;
    public static final int E_GROUP_SHOW_SURFACE = 0;
    public static final int E_GROUP_SHOW_SURFACE_EX = 1;

    private static final int mSeekBarMaxValue = 1000;
    private static final int BrightnessMaxValue = 40;
    private static final String KEY_BRIGHTNESS = "screen_brightness";

    private static final int MOVE_PIXEL_MAX_X = 20;
    private static final int MOVE_PIXEL_MAX_Y = 20; // 值太小会影响全屏菜单隐藏操作
    private static final int MOVE_PIXEL_X = 3;
    private static final int MOVE_PIXEL_Y = 3;
    private static final int OPERATE_TYPE_NULL = 0;
    private static final int OPERATE_TYPE_X = 1;
    private static final int OPERATE_TYPE_Y = 2;
    private static final int OPERATE_TYPE_INVALID = 3;

    private static int sMusicStreamMaxVolume = 40;

    private boolean mCreateView = false;
    private boolean mInitView = false;

    private View mLayoutProgress = null;

    /** 分屏是否隐藏视频播放进度条布局 */
    private boolean mSplitScreenHideProgressLayout = false;

    private SeekBar mSeekbarProgress = null;

    private boolean mSeekbarOperate = false;

    private TextView m_tvCurrentTime = null;
    private TextView m_tvTotalTime = null;
    private TextView m_tvTotalValue = null;
    private HTextView m_tvVideoName = null;

    private View mControlLayout = null;
    private View mBottomLayout = null;
    private View mVideoTitleLayout = null;

    private View mBlackLayout = null;
    private TextView mPromptText = null;

    private int mLastPosX = 0;
    private int mLastPosY = 0;
    private int mLastPlayTime = 0;
    private int mNewPlayTime = 0;

    private Handler mUserHandler;
    private final Handler mSurfaceHandler;
    private final Handler mUserDelayHandler;

    private boolean mShowBlackLayout = true;
    private ProgressBar mVolumeProgressBar = null;
    private ProgressBar mBrightnessProgressBar = null;
    private View mVolumeLayout = null;
    private View mBrightnessLayout = null;
    private View mSeekTimeLayout = null;
    private ImageView m_ivVolumeIcon = null;

    private TextView m_tvVolumeValue = null;
    private TextView m_tvBrightnessValue = null;
    private TextView m_tvNewTime = null;
    private TextView m_tvDelayTime = null;
    private View[] mButt = null;

    private AudioManager mAudioManager = null;

    /** menu item **/
    private interface BTN_ID {
        int PREV = 0;
        int PLAY_PAUSE = 1;
        int NEXT = 2;
        int REPEAT_MODE = 3;
        int LIST = 4;
        int EQ = 5;
        int SCALE_MODE = 6;
        int SIZE = 7;
    }

    /**
     * 视频缩放比例资源
     * <pre>
     *    auto
     *    16:9
     *    4:3
     *    fullscreen
     *    1:1
     * </pre>
     */
    private final int[] mScaleTypeDrawable = {
            R.drawable.btn_video_scale_auto, R.drawable.btn_video_scale_16_9,
            R.drawable.btn_video_scale_4_3, R.drawable.btn_video_scale_full,
            R.drawable.btn_video_scale_org,
    };

    /**
     * 播放菜单隐藏任务
     * <p> 隐藏播放控制菜单（播放/暂停、上下曲等）
     */
    private final Runnable mHideRunnable = new Runnable() {

        private void runDelayed() {
            mUserHandler.removeCallbacks(mHideRunnable);
            mUserHandler.postDelayed(mHideRunnable, 5000);
        }

        @Override
        public void run() {
            // systemui 在长按音量+-等操作时，不去隐藏状态栏 #25023
            if (HUtilsEx.getSystemProperty("sys.systemui.interacting", 0) == 1) {
                runDelayed();
                return;
            }

            VideoInfoFragment.this.setShowControlLayout(false);

            if (!mAppData.mFullScreen) {
                // 通知 VideoUI 更新状态
                VideoInfoFragment.this.sendPageEvent(
                        IMediaEvent.EVENT_CHANGE_FULL_SCREEN,
                        null,
                        null);
            }
        }
    };

    /**
     * 显示播放控制菜单
     * <p> 显示 5s 后自动隐藏；
     */
    private final Runnable mShowRunnable = () -> {
        setShowControlLayout(true);

        if (null != mUserHandler) {
            mUserHandler.removeCallbacks(mHideRunnable);
            mUserHandler.postDelayed(mHideRunnable, 5000);
        }
    };

    /**
     * 隐藏黑色遮挡布局
     * <p> e.g. 页面切换的时候，有些视频会有残留等不好的体验；
     */
    private final Runnable mSurfaceShowRunnable = () -> {
        if (mBlackLayout != null && !mShowBlackLayout) {
            mBlackLayout.setVisibility(View.GONE);
        }
    };

    private boolean mCanOperate = false;
    private int mOperateType = OPERATE_TYPE_NULL;
    private int mLastMoveY = 0;
    private int mLastMoveX = 0;

    /**
     * 当前显示的 SurfaceView 类型
     * <p> 软解和硬解不使用同一个 SurfaceView 渲染；
     */
    private int mShowGroupType = E_GROUP_SHOW_NULL;
    private MediaFragment mSurfaceViewFragment = null;
    private MediaFragment mSurfaceViewFragmentEx = null;

    @SuppressLint("ValidFragment")
    public VideoInfoFragment() {
        super(FRAGMENT_NAME);

        // 这么多 Handler 合成一个最好
        mUserHandler = new Handler(Looper.getMainLooper());
        mSurfaceHandler = new Handler(Looper.getMainLooper());
        mUserDelayHandler = new UserDelayHandler(Looper.getMainLooper());

        // 支持检查扩展皮肤包（逻辑扩展）
        String pageExtendResConfigName = "video_info_page_extend";
        if (xBoolean(pageExtendResConfigName)) {
            ApkClassLoaderEx classLoader = xClassLoader();
            if (!Objects.isNull(classLoader)) {
                String pageExtendClassName =
                        IExtend.VIDEO_PACKAGE_NAME + ".VideoInfoPageExtend";
                mPageExtend = classLoader.newPageExtendInterface(pageExtendClassName, this);
            }

            LogUtils.iTag(TAG, mPageExtend != null?
                    "Has VideoInfoPageExtend class.": "No VideoInfoPageExtend class.");
        }
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);

        // 音量控制接口
        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (null != mAudioManager) {
            sMusicStreamMaxVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
        }

        // default WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_NONE
        mAppData.mScreenBrightness =
                Preferences.readIntFromSharedPreferences(
                        requireContext(), KEY_BRIGHTNESS, -1);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // 分屏是否隐藏播放进度条（有些布局不隐藏会显示不全）
        mSplitScreenHideProgressLayout = xBoolean("split_screen_hide_progress_layout");
    }

    @Override
    protected void onOrientationChangedEvent(@NonNull Configuration newConfig) {
        super.onOrientationChangedEvent(newConfig);

        // 在画中画模式不处理；
        if (requireActivity().isInPictureInPictureMode()) {
            return;
        }

        // 使用了扩展皮肤包资源，更新资源；
        if (getSkinContentView() != null) {
            // 测试效果，暂时没什么卵用；
            // getResourcesEx().updateConfiguration(newConfig, null);
        }
    }

    @Override
    public void initFragment(boolean resume) {
        LogUtil.i(TAG, ">>> initFragment.");

        if (!mCreateView) {
            return;
        }

        // [检查恢复 SurfaceView 状态]
        onShowSurfaceViewEvent(resume);

        onChangeVideoName();
        onChangePlayCtrl(mAppData.mMediaPlayState);
        onChangeRepeatModeCtrl(mAppData.videoRepeatMode());
        onChangeSeekbarValue(mAppData.mPlayTimeInfo);

        mAppData.mFullScreen = false;
        mSurfaceHandler.removeCallbacksAndMessages(null);

        tryShowBlackLayout();
        mSurfaceHandler.postDelayed(mSurfaceShowRunnable, 500);

        // 非全屏模式处理
        if (!mAppData.mFullScreen) {
            boolean isPIPMode = requireActivity().isInPictureInPictureMode();
            mUserHandler.removeCallbacksAndMessages(null);

            if (isPIPMode) {
                // PIP 模式不能显示菜单　
                mUserHandler.post(mHideRunnable);
            } else {
                mUserHandler.post(mShowRunnable);
            }
        }

        // 在视频播放页面
        mAppData.mInVideoPlayUi = true;

        // 是否容许恢复播放（小心时序问题）
        if (mAppData.mAllowResumePlay) {
            if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
                if (mAppData.mFrontSurfaceHolder != null
                        || mAppData.mFrontSurfaceHolderEx != null) {
                    mVideoViewModel.playerRelay().accept(
                            BaseViewModel.IPlayer::requestShouldPlayEvent);
                }
            }
        }
    }

    @Override
    public void uninitFragment(boolean pause) {
        LogUtil.i(TAG, ">>> uninitFragment, pause: " + pause);

        // [毫无道理的设计]<全屏模式非视频播放界面不跳曲, 多窗口就跳曲?>
        if (!requireActivity().isInMultiWindowMode()) {
            // 在视频播放页面
            mAppData.mInVideoPlayUi = false;
        }

        if (!pause) {
            onHideSurfaceViewEvent();
        }

        // 全屏显示隐藏 Handler 和 BlackLayout 隐藏 Handler
        mUserHandler.removeCallbacksAndMessages(null);
        mSurfaceHandler.removeCallbacksAndMessages(null);

        // [不可以移除 DelayHandler 相关消息]
        // mUserDelayHandler.removeCallbacksAndMessages(null);

        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            // [正常模式 + onPause 状态]
            if (!Argument.isCanPlayVideoBack() && pause) {
                if (!requireActivity().isInMultiWindowMode()) {
                    // [以前是设置的 stop = true, 原因未知]
                    mVideoViewModel.playerRelay().accept(
                            t -> t.requestShouldPauseEvent(false, 0));
                } else {
                    mVideoViewModel.playerRelay().accept(
                            t -> t.requestShouldPauseEvent(false, 500));
                }
            }
        }

        setShowControlLayout(false);
    }

    @Override
    public void doCallbackEvent(int eventId, int arg1, int arg2) {
        // 过滤打印
        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME:
            case IMediaEvent.EVENT_CHANGE_FULL_SCREEN:
                break;
            default:
                LogUtil.low_i(TAG, "doCallbackEvent, eventId: " + eventId);
                break;
        }

        if (!mInitView) {
            return;
        }

        // 是否需要通知 SurfaceView 处理
        boolean notifySurfaceView = false;
        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
                onChangePlayCtrl(mAppData.mMediaPlayState);
                break;
            case IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME:
                onChangeSeekbarValue(mAppData.mPlayTimeInfo);
                break;
            case IMediaEvent.EVENT_CHANGE_VIDEO_ITEM:
                onChangeVideoName();
                break;
            // 播放前的过渡页面（黑色过渡显示）
            case IMediaEvent.EVENT_VIDEO_SHOW_BLACK_PAGE:
                mShowBlackLayout = true;
                if (mBlackLayout != null) {
                    if (View.VISIBLE != mBlackLayout.getVisibility()) {
                        notifySurfaceView = true;
                        mBlackLayout.setVisibility(View.VISIBLE);
                    }
                }
                if (null != mPromptText) {
                    mPromptText.setVisibility(View.INVISIBLE);
                }
                break;
            case IMediaEvent.EVENT_VIDEO_HIDE_BLACK_PAGE:
                mShowBlackLayout = false;
                if (mBlackLayout != null) {
                    if (View.GONE != mBlackLayout.getVisibility()) {
                        notifySurfaceView = true;
                        if (isVisible()) {
                            mBlackLayout.setVisibility(View.GONE);
                        }
                    }
                }
                break;
            case IMediaEvent.EVENT_UNSUPPORT_VIDEO_PROMPT_SHOW:
                if (null != mPromptText) {
                    mPromptText.setVisibility(View.VISIBLE);
                }
                break;
            case IMediaEvent.EVENT_UNSUPPORT_VIDEO_PROMPT_HIDE:
                if (null != mPromptText) {
                    mPromptText.setVisibility(View.INVISIBLE);
                }
                break;
            case IMediaEvent.EVENT_CHANGE_FULL_SCREEN:
                if (!mAppData.mFullScreen) {
                    mUserHandler.removeCallbacksAndMessages(null);
                    mUserHandler.post(mShowRunnable);
                } else {
                    mUserHandler.removeCallbacksAndMessages(null);
                    mUserHandler.post(mHideRunnable);
                }
                break;
            case IMediaEvent.EVENT_CHANGE_REPEAT_MODE:
                onChangeRepeatModeCtrl(mAppData.videoRepeatMode());
                break;
            case IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_TARGET:
                onShowSurfaceViewEvent(false);
                break;
            case IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_SIZE:
                notifySurfaceView = true;
                break;
            case IMediaEvent.EVENT_CONFIGURATION_CHANGED_SIZE:
                adjustSizeOfVideoDisplayElements(2);
                return; // [直接调用返回]
            case IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_LAYOUT:
                setSurfaceViewLayout(arg1, arg2);
                return; // [直接调用返回]
            case IMediaEvent.EVENT_SEEK_TO_COMPLETE:
            default:
                break;
        }

        // [竖屏处理][SurfaceView 显示处理]
        boolean portraitDevice =
                (Configuration.ORIENTATION_PORTRAIT == mAppData.mVideoUiOrientation);
        if (portraitDevice || notifySurfaceView) {
            if (null != mSurfaceViewFragment) {
                mSurfaceViewFragment.doCallbackEvent(eventId);
            }

            if (null != mSurfaceViewFragmentEx) {
                mSurfaceViewFragmentEx.doCallbackEvent(eventId);
            }
        }
    }


    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 刷新资源
        updatePlayerResource();
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_videoinfo;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
            @Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");

        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);
        assert view != null;
        initView(view);
        mCreateView = true;
        return view;
    }

    private void initView(View layout) {
        if (mInitView) {
            return;
        }

        mInitView = true;

        if (null == mButt) {
            mButt = new View[BTN_ID.SIZE];

            mButt[BTN_ID.PREV] = layout.findViewById(xId(R.id.btnPrev));
            mButt[BTN_ID.PLAY_PAUSE] = layout.findViewById(xId(R.id.btnPlay));
            mButt[BTN_ID.NEXT] = layout.findViewById(xId(R.id.btnNext));
            mButt[BTN_ID.LIST] = layout.findViewById(xId(R.id.btnList));

            mButt[BTN_ID.EQ] = layout.findViewById(xId(R.id.btnEQ));
            mButt[BTN_ID.SCALE_MODE] = layout.findViewById(xId(R.id.btnVideoScale));
            mButt[BTN_ID.REPEAT_MODE] = layout.findViewById(xId(R.id.btnRepeatMode));
        }

        mButt[BTN_ID.PREV].setOnClickListener(this);
        mButt[BTN_ID.PLAY_PAUSE].setOnClickListener(this);
        mButt[BTN_ID.NEXT].setOnClickListener(this);
        mButt[BTN_ID.LIST].setOnClickListener(this);

        // [有些皮肤不一定有 EQ]
        if (mButt[BTN_ID.EQ] != null) {
            mButt[BTN_ID.EQ].setOnClickListener(this);
        }

        // 部分历史皮肤不带显示尺寸切换
        if (mButt[BTN_ID.SCALE_MODE] != null) {
            mButt[BTN_ID.SCALE_MODE].setOnClickListener(this);
            refreshBtnVideoScale();
        }

        // 竖屏状态默认不带播放模式
        if (mButt[BTN_ID.REPEAT_MODE] != null) {
            mButt[BTN_ID.REPEAT_MODE].setOnClickListener(this);
        }

        mLayoutProgress = layout.findViewById(xId(R.id.layout_progress));
        mSeekbarProgress = layout.findViewById(xId(R.id.seekbar_progress));
        m_tvCurrentTime = layout.findViewById(xId(R.id.tvCurrentTime));
        m_tvTotalTime = layout.findViewById(xId(R.id.tvTotalTime));
        m_tvTotalValue = layout.findViewById(xId(R.id.tvTotalValue));
        m_tvVideoName = layout.findViewById(xId(R.id.tvVideoName));
        m_tvVideoName.setFilterTouchesWhenObscured(true);

        mControlLayout = layout.findViewById(xId(R.id.layout_control));
        mBottomLayout = layout.findViewById(xId(R.id.layout_bottom));
        mVideoTitleLayout = layout.findViewById(xId(R.id.llVideoTitleInfo));

        mBlackLayout = layout.findViewById(xId(R.id.layout_black));
        mPromptText = layout.findViewById(xId(R.id.txt_prompt_info));

        // 显示渲染布局触摸监听(亮度、音量、进度)
        View surfaceLayout = layout.findViewById(xId(R.id.fl_SurfaceView));
        if (surfaceLayout != null) {
            surfaceLayout.setOnTouchListener(this);
        }

        mVolumeProgressBar = layout.findViewById(xId(R.id.pbVolume));
        m_ivVolumeIcon = layout.findViewById(xId(R.id.ivVolumeIcon));
        m_tvVolumeValue = layout.findViewById(xId(R.id.tvVolumeValue));
        mVolumeLayout = layout.findViewById(xId(R.id.layoutVolume));
        mBrightnessProgressBar = layout.findViewById(xId(R.id.pbBrightness));
        m_tvBrightnessValue = layout.findViewById(xId(R.id.tvBrightnessValue));
        mBrightnessLayout = layout.findViewById(xId(R.id.layoutBrightness));
        mSeekTimeLayout = layout.findViewById(xId(R.id.layoutSeekTime));
        m_tvNewTime = layout.findViewById(xId(R.id.tvNewTime));
        m_tvDelayTime = layout.findViewById(xId(R.id.tvDelayTime));

        if (mVolumeProgressBar != null) {
            mVolumeProgressBar.setMax(sMusicStreamMaxVolume);
        }

        if (mBrightnessProgressBar != null) {
            mBrightnessProgressBar.setMax(BrightnessMaxValue);
        }

        initSeekbarProgress();
        adjustLayoutByStatusBar();
        adjustLayoutBySplitScreen();
    }

    private void initSeekbarProgress() {
        mSeekbarProgress.setMax(mSeekBarMaxValue);
        mSeekbarProgress.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mSeekbarOperate = false;
                mUserDelayHandler.removeMessages(UserDelayHandler.EVENT_SEEK_TO_TIME);

                int progress = seekBar.getProgress();
                if (progress < 100) {
                    progress = progress + 1;
                }

                float percentage = progress * 1.0f / mSeekBarMaxValue;
                int time = (int) (mAppData.mPlayTimeInfo.mTotalTime * percentage);
                mVideoViewModel.playerRelay().accept(
                        t -> t.requestExecuteAction(
                                IMediaAction.seekToTime, time, null));
                mVideoViewModel.playerRelay().accept(
                        BaseViewModel.IPlayer::requestShouldPlayEvent);

                onHideSeekTimeLayout();
                mUserHandler.postDelayed(mHideRunnable, 5000);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                mSeekbarOperate = true;

                mNewPlayTime = mAppData.mPlayTimeInfo.mCurrentTime;
                mLastPlayTime = mAppData.mPlayTimeInfo.mCurrentTime;

                mUserHandler.removeCallbacksAndMessages(null);
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (mSeekbarOperate) {
                    float percentage = progress * 1.0f / mSeekBarMaxValue;
                    mNewPlayTime = (int) (mAppData.mPlayTimeInfo.mTotalTime * percentage);

                    int targetTime = mNewPlayTime / 1000;
                    int deltaTime = (mNewPlayTime - mLastPlayTime) / 1000;
                    onShowSeekTimeLayout(targetTime, deltaTime);

                    // [SeekBar 的拖动点击处理]
                    if (!mUserDelayHandler.hasMessages(UserDelayHandler.EVENT_SEEK_TO_TIME)) {
                        mUserDelayHandler.sendEmptyMessageDelayed(
                                UserDelayHandler.EVENT_SEEK_TO_TIME, 1000);
                        mVideoViewModel.playerRelay().accept(
                                t -> t.requestShouldPauseEvent(false, 212));
                    }
                }
            }
        });
    }

    /**
     * 依据状态栏调整局部参数
     * <pre>
     *    不同的主题需要调整的位置不一样；
     *    这里主要调整需要配置 android:paddingTop="@*android:dimen/status_bar_height" 的元素；
     *    原因，AndroidStudio 编译的 apk 运行时系统不认识这个常量；
     * </pre>
     */
    private void adjustLayoutByStatusBar() {
        // 使用了显示过扫描配置，不需要预留状态栏高度
        if (PlatformUtils.isDisplayOverscanning()) {
            return;
        }

        int statusBarHeight = MiscUtils.statusBarHeight(mContext, R.dimen.status_bar_height);
        switch (Argument.E_THEME_GOD) {
            case ThemeX.ET_GOD_400:
            case ThemeX.ET_GOD_403:
            case ThemeX.ET_GOD_405:
            case ThemeX.ET_GOD_501:
            case ThemeX.ET_GOD_600:
                if (mBottomLayout != null) {
                    // 分屏显示调整(横屏分屏的时候不使用 status_bar_height)
                    mBottomLayout.setPadding(0, statusBarHeight, 0, 0);
                }
                break;
            case ThemeX.ET_GOD_204:
            case ThemeX.ET_GOD_206:
            case ThemeX.ET_GOD_401:
                if (mVideoTitleLayout != null) {
                    mVideoTitleLayout.setPadding(0, statusBarHeight, 0, 0);
                }
                break;
            default:
                break;
        }
    }

    /**
     * 根据分屏与否情况调整显示布局
     * <p> 例如：分屏的时候不显示进度条等
     */
    private void adjustLayoutBySplitScreen() {
        // 判断是分屏状态
        boolean isInSplitScreenMode = false;
        Configuration configuration = getResources().getConfiguration();
        if (configuration != null) {
            String configText = configuration.toString();
            if (!TextUtils.isEmpty(configText)) {
                boolean isContainsMultiWindow = Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU && configText.contains("mWindowingMode=multi-window");
                if (requireActivity().isInMultiWindowMode()
                        && (Build.VERSION.SDK_INT < Build.VERSION_CODES.P
                        || configText.contains("mWindowingMode=split-screen") || isContainsMultiWindow)) {
                    isInSplitScreenMode = true;
                }
            }
        }

        // 分屏是否要隐藏播放进度条（显示体验）
        if (mSplitScreenHideProgressLayout) {
            if (mLayoutProgress != null) {
                mLayoutProgress.setVisibility(
                        isInSplitScreenMode? View.GONE: View.VISIBLE);
            }
        } else {
            if (mLayoutProgress != null) {
                mLayoutProgress.setVisibility(View.VISIBLE);
            }

            // 历史客户皮肤处理（显示体验）
            switch (Argument.E_THEME_GOD) {
                case ThemeX.ET_GOD_206:
                    m_tvTotalTime.setVisibility(isInSplitScreenMode? View.GONE: View.VISIBLE);
                    m_tvCurrentTime.setVisibility(isInSplitScreenMode? View.GONE: View.VISIBLE);
                    mButt[BTN_ID.LIST].setVisibility(isInSplitScreenMode? View.GONE: View.VISIBLE);
                    break;
                case ThemeX.ET_GOD_NONE:
                default:
                    break;
            }
        }
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        // 如果是竖屏显示窗口
        if (isOrientation(Configuration.ORIENTATION_PORTRAIT)) {
            // 视频尺寸按钮和播放模式按钮不能在竖屏共存
            if (mButt[BTN_ID.SCALE_MODE] != null && !mIsKeepRepeatMode) {
                // 隐藏播放模式按钮
                if (mButt[BTN_ID.REPEAT_MODE] != null) {
                    mButt[BTN_ID.REPEAT_MODE].setVisibility(View.GONE);

                    View enclosureView = requireView()
                            .findViewById(xId(R.id.btnRepeatMode_Enclosure));
                    if (enclosureView != null) {
                        enclosureView.setVisibility(View.GONE);
                    }
                }
            }

            // 隐藏播放音响按钮
            if (mButt[BTN_ID.EQ] != null) {
                mButt[BTN_ID.EQ].setVisibility(View.GONE);

                View enclosureView = requireView()
                        .findViewById(xId(R.id.btnEQ_Enclosure));
                if (enclosureView != null) {
                    enclosureView.setVisibility(View.GONE);
                }
            }
        }
    }

    /** 页面后台处理 */
    private final Runnable uninitFragment = () -> {
        if (!isResumedEx()) {
            uninitFragment(true);
        }
    };

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onResume() {
        LogUtil.i(TAG, ">>>> onResume");

        super.onResume();
        H0.removeCallbacks(uninitFragment);

        // CPU 如果小于 8 核心，我们提高主线程优的先级
        int cpuCoreNum = Runtime.getRuntime().availableProcessors();
        if (cpuCoreNum < 8 &&  mVideoViewModel.inSoftDecodingHDVideo()) {
            // 未调节过主线程优先级，可以主动调节;
            ProcessCompat.setThreadPriority(null,
                    android.os.Process.THREAD_PRIORITY_AUDIO);
        }

        // 调节当前窗口显示亮度值（注意只对当前窗口有效）
        if (mAppData.mScreenBrightness
                != WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_NONE) {
            setBrightness(mAppData.mScreenBrightness, false);
        }

        initFragment(true);
        checkAndAdjustBottomMenu();
    }

    /**
     * 插件请求任务执行入口
     * <p> 后续根据实际情况扩展实现，确保满足常用需求；
     *
     * @param method  方法类型
     * @param objects 参数集
     * @return 执行结果（具体约定）
     */
    @Override
    protected Object requestExecuteMethod_Impl(String method, Object... objects) {
        switch (method) {
            case "playEvent":
                onPlayEvent();
                return null;
            case "test-case":
                LogUtil.v(TAG, "requestExecuteMethod_Impl/this is a test case!");
                return null;
            default:
                break;
        }

        return super.requestExecuteMethod_Impl(method, objects);
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    /**
     * 处理页面事件
     * <p> 注意页面事件使用需要避免嵌套调用；
     *
     * @param event 事件 ID
     * @param obj1  附加数据对象 1
     * @param obj2  附加数据对象 2
     */
    @Override
    protected void onHandlePageEvent(int event, Object obj1, Object obj2) {
        super.onHandlePageEvent(event, obj1, obj2);
        switch (event) {
            case IMediaEvent.EVENT_CHANGE_FULL_SCREEN:
                if (!mAppData.mFullScreen) {
                    mUserHandler.removeCallbacksAndMessages(null);
                    mUserHandler.post(mShowRunnable);
                } else {
                    mUserHandler.removeCallbacksAndMessages(null);
                    mUserHandler.post(mHideRunnable);
                }
                break;
            case IMediaEvent.EVENT_SPLIT_SCREEN_UPDATE_PLAY_STATE:
                onChangePlayCtrl(IMusicState.E_PLAY_STATE_STOP);
                break;
            case IMediaEvent.EVENT_NONE:
            default:
                break;
        }
    }

    /**
     * 页面显示隐藏状态改变
     * @param hidden 当前是隐藏的
     */
    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);

        boolean isPIPMode = requireActivity().isInPictureInPictureMode();
        LogUtil.i(TAG, ">>> onHiddenChanged: " + hidden + " Resumed: " + isResumed());

        if (hidden) {
            setMenuViewEnabled(false);
            onHideSurfaceViewEvent();
            restoreSystemBacklight();
        } else {
            boolean refresh = isResumed();
            if (!refresh && isPIPMode) {
                refresh = true;
            }

            onShowSurfaceViewEvent(refresh);
            if (mAppData.mScreenBrightness
                    != WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_NONE) {
                setBrightness(mAppData.mScreenBrightness, false);
            }

            // [延时菜单使能]
            mUserDelayHandler.sendEmptyMessageDelayed(
                    UserDelayHandler.EVENT_ENABLE_MENU_VIEW, 600);
        }
    }

    // [菜单使能: 避免页面切换的时候点击过快]
    private void setMenuViewEnabled(boolean enabled) {
        mUserDelayHandler.removeMessages(UserDelayHandler.EVENT_ENABLE_MENU_VIEW);

        if (null == mButt) {
            return;
        }

        for (int i = 0; i < BTN_ID.SIZE; i++) {
            if (mButt[i] != null) {
                mButt[i].setEnabled(enabled);
            }
        }
    }

    /** 显示黑色遮挡布局 */
    private void tryShowBlackLayout() {
        // 低版本先不处理（市场提出问题后再说）
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            return;
        }

        if (!ViewUtilsEx.isVisible(mBlackLayout)) {
            mBlackLayout.setVisibility(View.VISIBLE);
        }
    };

    private void setShowControlLayout(boolean bShow) {
        boolean isPIPMode = requireActivity().isInPictureInPictureMode();
        if (mControlLayout != null) {
            if (bShow) {
                if (isPIPMode) {
                    return; // 在画中画模式不操作菜单
                }

                mControlLayout.setVisibility(View.VISIBLE);
            } else {
                mControlLayout.setVisibility(View.GONE);
            }
        }
    }

    private void onChangeVideoName() {
        // 当前播放信息在播放列表有效性检查
        if (BaseMediaData.isValidIndex(
                mAppData.videoPlaylist(), mAppData.videoPlayPosition())) {
            changeTotalValue(
                    mAppData.videoPlayPosition(), mAppData.videoPlaylist().size());
            MusicInfo info = mAppData.videoPlayPositionInfo();

            if (m_tvVideoName != null) {
                m_tvVideoName.setText(info.mFileName);
            }
        }
    }

    private void changeTotalValue(int index, int total) {
        if (total > 0) {
            String text = String.format(Locale.getDefault(), "%02d/%d", index + 1, total);
            m_tvTotalValue.setText(text);
        }
    }

    private void onChangeSeekbarValue(MediaTimeInfo state) {
        int nTotalTime = state.mTotalTime / 1000;
        int nCurrentTime = state.mCurrentTime / 1000;

        onChangeSeekbarValue(nTotalTime, nCurrentTime);
    }

    @SuppressLint("SetTextI18n")
    private void onChangeSeekbarValue(int nTotalTime, int nCurrentTime) {
        if (nTotalTime > 0) {
            int value = nCurrentTime * mSeekBarMaxValue / nTotalTime;
            String totalTime = String.format(Locale.getDefault(), "%d:%02d:%02d",
                    nTotalTime / 60 / 60, nTotalTime / 60 % 60, nTotalTime % 60);
            String currentTime = String.format(Locale.getDefault(), "%d:%02d:%02d",
                    nCurrentTime / 60 / 60, nCurrentTime / 60 % 60, nCurrentTime % 60);

            m_tvCurrentTime.setText(currentTime);
            m_tvTotalTime.setText(totalTime);

            if (!mSeekbarOperate) {
                mSeekbarProgress.setProgress(value);
            }
        } else {
            m_tvCurrentTime.setText("0:00:00");
            m_tvTotalTime.setText("0:00:00");

            if (!mSeekbarOperate) {
                mSeekbarProgress.setProgress(0);
            }
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
            case R.id.btnList:
                onListEvent();
                break;
            case R.id.btnPlay:
                onPlayEvent();
                break;
            case R.id.btnPrev:
                onPrevEvent();
                break;
            case R.id.btnNext:
                onNextEvent();
                break;
            case R.id.btnRepeatMode:
                onRepeatModeEvent();
                break;
            case R.id.btnEQ:
                HFuncUtils.instance().gotoEQ(mContext);
                // 读取画中画配置，判断进入 EQ 后是否开启画中画（mcc400-mnc021 拓展需求）
                String pipConfig = "video_eq_trigger_pip_enable";
                if (xBoolean(pipConfig)) {
                    sendPageEvent(IMediaEvent.EVENT_TRIGGER_ENTER_PIP_MODE, null, null);
                }
                break;
            case R.id.btnVideoScale:
                onBtnVideoScale();
                break;
            default:
                break;
        }
    }

    private void onBtnVideoScale() {
        // 校准窗口大小
        Activity activity = getActivity();
        if (activity != null) {
            Point outSize = new Point();
            activity.getWindowManager().getDefaultDisplay().getSize(outSize);
            mAppData.mVideoUiWidth = outSize.x;
            mAppData.mVideoUiHeight = outSize.y;
            LogUtil.low_i(TAG, "----- size: " + outSize.x + " x " + outSize.y);
        }

        // 切换视频显示大小模式
        mAppData.mVideoScaleType = (mAppData.mVideoScaleType + 1) % HMediaConfig.VIDEO_SCALE_TYPE_COUNT;
        refreshBtnVideoScale();
        adjustSizeOfVideoDisplayElements(0);

        // 保存视频播放尺寸类型
        mVideoViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.writeVideoScaleType, mAppData.mVideoScaleType, null));
    }

    private void refreshBtnVideoScale() {
        if (mButt[BTN_ID.SCALE_MODE] != null) {
            if (mButt[BTN_ID.SCALE_MODE] instanceof ImageView) {
                ImageView view = (ImageView) mButt[BTN_ID.SCALE_MODE];
                view.setImageResource(
                        xDrawableId2(mScaleTypeDrawable[mAppData.mVideoScaleType]));
            } else if (mButt[BTN_ID.SCALE_MODE] instanceof Button) {
                mButt[BTN_ID.SCALE_MODE].setBackgroundResource(
                        xDrawableId2(mScaleTypeDrawable[mAppData.mVideoScaleType]));
            }
        }
    }

    /** 系统平台播放组件是否有效 **/
    private boolean isMediaPlayerValid() {
        final boolean[] isMediaPlayerValid = {false};
        mVideoViewModel.playerRelay().accept(
                t -> isMediaPlayerValid[0] =
                        t.requestQueryState(IMediaAction.isMediaPlayerValid, null));
        return isMediaPlayerValid[0];
    }

    /** Vitamio 播放组件是否有效 **/
    private boolean isVitamioPlayerValid() {
        final boolean[] isVitamioPlayerValid = {false};
        mVideoViewModel.playerRelay().accept(
                t -> isVitamioPlayerValid[0] =
                        t.requestQueryState(IMediaAction.isVitamioPlayerValid, null));
        return isVitamioPlayerValid[0];
    }

    /**
     * 调整与视频相关的显示元素大小
     * <pre>
     *    1、调整视频显示 Surface 的尺寸；
     *    2、调整视频播放菜单显示相关元素显隐；
     * </pre>
     *
     * @param reason 调用原因
     */
    private void adjustSizeOfVideoDisplayElements(int reason) {
        // 调整 SurfaceViewFragment 显示
        if (isMediaPlayerValid()) {
            if (null != mSurfaceViewFragment) {
                ((SurfaceViewFragment)
                        mSurfaceViewFragment).setVideoSurfaceSize(reason);
            }
        }

        // 调整 SurfaceViewFragmentEx 显示
        if (isVitamioPlayerValid()) {
            if (null != mSurfaceViewFragmentEx) {
                ((SurfaceViewFragmentEx)
                        mSurfaceViewFragmentEx).setVideoSurfaceSize(reason);
            }
        }

        // 这个手动调整布局好像没什么意义（多余）
        int width = mAppData.mVideoUiWidth;
        int height = mAppData.mVideoUiHeight;
        if (null != mBlackLayout) {
            ViewGroup.LayoutParams layoutParams = mBlackLayout.getLayoutParams();
            if (layoutParams.width != width
                    || layoutParams.height != height) {
                layoutParams.width = width;
                layoutParams.height = height;
                mBlackLayout.setLayoutParams(layoutParams);
            }
        }

        // EVENT_CONFIGURATION_CHANGED_SIZE
        // 横竖屏元素显示差异 （竖屏不显示部分按钮）
        if (reason == 2) {
            checkAndAdjustBottomMenu(width, height);
        }
    }

    /**
     * 检查并调整底部播放菜单显示显隐
     * @see #checkAndAdjustBottomMenu(int, int);
     */
    private void checkAndAdjustBottomMenu() {
        int width = mAppData.mVideoUiWidth;
        int height = mAppData.mVideoUiHeight;

        checkAndAdjustBottomMenu(width, height);
    }

    /**
     * 检查并调整底部播放菜单
     *
     * @param width 视频窗口宽度
     * @param height 视频窗口高度
     */
    private void checkAndAdjustBottomMenu(int width, int height) {
        boolean landscape = width > height;

        // 部分历史出货皮肤没有 SCALE_MODE 按钮；
        if (mButt[BTN_ID.SCALE_MODE] != null) {
            // 如果存在 SCALE_MODE 按钮，就隐藏显示 REPEAT_MODE 按钮；
            if (mButt[BTN_ID.REPEAT_MODE] != null) {
                mButt[BTN_ID.REPEAT_MODE].setVisibility((landscape || mIsKeepRepeatMode) ? View.VISIBLE : View.GONE);
            }
        } else {
            // TODO
            // 没有 SCALE_MODE 按钮的皮肤，就不再隐藏 SCALE_MODE 按钮；
        }

        // EQ 菜单显示按钮（竖屏不显示）
        if (mButt[BTN_ID.EQ] != null) {
            mButt[BTN_ID.EQ].setVisibility(landscape ? View.VISIBLE : View.GONE);
            View enclosureView = requireView().findViewById(xId(R.id.btnEQ_Enclosure));
            if (enclosureView != null) {
                enclosureView.setVisibility(landscape ? View.VISIBLE : View.GONE);
            }
        }

        Feature mFeature = Feature.instance();
        if(mFeature.hasFeature(REMOTE_CONTROL_FOCUS)){
            if (mButt[BTN_ID.EQ] != null) {
                mButt[BTN_ID.EQ].setVisibility(View.GONE);
            }
        }

        // 处理视频自由窗口模式菜单
        adjustBottomMenuLayoutByWindowingMode();

        // 调整显示布局
        adjustLayoutByStatusBar();
        adjustLayoutBySplitScreen();
    }

    /**
     * 根据当前窗口模式调整播放菜单显示布局
     * <pre>
     *    自由窗口只有 Android P 开始才支持
     *    这段代码应该转移到皮肤包中才合适（避免混用）；
     * </pre>
     */
    private void adjustBottomMenuLayoutByWindowingMode() {
        if (!ThemeEx.videoSupportFreeFormWindowingMode()) {
            return;
        }

        boolean isFreeformMode = false;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P) {
            isFreeformMode = requestUiModel()
                    .isVideoWindowingMode(WindowConfiguration.WINDOWING_MODE_FREEFORM);
        }

        // 具体业务逻辑由皮肤包去实现
        if (mPageExtend != null) {
            String result = mPageExtend.tryCallMethod(
                    "adjustBottomMenuLayout", isFreeformMode);
            LogUtil.v(TAG, "tryCallMethod/adjustBottomMenuLayout: " + result);
            onHideSeekTimeLayout();
            onHideVolumeLayout();
            onHideBrightnessLayout();
            return;
        }

        // 默认调试效果使用（实际皮肤包已经实现扩展逻辑接口）
        if (Argument.isThemeX(ThemeX.ET_GOD_400_100)) {
            // EQ 菜单显示按钮（小窗口不显示）
            if (mButt[BTN_ID.EQ] != null) {
                mButt[BTN_ID.EQ].setVisibility(isFreeformMode ? View.GONE : View.VISIBLE);
            }

            // 列表菜单显示按钮（小窗口不显示）
            if (mButt[BTN_ID.LIST] != null) {
                mButt[BTN_ID.LIST].setVisibility(isFreeformMode ? View.GONE : View.VISIBLE);
            }

            // 视频底部播放菜单布局
            LinearLayout bottomMenuLayout = (LinearLayout) findViewByName("bottom_menu_layout");
            if (!Objects.isNull(bottomMenuLayout)) {
                if (mButt[BTN_ID.SCALE_MODE] != null) {
                    bottomMenuLayout.removeView(mButt[BTN_ID.SCALE_MODE]);
                    if (isFreeformMode) {
                        if (mButt[BTN_ID.NEXT] != null) {
                            int index = bottomMenuLayout.indexOfChild(mButt[BTN_ID.NEXT]);
                            bottomMenuLayout.addView(mButt[BTN_ID.SCALE_MODE], index + 1);
                        }
                    } else {
                        if (mButt[BTN_ID.REPEAT_MODE] != null) {
                            int index = bottomMenuLayout.indexOfChild(mButt[BTN_ID.REPEAT_MODE]);
                            bottomMenuLayout.addView(mButt[BTN_ID.SCALE_MODE], index + 1);
                        }
                    }
                }
            }
        }
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onMultiWindowModeChanged(boolean isInMultiWindowMode) {
        super.onMultiWindowModeChanged(isInMultiWindowMode);

        // [退出多窗口时候系统回调状态比 onConfigurationChanged 慢]
        if (isInMultiWindowMode) {
            return;
        }

        adjustLayoutBySplitScreen();
    }

    /**
     * 设置视频显示视图布局
     * <p> 暂时只有在画中画的时候才允许调用；
     *
     * @param w 视频窗口宽度
     * @param h 视频窗口高度
     */
    private void setSurfaceViewLayout(int w, int h) {
        boolean isInPictureInPictureMode
                = requireActivity().isInPictureInPictureMode();
        if (!isInPictureInPictureMode) {
            return;
        }

        if (null != mSurfaceViewFragment) {
            ((SurfaceViewFragment) mSurfaceViewFragment).setSurfaceViewLayout(w, h);
        }

        if (null != mSurfaceViewFragmentEx) {
            ((SurfaceViewFragmentEx) mSurfaceViewFragmentEx).setSurfaceViewLayout(w, h);
        }

        if (null != mBlackLayout) {
            ViewGroup.LayoutParams layoutParams = mBlackLayout.getLayoutParams();
            if (layoutParams.width != w || layoutParams.height != h) {
                layoutParams.width = w;
                layoutParams.height = h;
                mBlackLayout.setLayoutParams(layoutParams);
            }
        }

        if (null != mControlLayout) {
            mControlLayout.setVisibility(View.GONE);
        }
    }

    private void onListEvent() {
        tryShowBlackLayout();
        mVideoViewModel.fragment2MainUi().execute(
                t -> t.onEvent(IMediaEvent.EVENT_GOTO_MUSIC_LIST_PAGE, null, null));
    }

    // [此函数按钮触发才会执行]
    private void onPlayEvent() {
        mUserHandler.removeCallbacksAndMessages(null);
        mUserHandler.post(mShowRunnable);

        if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PAUSE)) {
            mVideoViewModel.playerRelay().accept(
                    t -> t.requestPlayControl(IMusicState.PLAY_CMD_PLAY));
        } else {
            mVideoViewModel.playerRelay().accept(
                    t -> t.requestPlayControl(IMusicState.PLAY_CMD_PAUSE));
        }
    }

    private void onPrevEvent() {
        if (mUserDelayHandler.hasMessages(UserDelayHandler.EVENT_BUTT_PREV_FILTER)
                || mUserDelayHandler.hasMessages(UserDelayHandler.EVENT_BUTT_NEXT_FILTER)) {
            return;
        } else {
            mUserDelayHandler.sendEmptyMessageDelayed(
                    UserDelayHandler.EVENT_BUTT_PREV_FILTER, 600);
        }

        mUserHandler.removeCallbacksAndMessages(null);
        mUserHandler.post(mShowRunnable);

        mVideoViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.setSeekTimeZero, null, null));
        mVideoViewModel.playerRelay().accept(
                t -> t.requestPlayControl(IMusicState.PLAY_CMD_PREV));
    }

    private void onNextEvent() {
        if (mUserDelayHandler.hasMessages(UserDelayHandler.EVENT_BUTT_PREV_FILTER)
                || mUserDelayHandler.hasMessages(UserDelayHandler.EVENT_BUTT_NEXT_FILTER)) {
            return;
        } else {
            mUserDelayHandler.sendEmptyMessageDelayed(
                    UserDelayHandler.EVENT_BUTT_NEXT_FILTER, 600);
        }

        mUserHandler.removeCallbacksAndMessages(null);
        mUserHandler.post(mShowRunnable);

        mVideoViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.setSeekTimeZero, null, null));
        mVideoViewModel.playerRelay().accept(
                t -> t.requestPlayControl(IMusicState.PLAY_CMD_NEXT));
    }

    /**
     * 改变当前视频播放模式
     * <pre>
     *    这里的做法是有问题的，数据对象改变应该放到业务逻辑去，而不是在 UI；
     *    后续可以考虑统一调整（把全局数据对象的变更放到 Model 中去执行）；
     * </pre>
     */
    private void onRepeatModeEvent() {
        mUserHandler.removeCallbacksAndMessages(null);
        mUserHandler.post(mShowRunnable);

        mVideoViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.switchPlayRepeatMode, null, null));
        onChangeRepeatModeCtrl(mAppData.videoRepeatMode());
    }

    private void onChangePlayCtrl(int playState) {
        int nResId = 0;
        int nLanguageId = 0;

        switch (playState) {
            case IMusicState.E_PLAY_STATE_PAUSE:
            case IMusicState.E_PLAY_STATE_STOP:
                nResId = R.drawable.btn_video_play_bg;
                break;
            case IMusicState.E_PLAY_STATE_PLAY:
                nResId = R.drawable.btn_video_pause_bg;
                break;
            default:
                break;
        }

        if (mButt[BTN_ID.PLAY_PAUSE] != null) {
            if (mButt[BTN_ID.PLAY_PAUSE] instanceof ImageView) {
                ((ImageView) mButt[BTN_ID.PLAY_PAUSE]).setImageResource(xDrawableId2(nResId));
            } else if (mButt[BTN_ID.PLAY_PAUSE] instanceof Button) {
                mButt[BTN_ID.PLAY_PAUSE].setBackgroundResource(xDrawableId2(nResId));
            }
        }
    }

    private void onChangeRepeatModeCtrl(int playState) {
        int nResId = 0;
        int nLanguageId = 0;

        switch (playState) {
            case IMusicState.REPEAT_MODE_QUEUE:
                nResId = R.drawable.btn_video_repeat_queue_bg;
                break;
            case IMusicState.REPEAT_MODE_ALL:
                nResId = R.drawable.btn_video_repeat_all_bg;
                break;
            case IMusicState.REPEAT_MODE_ONE:
                nResId = R.drawable.btn_video_repeat_one_bg;
                break;
            case IMusicState.REPEAT_MODE_RANDOM:
                nResId = R.drawable.btn_video_repeat_random_bg;
                break;
            default:
                break;
        }

        if (mButt[BTN_ID.REPEAT_MODE] != null) {
            if (mButt[BTN_ID.REPEAT_MODE] instanceof ImageView) {
                ImageView view = (ImageView) mButt[BTN_ID.REPEAT_MODE];
                view.setImageResource(xDrawableId2(nResId));
            } else if (mButt[BTN_ID.REPEAT_MODE] instanceof Button) {
                mButt[BTN_ID.REPEAT_MODE].setBackgroundResource(xDrawableId2(nResId));
            }
        }
    }

    // [滑动播放界面的手势处理]
    private void onSeekToTimeEvent(int time) {
        mNewPlayTime = mNewPlayTime + time * 1000;

        if (mNewPlayTime < 0) {
            mNewPlayTime = 0;
        } else if (mNewPlayTime > mAppData.mPlayTimeInfo.mTotalTime) {
            mNewPlayTime = mAppData.mPlayTimeInfo.mTotalTime;
        }

        int targetTime = mNewPlayTime / 1000;
        int deltaTime = (mNewPlayTime - mLastPlayTime) / 1000;
        int totalTime = mAppData.mPlayTimeInfo.mTotalTime / 1000;

        onShowSeekTimeLayout(targetTime, deltaTime);
        onChangeSeekbarValue(totalTime, targetTime);

        if (!mUserDelayHandler.hasMessages(UserDelayHandler.EVENT_SEEK_TO_TIME)) {
            // [MOVING]真正的功能设置需要延时处理
            mUserDelayHandler.sendEmptyMessageDelayed(
                    UserDelayHandler.EVENT_SEEK_TO_TIME, 1000);

            mVideoViewModel.playerRelay().accept(
                    t -> t.requestShouldPauseEvent(false, 211));
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        int moveX = 0;
        int moveY = 0;
        int moveOffsetX = 0;
        int moveOffsetY = 0;
        int centerPosX = mAppData.mVideoUiWidth / 2;

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN: {
                mSeekbarOperate = false;
                mCanOperate = false;

                mOperateType = OPERATE_TYPE_NULL;
                mLastPlayTime = mAppData.mPlayTimeInfo.mCurrentTime;
                mNewPlayTime = mAppData.mPlayTimeInfo.mCurrentTime;

                mLastPosX = (int) event.getX();
                mLastPosY = (int) event.getY();
                mLastMoveY = (int) event.getY();
                mLastMoveX = (int) event.getX();

                if (mLastPosY > 70) {
                    // 去除向下滑动状态栏动作
                    mCanOperate = true;
                }

                mUserHandler.removeCallbacksAndMessages(null);
                break;
            }
            case MotionEvent.ACTION_MOVE: {
                if (!mCanOperate) {
                    return true;
                }

                moveX = (int) (event.getX() - mLastPosX);
                moveY = (int) (event.getY() - mLastPosY);

                moveOffsetX = (int) (event.getX() - mLastMoveX);
                moveOffsetY = (int) (event.getY() - mLastMoveY);

                switch (mOperateType) {
                    case OPERATE_TYPE_NULL: {
                        if (Math.abs(moveX) > 40 && Math.abs(moveY) > 40) {
                            mOperateType = OPERATE_TYPE_INVALID;
                        } else if (Math.abs(moveX) > Math.abs(moveY)
                                && Math.abs(moveX) > MOVE_PIXEL_MAX_X) {
                            mOperateType = OPERATE_TYPE_X;
                        } else if (Math.abs(moveX) < Math.abs(moveY)
                                && Math.abs(moveY) > MOVE_PIXEL_MAX_Y) {
                            mOperateType = OPERATE_TYPE_Y;
                        }
                        break;
                    }
                    case OPERATE_TYPE_X: {
                        if (Math.abs(moveX) > MOVE_PIXEL_X) {
                            if (Math.abs(moveOffsetX) > MOVE_PIXEL_X) {
                                mLastMoveX = (int) event.getX();

                                // 左右滑动触发播放进度定位
                                if (moveOffsetX < 0) {
                                    onSeekToTimeEvent(-1);
                                } else {
                                    onSeekToTimeEvent(1);
                                }
                            }
                        }
                        break;
                    }
                    case OPERATE_TYPE_Y: {
                        if (Math.abs(moveY) > MOVE_PIXEL_Y) {
                            if (Math.abs(moveOffsetY) > MOVE_PIXEL_Y) {
                                mLastMoveY = (int) event.getY();
                                if (moveOffsetY < 0) {
                                    if (mLastPosX < centerPosX) {
                                        onShowBrightnessLayout();
                                        setBrightness(mAppData.mScreenBrightness, true);
                                        mAppData.mScreenBrightness = Math.min(
                                                mAppData.mScreenBrightness + 2, BrightnessMaxValue);
                                    } else {
                                        setVol(getVol() + 1);
                                    }
                                } else {
                                    if (mLastPosX < centerPosX) {
                                        onShowBrightnessLayout();
                                        setBrightness(mAppData.mScreenBrightness, true);
                                        mAppData.mScreenBrightness = Math.max(
                                                mAppData.mScreenBrightness - 2, 0);
                                    } else {
                                        setVol(getVol() - 1);
                                    }
                                }
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }

            case MotionEvent.ACTION_UP: {
                onHideBrightnessLayout();
                onHideVolumeLayout();
                onHideSeekTimeLayout();

                if (!mCanOperate) {
                    if (mAppData.mFullScreen) {
                        sendPageEvent(IMediaEvent.EVENT_CHANGE_FULL_SCREEN, null, null);
                    }

                    return true;
                }

                switch (mOperateType) {
                    case OPERATE_TYPE_NULL: {
                        sendPageEvent(IMediaEvent.EVENT_CHANGE_FULL_SCREEN, null, null);
                        break;
                    }
                    case OPERATE_TYPE_X: {
                        // [松手直接设置生效]
                        mUserDelayHandler.removeMessages(UserDelayHandler.EVENT_SEEK_TO_TIME);
                        mVideoViewModel.playerRelay().accept(
                                t -> t.requestExecuteAction(
                                        IMediaAction.seekToTime, mNewPlayTime, null));

                        // [滑动后不一定会触发播放]
                        mVideoViewModel.playerRelay().accept(
                                BaseViewModel.IPlayer::requestShouldPlayEvent);
                        break;
                    }
                    default:
                        break;
                }

                mUserHandler.postDelayed(mHideRunnable, 5000);
                break;
            }
            default:
                break;
        }

        return true;
    }

    private void onShowBrightnessLayout() {
        //首次调节时默认最大亮度
        if (mAppData.mScreenBrightness == WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_NONE) {
            mAppData.mScreenBrightness = BrightnessMaxValue;
        }

        if (mBrightnessLayout != null) {
            mBrightnessLayout.setVisibility(View.VISIBLE);
        }

        if (mBrightnessProgressBar != null) {
            mBrightnessProgressBar.setProgress(mAppData.mScreenBrightness);
        }

        if (m_tvBrightnessValue != null) {
            m_tvBrightnessValue.setText(String.valueOf(mAppData.mScreenBrightness));
        }
    }

    private void onHideBrightnessLayout() {
        if (mBrightnessLayout != null) {
            mBrightnessLayout.setVisibility(View.GONE);
        }
    }

    private void onShowVolumeLayout(int value) {
        if (mVolumeLayout != null) {
            mVolumeLayout.setVisibility(View.VISIBLE);
        }

        if (mVolumeProgressBar != null) {
            mVolumeProgressBar.setProgress(value);
        }

        if (m_tvVolumeValue != null) {
            m_tvVolumeValue.setText(String.valueOf(value));
        }

        if (m_ivVolumeIcon != null) {
            int res = value > 0 ? R.drawable.icon_volume : R.drawable.icon_volume_mute;
            m_ivVolumeIcon.setBackgroundResource(xDrawableId2(res));
        }
    }

    private void onHideVolumeLayout() {
        if (mVolumeLayout != null) {
            mVolumeLayout.setVisibility(View.GONE);
        }
    }


    /**
     * 更新显示播放事件进度相关视图信息
     * <p> 这个函数名字和参数名取的就很奇怪；
     * @param newTime 为需要 Seek 的目标时间;
     * @param delayTime 为需要 Seek 的时间差;
     */
    private void onShowSeekTimeLayout(int newTime, int delayTime) {
        if (mSeekTimeLayout != null) {
            mSeekTimeLayout.setVisibility(View.VISIBLE);
        }

        String[] seekTime = computeSeekBarDelayTime(newTime, delayTime);

        if (m_tvNewTime != null) {
            m_tvNewTime.setText(seekTime[0]);
        }

        if (m_tvDelayTime != null) {
            m_tvDelayTime.setText(seekTime[1]);
        }
    }

    private void onHideSeekTimeLayout() {
        if (mSeekTimeLayout != null) {
            mSeekTimeLayout.setVisibility(View.GONE);
        }
    }

    private int getVol() {
        int volume = 0;
        if (null != mAudioManager) {
            volume = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
        }
        return volume;
    }

    private void setVol(int volume) {
        if (null == mAudioManager) {
            return;
        }

        if (volume > sMusicStreamMaxVolume) {
            volume = sMusicStreamMaxVolume;
        } else if (volume < 0) {
            volume = 0;
        }

        onShowVolumeLayout(volume);
        mAudioManager.setStreamVolume(
                AudioManager.STREAM_MUSIC, volume, AudioManager.FLAG_PLAY_SOUND);
    }

    /** 恢复当前系统设置的背光亮度值 **/
    private void restoreSystemBacklight() {
        if (getActivity() == null) {
            return;
        }
        WindowManager.LayoutParams lp = getActivity().getWindow().getAttributes();
        lp.screenBrightness = WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_NONE;

        getActivity().getWindow().setAttributes(lp);
    }

    private void setBrightness(int brightness, boolean save) {
        if (null == getActivity()) {
            return;
        }

        WindowManager.LayoutParams lp = getActivity().getWindow().getAttributes();

        if (brightness > BrightnessMaxValue) {
            brightness = BrightnessMaxValue;
        } else if (brightness < 0) {
            brightness = 0;
        }

        // Android 9.0 如果设置 lp.screenBrightness = 0，就是系统全局背光
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            if (brightness <= 0) {
                brightness = 1;
            }
        }

        lp.screenBrightness = brightness * 1.0f / BrightnessMaxValue;
        getActivity().getWindow().setAttributes(lp);

        if (save) {
            mAppData.mScreenBrightness = brightness;
            Preferences.writeIntToSharedPreferences(
                    requireContext(), KEY_BRIGHTNESS, mAppData.mScreenBrightness);
        }
    }

    private void onShowSurfaceViewEvent(final boolean resume) {
        int showGroupType =
                mAppData.mSoftCodeFlag ?
                        E_GROUP_SHOW_SURFACE_EX : E_GROUP_SHOW_SURFACE;

        if ((showGroupType == mShowGroupType) && !resume) {
            return;
        }

        Log.i(TAG, "onShowSurfaceViewEvent, type: " + showGroupType + ", resume: " + resume);

        mShowGroupType = showGroupType;
        MediaFragment surfaceViewFragment;
        FragmentManager fragmentManager = getChildFragmentManager();
        FragmentTransaction transaction = fragmentManager.beginTransaction();

        switch (showGroupType) {
            case E_GROUP_SHOW_SURFACE: {
                if (null == mSurfaceViewFragment) {
                    mSurfaceViewFragment = new SurfaceViewFragment(mContext, null);

                    Bundle arg = new Bundle();
                    arg.putInt("surface_type", showGroupType);
                    mSurfaceViewFragment.setArguments(arg);
                }

                surfaceViewFragment = mSurfaceViewFragment;
                break;
            }

            case E_GROUP_SHOW_SURFACE_EX: {
                if (null == mSurfaceViewFragmentEx) {
                    mSurfaceViewFragmentEx = new SurfaceViewFragmentEx(mContext, null);

                    Bundle arg = new Bundle();
                    arg.putInt("surface_type", showGroupType);
                    mSurfaceViewFragmentEx.setArguments(arg);
                }

                surfaceViewFragment = mSurfaceViewFragmentEx;
                break;
            }

            default:
                return;
        }

        // [这里不能设置 Fragment 切换动画, 因为如果设置动画会导致本该 show 出来的 Fragement
        //  会在动画结束时被 View.GONE, 参考: FragmentManager::completeShowHideFragment() ]
        // transaction.setCustomAnimations(android.R.animator.fade_in, android.R.animator.fade_out);

        switch (showGroupType) {
            case E_GROUP_SHOW_SURFACE: {
                if (null != mSurfaceViewFragmentEx) {
                    mSurfaceViewFragmentEx.uninitFragment();
                    transaction.hide(mSurfaceViewFragmentEx);
                }
                break;
            }

            case E_GROUP_SHOW_SURFACE_EX: {
                if (null != mSurfaceViewFragment) {
                    mSurfaceViewFragment.uninitFragment();
                    transaction.hide(mSurfaceViewFragment);
                }
                break;
            }

            default: {
                break;
            }
        }

        if (!surfaceViewFragment.isAdded()) {
            transaction.add(xId(R.id.fl_SurfaceView), surfaceViewFragment);
        } else {
            Log.i(TAG, "onShowSurfaceViewEvent: isAdded!");
            surfaceViewFragment.initFragment();
            transaction.show(surfaceViewFragment);
        }

        transaction.commitAllowingStateLoss();
    }

    private void onHideSurfaceViewEvent() {
        FragmentManager fragmentManager = getFragmentManager();
        assert fragmentManager != null;
        FragmentTransaction transaction = fragmentManager.beginTransaction();

        List<Fragment> listFragment = fragmentManager.getFragments();
        for (Fragment fragment : listFragment) {
            if (fragment.isVisible()) {
                transaction.hide(fragment);
            }
        }

        transaction.commitAllowingStateLoss();
    }

    @Override
    public void onPause() {
        LogUtil.i(TAG, ">>>> onPause");
        super.onPause();

        // 延时处理（针对刷新）
        H0.postDelayed(uninitFragment,60);

        // 恢复当前进程优先级
        ProcessCompat.setThreadPriority(null,
                ProcessCompat.THREAD_PRIORITY_TOP_APP_BOOST);
    }

    @RequiresApi(api = Build.VERSION_CODES.Q)
    @Override
    public void onStop() {
        super.onStop();

        if (H0.hasCallbacks(uninitFragment)) {
            H0.removeCallbacks(uninitFragment);
            uninitFragment.run();
        }
    }

    /**
     * onStop 后触发调用
     * <p> 在历史版本 Android 3.0 之前，是 onPause 后触发；
     *
     * @param outState
     */
    @Override
    public void onSaveInstanceState(@NonNull Bundle outState) {
        Log.e(TAG, ">>> onSaveInstanceState().");

        // [如果 Activity 屏蔽了 onSaveInstanceState(Bundle outState), 就不会调用到这里]
        super.onSaveInstanceState(outState);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();

        // 测试接口约束（纯粹演示用）
        if (HUtilsEx.isAppDebug()) {
            // 演示方法禁止放到初始化显示流程上（避免性能影响）
            Object result = requestExecuteMethod("test-none");
            if (result instanceof String) {
                LogUtil.v(TAG, "onDestroyView/" +
                        "requestExecuteMethod, result = " + result);
            }
        }
    }

    @Override
    public void onDestroy() {
        LogUtil.i(TAG, ">>> onDestroy.");
        super.onDestroy();

        mShowGroupType = E_GROUP_SHOW_NULL;

        mUserHandler.removeCallbacksAndMessages(null);
        mSurfaceHandler.removeCallbacksAndMessages(null);
        mUserDelayHandler.removeCallbacksAndMessages(null);
    }

    /**
     * 更新播放资源，重新触发设置图标资源
     * <p> 用于白天黑夜切换 </>
     *
     * @see #onUpdateUiModeView(boolean)
     */
    protected void updatePlayerResource() {
        onChangePlayCtrl(mAppData.mMediaPlayState);
        onChangeRepeatModeCtrl(mAppData.videoRepeatMode());
    }

    private class UserDelayHandler extends Handler {
        private static final int EVENT_SEEK_TO_TIME = 1;
        private static final int EVENT_ENABLE_MENU_VIEW = 2;
        private static final int EVENT_BUTT_NEXT_FILTER = 4;
        private static final int EVENT_BUTT_PREV_FILTER = 5;

        public UserDelayHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SEEK_TO_TIME: {
                    mVideoViewModel.playerRelay().accept(
                            t -> t.requestExecuteAction(
                                    IMediaAction.seekToTime, mNewPlayTime, null));
                    break;
                }
                case EVENT_ENABLE_MENU_VIEW: {
                    if (isVisible()) {
                        setMenuViewEnabled(true);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}
