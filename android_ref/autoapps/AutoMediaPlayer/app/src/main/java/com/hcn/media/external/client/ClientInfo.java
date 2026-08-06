package com.hcn.media.external.client;

import android.os.IBinder;

import androidx.annotation.NonNull;

import com.hcn.media.api.IMediaCallback;

/**
 * 客户端信息
 * @author 65821
 */
public class ClientInfo {
    /** 客户端名字 **/
    private String name;

    /** 客户端对象 **/
    private IBinder binder;

    /** 客户端回调 **/
    private IMediaCallback callback;

    /** 客户端对象死亡接收者 **/
    private IBinder.DeathRecipient deathRecipient;

    /**
     * 构造无名称对象
     * @param binder 客户端对象
     */
    public ClientInfo(@NonNull IBinder binder) {
        this.binder = binder;
    }

    /**
     * 构造有名称对象
     *
     * @param name 客户端名称
     * @param binder 客户端对象
     */
    public ClientInfo(@NonNull String name,
                      @NonNull IBinder binder) {
        this.name = name;
        this.binder = binder;
        this.callback = null;
    }

    /** 获取客户端名称 **/
    public String getName() {
        return this.name;
    }

    /** 获取客户端对象 **/
    public IBinder getBinder() {
        return this.binder;
    }

    public IMediaCallback getCallback() {
        return this.callback;
    }

    /** 获取客户端的死亡监听者 **/
    public IBinder.DeathRecipient getDeathRecipient() {
        return this.deathRecipient;
    }

    /**
     * 设置客户端回调接口
     * <p>
     *
     * @param callback 回调对象
     */
    public void setCallback(IMediaCallback callback) {
        this.callback = callback;
    }

    /**
     * 设置客户端对象死亡监听者
     * @param dr 客户端对象的死亡监听器
     */
    public void setDeathRecipient(@NonNull IBinder.DeathRecipient dr) {
        this.deathRecipient = dr;
    }
}
