package com.hcn.media_data.folder;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * 记录媒体加载状态
 * <p> 用来扫描标记扩展使用；
 *
 * @author 65821
 */
public class MediaLoadState {
    public volatile boolean mIsLoadFinished;
    public volatile AtomicInteger mLoadingIndex = new AtomicInteger(0);

    public MediaLoadState() {
        mIsLoadFinished = false;
    }
}
