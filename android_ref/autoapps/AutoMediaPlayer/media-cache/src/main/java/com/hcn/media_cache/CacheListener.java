package com.hcn.media_cache;

import java.io.File;

/**
 * Listener for cache availability.
 *
 * @author Egor Makovsky (yahor.makouski@gmail.com)
 * @author Alexey Danilov (danikula@gmail.com).
 */
public interface CacheListener {

    /**
     * 缓存百分比接口
     *
     * @param cacheFile
     * @param url
     * @param percentsAvailable
     */
    void onCacheAvailable(File cacheFile, String url, int percentsAvailable);
}
