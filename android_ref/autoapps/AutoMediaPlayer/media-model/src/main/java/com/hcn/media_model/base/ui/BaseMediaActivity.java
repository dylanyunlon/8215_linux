package com.hcn.media_model.base.ui;

import android.app.UiModeManager;
import android.content.res.Configuration;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HFileUtils;
import com.hcn.config.Feature;
import com.hcn.media_base.activity.IResume;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.ui.MediaPageState;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.media_model.MediaModel;
import com.hcn.media_model.base.IUiModel;
import com.hcn.media_theme.ThemeEx;
import com.hcn.media_theme.ThemeUtilsEx;
import com.hcn.skinx.SkinX;
import com.hcn.skinx.extend.SkinExCompatActivity;
import com.hcn.skinx_config.HFeatureKeys;

import io.reactivex.rxjava3.disposables.CompositeDisposable;

/**
 * 基类活动类
 * @author 65821
 */
public abstract class BaseMediaActivity
        extends SkinExCompatActivity implements IResume {

    private static final String TAG = BaseMediaActivity.class.getSimpleName();
    /**
     * 当前模块主题类型
     * <p> 这个表示代码最终执行的主题类型，是 MCC/MNC 主题资源代号；
     */
    protected int E_THEME_TYPE = -1;

    /**
     * 全局唯一的实例对象定义
     * <p> 例如：全局数据结构对象等；
     */
    protected AppGlobalData mAppData = null;

    /**
     * UI 是否在 onResume 状态
     * <p> [配合 onResume/onPause 一起使用]
     */
    private boolean mIsResumed = false;

    /**
     * UI 是否在 onStop 状态
     * <p> [配合 onStart/onStop 一起使用]
     */
    private boolean mIsStopped = false;

    /**
     * UI 结束（Finish）的原因
     * <p> 标记调用退出的原因，辅助调试使用；
     */
    private int mFinishReason = -1;

    /**
     * 是否触发了 recreate() 调用标记
     *
     * <pre>
     *    纯粹标记变量，用来标记事件状态;
     *    例如：避免 onDestroy() 时候强制退出进程（killProcess）；
     * </pre>
     */
    private boolean mIsExecuteRecreateTask = false;

    /**
     * UI 是否有焦点
     * <p> 音视频分屏时调整窗口大小保证焦点在当前页面上
     */
    protected boolean mIsTopResumedActivity = false;

    /**
     * 订阅资源管理器
     * <p> 管理当前所有订阅资源，只能回收释放；
     */
    protected CompositeDisposable mCompositeDisposable;

    /**
     * 是 onResume 状态
     * <pre>
     *    注意：函数名称 isResumed() 已经在 Activity 中存在；
     *    当前 Activity 是否已经执行 super.onResume() 函数；
     * </pre>
     * @return 是/否
     */
    protected boolean isResumedEx() {
        return mIsResumed;
    }

    /**
     * 是 onStop 状态
     * <p> 当前 Activity 是否已经执行 super.onStop() 函数；
     * @return 是/否
     */
    protected boolean isStopped() {
        return mIsStopped;
    }

    /**
     * 当前 Finishing 的原因；
     * @return 原因代号
     */
    protected int finishReason() {
        return mFinishReason;
    }

    /**
     * 当前活动 Finish 的原因
     * @param reason 原因代码
     */
    protected void finishReason(int reason) {
        mFinishReason = reason;
    }

    /**
     * 是在重创建中
     * <p> 是执行 recreate() 任务；
     * @return 是/否
     */
    protected boolean isRecreating() {
        return mIsExecuteRecreateTask;
    }

    /**
     * 获取 UiModel 对象
     * <p> 无生命周期约束；
     *
     * @return {@link IUiModel}
     */
    protected IUiModel requestUiModel() {
        return MediaModel.call().uiModel();
    }

    /** 用以设置 SystemUI Flag 的属性 */
    protected void tryUpdateWindowFlags(Window window) {
        window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        if (Feature.instance().hasFeature(Feature.BIT.THEME_SUPPORT_NIGHT_MODE)) {
            UiModeManager mUiModeManager = (UiModeManager) getSystemService(UI_MODE_SERVICE);
            if (mUiModeManager.getNightMode() == UiModeManager.MODE_NIGHT_YES) {
                window.getDecorView().setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_VISIBLE);
            } else {
                window.getDecorView().setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
            }
        } else {
            window.getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        }
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // [一次性可回收复合对象]
        mCompositeDisposable = new CompositeDisposable();

        // [全局组件和数据对象]
        mAppData = AppGlobalData.getInstance();

        // [重置 recreate 标记]
        mIsExecuteRecreateTask = false;

        // [设定 白天黑夜支持特性]
        if (SkinX.getBoolean(HFeatureKeys.NEED_STATUS_BAR_CHANGE)) {
            Feature.instance().addFeature(Feature.BIT.THEME_SUPPORT_NIGHT_MODE);
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        mIsStopped = false;
        mFinishReason = -1;
    }

    /**
     * 生命周期扩展函数
     * <p> 固定 onResume() 前调用;
     */
    protected abstract void onPreResume();

    /**
     * 检查并同步背景
     * <p> {@link ThemeUtilsEx} 共享背景机制；
     */
    protected abstract void checkAndSyncBackground();


    @Override
    protected void onResume() {
        onPreResume();
        super.onResume();
        mIsResumed = true;
    }

    /**
     * 判断当前 Activity 显示方向配置
     * <p> {@link #onConfigurationChanged(Configuration)}
     *
     * @param orientation 期望的方向配置
     * @return 是/否
     */
    public abstract boolean isOrientation(int orientation);

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();
        // 只有当前主题支持壁纸功能才初始化
        // 在ACC OFF 之后改变 B+/B-, ACC ON 不会触发 onConfigurationChanged
        initWallpaperConfig(true);
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // 当前主题只有深色浅色正常切换且与上次状态不同且UI模式发生改变才同步壁纸状态
        int flags = newConfig.uiMode & Configuration.UI_MODE_NIGHT_MASK;
        if (flags == Configuration.UI_MODE_NIGHT_NO || flags == Configuration.UI_MODE_NIGHT_YES) {
            initWallpaperConfig(false);
        }
    }

    @Override
    public void onTopResumedActivityChanged(boolean isTopResumedActivity) {
        super.onTopResumedActivityChanged(isTopResumedActivity);
        mIsTopResumedActivity = isTopResumedActivity;
    }

    /**
     * 执行重新创建任务
     * <pre>
     *    注意，recreate() 会调用到 onDestroy() 函数，
     *    在 onDestroy() 中需区分是否需要执行 killProcess() 动作；
     * </pre>
     */
    protected void executeRecreateTasK() {
        tryExecuteRecreateTasK("none");
    }

    /**
     * 执行重新创建人物
     * <pre>
     *    注意，recreate() 会调用到 onDestroy() 函数，
     *    在 onDestroy() 中需区分是否需要执行 killProcess() 动作；
     * </pre>
     *
     * @param reason 调用原因
     */
    @Override
    protected void tryExecuteRecreateTasK(@NonNull String reason) {
        // 处理 drawable-night 资源配置包
        if ("uimode".equals(reason)) {
            if (!ThemeEx.supportUiModeRecreate()) {
                return;
            }
        }

        // 重启并设置重启标记
        recreate();
        mIsExecuteRecreateTask = true;
    }

    /**
     * 显示双屏异显效果
     * <p> 视频界面点击 Home 按钮退到后台的时候可以调用它，实现双屏异显；
     */
    public void showPresentationDisplay() {
        // TODO: 谁需要自己去实现
    }

    /**
     * 关闭双屏异显
     * <p> 视频页面销毁的时候需要调用，切换到音乐的时候也需要检查调用；
     */
    public void hidePresentationDisplay() {
        // TODO: 谁需要自己去实现
    }

    @Override
    protected void onPause() {
        super.onPause();
        mIsResumed = false;
    }

    @Override
    protected void onStop() {
        super.onStop();
        mIsStopped = true;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        // 销毁容器资源
        mCompositeDisposable.clear();
        mCompositeDisposable.dispose();
    }

    /**
     * 初始化壁纸配置到数据库
     * @param firstInit 是不是第一次初始化
     */
    protected void initWallpaperConfig(boolean firstInit) {
        LogUtils.vTag(TAG, "Configuration isChanged : " + firstInit);
        if (SkinX.getBoolean("support_wallpaper_customized")) {
            String savePath;
            if (MediaPageState.instance().getWallpaperData().isNight(getResources().getConfiguration())) {
                savePath = MediaPageState.instance().getWallpaperData().getNightWallpaperPath();
                // 如果为空 读取默认浅色的壁纸配置，兼容旧版本
                if (TextUtils.isEmpty(savePath)) {
                    savePath = MediaPageState.instance().getWallpaperData().getDayWallpaperPath();
                }
            } else {
                savePath = MediaPageState.instance().getWallpaperData().getDayWallpaperPath();
            }
            if (HFileUtils.isFileExists(savePath)) {
                MediaPageState.instance().write(
                        PageDataKV.Key.MUSIC_WALLPAPER_PATH,
                        savePath, true);
            }
        }
        // 子类必须实现背景同步逻辑
        checkAndSyncBackground();
    }
}

