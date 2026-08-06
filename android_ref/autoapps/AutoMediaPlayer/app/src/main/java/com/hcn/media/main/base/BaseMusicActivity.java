package com.hcn.media.main.base;

import static androidx.lifecycle.ViewModelProvider.*;

import android.annotation.SuppressLint;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.Point;
import android.os.Build;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.view.Window;
import android.view.WindowManager;

import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;
import androidx.lifecycle.ViewModelProvider;
import androidx.lifecycle.ViewModelProvider.AndroidViewModelFactory;

import com.hcn.common.misc.LogUtils;
import com.hcn.config.Feature;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.media_model.base.ui.BaseMediaActivity;
import com.hcn.media_theme.Argument;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media_theme.ThemeEx;
import com.hcn.media_theme.ThemeX;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_base.fragment.IMusicPage;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media.main.observer.SettingsKeyObserver;
import com.hcn.media.main.MusicUI;
import com.hcn.media.vm.MusicViewModel;
import com.hcn.skinx.config.Skin;
import com.hcn.skinx.SkinX;
import com.orhanobut.logger.Logger;

/**
 * 音乐 Activity 基类
 * <p> 现阶段简介代码使用，提取部分共性的代码到此处，为后续扩展前装使用；
 *
 * @author 65821
 */
public abstract class BaseMusicActivity
        extends BaseMediaActivity implements IMusicPage {
    protected static final String TAG = MusicUI.class.getSimpleName();

    /**
     * 音乐视图模型
     * <p> 现阶段只是做时间数据交互的桥梁使用，扩展中...
     */
    protected MusicViewModel mViewModel = null;

    /**
     * 设置改变监听观察者
     * <p> 默认不初始化，谁用谁自己初始化/注册；
     */
    protected SettingsKeyObserver mObserver;

    /**
     * 默认无参构造函数
     * <p> Activity 必须是无参构造函数；
     */
    public BaseMusicActivity() {
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
        E_THEME_TYPE = Argument.getThemeX();

        // 先检查皮肤包
        final String skinType = SkinX.currentSkinType();
        Logger.t(TAG).d("calibrationEThemeMap, current SkinX type: " + skinType);
        switch (skinType) {
            case Skin.SKIN_UIS8581_BLUE01:
            case Skin.SKIN_MT8163_BLUE01_VIOLET:
                E_THEME_TYPE = ThemeX.ET_GOD_204;
                return;
            case Skin.SKIN_NONE:
            default:
                break;
        }
        
        // 映射主题类型
        switch (E_THEME_TYPE) {
            case ThemeX.ET_GOD_209:
            case ThemeX.ET_GOD_400:
            case ThemeX.ET_GOD_403:
            case ThemeX.ET_GOD_501:
            case ThemeX.ET_GOD_600:
                E_THEME_TYPE = ThemeX.ET_GOD_204;
                break;
            default:
                break;
        }

        // 支持事业部随意扩展子主题
        switch (Argument.E_THEME_GOD) {
            case ThemeX.ET_GOD_400:
            case ThemeX.ET_GOD_403:
            case ThemeX.ET_GOD_501:
            case ThemeX.ET_GOD_600:
                // 检查主题（mcc400、mcc403 都归类到 204 逻辑处理）
                E_THEME_TYPE = ThemeX.ET_GOD_204;
            default:
                break;
        }

        // 使用 support_music_info_ext 控制皮肤包支持mcc153类型的布局
        final boolean skinSupportMusicInfoExt = ThemeEx.musicSupportMusicInfoExt();
        if (skinSupportMusicInfoExt){
            E_THEME_TYPE = ThemeX.ET_GOD_154;
        }
    }

    /**
     * 结束当前 Activity
     * <p> 如果要销毁当前 Activity，建议统一调用该函数。
     *
     * @param reason 结束原因
     */
    protected void musicUiFinish(int reason) {
        LogUtil.i(TAG, "musicUiFinish: reason = " + reason);
        finishReason(reason);
        finish();
    }

    /**
     * 初始化窗口标志位，显示状态
     */
    protected void initWindowFlags() {
        // 设置透明状态栏
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Window window = getWindow();
            tryUpdateWindowFlags(window);
            LogUtils.vTag(TAG, "onCreate tryUpdateWindowFlags");
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.TRANSPARENT);
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

        // [更新活动显示关联数据]
        mAppData.mMusicUiWidth = getWindowManager().getDefaultDisplay().getWidth();
        mAppData.mMusicUiHeight = getWindowManager().getDefaultDisplay().getHeight();
        mAppData.mMusicUiOrientation = getResources().getConfiguration().orientation;

        // [获取物理设备屏幕尺寸]
        DisplayMetrics dm = getApplicationContext().getResources().getDisplayMetrics();

        // [分屏切换到全屏的时候就有可能不准确]
        Point outSize = new Point(dm.widthPixels, dm.heightPixels);
        getWindowManager().getDefaultDisplay().getSize(outSize);
        mAppData.mMusicUiWidth = outSize.x;
        mAppData.mMusicUiHeight = outSize.y;
        LogUtil.d(TAG, "    size: " + outSize.x + " x " + outSize.y);

        // [全局变量状态更新]
        mAppData.mShowToast = true;
        mAppData.mMediaType = IMusicState.MEDIA_TYPE_MUSIC;

        // [MusicViewModel]
        mViewModel = new ViewModelProvider(this,
                (Factory) new AndroidViewModelFactory(getApplication())).get(MusicViewModel.class);
        mViewModel.fragment2MainUi().observe(this,
                action -> action.exec((event, obj1, obj2) -> {
                    // 不处理视频的页面显示事件
                    if (event == IMediaEvent.EVENT_SHOW_VIDEO_FRAGMENT) {
                        return;
                    }

                    onFragment2MainEvent(event, obj1, obj2);
                }));

        // [标记当前活动对象]
        requestUiModel().setMusicUiActivity(this);
    }

    @Override
    public void setContentView(@LayoutRes int layoutResId) {
        super.setContentView(layoutResId);
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
     * 生命周期扩展函数
     * <p> 固定 onResume() 前调用;
     */
    @Override
    protected void onPreResume() {
        mAppData.mIsControlPage = true;
        mAppData.mMediaType = IMusicState.MEDIA_TYPE_MUSIC;
    }

    @Override
    protected void onResume() {
        super.onResume();

        // [重新同步窗口大小]
        Point outSize = new Point(0, 0);
        getWindowManager().getDefaultDisplay().getSize(outSize);
        mAppData.mMusicUiWidth = outSize.x;
        mAppData.mMusicUiHeight = outSize.y;
        LogUtil.d(TAG, "    size: " + outSize.x + " x " + outSize.y);
    }

    @Override
    protected void onPostResume() {
        super.onPostResume();

        // 低版本处理方式（Q 以下没有 onTopResumedActivityChanged 接口）
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            // 通知 Video Player UI 已进入显示模式
            enterAndResumeMusicPlayerUI(POST_RESUME_STATE);
        }
    }

    @Override
    public void onTopResumedActivityChanged(boolean isTopResumedActivity) {
        super.onTopResumedActivityChanged(isTopResumedActivity);
        LogUtils.vTag(TAG, "onTopResumedActivityChanged: " + isTopResumedActivity);

        if (isTopResumedActivity) {
            // 通知 Video Player UI 已进入显示模式
            enterAndResumeMusicPlayerUI(TOP_RESUMED_STATE);
        }

        // 音乐和视频同时分屏情况,根据最上方活动来请求播放任务.
        if (isTopResumedActivity
                && mAppData.isMediaType(IMusicState.MEDIA_TYPE_VIDEO)) {
            // 不考虑是否分屏(如果播放任务在视频，音乐手动滑动到全屏 onMultiWindowModeChanged -> onTopResumedActivityChanged)
            mAppData.mMediaType = IMusicState.MEDIA_TYPE_MUSIC;
            mViewModel.playerRelay().accept(
                    BaseViewModel.IPlayer::requestPlayTask);
        }
    }

    @SuppressLint("ObsoleteSdkInt")
    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        Logger.t(TAG).d("onConfigurationChanged: " + newConfig);
        super.onConfigurationChanged(newConfig);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            if (Feature.instance().hasFeature(Feature.BIT.THEME_SUPPORT_NIGHT_MODE)) {
                // 系统主题变换时，需要重新设置状态栏颜色
                tryUpdateWindowFlags(getWindow());
                getWindow().getDecorView().invalidate();
                LogUtils.vTag(TAG, "onConfigurationChanged tryUpdateWindowFlags");
            }
        }

        // 同步窗口大小
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

        mAppData.mMusicUiWidth = width;
        mAppData.mMusicUiHeight = height;

        // [显示由画中画切换到全屏的时候, newConfig 不带状态栏高度]
        if (dm.heightPixels == height + statusBarHeight) {
            // [这个限制条件可以确保当前配置改变是全屏]
            if (dm.widthPixels == width) {
                mAppData.mMusicUiHeight = dm.heightPixels;
            }
        }

        LogUtil.i(TAG, ">>>>> onConfigurationChanged: " +
                mAppData.mMusicUiWidth + " x " + mAppData.mMusicUiHeight);

        if (mAppData.mMusicUiOrientation != newConfig.orientation) {
            mAppData.mMusicUiOrientation = newConfig.orientation;
        }
    }

    @Override
    public boolean isOrientation(int orientation) {
        return mAppData.mMusicUiOrientation == orientation;
    }

    /**
     * 判定指定名称的 Fragment 是否在回退栈中
     *
     * @param key An optional name for this back stack state
     * @return 是否在回退栈中
     */
    protected boolean inFragmentBackStack(@NonNull String key) {
        int count = getSupportFragmentManager().getBackStackEntryCount();
        for (int i = 0; i < count; i++) {
            FragmentManager.BackStackEntry entry = getSupportFragmentManager().getBackStackEntryAt(i);
            String name = entry.getName();
            if (!TextUtils.isEmpty(key) && key.equals(name)) {
                return true;
            }
        }
        return false;
    }

    /**
     * 遍历回退栈信息
     * <p> 调试问题使用，用来查看回退栈状态；
     */
    protected void traversalFragmentBackStack() {
        int count = getSupportFragmentManager().getBackStackEntryCount();
        for (int i = 0; i < count; i++) {
            FragmentManager.BackStackEntry entry = getSupportFragmentManager().getBackStackEntryAt(i);
            String name = entry.getName();
            LogUtil.d(TAG, "traversalFragmentBackStack: " + name);
        }
    }

    /**
     * 进入音乐界面
     * <p> 媒体类型变化，通知视频页面销毁；
     * @param process 当前调用过程状态
     */
    protected void enterAndResumeMusicPlayerUI(final String process) {
        // 保护历史版本状态逻辑代码
        if (PRE_RESUME_STATE.equals(process)) {
            if (!Argument.isThemeGod(ThemeX.ET_GOD_402)) {
                // 先在 402 测试验证，如果没问题，后续全部放开。
                return;
            }
        }

        // 如果显示状态了，且在多窗口，不处理
        if (isInMultiWindowMode()) {
            switch (process) {
                case POST_RESUME_STATE:
                case TOP_RESUMED_STATE:
                    return;
                default:
                    break;
            }
        }

        HBroadcastEx.sendLocalBroadcast(this,
                IMediaEvent.EVENT_GOTO_RESUME_MUSIC_PLAYER_UI, process);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mAppData.mIsControlPage = false;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        // [重置全局变量状态]
        mAppData.mShowToast = false;
        requestUiModel().setMusicUiActivity(null);
    }
}
