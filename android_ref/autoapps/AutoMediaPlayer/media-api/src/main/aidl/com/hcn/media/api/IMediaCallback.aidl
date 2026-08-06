// IMediaCallback.aidl
package com.hcn.media.api;

/**
 * 媒体事件回调
 * @author 65821
 */
interface IMediaCallback {

    /**
     * 客户端的名字
     * <p> 客户端的名字必须不能为空;
     */
    String clientName();

    /**
     * 媒体事件回调接口
     * <p> oneway 表示是异步调用；
     *
     * @param event 事件类型
     * @param arg0 附加参数 1
     * @param arg1 附加参数 2
     */
    oneway void onEvent(String event, String arg0, String arg1);
}
