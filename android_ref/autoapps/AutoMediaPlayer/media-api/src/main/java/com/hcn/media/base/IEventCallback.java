package com.hcn.media.base;

/**
 * 媒体事件回调
 * @author 65821
 */
public interface IEventCallback {
    /**
     * 媒体事件回调接口
     * <p> oneway 表示是异步调用；
     *
     * @param event 事件类型
     * @param obj0 附加参数 1
     * @param obj1 附加参数 2
     */
    void onEvent(@IEvent String event, Object obj0, Object obj1);
}