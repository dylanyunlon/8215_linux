package com.hcn.media.base;

/**
 * 播放信息改变回调接口
 * <p> 现阶段只支持音乐信息回调；
 *
 * @author 65821
 */
public interface IPlayInfoChanged {

    /**
     * 播放信息改变回调
     *
     * @param event 事件名字 {@link IEvent}
     * @param info 播放信息
     */
    void onPlayInfoChanged(@IEvent String event, MediaPlayInfo info);
}
