package com.hcn.media_dummy.cache;

import com.hcn.common.misc.LogUtils;
import com.hcn.media_cache.headers.HeaderInjector;
import com.hcn.media_dummy.Config;

import java.util.HashMap;
import java.util.Map;

/**
 * for android video cache header
 * @author 65821
 */
public class ProxyCacheUserAgentHeadersInjector implements HeaderInjector {

    public final static Map<String, String> mMapHeadData = new HashMap<>();

    @Override
    public Map<String, String> addHeaders(String url) {
        LogUtils.vTag(Config.TAG,
                "****** Proxy addHeaders ****** / size = " + mMapHeadData.size());
        return mMapHeadData;
    }
}