package com.hcn.media_base;

/**
 * 媒体事件定义
 * <pre>
 *    由于前 51 个定义和 HMediaService 是共用的；
 *    如果需要修改，需要同步 2 个工程调整，否则会出错；
 * </pre>
 *
 * @author 86158
 */
public interface IMediaEvent {
    int EVENT_EXIT_PROCESS = -9999;
    int EVENT_NONE = -1;

    int EVENT_SERVICE_INITIALIZED = 0;
    int EVENT_GOTO_MUSIC_LIST_PAGE = 1;
    int EVENT_GOTO_MUSIC_SEARCH_PAGE = 2;
    int EVENT_MEDIA_MOUNTED = 3;
    int EVENT_MEDIA_UNMOUNTED = 4;
    int EVENT_MEDIA_LOADING_COMPLETE = 5;
    int EVENT_MEDIA_NO_MUSIC_FILE = 6;
    int EVENT_CURRENT_STORAGE_CHANGED = 7;
    int EVENT_MEDIA_LOADING_FILE = 8;
    int EVENT_MEDIA_LOADING_START = 9;

    int EVENT_CHANGE_MUSIC_ITEM = 10;
    int EVENT_CHANGE_MUSIC_LIST = 11;
    int EVENT_CHANGE_MUSIC_STORAGE = 12;
    int EVENT_CHANGE_MUSIC_FAVORITE_STATE = 13;
    int EVENT_CHANGE_PLAY_STATE_PLAY = 14;
    int EVENT_CHANGE_PLAY_STATE_PAUSE = 15;
    int EVENT_CHANGE_PLAY_STATE_STOP = 16;
    int EVENT_CHANGE_MEDIA_PLAYTIME = 17;
    int EVENT_MEDIA_COMPLETION = 18;
    int EVENT_CHANGE_PLAY_STATE = 19;

    int EVENT_CHANGE_REPEAT_MODE = 20;
    int EVENT_UPDATE_AUTO_BRAKE_STATUS = 21;
    int EVENT_ID3_SCAN_FINISHED = 22;
    int EVENT_UPDATE_MUSIC_LIST = 23;
    int EVENT_DEEP_SLEEP_STATUS = 24;

    int EVENT_SCROLL_SEEKBAR = 25;
    int EVENT_STOP_SCROLL_SEEKBAR = 26;
    int EVENT_GOTO_MUSIC_FILE_SEARCH_PAGE = 27;

    /**
     * 播放完成事件
     * <p> MediaPlayer#onSeekComplete
     */
    int EVENT_SEEK_TO_COMPLETE = 28;
    int EVENT_UPDATE_MUSIC_ID3 = 29;

    int EVENT_CHANGE_VIDEO_ITEM = 30;
    int EVENT_CHANGE_VIDEO_LIST = 31;
    int EVENT_CHANGE_FULL_SCREEN = 32;

    /**
     * 显示黑色遮罩布局
     * <p> {@link "R.id.layout_black"}
     */
    int EVENT_VIDEO_SHOW_BLACK_PAGE = 33;

    /**
     * 隐藏黑色遮罩布局
     * <p> {@link "R.id.layout_black"}
     */
    int EVENT_VIDEO_HIDE_BLACK_PAGE = 34;
    int EVENT_CANCEL_NOTIFICATION = 35;
    int EVENT_UNSUPPORT_VIDEO_CODE = 36;
    int EVENT_UNSUPPORT_AUDIO_CODE = 37;
    int EVENT_UNSUPPORT_SEEKABLE = 38;
    int EVENT_UNKNOWN_ERROR = 39;

    int EVENT_CODE_UNSUPPORT = 40;
    int EVENT_ERROR_FILE_NOT_EXIST = 41;
    int EVENT_MUSIC_PLAYER_PREPARING = 42;
    int EVENT_VIDEO_PLAYER_PREPARING = 43;

    int EVENT_ERROR_FILE_IS_TOO_SMALL = 47;

    /** 音乐和视频分屏情况下播放任务发生改变，更新对应 UI **/
    int EVENT_SPLIT_SCREEN_UPDATE_PLAY_STATE = 49;

    int EVENT_REQUEST_MEDIA_PLAY = 50;
    int EVENT_REQUEST_MEDIA_PAUSE = 51;

    int EVENT_CONTROL_SMART_CW = 60;
    int EVENT_CONTROL_SMART_CCW = 61;
    int EVENT_CANCEL_SMART_CONTROL = 62;
    int EVENT_CONTROL_SMART_ENTER= 63;

    int EVENT_VITAMIO_CODE_UNSUPPORT = 70;
    int EVENT_UNSUPPORT_VIDEO_CODE2 = 71;
    int EVENT_UNSUPPORT_VIDEO_PROMPT_SHOW = 72;
    int EVENT_UNSUPPORT_VIDEO_PROMPT_HIDE = 73;

    /** 多媒体收藏列表操作相关事件 **/
    int EVENT_MEDIA_FAVORITE_OPERATE = 80;
    int EVENT_MUSIC_FAVORITE_OPERATE = 81;
    int EVENT_VIDEO_FAVORITE_OPERATE = 82;

    /** 媒体会话事件/MediaButton **/
    int EVENT_MEDIA_SESSION_COMPAT_EVENT = 90;

    int EVENT_GOTO_FILE_LIST_ITEM_PAGE = 97;
    int EVENT_GOTO_FILE_LIST_PAGE = 98;
    int EVENT_GOTO_ALL_LIST_PAGE = 99;
    int EVENT_GOTO_MUSIC_INFO_PAGE = 100;
    int EVENT_CHANGE_SURFACE_VIEW_TARGET = 101;
    int EVENT_CODE_PLAY_ERROR = 102;

    int EVENT_CHANGE_SURFACE_VIEW_SIZE = 103;
    int MCC201_EVENT_GOTO_MUSIC_LIST = 104;

    int EVENT_CONFIGURATION_CHANGED_SIZE = 109;
    int EVENT_CHANGE_SURFACE_VIEW_LAYOUT = 110;

    int EVENT_GOTO_RESUME_MUSIC_PLAYER_UI = 201;
    int EVENT_GOTO_RESUME_VIDEO_PLAYER_UI = 202;

    /** 调试媒体 API 接口 **/
    int EVENT_DEBUG_MEDIA_API = 250;

    /** 正常退出当前 APP 前处理 **/
    int EVENT_PRE_NORMAL_EXIT_APP = 251;

    /** 请求对外广播音乐播放信息 */
    int EVENT_REQUEST_BROADCAST_MUSIC_PLAY_INFO = 252;

    /** 显示期望的音视频目标页面事件 **/
    int EVENT_SHOW_MUSIC_FRAGMENT = 300;
    int EVENT_SHOW_VIDEO_FRAGMENT = 301;

    /** 触发检查是否需要进入画中画 */
    int EVENT_TRIGGER_ENTER_PIP_MODE = 311;

    /**
     * 媒体事件定义限制阈值
     * <pre>
     *    由于媒体事件定义可能和其它事件定义混用同一套处理函数；
     *    所以：所有新定义的媒体事件值必须小于它，越界会产生不可预知的后果（因为后面的值可能被其它业务使用了）；
     *    e.g. 页面事件（PageEvent）就和媒体事件（IMediaEvent）混用了一套事件处理机制；
     * </pre>
     * @see com.hcn.media_base.fragment.PageEvent
     */
    int EVENT_DEFINE_LIMIT_THRESHOLD = 1000;
}
