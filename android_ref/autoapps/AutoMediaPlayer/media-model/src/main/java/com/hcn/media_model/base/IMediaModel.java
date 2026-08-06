package com.hcn.media_model.base;

/**
 * 媒体模型接口
 * @author 65821
 */
public interface IMediaModel extends MediaModule {
    /**
     * 低内存的时候调用
     * @param reason 原因
     */
    void onLowMemory(int reason);
}
