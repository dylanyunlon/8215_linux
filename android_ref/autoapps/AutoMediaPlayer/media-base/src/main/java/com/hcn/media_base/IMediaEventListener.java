package com.hcn.media_base;

/**
 * 处理媒体事件监听
 * @author 86158
 */
public interface IMediaEventListener extends IMediaEvent {

    /**
     * 媒体事件回调接口
     *
     * @param eventId 事件 ID
     * @param wParam 附加参数 1
     * @param lParam 附加阐述 2
     */
    void onMediaEvent(int eventId, Object wParam, Object lParam);
}
