package com.hcn.media_base.impl;

import com.hcn.media_base.IMediaEvent;

/**
 * 媒体事件工具接口
 * <p> 供程序调试打印使用；
 *
 * @author 65821
 */
public abstract class MediaEvent implements IMediaEvent {
    /**
     * 打印事件名称
     * @param event 事件类型
     * @return 字符串名称
     */
    public static String name(final int event) {
        switch (event) {
            case EVENT_SERVICE_INITIALIZED:
                return "service-initialized";
            case EVENT_MEDIA_LOADING_COMPLETE:
                return "loading-completed";
            case EVENT_MEDIA_NO_MUSIC_FILE:
                return "no-media-file";
            case IMediaEvent.EVENT_MUSIC_PLAYER_PREPARING:
                return "player-preparing";
            case IMediaEvent.EVENT_REQUEST_MEDIA_PAUSE:
                return "request-pause";
            case IMediaEvent.EVENT_DEBUG_MEDIA_API:
                return "debug-media-api";
            default:
                break;
        }

        return "" + event;
    }
}
