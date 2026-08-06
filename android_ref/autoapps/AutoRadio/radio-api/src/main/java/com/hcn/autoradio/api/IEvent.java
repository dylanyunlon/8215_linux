package com.hcn.autoradio.api;

/**
 * @author simon
 */
public @interface IEvent {
    /**
     * APP主动请求数据后回调
     */
    public static final int UPDATE_DATA_READY = 0;
    /**
     * 波段范围发生变化事件
     */
    public static final int UPDATE_DATA_RANGE = 1;
    /**
     * 所有收音信息发生变化时
     */
    public static final int UPDATE_DATA_INFO = 2;
    /**
     * 收音的RDS信息变化事件
     */
    public static final int UPDATE_DATA_RDS_INFO = 3;
    /**
     * 收音机的播放状态事件
     */
    public static final int UPDATE_DATA_PLAY_STATE = 4;
    /**
     * 立体声变化
     */
    public static final int EVENT_STEREO_CHANGE = 5;
    /**
     * 完成电台扫描
     */
    public static final int EVENT_SCAN_DONE = 6;
    /**
     * 客户端绑定远程服务成功
     */
    public static final int EVENT_SERVICE_CONNECT_SUCCESS = 1000;
    /**
     * 客户端断开连接远程服务
     */
    public static final int EVENT_SERVICE_CONNECT_FAILED = 1001;

    /**
     * 进程退出
     */
    public static final int EVENT_RADIO_APP_EXIT = 1002;

}
