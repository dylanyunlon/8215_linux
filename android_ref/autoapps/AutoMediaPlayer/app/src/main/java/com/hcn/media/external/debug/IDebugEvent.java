package com.hcn.media.external.debug;

/**
 * 多媒体 API 调试事件
 * @author 65821
 */
public interface IDebugEvent {
    /**
     * 调试事件回调
     *
     * @param event {@link BroadcastApi.ITypeValue} 事件类型值
     * @param obj 附加参数
     */
    void onDebugEvent(int event, Object obj);
}
