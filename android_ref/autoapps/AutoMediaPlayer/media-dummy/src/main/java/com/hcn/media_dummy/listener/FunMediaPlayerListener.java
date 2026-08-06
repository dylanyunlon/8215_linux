package com.hcn.media_dummy.listener;

import tv.danmaku.ijk.media.player.IMediaPlayer;

/**
 * 媒体播放器事件监听
 * <p> 播放状态回调接口，主要是回调给 UI 使用的；
 * @author 65821
 */
public interface FunMediaPlayerListener {
    /**
     * 播放源准备完成
     * @see IMediaPlayer.OnPreparedListener
     */
    void onPrepared();

    /**
     * 播放源播放完成
     * @see IMediaPlayer.OnCompletionListener
     */
    void onAutoCompletion();

    /**
     * 播放源播放完成
     * <pre>
     *    1、MediaPlayer/release 时候调用;
     *    2、以及播放新的视频前的时候补发调用；
     * </pre>
     */
    void onCompletion();

    /**
     * 缓存加载百分比更新
     * @see IMediaPlayer.OnBufferingUpdateListener
     *
     * @param percent 百分比
     */
    void onBufferingUpdate(int percent);

    /**
     * 拖动完成事件(seekTo(time))
     * @see IMediaPlayer.OnSeekCompleteListener
     */
    void onSeekComplete();

    /**
     * 播放内核错误事件
     * @see IMediaPlayer.OnErrorListener
     *
     * @param what 事件代号
     * @param extra 扩展信息
     */
    void onError(int what, int extra);

    /**
     * 播放内核信息事件
     * @see IMediaPlayer.OnInfoListener
     *
     * @param what 事件代号
     * @param extra 扩展信息
     */
    void onInfo(int what, int extra);

    /**
     * 视频大小改变事件
     * <p> 一般在播放视频的时候，视频信息解析出来后触发；
     * @see IMediaPlayer.OnVideoSizeChangedListener
     */
    void onVideoSizeChanged();

    /**
     * 退出全屏
     * <p> 如果支持全屏按钮，点击 'Back' 后触发；
     */
    void onBackFullscreen();

    /**
     * 处理媒体暂停播放
     * <p> 由 UI 部分主动调用触发；
     */
    void onMediaPause();

    /**
     * 处理媒体恢复播放
     * <p> 由 UI 部分主动调用触发；
     */
    void onMediaResume();

    /**
     * 处理媒体恢复播放
     * <p> 由 UI 部分主动调用触发；
     *
     * @param seek 是否是 seek 触发；
     */
    void onMediaResume(boolean seek);
}
