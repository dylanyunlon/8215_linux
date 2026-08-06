package com.hcn.media_model.impl;

import android.content.Context;
import android.util.DisplayMetrics;

import androidx.annotation.NonNull;

import com.hcn.auto_compat.app.WindowConfiguration;
import com.hcn.auto_compat.app.WindowConfiguration.WindowingMode;
import com.hcn.media_model.base.ui.BaseMediaActivity;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_model.base.BaseModel;
import com.hcn.media_model.base.IUiModel;

import java.util.Objects;

/**
 * 媒体 UI 模型
 * <pre>
 *    存放 UI 部分相关的状态和对象、提供 UI 关联的状态方法；
 *    后续 UI 相关业务逻辑也可以放到这里处理；
 * </pre>
 *
 * @author 65821
 */
final class MediaUiModel extends BaseModel
        implements IUiModel {
    private static final String TAG = MediaUiModel.class.getSimpleName();

    /** Model 必须是唯一实例设计 **/
    private static MediaUiModel sInstance = null;

    /** IUiModel 对外接口实例 **/
    public static MediaUiModel instance() {
        if (Objects.isNull(sInstance)) {
            throw new RuntimeException(
                    "Please initialize [MediaUiModel] Object!");
        }

        return sInstance;
    }

    /**
     * 初始化 MediaUiModel 模型
     * <p> 注意 IUiModel 禁止访问其它模型；
     *
     * @param context 当前应用上下文环境
     */
    public static void init(@NonNull Context context) {
        if (Objects.isNull(sInstance)) {
            sInstance = new MediaUiModel(context);
        } else {
            throw new RuntimeException(
                    "[MediaPlayerModel] already initialized!");
        }
    }

    /**
     * 当前播放器的 Activity 组件对象引用
     * <pre>
     *    历史原因遗留的逻辑，不再维护；
     *    理论上 Application 不需要引用 UI 组件对象；
     * </pre>
     */
    private BaseMediaActivity mMusicUiActivity = null;
    private BaseMediaActivity mVideoUiActivity = null;

    /**
     * 视频后台播放标记
     * <pre>
     *    历史原因遗留的逻辑，不再维护；
     *    主要是为了简单判断当前视频是否在后台播放状态；
     * </pre>
     */
    private boolean mIsVideoActivityBackground = false;

    /**
     * 视频是否在分屏副屏模式
     * <pre>
     *    如果当前是竖屏模式，且视频分屏状态在副屏，需要对列表显示做调整，避免空出 StatusBar 位置。
     *    我们通过 "mWindowingMode=split-screen-secondary" 和竖屏的屏幕坐标结合判断来区分；
     * </pre>
     */
    private boolean mVideoInSplitScreenSecondary = false;

    /**
     * 是否在视频模式窗口焦点状态；
     * <pre>
     *    标记视频模式窗口的焦点状态;
     *    参考 {@link  android.app.Activity#onWindowFocusChanged} ;
     *    主要是用来区分当前 视频 UI 窗口的显示位置（相对系统屏幕的坐标点）；
     *    当视频窗口有焦点的时候，可以结合 getWindow().getDecorView().getLocationOnScreen(...) 接口
     *    可以获取到当前窗口的位置，根据位置的 y 坐标可以判断竖屏分屏状态下当前视频是显示在上半部分还是下半部分；
     * </pre>
     */
    private boolean mIsVideoUiWindowFocus = false;

    /**
     * 标记当前视频页面的窗口模式；
     * <p> 全屏、分屏（主副屏）、画中画、自由窗口；
     */
    @WindowingMode
    private int mVideoWindowingMode = WindowConfiguration.WINDOWING_MODE_UNDEFINED;

    /** 禁止构造无参对象 **/
    private MediaUiModel() {
        super(null, null);
        throw new RuntimeException(
                "Prohibit the construction of parameterless objects");
    }

    /**
     * 默认构造函数
     * @param context 上下文环境
     */
    public MediaUiModel(@NonNull Context context) {
        super(context, null);
    }

    /** [强制退出视频 UI 组件] **/
    @Override
    public void finishVideoUI(int reason) {
        if (mVideoUiActivity != null) {
            if (!mVideoUiActivity.isFinishing()) {
                LogUtil.i(TAG, "finishVideoUI: reason = " + reason);
                mVideoUiActivity.finish();
            }
        }
    }

    /** [强制退出音乐 UI 组件] **/
    @Override
    public void finishMusicUI(int reason) {
        if (mMusicUiActivity != null) {
            if (!mMusicUiActivity.isFinishing()) {
                LogUtil.i(TAG, "finishMusicUI: reason = " + reason);
                mMusicUiActivity.finish();
            }
        }
    }

    /**
     * 低内存处理函数
     * @param reason 原因
     */
    @Override
    public void onLowMemory(int reason) {
        finishMusicUI(reason);
        finishVideoUI(reason);
    }

    /**
     * 是视频活动后台
     * @return 是/否
     */
    @Override
    public boolean isVideoActivityBackground() {
        return mIsVideoActivityBackground;
    }

    /**
     * 设置视频后台标记
     * @param isBackground 后台标记
     */
    @Override
    public void setVideoActivityBackground(boolean isBackground) {
        mIsVideoActivityBackground = isBackground;
    }

    /** [VideoUI] **/
    @Override
    public BaseMediaActivity getVideoActivity() {
        return mVideoUiActivity;
    }

    @Override
    public void setVideoUiActivity(BaseMediaActivity videoUiActivity) {
        mVideoUiActivity = videoUiActivity;
    }

    /** [MusicUI] **/
    @Override
    public BaseMediaActivity getMusicActivity() {
        return mMusicUiActivity;
    }

    @Override
    public void setMusicUiActivity(BaseMediaActivity musicUiActivity) {
        mMusicUiActivity = musicUiActivity;
    }

    @Override
    public boolean isVideoWindowingMode(int windowingMode) {
        return mVideoWindowingMode == windowingMode;
    }

    @Override
    public void setVideoWindowingMode(int windowingMode) {
        mVideoWindowingMode = windowingMode;
    }

    /**
     * 视频是否显示在下半屏
     * <pre>
     *     需要是竖屏，且在分屏是在下部分；
     *     竖屏判定：屏幕高度 > 屏幕宽度;
     *     注意：视频窗口没有焦点的时候调用，这个接口返回的结果无效；
     * </pre>
     *
     * @return {@code true}: yes<br>{@code false}: no
     */
    @Override
    public boolean videoShowInBottomHalfScreen() {
        // 先判断是否是竖屏
        Context context = mContextRef.get();
        DisplayMetrics dm = context.getApplicationContext().getResources().getDisplayMetrics();
        LogUtil.v(TAG, "videoShowInBottomHalfScreen, " + dm.widthPixels + " x " + dm.heightPixels);

        if (!mIsVideoUiWindowFocus) {
            // 视频没有焦点的时候，
            LogUtil.w(TAG, "videoShowInBottomHalfScreen:" +
                    " Please do not call this interface when the video has no focus.");
        } else {
            int[] location = new int[2];
            if (mVideoUiActivity != null) {
                mVideoUiActivity.getWindow().getDecorView().getLocationOnScreen(location);
                int videoUiWindowTop = location[1];
                int expectedHalfScreenHeight = dm.heightPixels / 2;

                // 只要视频的窗口顶点 Y 坐标大于预期的半屏高度，我们就认为它显示在底部副屏；
                return (videoUiWindowTop > expectedHalfScreenHeight) && (dm.widthPixels < dm.heightPixels);
            }
        }

        return mVideoInSplitScreenSecondary && (dm.widthPixels < dm.heightPixels);
    }

    /**
     * 设置视频显示在副屏模式
     * <p> 必须是分屏模式，UI 才可能是在副屏状态。
     *
     * @param secondary 是副屏模式
     */
    @Override
    public void setVideoInSplitScreenSecondary(boolean secondary) {
        mVideoInSplitScreenSecondary = secondary;
    }

    /**
     * 设置视频窗口焦点状态
     * <p> 调用者 {@link android.app.Activity#onWindowFocusChanged(boolean)}
     *
     * @param hasFocus 视频窗口焦点状态
     */
    @Override
    public void setIsVideoUiWindowFocus(boolean hasFocus) {
        mIsVideoUiWindowFocus = hasFocus;
    }
}
