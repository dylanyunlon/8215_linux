package com.hcn.media_dummy.listener;

/**
 * 视频播放回调接口
 * <p> UI 部分只需要关心这些状态就好；
 * @author 65821
 */
public interface VideoAllCallBack {
    /**
     * 开始加载
     * <pre>
     *    objects[0] 是 title
     *    objects[1] 是当前所处播放器（全屏或非全屏）
     * </pre>
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onStartPrepared(String url, Object... objects);

    /**
     * 加载成功
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onPrepared(String url, Object... objects);

    /**
     * 点击了开始按键播放
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickStartIcon(String url, Object... objects);

    /**
     * 点击了错误状态下的开始按键
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickStartError(String url, Object... objects);

    /**
     * 点击了播放状态下的开始按键/停止
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickStop(String url, Object... objects);

    /**
     * 点击了全屏播放状态下的开始按键/停止
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickStopFullscreen(String url, Object... objects);

    /**
     * 点击了暂停状态下的开始按键/播放
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickResume(String url, Object... objects);

    /**
     * 点击了全屏暂停状态下的开始按键/播放
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickResumeFullscreen(String url, Object... objects);

    /**
     * 点击了空白弹出 seekbar
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickSeekbar(String url, Object... objects);

    /**
     * 击了全屏的 seekbar
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickSeekbarFullscreen(String url, Object... objects);

    /**
     * 播放完了
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onAutoComplete(String url, Object... objects);

    /**
     * 非正常播放完了
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onComplete(String url, Object... objects);

    /**
     * 进去全屏
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onEnterFullscreen(String url, Object... objects);

    /**
     * 退出全屏
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onQuitFullscreen(String url, Object... objects);

    /**
     * 进入小窗口
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onQuitSmallWidget(String url, Object... objects);

    /**
     * 退出小窗口
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onEnterSmallWidget(String url, Object... objects);

    /**
     * 触摸调整声音
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onTouchScreenSeekVolume(String url, Object... objects);

    /**
     * 触摸调整进度
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onTouchScreenSeekPosition(String url, Object... objects);

    /**
     * 触摸调整亮度
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onTouchScreenSeekLight(String url, Object... objects);

    /**
     * 播放错误
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onPlayError(String url, Object... objects);

    /**
     * 点击了空白区域开始播放
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickStartThumb(String url, Object... objects);

    /**
     * 点击了播放中的空白区域
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickBlank(String url, Object... objects);

    /**
     * 点击了全屏播放中的空白区域
     * <p> objects 参考 {@link #onStartPrepared} 接口
     *
     * @param url 播放地址
     * @param objects 参数集
     */
    void onClickBlankFullscreen(String url, Object... objects);
}
