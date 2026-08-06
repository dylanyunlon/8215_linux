package com.hcn.media.main.base;

import static androidx.lifecycle.ViewModelProvider.*;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.Point;
import android.hardware.display.DisplayManager;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.lifecycle.ViewModelProvider;
import androidx.lifecycle.ViewModelProvider.AndroidViewModelFactory;

import com.hcn.auto_compat.app.WindowConfiguration;
import com.hcn.common.misc.LogUtils;
import com.hcn.common.widget.HBarUtils;
import com.hcn.config.Feature;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.media.vm.base.VmCommand;
import com.hcn.media_base.fragment.IPageEventListener;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_model.base.ui.BaseMediaActivity;
import com.hcn.media_theme.Argument;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.fragment.IVideoPage;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media.main.observer.SettingsKeyObserver;
import com.hcn.media.main.VideoUI;
import com.hcn.media.video.RearDispPresentation;
import com.hcn.media.vm.VideoViewModel;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media_theme.ThemeX;
import com.hcn.skinx.SkinX;

import io.reactivex.rxjava3.functions.Consumer;

/**
 * 视频 Activity 基类
 * <p> 现阶段简介代码使用，提取部分共性的代码到此处，为后续扩展前装使用；
 *
 * @author 65821
 */
public abstract class BaseVideoActivity
        extends BaseMediaActivity implements IVideoPage {
    protected static final String TAG = VideoUI.class.getSimpleName();

    /**
     * 视频视图模型
     * <p> 现阶段只是做时间数据交互的桥梁使用，扩展中...
     */
    protected VideoViewModel mViewModel = null;

    /**
     * 特定的配置改变场景
     * <pre>
     *    可以用来处理特定的需求;
     *    例如：可能是主界面侧窗模式滑动隐藏视频显示；
     * </pre>
     */
    private int mSpecialConfigChangedScene = 0;

    /**
     * 系统相关服务定义
     * <p> Audio/Display/Window/...
     */
    protected AudioManager mAudioManager = null;
    protected DisplayManager mDisplayManager;

    /**
     * 双屏异显示功能
     * <p> 只有视频 SurfaceView 能输出到副屏幕显示；
     */
    protected boolean mHasInitPresentation = false;
    protected RearDispPresentation mRearDispPresentation = null;

    /**
     * 设置改变监听观察者
     * <p> 默认不初始化，谁用谁自己初始化/注册；
     */
    protected SettingsKeyObserver mObserver;

    /**
     * 默认无参构造函数
     * <p> Activity 必须是无参构造函数；
     */
    public BaseVideoActivity() {
        super();

        // 主题映射关系
        calibrationEThemeMap();
    }

    /**
     * [UI 配置的映射]
     * <pre>
     *    尽可能复用现有的类，不要搞新的重复的东西；
     *    历史主题机制和扩展皮肤包兼容的时候，先检查是否存在扩展皮肤包；
     *    既要展望未来，也要正视历史（兼容旧的主题设计）；
     * </pre>
     */
    private void calibrationEThemeMap() {
        // 计算主题权重
        E_THEME_TYPE = Argument.E_THEME_GOD + Argument.E_THEME_SUB;
        if (!Argument.isThemeSub(0)) {
            E_THEME_TYPE = Argument.E_THEME_GOD * 1000 + Argument.E_THEME_SUB;
        }
    }

    /**
     * 结束当前 Activity
     * <p> 如果要销毁当前 Activity，建议统一调用该函数。
     *
     * @param reason 结束原因
     */
    protected void videoUiFinish(int reason) {
        LogUtil.i(TAG, "videoUiFinish: reason = " + reason);
        finishReason(reason);
        finish();
    }

    /**
     * 初始化窗口标志位，显示状态
     */
    protected void initWindowFlags() {
        // [获取物理设备屏幕尺寸]
        DisplayMetrics dm = getApplicationContext().getResources().getDisplayMetrics();
        LogUtils.vTag(TAG, "    DisplayMetrics: " + dm.widthPixels + " x " + dm.heightPixels);

        // [是横屏，如果是 5.0 以上系统]
        if (dm.widthPixels > dm.heightPixels) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                Window window = getWindow();
                tryUpdateWindowFlags(window);
                window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
                window.setStatusBarColor(Color.TRANSPARENT);
            }
        }
    }

    @Override
    protected void onPreContentViewCreated() {
        // 初始化窗口属性
        initWindowFlags();
    }

    @SuppressLint("ObsoleteSdkInt")
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // [分屏切换到全屏的时候就有可能不准确]
        DisplayMetrics dm = getApplicationContext().getResources().getDisplayMetrics();
        Point outSize = new Point(dm.widthPixels, dm.heightPixels);
        getWindowManager().getDefaultDisplay().getSize(outSize);

        // [更新活动显示关联数据]
        mAppData.mVideoUiWidth = outSize.x;
        mAppData.mVideoUiHeight = outSize.y;
        mAppData.mVideoUiOrientation = getResources().getConfiguration().orientation;

        LogUtil.low_i(TAG, "    size: " + outSize.x + " x " + outSize.y);

        // [全局变量状态更新]
        mAppData.mShowToast = true;
        mAppData.mMediaType = IMusicState.MEDIA_TYPE_VIDEO;

        // [VideoViewModel]
        mViewModel = new ViewModelProvider(this,
                (Factory) new AndroidViewModelFactory(getApplication())).get(VideoViewModel.class);
        mViewModel.fragment2MainUi().observe(this,
                action -> action.exec((event, obj1, obj2) -> {
                    // 不处理音乐的页面显示事件
                    if (event == IMediaEvent.EVENT_SHOW_MUSIC_FRAGMENT) {
                        return;
                    }

                    onFragment2MainEvent(event, obj1, obj2);
                }));

        // [页面事件监听器]
        mCompositeDisposable.add(
                mViewModel.pageEventRelay().subscribe(
                        pageEventAction -> pageEventAction.exec(this::onHandlePageEvent)));

        // [标记当前活动对象]
        requestUiModel().setVideoUiActivity(this);

        // [显示设备状态监听]
        mDisplayManager = (DisplayManager) getSystemService(Context.DISPLAY_SERVICE);
        if (mDisplayManager != null) {
            mDisplayManager.registerDisplayListener(mDisplayListener, null);
        }

        // [获取声音服务对象]
        mAudioManager = (AudioManager) this.getSystemService(Context.AUDIO_SERVICE);
    }

    @Override
    public void setContentView(@LayoutRes int layoutResId) {
        super.setContentView(layoutResId);
    }

    /**
     * Fragment 抛出来的事件
     * <pre>
     *    这个事件受生命周期约束；
     *    一般用来传递用户操作点击相关事件；
     * </pre>
     *
     * @param event 事件类型
     * @param obj1 附加参数对象 1
     * @param obj2 附加参数对象 2
     */
    protected abstract void onFragment2MainEvent(int event, Object obj1, Object obj2);

    /**
     * 需要重载的页面事件接受函数
     * <pre>
     *    使用方法：{@link VideoViewModel#pageEventRelay()}
     *    方法提示：当前页面事件生命周期约束，[onStart, onStop]；
     * </pre>
     *
     * @param event 事件 ID
     * @param obj1 附加数据对象 1
     * @param obj2 附加数据对象 2
     */
    protected void onHandlePageEvent(int event, Object obj1, Object obj2) {
        // TODO: 子类要使用就重载该函数
    }

    /**
     * 监听可用显示设备的更改
     * <p> 例如：显示设备的插拔（AV-OUT/HDMI）状态改变；
     */
    private final DisplayManager.DisplayListener
            mDisplayListener = new DisplayManager.DisplayListener() {

        @Override
        public void onDisplayAdded(int displayId) {
            Log.d(TAG, "onDisplayAdded, displayId = " + displayId);

            if (!mAppData.isFrontVideo) {
                showPresentationDisplay();
            }
        }

        @Override
        public void onDisplayRemoved(int displayId) {
            Log.d(TAG, "onDisplayRemoved, displayId = " + displayId);

            // 销毁 Presentation 对象
            if (mRearDispPresentation != null) {
                Display display = mRearDispPresentation.getDisplay();
                if (display != null && display.getDisplayId() == displayId) {
                    hidePresentationDisplay();
                }
            }
        }

        @Override
        public void onDisplayChanged(int displayId) {
            Log.d(TAG, "onDisplayChanged, displayId = " + displayId);
        }
    };

    private void initRearDispPresentation() {
        if (mRearDispPresentation != null) {
            return;
        }

        // 获取当前系统支持 Presentation 的设备
        Display[] presentationDisplays = mDisplayManager.getDisplays(
                DisplayManager.DISPLAY_CATEGORY_PRESENTATION);
        boolean isSupportPresentation = (presentationDisplays.length > 0);

        if (isSupportPresentation) {
            mRearDispPresentation = new RearDispPresentation(
                    getApplicationContext(), presentationDisplays[0], mViewModel);
            int displayId = presentationDisplays[0].getDisplayId();
            Log.d(TAG, "initRearDispPresentation, DisplayId = " + displayId);
        } else {
            Log.e(TAG, "initRearDispPresentation, There's no displays connected.");
            mRearDispPresentation = null;
        }
    }

    /**
     * 本地服务是连接的
     * @return 连接的/未连接
     */
    protected boolean localServiceConnected() {
        final boolean[] isLocalConnected = {false};
        mViewModel.playerRelay().accept(
                t -> isLocalConnected[0] =
                        t.requestQueryState(IMediaAction.isLocalConnected, null));
        return isLocalConnected[0];
    }

    /**
     * 本地服务是连接的
     * @return 连接的/未连接
     */
    protected boolean isCanWatchVideo() {
        final boolean[] isCanWatchVideo = {false};
        mViewModel.playerRelay().accept(
                t -> isCanWatchVideo[0] =
                        t.requestQueryState(IMediaAction.isCanWatchVideo, null));
        return isCanWatchVideo[0];
    }

    @Override
    public void onMultiWindowModeChanged(boolean isInMultiWindowMode, Configuration newConfig) {
        super.onMultiWindowModeChanged(isInMultiWindowMode, newConfig);
        LogUtil.e(TAG, ">>> onMultiWindowModeChanged: " + isInMultiWindowMode);

        if (isInMultiWindowMode) {
            // [onMultiWindowModeChanged 可能在调用 onPause 后才执行]
            mAppData.mVideoUiShow = true;
        } else {
            // 避免后台插入 U 盘还自动播放视频;
            if (isStopped()) {
                mAppData.mVideoUiShow = false;
            }
        }

        // 获取当前页面宽高
        if (isResumedEx()) {
            Point outSize = new Point();
            getWindowManager().getDefaultDisplay().getSize(outSize);

            mAppData.mVideoUiWidth = outSize.x;
            mAppData.mVideoUiHeight = outSize.y;
            LogUtil.low_i(TAG, "    size: " + outSize.x + " x " + outSize.y);
        }
    }

    /**
     * 生命周期扩展函数
     * <p> 固定 onResume() 前调用;
     */
    @Override
    protected void onPreResume() {
        // [重置媒体类型]
        mAppData.mMediaType = IMusicState.MEDIA_TYPE_VIDEO;

        // [重置特定配置场景标记]
        mSpecialConfigChangedScene = 0;

        // [重置状态，检查视频分屏是否在副屏状态]
        requestUiModel().setVideoInSplitScreenSecondary(false);
        Configuration configuration = getResources().getConfiguration();
        if (configuration != null) {
            String configText = configuration.toString();
            LogUtil.v(TAG, "onPreResume: " + configText);

            if (!TextUtils.isEmpty(configText)) {
                if (isInMultiWindowMode()
                        && configText.contains("mWindowingMode=split-screen-secondary")) {
                    requestUiModel().setVideoInSplitScreenSecondary(true);
                }
            }

            // Android P 开始支持 WindowConfiguration 类
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                int windowingMode = WindowConfiguration.getWindowingMode(configuration);
                requestUiModel().setVideoWindowingMode(windowingMode);
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        // [重新同步窗口大小]
        Point outSize = new Point(0, 0);
        getWindowManager().getDefaultDisplay().getSize(outSize);
        mAppData.mVideoUiWidth = outSize.x;
        mAppData.mVideoUiHeight = outSize.y;
        LogUtil.d(TAG, "    size: " + outSize.x + " x " + outSize.y);
    }

    @Override
    public void onTopResumedActivityChanged(boolean isTopResumedActivity) {
        super.onTopResumedActivityChanged(isTopResumedActivity);
        LogUtils.vTag(TAG, "onTopResumedActivityChanged: " + isTopResumedActivity);

        // 音乐和视频同时分屏情况,根据最上方活动来请求播放任务.
        if (isTopResumedActivity
                && !isInPictureInPictureMode()
                && mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
            // 不考虑是否分屏(如果播放任务在音乐，视频手动滑动到全屏 onMultiWindowModeChanged -> onTopResumedActivityChanged)
            mAppData.mMediaType = IMusicState.MEDIA_TYPE_VIDEO;
            mViewModel.playerRelay().accept(
                    BaseViewModel.IPlayer::requestPlayTask);
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        int[] location = new int[2];
        getWindow().getDecorView().getLocationOnScreen(location);
        LogUtil.v(TAG, "onWindowFocusChanged["
                + hasFocus + "]: " + location[0] + "x" + location[1]);

        requestUiModel().setIsVideoUiWindowFocus(hasFocus);
    }

    /**
     * 显示双屏异显效果
     * <p> 视频界面点击 Home 按钮退到后台的时候可以调用它，实现双屏异显；
     */
    @Override
    public void showPresentationDisplay() {
        Log.i(TAG, "showPresentationDisplay.");
        Log.d(TAG, "mHasInitPresentation = " + mHasInitPresentation);

        if (mHasInitPresentation) {
            return;
        }

        Display[] presentationDisplays = mDisplayManager.getDisplays(
                DisplayManager.DISPLAY_CATEGORY_PRESENTATION);
        boolean isSupportPresentation = (presentationDisplays.length > 0);

        if (isSupportPresentation) {
            Log.d(TAG, "mRearDispPresentation = " + mRearDispPresentation);
            initRearDispPresentation();

            if (mRearDispPresentation != null) {
                mHasInitPresentation = true;
                mRearDispPresentation.show();
                Log.i(TAG, "mRearDispPresentation show.");
            } else {
                Log.e(TAG, "showPresentationDisplay fail for mRearDispPresentation not init!");
            }
        }

        // [重复的设置, 好像没什么意义]
        if ((mRearDispPresentation != null) &&
                (mRearDispPresentation.mRearVideoView != null)) {
            mRearDispPresentation.mRearVideoView.setBackgroundColor(Color.TRANSPARENT);
        }
    }

    /**
     * 关闭双屏异显
     * <p> 视频页面销毁的时候需要调用，切换到音乐的时候也需要检查调用；
     */
    @Override
    public void hidePresentationDisplay() {
        Log.d(TAG, "[hidePresentationDisplay]");

        if (mRearDispPresentation != null) {
            mRearDispPresentation.hide();
            mRearDispPresentation.dismiss();

            mRearDispPresentation = null;
            mHasInitPresentation = false;
        }
    }

    /**
     * 配置改变到物理全屏
     * <pre>
     *    由 onConfigurationChanged(...) 触发回调；
     *    由子类强制重载实现，子类可在此处理相关状态变更；
     * </pre>
     */
    protected abstract void onConfigurationToFullScreen();

    /**
     * 配置改变到视图元素
     * <pre>
     *    由 onConfigurationChanged(...) 触发回调；
     *    由子类强制重载实现，子类可在此处理视图元素更新；
     * </pre>
     */
    protected abstract void onConfigurationToViewElement();

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        LogUtil.i(TAG, ">>>> onConfigurationChanged: " + newConfig);
        super.onConfigurationChanged(newConfig);
        boolean isInPipMode = isInPictureInPictureMode();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            if (Feature.instance().hasFeature(Feature.BIT.THEME_SUPPORT_NIGHT_MODE)) {
                // 系统主题变换时，需要重新设置状态栏颜色
                tryUpdateWindowFlags(getWindow());
                getWindow().getDecorView().invalidate();
                Log.w(TAG, "onConfigurationChanged tryUpdateWindowFlags");
            }
        }

        // Android P 开始支持 WindowConfiguration 类
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            int windowingMode = WindowConfiguration.getWindowingMode(newConfig);
            requestUiModel().setVideoWindowingMode(windowingMode);
        }

        // 同步当前视频窗口大小
        int width = newConfig.screenWidthDp;
        int height = newConfig.screenHeightDp;
        DisplayMetrics dm = getApplicationContext().getResources().getDisplayMetrics();
        int statusBarHeight = MiscUtils.getStatusBarHeight(this);

        // 检查当前应用程序配置的最小宽度
        if (newConfig.smallestScreenWidthDp != Configuration.SMALLEST_SCREEN_WIDTH_DP_UNDEFINED) {
            // 如果当前 screenWidthDp 小于最小宽度, 则取最小宽度
            if (width < newConfig.smallestScreenWidthDp) {
                width = newConfig.smallestScreenWidthDp;
            }
        }

        int widthPixels = width * dm.densityDpi / DisplayMetrics.DENSITY_DEFAULT;
        int heightPixels = height * dm.densityDpi / DisplayMetrics.DENSITY_DEFAULT;

        mAppData.mVideoUiWidth = widthPixels;
        mAppData.mVideoUiHeight = heightPixels;

        // [显示由画中画切换到全屏的时候, newConfig 不带状态栏高度]
        if (Math.abs(dm.heightPixels - (heightPixels + statusBarHeight)) < 2) {
            // [这个限制条件可以确保当前配置改变是全屏]
            if (Math.abs(dm.widthPixels - widthPixels) < 2) {
                mAppData.mVideoUiHeight = dm.heightPixels;
            }
        }

        // [如果是物理全屏显示, 需要重置部分变量]
        if (dm.widthPixels == mAppData.mVideoUiWidth
                && dm.heightPixels == mAppData.mVideoUiHeight) {
            onConfigurationToFullScreen();
        }

        LogUtil.i(TAG, ">>>>> onConfigurationChanged: " +
                mAppData.mVideoUiWidth + " x " + mAppData.mVideoUiHeight);

        // 更新视图元素（例：更新当前 SurfaceView 的大小）
        onConfigurationToViewElement();

        // 横竖屏切换 Activity 需要 recreate()
        if (mAppData.mVideoUiOrientation != newConfig.orientation) {
            mAppData.mVideoUiOrientation = newConfig.orientation;

            // [多余的判断]如果是正方形的画中画, 长宽比一比一系统默认是竖屏。
            if (newConfig.screenWidthDp == newConfig.screenHeightDp) {
                LogUtil.i(TAG, ">>> <w == h> no need recreate.");
                return;
            }

            // [分屏、横竖屏变换等, 画中画触发的不用处理]
            if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
                if (!isInPipMode) {
                    // [物理分辨率如果是横屏, 不再 recreate()]
                    if (dm.widthPixels > dm.heightPixels) {
                        // 在 onPause 状态且屏幕状态改变<横竖屏变化>
                        if (!isResumedEx() && !isStopped()) {
                            // 回到物理全屏配置
                            if (dm.widthPixels == mAppData.mVideoUiWidth
                                    && dm.heightPixels == mAppData.mVideoUiHeight) {
                                mSpecialConfigChangedScene = 1;
                                Log.d(TAG, "special Config Changed Scene: " + mSpecialConfigChangedScene);
                            }
                        }

                        // 就是物理横屏情况下分屏不再处理 recreate(), 布局已经做了自适应。
                        return;
                    }

                    executeRecreateTasK();
                    LogUtil.i(TAG, "[recreate()]onConfigurationChanged: orientation!");
                }
            }
        }
    }

    @Override
    public boolean isOrientation(int orientation) {
        return mAppData.mVideoUiOrientation == orientation;
    }

    /**
     * 是分屏侧滑隐藏当前窗口
     * <p> 分屏状态下，再按 Home 进入侧边显示状态，再滑动隐藏；
     *
     * @return 是/否
     */
    protected boolean isSplitScreenSideslipHidden() {
        return 1 == mSpecialConfigChangedScene;
    }

    /**
     * 进入视频界面
     * <p> 媒体类型变化，通知音乐页面销毁；
     */
    protected void enterAndResumeVideoPlayerUI() {
        if (!Argument.isThemeGod(ThemeX.ET_GOD_402)) {
            // 先在 402 测试验证，如果没问题，后续全部放开。
            return;
        }

        HBroadcastEx.sendLocalBroadcast(this,
                IMediaEvent.EVENT_GOTO_RESUME_VIDEO_PLAYER_UI);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        // [重置全局变量状态]
        mAppData.mShowToast = false;
        mAppData.mVideoUiShow = false;
        requestUiModel().setVideoUiActivity(null);

        // [关闭双屏异显效果]
        hidePresentationDisplay();
        if (mDisplayManager != null) {
            mDisplayManager.unregisterDisplayListener(mDisplayListener);
        }
    }
}
