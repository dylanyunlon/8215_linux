package com.hcn.media_base.fragment;

import androidx.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * 当前页面的媒体类型
 * <p> 暂时只有 Music 和 Video 两种；
 * @author 65821
 */

@Retention(RetentionPolicy.SOURCE)
@IntDef({IMediaType.MEDIA_FRAGMENT, IMediaType.MUSIC_FRAGMENT, IMediaType.VIDEO_FRAGMENT})

public @interface IMediaType {
    /**
     * 媒体页面类型
     * <p> 默认页面类型；
     */
    int MEDIA_FRAGMENT = 0;

    /**
     * 音乐页面类型
     * <p> 音乐相关的页面如需要可以归类到此类型，如无特定需求可不归类；
     */
    int MUSIC_FRAGMENT = 1;

    /**
     * 视频页面类型
     * <p> 视频相关的页面如需要可以归类到此类型，如无特定需求可不归类；
     */
    int VIDEO_FRAGMENT = 2;
}
