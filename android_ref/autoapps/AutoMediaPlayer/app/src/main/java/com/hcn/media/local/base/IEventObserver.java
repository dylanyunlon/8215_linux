package com.hcn.media.local.base;

/**
 * 媒体事件观察者
 * <pre>
 *    结合对外服务 ReceptionService 使用；
 *    用来观察和分发媒体事件，主要是用来对外分发；
 * </pre>
 */
public interface IEventObserver {
    /**
     * 分发事件
     * <p> 用来分发事件；
     * @param event {@link com.hcn.media_base.IMediaEvent}
     * @param wParam 附加参数 w
     * @param lParam 附加参数 l
     */
    void dispatchMediaEvent(int event, Object wParam, Object lParam);
}
