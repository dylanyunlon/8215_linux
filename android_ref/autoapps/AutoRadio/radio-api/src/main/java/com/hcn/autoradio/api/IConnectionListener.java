package com.hcn.autoradio.api;


/**
 * @author simon
 */
public interface IConnectionListener {
    /**
     * 服务连接监听
     */
    void onServiceConnected();

    /**
     * 服务断开监听
     */
    void onServiceDisconnected();
}
