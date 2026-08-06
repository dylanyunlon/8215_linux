package com.hcn.media_dummy.cache;

import com.hcn.media_dummy.base.cache.ICacheManager;

/**
 * 构建缓存管理器
 * @author 65821
 */
public class CacheFactory {
    private static Class<? extends ICacheManager> sICacheManager;

    public static void setCacheManager(Class<? extends ICacheManager>  cacheManager) {
        sICacheManager = cacheManager;
    }

    public static ICacheManager getCacheManager() {
        if (sICacheManager == null) {
            sICacheManager = ProxyCacheManager.class;
        }

        try {
            return sICacheManager.newInstance();
        } catch (InstantiationException | IllegalAccessException e) {
            e.printStackTrace();
        }

        return null;
    }
}