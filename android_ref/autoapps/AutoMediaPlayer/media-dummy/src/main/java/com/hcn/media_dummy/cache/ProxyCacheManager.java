package com.hcn.media_dummy.cache;

import android.content.Context;
import android.net.Uri;
import android.text.TextUtils;

import com.hcn.common.utils.HFileUtils;
import com.hcn.media_cache.CacheListener;
import com.hcn.media_cache.HttpProxyCacheServer;
import com.hcn.media_cache.StorageUtils;
import com.hcn.media_cache.file.FileNameGenerator;
import com.hcn.media_cache.file.Md5FileNameGenerator;
import com.hcn.media_dummy.base.cache.ICacheManager;
import com.hcn.media_dummy.utils.FileUtils;

import java.io.File;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.Map;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.TrustManager;

import tv.danmaku.ijk.media.player.IMediaPlayer;

/**
 * 代理缓存管理器
 * @author 65821
 */
public class ProxyCacheManager implements ICacheManager, CacheListener {

    /** [单例]唯一实例 */
    private static ProxyCacheManager proxyCacheManager;

    /** 默认缓存 512M */
    public static long DEFAULT_MAX_SIZE = 512 * 1024 * 1024;
    public static int DEFAULT_MAX_COUNT = -1;

    /** HTTP 代理缓存服务 */
    protected HttpProxyCacheServer proxy;

    /** 缓存目录文件 */
    protected File mCacheDir;

    /** 是播放缓存文件 */
    protected boolean mCacheFile;

    /** 文件名字生成器 */
    private static FileNameGenerator fileNameGenerator;

    private ICacheManager.ICacheAvailableListener cacheAvailableListener;

    /** 扩展使用，方便 View 层暂停播放情况下也可以拿到缓存百分比 */
    private WeakReference<ICacheAvailableListener> mCacheAvailableListenerEx;

    protected ProxyCacheUserAgentHeadersInjector userAgentHeadersInjector = new ProxyCacheUserAgentHeadersInjector();

    private HostnameVerifier v;

    private TrustManager[] trustAllCerts;

    /**
     * 单例管理器
     * @return {@link ProxyCacheManager}
     */
    public static synchronized ProxyCacheManager instance() {
        if (proxyCacheManager == null) {
            proxyCacheManager = new ProxyCacheManager();
        }
        return proxyCacheManager;
    }

    @Override
    public void onCacheAvailable(File cacheFile, String url, int percentsAvailable) {
        if (cacheAvailableListener != null) {
            cacheAvailableListener.onCacheAvailable(cacheFile, url, percentsAvailable);
        }
    }

    /**
     * 用来通知缓存进度到播放器
     *
     * @param cacheFile 缓存文件
     * @param url 播放 url
     * @param percentsAvailable 缓存百分比
     */
    public void onCacheAvailableEx(File cacheFile, String url, int percentsAvailable) {
        if (mCacheAvailableListenerEx == null) {
            return;
        }

        ICacheAvailableListener listener = mCacheAvailableListenerEx.get();
        if (listener != null) {
            listener.onCacheAvailable(cacheFile, url, percentsAvailable);
        }
    }

