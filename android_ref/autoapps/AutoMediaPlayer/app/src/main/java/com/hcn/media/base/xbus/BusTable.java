package com.hcn.media.base.xbus;

import android.util.Log;

import com.hcn.AutoMediaPlayer.BuildConfig;
import com.hcn.common.lang.HReflectUtils;
import com.hcn.common.misc.HBusUtils;
import com.hcn.media.external.ReceptionService;
import com.hcn.mediaservice.data.MediaTimeInfo;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.Objects;

/**
 * 总线相关接口地址表
 * <pre>
 *    可以采用手动注册的机制，也可以采用自动注入的机制
 *    自动注入由  HBusPlugin 插件实现编译时注入（${class-name}.class -- ${class-name}.dex）
 *    配置方法：
 *       # buildSrc/src/main/groovy/BuildConfig.groovy
 *       > static supportHBusPluginInject = true;
 * </pre>
 *
 * @author 65821
 */
public class BusTable {
    private static final String EMPTY =  "";

    /** 初始化总线关系 **/
    public static void init() {
        if (BuildConfig.SUPPORT_HBUS_PLUGIN_INJECT) {
            Log.w("BusTable", "init/support HBusPlugin auto inject!");
            return;
        }

        // 反射 HBusUtils 接口对象
        HReflectUtils getInstance =
                HReflectUtils.reflect(HBusUtils.class).method("getInstance");
        if (Objects.isNull(getInstance)) {
            Log.w("BusTable", "HBusUtils/getInstance: is null!");
            return;
        }

        // 音乐播放信息更新总线事件
        getInstance.method("registerBus",
                IBusTag.UPDATE_MUSIC_PLAY_INFO, ReceptionService.class.getName(),
                "onUpdateMusicPlayInfo", MusicInfo.class.getName(), "info", false, "POSTING");

        // 音乐播放状态改变总线事件
        getInstance.method("registerBus",
                IBusTag.UPDATE_MUSIC_PLAY_STATE, ReceptionService.class.getName(),
                "onUpdateMusicPlayState", String.class.getName(), "param", false, "POSTING");

        // 音乐播放时间改变总线事件
        getInstance.method("registerBus",
                IBusTag.UPDATE_MUSIC_PLAY_TIME, ReceptionService.class.getName(),
                "onUpdateMusicPlayTime", MediaTimeInfo.class.getName(), "param", false, "POSTING");

        // 媒体进程将要退出总线事件
        getInstance.method("registerBus",
                IBusTag.MEDIA_APP_WILL_EXIT, ReceptionService.class.getName(),
                "onMediaAppWillExit", EMPTY, EMPTY, false, "MAIN");
    }
}
