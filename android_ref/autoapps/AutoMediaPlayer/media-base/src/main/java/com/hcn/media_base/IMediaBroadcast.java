package com.hcn.media_base;

/**
 * 广播定义
 * @author 86158
 */
public interface IMediaBroadcast {
    /** 语音控制类广播 **/
    String ACTION_VOICE_EVENT_VIDEO_PLAY = "com.txznet.extra.video.play";
    String ACTION_VOICE_EVENT_VIDEO_PAUSE = "com.txznet.extra.video.pause";
    String ACTION_VOICE_EVENT_PLAY = "com.txznet.extra.play";
    String ACTION_VOICE_EVENT_PAUSE = "com.txznet.extra.pause";
    String ACTION_VOICE_EVENT_NEXT = "com.txznet.extra.next";
    String ACTION_VOICE_EVENT_PREV = "com.txznet.extra.pre";
    String ACTION_VOICE_EVENT_REWIND = "com.txznet.extra.rewind";
    String ACTION_VOICE_EVENT_FAST_FORWARD = "com.txznet.extra.fast.forward";
    String ACTION_VOICE_EVENT_MODE_LOOP_ALL = "com.txznet.extra.mode.loop.all";
    String ACTION_VOICE_EVENT_MODE_LOOP_ONE = "com.txznet.extra.mode.loop.one";
    String ACTION_VOICE_EVENT_MODE_RANDOM = "com.txznet.extra.random";

    /** Smart 按键控制广播 **/
    String ACTION_EVENT_K_SCROLL_L = "action.event.scroll.l";
    String ACTION_EVENT_K_SCROLL_R = "action.event.scroll.r";
    String ACTION_EVENT_K_ENTER = "action.event.enter";

    /** 播放暂停广播 **/
    String ACTION_MUSIC_PLAY = "com.auto.apimediaplayer.action.PLAY";
    String ACTION_MUSIC_PAUSE = "com.auto.apimediaplayer.action.PAUSE";

    /** 单曲循环广播 **/
    String ACTION_MUSIC_SINGLE_MODEL =
            "com.auto.apimediaplayer.action.SINGLE_MODEL";

    /** 随机模式广播 **/
    String ACTION_MUSIC_RANDOM_MODEL =
            "com.auto.apimediaplayer.action.RANDOM_MODEL";

    /** 全部循环广播 **/
    String ACTION_MUSIC_ALL_LOOP_MODEL =
            "com.auto.apimediaplayer.action.ALLLOOP_MODEL";

    /** 切换上一曲广播 **/
    String ACTION_NOTIFICATION_PREV =
            "com.hcn.AutoMediaPlayer.MSG_NotificationPre";

    /** 切换下一曲广播 **/
    String ACTION_NOTIFICATION_NEXT =
            "com.hcn.AutoMediaPlayer.MSG_NotificationNext";

    /** 通知扫盘广播 **/
    String ACTION_REFRESH_PATH =
            "com.hcn.AutoMediaPlayer.action.REFRESH_PATH";
}
