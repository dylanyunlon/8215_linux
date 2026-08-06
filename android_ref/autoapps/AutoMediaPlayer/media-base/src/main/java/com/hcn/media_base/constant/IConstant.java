package com.hcn.media_base.constant;

/**
 * 常量定义
 *
 * @author 65821
 */
public interface IConstant {
    /** 存储路径相关定义 **/
    String PATH_SD = "/storage/ext_sdcard1";
    String PATH_SD2 = "/storage/sdcard2";

    String PATH_USB = "/storage";
    String PATH_USB_PREFIX = "/storage/udisk";
    String PATH_FLASH = "/storage/emulated/0";

    /** USB 路径特征标记 **/
    String USB_PATH_MARK = "udisk";

    /**
     * 通知栏视图控制广播
     * <p> Notification 播放/暂停/上下曲/显示-隐藏...
     */
    String ACTION_NOTIFICATION_PREV =
            "com.auto.apimediaplayer.notification.PREV";
    String ACTION_NOTIFICATION_PLAYPAUSE =
            "com.auto.apimediaplayer.notification.PLAYPAUSE";
    String ACTION_NOTIFICATION_NEXT =
            "com.auto.apimediaplayer.notification.NEXT";
    String ACTION_NOTIFICATION_SHOW =
            "com.auto.apimediaplayer.notification.SHOW";
    String ACTION_NOTIFICATION_CANCEL =
            "com.auto.apimediaplayer.notification.CANCEL";

    /**
     * 语音控制音视频切换时触发的事件广播
     * <p> e.g. ”open_video“, ”open_music“...
     */
    String ACTION_VOICE_2_HMEDIA = "com.hcn.AutoMediaPlayer.Action.Voices";
    String EXTRA_MEDIA_EVENT = "extra_media_event";
}
