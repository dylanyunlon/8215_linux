package com.hcn.media.base;

/**
 * Media 接口连接状态
 * @author 65821
 */
public interface IConnectionState {
    /** 连接空闲状态 **/
    String IDLE = "idle";

    /** 连接成功状态 **/
    String CONNECTED = "connected";

    /** 请求连接状态 **/
    String CONNECTING = "connecting";

    /** 连接断开状态 **/
    String DISCONNECTED = "disconnected";

    /** 连接断开状态 **/
    String DIED = "died";

    /**
     * 媒体 SDK 连接成功
     * <p> 连接成功后才可以调用媒体 api 控制接口；
     */
    void onConnected();

    /**
     * 媒体 SDK 断开连接
     * <p> 如果断开连接了，不再可以操作 api 控制接口；
     */
    void onDisconnected();

    /**
     * 媒体 SDK 连接死亡
     * <p> 如果连接死亡了，不再可以操作 api 控制接口；
     */
    void onDied();
}
