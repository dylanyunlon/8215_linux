package com.hcn.media_common.debug;

import com.hcn.common.utils.HUtilsEx;

/**
 * 调试配置
 *
 * @author 86158
 */
public interface MediaDebug {
    /**
     * 打印标记
     * <p> 打印标签格式: HMedia_$(subTag)
     */
    String TAG = "HMedia";

    /**
     * 模块标签
     * <pre>
     *    调试使用，必须配置；
     *    用来拼接调试控制属性、调试广播等；
     * </p>
     */
    String MODULE_KEY = "mediaplayer";

    /**
     * 配置开始前台服务
     * <pre>
     *    是否把 LocalService 启动成前台服务程序；
     *    主要是避免其被 AMS 回收掉，默认配置不启动；
     *    system.uid 的前台服务，即使被 kill [pid] 也会被再次拉起来；
     *    am force-stop com.hcn.AutoMediaPlayer 可以干掉它；
     * </pre>
     */
    boolean START_FOREGROUND_SERVICE = "1".equals(HUtilsEx.getSystemProperty(
            "persist.sys.media.fg.service", "0"));
}