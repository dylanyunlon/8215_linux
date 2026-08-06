package com.hcn.media_model.base;

import com.hcn.auto_compat.app.WindowConfiguration.WindowingMode;
import com.hcn.media_model.base.ui.BaseMediaActivity;

/**
 * 本地 UI 状态接口
 * <p> 提供访问 Ui 模型的接口；
 *
 * @author 65821
 */
public interface IUiModel {

    /**
     * [强制退出音乐 UI 组件]
     * @param reason 音乐活动退出原因
     */
    void finishMusicUI(int reason);

    /**
     * [强制退出视频 UI 组件]
     * @param reason 视频活动退出原因
     */
    void finishVideoUI(int reason);

    /**
     * 是否在后台播放
     * @return 是/否
     */
    boolean isVideoActivityBackground();

    /**
     * 设置视频后台播放状态
     * @param isBackground
     */
    void setVideoActivityBackground(boolean isBackground);

    /**
     * 获取当前视频活动对象
     * @return  {@link BaseMediaActivity}
     */
    BaseMediaActivity getVideoActivity();

    /**
     * 设置当前视频活动对象
     * @param videoUiActivity {@link BaseMediaActivity}
     */
    void setVideoUiActivity(BaseMediaActivity videoUiActivity);

    /**
     * 获取当前音乐活动对象
     * @return  {@link BaseMediaActivity}
     */
    BaseMediaActivity getMusicActivity();

    /**
     * 设置当前音乐活动对象
     * @param musicUiActivity {@link BaseMediaActivity}
     */
    void setMusicUiActivity(BaseMediaActivity musicUiActivity);

    /**
     * 是否是目标视频窗口模式
     * @param windowingMode 目标窗口模式
     * @return 是/不是
     */
    boolean isVideoWindowingMode(@WindowingMode int windowingMode);

    /**
     * 设置标记视频窗口模式
     * @param windowingMode 窗口模式
     */
    void setVideoWindowingMode(@WindowingMode int windowingMode);

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
    boolean videoShowInBottomHalfScreen();

    /**
     * 设置视频显示在副屏模式
     * <p> 必须是分屏模式，UI 才可能是在副屏状态。
     *
     * @param secondary 是副屏模式
     */
    void setVideoInSplitScreenSecondary(boolean secondary);

    /**
     * 设置视频窗口焦点状态
     * <p> 调用者 {@link android.app.Activity#onWindowFocusChanged(boolean)}
     *
     * @param hasFocus 视频窗口焦点状态
     */
    void setIsVideoUiWindowFocus(boolean hasFocus);

    /**
     * 低内存的时候调用
     * @param reason 原因
     */
    void onLowMemory(int reason);
}
