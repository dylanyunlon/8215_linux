package com.hcn.media.music;

import com.hcn.media_data.storage.StorageDeviceEx;

/**
 * 看上去是为元素之间回调设计的
 * <p> 没什么卵用，已经淘汰了；
 *
 * @author 65821
 * @deprecated 竟可能不要使用它
 */
@Deprecated
public interface ILayoutCallback {
    /**
     * 回调事件接口
     *
     * @param device
     * @param eventID
     */
    void onCallback(StorageDeviceEx device, int eventID);
}
