package com.hcn.media_model.player.base;


import androidx.annotation.NonNull;

import com.hcn.media_common.HMessage;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.rxrelay3.Relay;

/**
 * 播放组件接口
 * @author 65821
 */
public interface IMediaPlayer {
    /**
     * 事件中继器
     * <p> 可以用来监听媒体播放组件对外事件；
     * @return 中继器对象
     */
    @NonNull
    Relay<HMessage> eventRelay();

    /**
     * 播放组件是初始化的
     * @return 是/否
     */
    boolean isInited();

    /**
     * 播放组件是 Prepared 状态
     * @return 是/否
     */
    boolean isPrepared();

    /**
     * 当前播放是 1080P 视频源
     * @return 是/否
     */
    default boolean is1080PVideoSource() {
        return false;
    }

    /**
     * 请求调解 Track 音量
     * @param volume [0.0F ~ 1.0F]
     */
    void requestSetVolume(float volume);

    /**
     * 当前组件是播放状态
     * @return 是/否
     */
    boolean isPlayState();

    /**
     * 获取当前播放时间
     * @return 单位 ms
     */
    int getCurrentPosition();

    /**
     * 获取当前歌曲总时间
     * @return 单位 ms
     */
    int getTotalTime();

    /**
     * Seek 到指定时间
     * @param time 时间参数/单位 msec
     */
    void seekToTime(int time);

    /**
     * 更新 Surface Holder 对象
     * @param init 是否是初始化调用
     */
    void updateSurfaceHolder(boolean init);

    /**
     * 处理重头开始播放状态
     * <p> 没什么卵用的接口，可以干掉
     * @deprecated 历史遗漏，可能淘汰；
     */
    void onSetSeekTimeZero();

    /**
     * 是异步释放资源中
     * @return 是/否
     */
    default boolean isAsyncReleasing() {
        return false;
    }

    /**
     * 下发播放命令
     * @param command 播放命令
     */
    void onPlayControlEvent(int command);

    /**
     * 下发播放命令
     * @param command 播放命令
     * @param reason 执行原因/调试用
     */
    void onPlayControlEvent(int command, int reason);

    /**
     * 设置播放数据源
     * @param info 播放信息对象
     */
    void onSetDataSourceEvent(MusicInfo info);

    /**
     * 获取当前播放视频源宽度
     * @return 宽度
     */
    int getVideoWidth();

    /**
     * 获取当前播放视频源高度
     * @return 高度
     */
    int getVideoHeight();
}
