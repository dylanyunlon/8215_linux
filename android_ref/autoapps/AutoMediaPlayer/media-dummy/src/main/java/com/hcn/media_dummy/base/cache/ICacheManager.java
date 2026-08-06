package com.hcn.media_dummy.base.cache;

import android.content.Context;

import java.io.File;
import java.util.Map;

import tv.danmaku.ijk.media.player.IMediaPlayer;
/**
 * 缓存管理接口
 * @author 65821
 */
public interface ICacheManager {

    /**
     * 开始缓存逻辑
     *
     * @param context 上下文
     * @param mediaPlayer 播放内核
     * @param url 播放url
     * @param header 头部信息
     * @param cachePath 缓存路径，可以为空
     */
    void doCacheLogic(Context context,
                      IMediaPlayer mediaPlayer,
                      String url,
                      Map<String, String> header,
                      File cachePath);

    /**
     * 清除缓存
     *
     * @param context 上下文
     * @param cachePath 可以为空，空时用默认
     * @param url 可以为空，空时清除所有
     */
    void clearCache(Context context, File cachePath, String url);

    /**
     * 是否缓存管理
     */
    void release();

    /**
     * 播放中判断是否缓存，会频繁调用
     * @return 有/无
     */
    boolean hadCached();

    /**
     * 播放前判断是否缓存
     *
     * @param context 上下文环境
     * @param cacheDir 缓存目录
     * @param url 远程文件地址
     * @return 是否已经缓存到本地
     */
    boolean cachePreview(Context context, File cacheDir, String url);

    /**
     * 设置缓存有效监听
     * <p> 给 Player 用，用来监听是否缓存完成；
     *
     * @param cacheAvailableListener 监听对象
     */
    void setCacheAvailableListener(ICacheAvailableListener cacheAvailableListener);

    /**
     * 缓存进度回调接口
     * <p> 用来给 Player 用，根据百分比显示加载进度；
     */
    interface ICacheAvailableListener {
        /**
         * 可用缓存状态回调
         *
         * @param cacheFile 缓存文件
         * @param url 播放 url
         * @param percentsAvailable 缓存百分比
         */
        void onCacheAvailable(File cacheFile, String url, int percentsAvailable);
    }
}
