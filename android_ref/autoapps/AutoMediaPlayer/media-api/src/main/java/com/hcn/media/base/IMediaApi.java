package com.hcn.media.base;

import androidx.annotation.StringDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * 媒体接口
 * @author 65821
 */
@Retention(RetentionPolicy.SOURCE)
@StringDef({
        IMediaApi.PLAY,
        IMediaApi.PAUSE,
        IMediaApi.PLAY_PAUSE,
        IMediaApi.NEXT,
        IMediaApi.PREV,
        IMediaApi.PLAY_MODE,
        IMediaApi.PLAY_INFO,
})

public @interface IMediaApi {
    /** 播放 **/
    String PLAY = "play";

    /** 暂停 **/
    String PAUSE = "pause";

    /** 播放暂停 **/
    String PLAY_PAUSE = "play_pause";

    /** 下一曲 **/
    String NEXT = "next";

    /** 上一曲 **/
    String PREV = "prev";

    /** 播放模式 **/
    String PLAY_MODE = "play_mode";

    /** 播放模式 **/
    String PLAY_INFO = "play_info";
}