    /**
     * 开始缓存逻辑
     *
     * @param context 上下文环境
     * @param mediaPlayer 播放内核
     * @param originUrl 播放 url
     * @param header 头部信息
     * @param cachePath 缓存路径，可以为空
     */
    @Override
    public void doCacheLogic(Context context,
                             IMediaPlayer mediaPlayer,
                             String originUrl,
                             Map<String, String> header,
                             File cachePath) {
        String url = originUrl;

        ProxyCacheUserAgentHeadersInjector.mMapHeadData.clear();
        if (header != null) {
            ProxyCacheUserAgentHeadersInjector.mMapHeadData.putAll(header);
        }

        if (url.startsWith("http")
                && !url.contains("127.0.0.1")
                && !url.contains(".m3u8")) {
            HttpProxyCacheServer proxy = getProxy(context.getApplicationContext(), cachePath);
            if (proxy != null) {
                // 此处转换了 url，然后再赋值给 mUrl。
                url = proxy.getProxyUrl(url);
                mCacheFile = (!url.startsWith("http"));
                if (!mCacheFile) {
                    // 注册缓冲监听
                    proxy.registerCacheListener(this, originUrl);
                }
            }
        } else if ((!url.startsWith("http")
                && !url.startsWith("rtmp")
                && !url.startsWith("rtsp")
                && !url.contains(".m3u8"))) {
            mCacheFile = true;
        }

        try {
            mediaPlayer.setDataSource(context, Uri.parse(url), header);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * 清除缓存
     *
     * @param context 上下文
     * @param cachePath 可以为空，空时用默认
     * @param url 可以为空，空时清除所有
     */
    @Override
    public void clearCache(Context context, File cachePath, String url) {
        if (TextUtils.isEmpty(url)) {
            if (cachePath == null) {
                String path = StorageUtils.getIndividualCacheDirectory
                        (context.getApplicationContext()).getAbsolutePath();
                FileUtils.deleteFiles(new File(path));
            } else {
                FileUtils.deleteFiles(cachePath);
            }
        } else {
            FileNameGenerator md5FileNameGenerator = new Md5FileNameGenerator();
            if (ProxyCacheManager.fileNameGenerator != null) {
                md5FileNameGenerator = ProxyCacheManager.fileNameGenerator;
            }

            String name = md5FileNameGenerator.generate(url);
            if (cachePath != null) {
                String tmpPath = cachePath.getAbsolutePath() + File.separator + name + ".download";
                String path = cachePath.getAbsolutePath() + File.separator + name;
                HFileUtils.delete(tmpPath);
                HFileUtils.delete(path);
            } else {
                String pathTmp = StorageUtils.getIndividualCacheDirectory
                        (context.getApplicationContext()).getAbsolutePath()
                        + File.separator + name + ".download";
                String path = StorageUtils.getIndividualCacheDirectory
                        (context.getApplicationContext()).getAbsolutePath()
                        + File.separator + name;
                HFileUtils.delete(pathTmp);
                HFileUtils.delete(path);
            }
        }
    }

    @Override
    public void release() {
        if (proxy != null) {
            try {
                proxy.unregisterCacheListener(this);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public boolean cachePreview(Context context, File cacheDir, String url) {
        HttpProxyCacheServer proxy = getProxy(context.getApplicationContext(), cacheDir);
        if (proxy != null) {
            // 此处转换了 url，然后再赋值给 mUrl。
            url = proxy.getProxyUrl(url);
        }
        return (!url.startsWith("http"));
    }

    @Override
    public boolean hadCached() {
        return mCacheFile;
    }

    @Override
    public void setCacheAvailableListener(ICacheAvailableListener cacheAvailableListener) {
        this.cacheAvailableListener = cacheAvailableListener;
    }

    /**
     * 创建缓存代理服务，带文件目录的
     *
     * @param context 上下文
     * @param file 缓存目录文件
     * @return {@link HttpProxyCacheServer}
     */
    public HttpProxyCacheServer newProxy(Context context, File file) {
        if (!file.exists()) {
            boolean ignored = file.mkdirs();
        }

        HttpProxyCacheServer.Builder builder = new HttpProxyCacheServer.Builder(context);
        builder.cacheDirectory(file);

        if (DEFAULT_MAX_COUNT > 0) {
            builder.maxCacheFilesCount(DEFAULT_MAX_COUNT);
        } else {
            builder.maxCacheSize(DEFAULT_MAX_SIZE);
        }

        builder.headerInjector(userAgentHeadersInjector);
        builder.hostnameVerifier(v);
        builder.trustAllCerts(trustAllCerts);
        if (fileNameGenerator != null) {
            builder.fileNameGenerator(fileNameGenerator);
        }

        mCacheDir = file;
        return builder.build();
    }

    public void setProxy(HttpProxyCacheServer proxy) {
        this.proxy = proxy;
    }

    /**
     * 创建缓存代理服务
     *
     * @param context 上下文
     * @return {@link HttpProxyCacheServer}
     */
    public HttpProxyCacheServer newProxy(Context context) {
        HttpProxyCacheServer.Builder builder = new HttpProxyCacheServer
                .Builder(context.getApplicationContext())
                .headerInjector(userAgentHeadersInjector);

        if (DEFAULT_MAX_COUNT > 0) {
            builder.maxCacheFilesCount(DEFAULT_MAX_COUNT);
        } else {
            builder.maxCacheSize(DEFAULT_MAX_SIZE);
        }

        builder.hostnameVerifier(v);
        builder.trustAllCerts(trustAllCerts);
        return builder.build();
    }

    /**
     * 获取缓存代理服务
     *
     * @param context 上下文
     * @return {@link HttpProxyCacheServer}
     */
    protected static HttpProxyCacheServer getProxy(Context context) {
        HttpProxyCacheServer proxy = ProxyCacheManager.instance().proxy;
        return proxy == null ? (ProxyCacheManager.instance().proxy =
                ProxyCacheManager.instance().newProxy(context)) : proxy;
    }

    /**
     * 获取缓存代理服务，带文件目录的
     *
     * @param context 上下文
     * @param file 目标目录文件
     * @return {@link HttpProxyCacheServer}
     */
    public static HttpProxyCacheServer getProxy(Context context, File file) {
        // 如果为空，返回默认的
        if (file == null) {
            return getProxy(context);
        }

        // 如果已经有缓存文件路径，那么判断缓存文件路径是否一致
        File cacheDir = ProxyCacheManager.instance().mCacheDir;
        if (cacheDir != null
                && !cacheDir.getAbsolutePath().equals(file.getAbsolutePath())) {
            // 不一致先关了旧的
            HttpProxyCacheServer proxy = ProxyCacheManager.instance().proxy;
            if (proxy != null) {
                proxy.shutdown();
            }

            // 开启新的
            return (ProxyCacheManager.instance().proxy =
                    ProxyCacheManager.instance().newProxy(context, file));
        } else {
            // 还没有缓存文件的或者一致的，返回原来
            HttpProxyCacheServer proxy = ProxyCacheManager.instance().proxy;
            return proxy == null ? (ProxyCacheManager.instance().proxy =
                    ProxyCacheManager.instance().newProxy(context, file)) : proxy;
        }
    }

    public static void setFileNameGenerator(FileNameGenerator fileNameGenerator) {
        ProxyCacheManager.fileNameGenerator = fileNameGenerator;
    }

    public static void clearFileNameGenerator() {
        ProxyCacheManager.fileNameGenerator = null;
    }

    public HostnameVerifier getHostnameVerifier() {
        return v;
    }

    public void setHostnameVerifier(HostnameVerifier v) {
        this.v = v;
    }

    public TrustManager[] getTrustAllCerts() {
        return trustAllCerts;
    }

    public void setTrustAllCerts(TrustManager[] trustAllCerts) {
        this.trustAllCerts = trustAllCerts;
    }
}
