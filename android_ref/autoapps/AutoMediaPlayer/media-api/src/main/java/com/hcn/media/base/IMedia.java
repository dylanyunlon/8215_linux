package com.hcn.media.base;

/**
 * 媒体服务相关定义
 * @author 65821
 */
public interface IMedia {
    /** 媒体服务的包名 **/
    String MEDIA_SERVICE_PACKAGE_NAME
            = "com.hcn.AutoMediaPlayer";

    /** 媒体服务的动作 **/
    String MEDIA_SERVICE_ACTION
            = "HMEDIAPLAYER.ACTION.RECEPTIONSERVICE";

    /** 媒体服务的类名 **/
    String MEDIA_SERVICE_CLASS_NAME
            = "com.hcn.media.external.ReceptionService";

    /** 媒体音乐的类名 **/
    String MEDIA_MUSIC_CLASS_NAME
            = "com.hcn.MediaActivity.MusicPlayerUiActivity";

    /** 媒体视频的类名 **/
    String MEDIA_VIDEO_CLASS_NAME
            = "com.hcn.MediaActivity.VideoPlayerUiActivity";

    /** 服务启动的原因 **/
    String START_REASON_EXTRA_KEY = "start_reason";

    /**
     * 媒体模式类型
     * <p> 用来标记媒体进程当前工作模式；
     */
    interface Type {
        /** 没有媒体类型 **/
        int MEDIA_TYPE_IDLE = -1;

        /** 音乐媒体类型 **/
        int MEDIA_TYPE_MUSIC = 0;

        /** 视频媒体类型 **/
        int MEDIA_TYPE_VIDEO = 1;
    }

    /**
     * 媒体 API 接口触发原因
     * <p> 如果是用户操作触发的播放，必须执行；
     */
    interface TriggerReason {
        /** 空的触发原因 **/
        String EMPTY = "";

        /** 外部广播接口 **/
        String BROADCAST_API = "broadcast-api";

        /** 用户操作触发 **/
        String USER_OPERATION = "user-operation";

        /** 由 startService 触发 **/
        String START_SERVICE = "start-service";
    }
}
