package com.hcn.media_base.constant;

import com.hcn.media.base.IMedia;

/**
 * 播放状态、命令等
 * @author 86158
 */
public interface IMusicState {
    /**
     * 媒体类型
     * <p> 音乐/视频
     */
    int MEDIA_TYPE_IDLE = IMedia.Type.MEDIA_TYPE_IDLE;
    int MEDIA_TYPE_MUSIC = IMedia.Type.MEDIA_TYPE_MUSIC;
    int MEDIA_TYPE_VIDEO = IMedia.Type.MEDIA_TYPE_VIDEO;

    /**
     * 播放模式
     * <p> 循序播放/群不循环/单曲循环/随机
     */
    int REPEAT_MODE_QUEUE = 0;
    int REPEAT_MODE_ALL = 1;
    int REPEAT_MODE_ONE = 2;
    int REPEAT_MODE_RANDOM = 3;

    /**
     * 播放状态
     * <p> 暂停/播放/停止
     */
    int E_PLAY_STATE_PAUSE = 0;
    int E_PLAY_STATE_PLAY = 1;
    int E_PLAY_STATE_STOP = 2;

    /**
     * ViewPage 索引
     * <p> ViewPaper 相关的页面定义
     */
    int PAGE_INDEX_PLAY = 0;
    int PAGE_INDEX_FLASH = 1;
    int PAGE_INDEX_USB = 2;
    int PAGE_INDEX_SD = 3;
    int PAGE_INDEX_FOLDER = 4;
    int PAGE_INDEX_FAVORITE = 5;

    int PAGE_INDEX_MUSIC = 6;
    int PAGE_INDEX_ALBUM = 7;
    int PAGE_INDEX_ARTIST = 8;

    /**
     * 音乐效果显示
     * <p> 频率视图/歌词视图
     */
    int E_SHOW_FREQUENCY = 0;
    int E_SHOW_LYRICS = 1;

    /**
     * 播放命令
     * <p> 播放/暂停/停止/上一曲/下一曲...
     */
    int PLAY_CMD_PLAY = 1;
    int PLAY_CMD_PAUSE = 2;
    int PLAY_CMD_PLAY_PAUSE = 3;
    int PLAY_CMD_STOP = 4;
    int PLAY_CMD_PREV = 5;
    int PLAY_CMD_NEXT = 6;
    int PLAY_CMD_POS = 7;
    int PLAY_CMD_SWITCH_MEDIA_TYPE = 8;
    int PLAY_CMD_SMART_CW = 9;
    int PLAY_CMD_SMART_CCW = 10;
    int PLAY_CMD_SMART_ENTER = 11;

    /**
     * ERROR_CODE
     * <p> 错误代码定义
     */
    int ERROR_CODE_UNSUPPORT = 1;
    int ERROR_CODE_PLAY_END = 2;
    int ERROR_CODE_FIRST_TRACK = 3;
    int ERROR_CODE_LAST_TRACK = 4;
    int ERROR_CODE_FILE_NO_EXISTS = 5;
    int ERROR_CODE_UNSUPPORT_VIDEO_CODE = 6;
    int ERROR_CODE_UNSUPPORT_AUDIO_CODE = 7;
    int ERROR_CODE_NO_SEEKABLE = 8;
}
